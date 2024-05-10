
import logging
import boto3
from botocore.exceptions import ClientError
import json
import sys
import os

from m59utils import util_get_output_value, meridian_domain_name

logger = logging.getLogger()
logger.setLevel(logging.INFO)

send_from_email_address = os.environ["send_from_email_address"]
aws_region = os.environ["region_name"]
domain_name = os.environ["domain_name"] or meridian_domain_name
aws_queue_group_id = os.environ["queue_group_id"]

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
            'Location': f"{domain_name}/password-reset-success"
        },
        'body': json.dumps({'message': 'Redirecting...'})
    }
 

def lambda_handler(event, context):
    
    try:
    
        if not event or "queryStringParameters" not in event:
            logger.error("missing queryStringParameters!")
            sys.exit(1)
    
        if "e" not in event["queryStringParameters"] or "v" not in event["queryStringParameters"]  or "s" not in event["queryStringParameters"]:
            logger.error("missing queryStringParameters e, v or s!")
            sys.exit(1)
            
        email       = event["queryStringParameters"]["e"]
        security    = event["queryStringParameters"]["v"]
        server      = event["queryStringParameters"]["s"]
    
        # get account details from s3 (username and password) and validate security code
        result = "unknown"
        username = ""
        password = ""
        if server != "101" or server != "102":
            server = "101"
        
        s3_client = boto3.client('s3')
        bucket_name = util_get_output_value("NewM59AccountsToProcessBucketName")
        object_key_read = f"{email}_{server}_reset.json"
        security_check = None
        account_object = ""

        try:
            logger.info(f"Attempting to open S3 {bucket_name}:{object_key_read}")
            file_obj = s3_client.get_object(Bucket=bucket_name, Key=object_key_read)
            file_content = file_obj['Body'].read().decode('utf-8')
            account_object = json.loads(file_content)
            
            username = account_object["username"]
            password = account_object["password"]
            security_check = account_object["security"]
            email = account_object["email"]
            
        except ClientError as e:
            # Check if the error was due to the file not being found
            if e.response['Error']['Code'] == 'NoSuchKey':
                logger.error(f"S3 NoSuchKey! Email file not found")
            else:
                logger.error(f"S3 error: {e.response}")
                
        if security != security_check:
            logger.error(f"Security check failed: {security} vs {security_check}")
            return do_failure()

        logger.info("trying to connect to SQS")
        
        # add to SQS queue job
        sqs = boto3.client('sqs')
        if server == "101":
            queue_url = util_get_output_value("GameServerCommunicationQueue101URL")
        else:
            queue_url = util_get_output_value("GameServerCommunicationQueue102URL")
        
        message_data = {
            "message": "reset",
            "username": username,
            "password": password, # encrypted payload
            "server": server,
            "email": email,
            "object_key_read": object_key_read
        }
        
        # payload needs to be a string
        message_body = json.dumps(message_data)
        
        try:
            
            response = sqs.send_message(
                QueueUrl=queue_url,
                MessageBody=message_body,
                MessageGroupId=aws_queue_group_id
            )
            logger.info(f"Message sent! Message ID: {response['MessageId']}")
            
        except Exception as e:
            logger.error(f"Error sending message to SQS: {e}")
            return do_failure()

        return do_success()
    
    except Exception as e:
        logger.info("exception")
        logger.info(str(e))
        return do_failure()