import json
import sys
import logging
import os
import boto3
import time
import csv
from datetime import datetime
import base64

from m59utils import (
    util_decode_request_body,
    util_get_random_string,
    util_valid_email,
    util_valid_username_characters,
    util_valid_username,
    util_valid_server,
    util_validate_password,
    util_get_output_value,
    util_load_account_db,
    util_check_online,
    meridian_domain_name
)

from m59crypto import util_encrypt_password, util_decrypt_password


# Emails that are allowed to sign up multiple times for testing purposes.
excluded_emails = json.loads(os.environ["test_emails"])

SIGNUP_SUCCCESS = 0
SIGNUP_FAILURE = 1 << 0
SIGNUP_INVALID_USERNAME = 1 << 1
SIGNUP_INVALID_EMAIL = 1 << 2
SIGNUP_EMAIL_IN_USE = 1 << 3
SIGNUP_INVALID_PASSWORD = 1 << 4
SIGNUP_INVALID_SERVER = 1 << 5
SIGNUP_OFFLINE = 1 << 6
SIGNUP_FAILED = 1 << 7

logger = logging.getLogger()
logger.setLevel(logging.INFO)

lookup_email = {}
lookup_username = {}
db_check = "unknown"
db_check_latest = "unknown"

def signup_in_progress(email_check, username, server):
    email_in_use = False
    username_in_use = False
    account_processing_bucket = util_get_output_value("NewM59AccountsToProcessBucketName")
    try:
        s3_client = boto3.client("s3")
        paginator = s3_client.get_paginator("list_objects_v2")
        pages = paginator.paginate(Bucket=account_processing_bucket, Prefix="")

        for page in pages:
            if "Contents" in page:
                for obj in page["Contents"]:
                    file_obj = s3_client.get_object(Bucket=account_processing_bucket, Key=obj["Key"])
                    file_content = file_obj["Body"].read().decode("utf-8")

                    #logger.info(f"Checking: {file_content}")
                    #logger.info("------")

                    account_obj = json.loads(file_content)

                    # if the file exists with the username or email then they are in use (until they expire)
                    if "username" in account_obj and account_obj["username"] == username:
                        username_in_use = True
                    if "email" in account_obj and account_obj["email"] == email_check:
                        email_in_use = True

    except Exception as e:
        logger.error(f"Exception during check in progress: {e}")
    return email_in_use, username_in_use


def lambda_handler(event, context):
    global lookup_email
    global lookup_username
    global db_check
    global db_check_latest
    
    send_from_email_address = os.environ["send_from_email_address"]
    aws_region = os.environ["region_name"]
    domain_name = os.environ["domain_name"] or meridian_domain_name

    # Check if we are up and running for new accounts!
    online, db_check_latest = util_check_online(db_check_latest)
    if not online:
        return {"statusCode": 200, "body": SIGNUP_FAILED}

    lookup_email, lookup_username, db_check, db_check_latest = util_load_account_db(
        lookup_email, lookup_username, db_check, db_check_latest
    )

    body = event["body"]
    headers = event["headers"]
    hostname = "unknown"
    ip = "unknown"

    try:
        ip = event["requestContext"]["http"]["sourceIp"]
        hostname = str(socket.gethostbyaddr(ip))
    except:
        logger.error("Failed to get ip address")

    request = util_decode_request_body(body)

    logger.info(f"Incoming request from {ip} {hostname}")

    err = 0

    data_available = all(key in request for key in ["submit", "username", "email", "pw1", "pw2", "server"])

    # skip for testing
    if (not data_available) or (request["submit"] != "1"):
        logger.error("Required request fields missing!")
        return {"statusCode": 200, "body": SIGNUP_FAILED}

    email = request["email"].strip().lower()
    username = request["username"].strip().lower()
    password1 = request["pw1"].strip()
    password2 = request["pw2"].strip()
    server = request["server"].strip()

    # Check if email already in use for the same server
    username_in_use = False
    email_in_use = False

    username_check = f"{username}_{server}"
    email_check = f"{email}_{server}"

    #  also check in-progress signups (s3 files) in addition to the main database
    email_in_use, username_in_use = signup_in_progress(email_check, username, server)

    if username_check in lookup_username:
        username_in_use = True

    if email_check in lookup_email:
        email_in_use = True

    if not util_valid_email(email) or (email not in excluded_emails and email_in_use):
        logger.error("Invalid email provided")
        err = err | SIGNUP_INVALID_EMAIL

    if not util_valid_username(username) or not util_valid_username_characters(username) or username_in_use:
        logger.error("Invalid username provided")
        err = err | SIGNUP_INVALID_USERNAME

    if not util_valid_server(server):
        logger.error("Invalid server provided")
        err = err | SIGNUP_INVALID_SERVER

    if not util_validate_password(password1, password2):
        logger.error("Invalid passwords provided")
        err = err | SIGNUP_INVALID_PASSWORD

    security = util_get_random_string(28)

    if err == 0:

        verification_url = util_get_output_value("VerifyAccountApi")

        subject = "[Meridian 59] New account verification (action required)"

        body_text = f"""
Dear Adventurer,

Your Meridian 59 adventure starts right here and the best part is that your account is absolutely free.

In order to complete the account creation, please verify the account using the link below:
{verification_url}?e={email}&v={security}&s={server}

Your account details follow

Username: {username}
Server: {server}

Have fun in the game!
The Meridian 59 Team
{domain_name}
"""

        ses_client = boto3.client("ses", region_name=aws_region)
        response = ses_client.send_email(
            Destination={"ToAddresses": [email]},
            Message={
                "Body": {
                    # 'Html': {'Data': body_text}, # could add some fancy formatting
                    "Text": {"Data": body_text},
                },
                "Subject": {"Data": subject},
            },
            Source=send_from_email_address,
        )

        # Use encrypted password for transport to game server
        password_cipher = util_encrypt_password(password1)

        if response:
            # Record new account details to s3 (awaiting verification)
            account_object = {
                "username": username,   
                "password": password_cipher,
                "server": server,
                "ip": ip,
                "email": email,
                "hostname": hostname,
                "security": security,
                "timestamp": str(datetime.utcnow().isoformat()),
            }
            logger.info(account_object)
            try:
                account_processing_bucket = util_get_output_value("NewM59AccountsToProcessBucketName")
                logger.info(f"Sending account object to {account_processing_bucket}")
                logger.info(account_object)
                # Write the content to a new file in the specified S3 bucket
                s3_client = boto3.client("s3")
                object_name = f"{email}_{server}.json"
                s3_client.put_object(Bucket=account_processing_bucket, Key=object_name, Body=json.dumps(account_object))
                logger.info(f"New account object written to {account_processing_bucket}/{object_name}")
            except Exception as e:
                logger.error(f"Error writing content to S3: {e}")
                err = err | SIGNUP_FAILED

        else:
            logger.error("Failed to send email and account creation failed")
            err = err | SIGNUP_FAILED

    return {"statusCode": 200, "body": err}
