import re
import logging
import boto3
from botocore.exceptions import ClientError
import json
from datetime import datetime
import sys
import os

from m59utils import util_get_output_value, meridian_domain_name

logger = logging.getLogger()
logger.setLevel(logging.INFO)

send_from_email_address = os.environ["send_from_email_address"]
aws_region = os.environ["region_name"]
domain_name = os.environ["domain_name"] or meridian_domain_name

def do_failure():
    logger.info("Running failure redirect...")
    return {
        'statusCode': 302, 
        'headers': {
            'Location': f"{domain_name}/account-created-failure"
        },
        'body': json.dumps({'message': 'Redirecting...'})
    }

def do_success():
    logger.info("Running success redirect...")
    return {
        'statusCode': 302, 
        'headers': {
            'Location': f"{domain_name}/account-created-success"
        },
        'body': json.dumps({'message': 'Redirecting...'})
    }

def lambda_handler(event, context):

    try:

        if not event or "queryStringParameters" not in event:
            logger.error("missing queryStringParameters entirely.")
            return do_failure();

        if (
            "e" not in event["queryStringParameters"]
            or "v" not in event["queryStringParameters"]
            or "s" not in event["queryStringParameters"]
        ):
            logger.error("missing one or more query string parameters e, v or s!")
            return do_failure()
            sys.exit(1)

        email = event["queryStringParameters"]["e"]
        security = event["queryStringParameters"]["v"]
        server = event["queryStringParameters"]["s"]

        # get account details from s3 (username and password) and validate security code
        result = "unknown"
        username = ""
        password = ""
        if server != "101" or server != "102":
            server = "101"

        s3_client = boto3.client("s3")
        bucket_name = util_get_output_value("NewM59AccountsToProcessBucketName")
        object_key_read = f"{email}_{server}.json"
        security_check = None
        account_object = ""
        email = ""
        verified = None

        try:
            logger.info(f"Attempting to open S3 {bucket_name}:{object_key_read}")
            file_obj = s3_client.get_object(Bucket=bucket_name, Key=object_key_read)
            file_content = file_obj["Body"].read().decode("utf-8")
            logger.info("Found account file.")
            logger.info(file_content)
            account_object = json.loads(file_content)

            username = account_object["username"]
            password = account_object["password"]
            security_check = account_object["security"]
            email = account_object["email"]

            verified = True if "verified" in account_object else None

        except ClientError as e:
            # Check if the error was due to the file not being found
            if e.response["Error"]["Code"] == "NoSuchKey":
                logger.error(f"S3 NoSuchKey! Email file not found")
            else:
                logger.error(f"S3 error: {e.response}")

        if security != security_check:
            logger.error(f"{security} vs {security_check}")
            raise Exception("security check failed!")

        if verified != None:
            logger.error("Account already verified!")
            return do_success()
            sys.exit(1)

        # add to SQS queue job
        sqs = boto3.client("sqs")
        if server == "101":
            queue_url = util_get_output_value("GameServerCommunicationQueue101URL")
        else:
            queue_url = util_get_output_value("GameServerCommunicationQueue102URL")
        logger.info(f"Queue URL: {queue_url}")

        # The message you want to send
        message_data = {
            "message": "create",
            "username": username,
            "password": password,  # this is an encrypted payload.
            "server": server,
            "email": email,
            "object_key_read": object_key_read,
        }

        # payload needs to be a string
        message_body = json.dumps(message_data)

        # Sending the message to SQS
        try:

            response = sqs.send_message(
                QueueUrl=queue_url, MessageBody=message_body, MessageGroupId=aws_region
            )
            logger.info(f"Message sent to server queue! Message ID: {response['MessageId']}")

        except Exception as e:
            logger.error(f"Error sending message to SQS: {e}")
            return do_failure()
            sys.exit(1)
            

        try:
            # Write the content to a new file in the specified S3 bucket
            account_object["verified"] = True
            account_object["verified_stamp"] = str(datetime.utcnow().isoformat())

            s3_client = boto3.client("s3")
            s3_client.put_object(Bucket=bucket_name, Key=object_key_read, Body=json.dumps(account_object))
            logger.info(f"Account object updated at {bucket_name}/{object_key_read}")
        except Exception as e:
            logger.error(f"Error writing content to S3: {e}")
            raise Exception("Error writing content to S3")

        return do_success()

    except Exception as e:
        logger.info("exception:")
        logger.info(str(e))
        return do_failure()
