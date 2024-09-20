import json
import sys
import logging
import os
import boto3
from datetime import datetime
from botocore.exceptions import ClientError

from m59utils import util_get_output_value, meridian_domain_name

logger = logging.getLogger()
logger.setLevel(logging.INFO)


def lambda_handler(event, context):
    for message in event["Records"]:
        process_message(message)
    return {"statusCode": 200, "body": "done"}


def process_message(message):
    send_from_email_address = os.environ["send_from_email_address"]
    aws_region = os.environ["region_name"]
    domain_name = os.environ["domain_name"] or meridian_domain_name
    try:
        logger.info(f"Processing message {message['body']}")
        payload = json.loads(message["body"])
        username = payload["username"]
        email = payload["email"]
        account_number = payload["account_number"]
        server = payload["server"]
        action = payload["action"]

        if action == "password reset":

            subject = f"[Meridian 59] Password reset successful for ({username})!"

            # Email to share that the account has been succesfully created
            body_text = f"""
Dear Adventurer,
        
Your account password has now been reset successfully.
        
Please login to your new account using your username and new password.

Have fun in game and stay safe!
        
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

        if action == "account created":

            try:
                s3_client = boto3.client("s3")
                bucket_name = util_get_output_value("NewM59AccountsToProcessBucketName")
                object_key_read = f"{email}_{server}.json"
                logger.info(f"Attempting to open S3 {bucket_name}:{object_key_read}")
                file_obj = s3_client.get_object(Bucket=bucket_name, Key=object_key_read)
                file_content = file_obj["Body"].read().decode("utf-8")
                logger.info(file_content)
                account_object = json.loads(file_content)

            except ClientError as e:
                # Check if the error was due to the file not being found
                if e.response["Error"]["Code"] == "NoSuchKey":
                    logger.error(f"S3 NoSuchKey! Account file not found")
                else:
                    logger.error(f"S3 error: {e.response}")

            try:
                # Write the content to a new file in the specified S3 bucket
                account_object["created"] = True
                account_object["created_stamp"] = str(datetime.utcnow())
                account_object["account_number"] = account_number
                account_object["password"] = "-clear-"
                # Account files will be deleted after 24 hours and merged into the main account file.
                s3_client = boto3.client("s3")
                s3_client.put_object(Bucket=bucket_name, Key=object_key_read, Body=json.dumps(account_object))
                logger.info(f"Account object updated at {bucket_name}/{object_key_read}")
            except Exception as e:
                logger.error(f"Error writing content to S3: {e}")
                raise Exception("Error writing content to S3")

            subject = f"[Meridian 59] Account ({username}) successfully created!"

            # Email to share that the account has been succesfully created
            body_text = f"""
Dear Adventurer,
        
Your Meridian 59 adventure starts right here and the best part is that your account is absolutely free.
    
Your account has now been successfully created and activated.
        
Please login to your new account using your chosen username and password.
        
Username: {username}
Server: {server}
        
Have fun in game and stay safe!
        
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

    except Exception as err:
        logger.error(f"An error occurred! {err}")
        raise err
