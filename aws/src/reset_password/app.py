import json
import sys
import logging
import os
import boto3
import socket
from datetime import datetime

from m59utils import (
    util_get_random_string,
    util_valid_username,
    util_valid_server,
    util_validate_password,
    util_get_output_value,
    util_load_account_db,
    util_check_online,
    util_decode_request_body,
    meridian_domain_name,
)

from m59crypto import util_encrypt_password

RESET_SUCCCESS = 0
RESET_FAILURE = 1 << 0

logger = logging.getLogger()
logger.setLevel(logging.INFO)

lookup_email = {}
lookup_username = {}
db_check = "unknown"
db_check_latest = "unknown"

send_from_email_address = os.environ["send_from_email_address"]
aws_region = os.environ["region_name"]
domain_name = os.environ["domain_name"] or meridian_domain_name

def do_failure():
    return {
        'statusCode': 302, 
        'headers': {
            'Location': f"{domain_name}/password-reset-failure"
        },
        'body': json.dumps({'message': 'Redirecting...'})
    }

def do_success():
    return {
        'statusCode': 302, 
        'headers': {
            'Location': f"{domain_name}/password-reset"
        },
        'body': json.dumps({'message': 'Redirecting...'})
    }


def lambda_handler(event, context):
    global lookup_email
    global lookup_username
    global db_check
    global db_check_latest
    # Check if we are up and running for new accounts!
    online, db_check_latest = util_check_online(db_check_latest)
    if not online:
        return {"statusCode": 200, "body": RESET_FAILURE}

    lookup_email, lookup_username, db_check, db_check_latest = util_load_account_db(
        lookup_email, lookup_username, db_check, db_check_latest
    )

    body = event["body"]
    headers = event["headers"]
    hostname = "unknown"
    ip = "unknown"

    try:
        ip = event["requestContext"]["identity"]["sourceIp"]
        hostname = str(socket.gethostbyaddr(ip))
    except:
        logger.error("Failed to get ip address")

    logger.info(f"Incoming request from {ip} {hostname}")

    err = 0
    
    request = util_decode_request_body(body)
    data_available = all(key in request for key in ["submit", "email", "username", "pw1", "pw2", "server"])

    if (not data_available) or (request["submit"] != "1"):
        logger.error("Required request fields missing!")
        return do_failure()

    email = request["email"].strip().lower()
    username = request["username"].strip().lower()
    password1 = request["pw1"].strip()
    password2 = request["pw2"].strip()
    server = request["server"].strip()

    # Check if email already in use for the same server
    username_check = f"{username}_{server}"

    if username_check not in lookup_username:
        # only accounts in the master database can be reset
        logger.error("Account doesn't exist in master db (needs to be at least 24 hours old) - aborting.")
        return do_failure()

    if not util_valid_server(server):
        logger.error("Invalid server provided - aborting.")
        return do_failure()

    if not util_validate_password(password1, password2):
        logger.error("Invalid new passwords provided - aborting.")
        return do_failure()

    security = util_get_random_string(28)

    details = lookup_username[username_check].split("_")

    email = details[0]
    account = details[1]
    
    if email == "" or len(email) == 0:
        logger.error("Account email was not found in the database - aborting.")
        return do_failure()

    if account == "" or len(account) == 0:
        logger.error("Account number was not found in the database - aborting.")
        return do_failure()

    if err == 0:

        verification_url = util_get_output_value("VerifyResetFunctionApi")

        subject = "[Meridian 59] Account password reset (action required)"

        body_text = f"""
Dear Adventurer,

You have requested a password reset for your account [{username}] with your new desired password.
In order to complete this request, please confirm the reset using the link below:

{verification_url}?e={email}&v={security}&s={server}
        
Have fun in the game!
The Meridian 59 Team
{domain_name}
"""

        ses_client = boto3.client("ses", region_name=aws_region)
        response = ses_client.send_email(
            Destination={"ToAddresses": [email]},
            Message={
                "Body": {
                    #'Html': {'Data': body_text},
                    "Text": {"Data": body_text},
                },
                "Subject": {"Data": subject},
            },
            Source=send_from_email_address,
        )

        # Use encrypted password for transport to game server
        password_cipher = util_encrypt_password(password1)

        if response:
            # Record password reset details to s3
            account_object = {
                "username": username,
                "password": password_cipher,
                "server": server,
                "security": security,
                "account": account,
                "email": email,
                "timestamp": str(datetime.utcnow().isoformat()),
            }
            try:
                # Write the content to a new file in the specified S3 bucket
                s3_client = boto3.client("s3")
                bucket_name = util_get_output_value("NewM59AccountsToProcessBucketName")
                object_name = f"{email}_{server}_reset.json"
                s3_client.put_object(Bucket=bucket_name, Key=object_name, Body=json.dumps(account_object))
                logger.info(f"New account password reset file written to {bucket_name}/{object_name}")
            except Exception as e:
                logger.error(f"Error writing content to S3: {e}")
                err = err | RESET_FAILURE

        else:
            logger.error("Required request fields missing!")
            return do_failure()

    return do_success()
