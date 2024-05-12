import logging
import json
import os
import boto3

from m59utils import util_decode_request_body, util_get_output_value, meridian_domain_name

logger = logging.getLogger()
logger.setLevel(logging.INFO)

def lambda_handler(event, context):

    process_bucket_name = util_get_output_value("NewM59AccountsToProcessBucketName")
  
    send_from_email_address = os.environ["send_from_email_address"]
    aws_region = os.environ["region_name"]
    domain_name = os.environ["domain_name"] or meridian_domain_name
    
    err = 0
    body = event['body']
    request = util_decode_request_body(body)

    data_available = all(key in request for key in ['submit','username', 'server'])
 
    if (not data_available) or (request["submit"] != "1"):
        return {
            "statusCode": 200,
            "body": err
        }

    username    = request["username"]
    server      = request["server"]
    
    logger.info(f"Resending verification for account: {username} on server:{server}")
	
    # Unverified accounts will never be in the master database and only unique s3 files
    try:
        s3_client = boto3.client('s3')
        paginator = s3_client.get_paginator('list_objects_v2')
        pages = paginator.paginate(Bucket=process_bucket_name, Prefix="")
     
        for page in pages:
            if "Contents" in page:
                for obj in page['Contents']:
                    file_obj = s3_client.get_object(Bucket=process_bucket_name, Key=obj['Key'])
                    file_content = file_obj['Body'].read().decode('utf-8')
                    
                    #logger.info(f"Checking: {file_content}")
                    #logger.info("------")
                    
                    account_obj = json.loads(file_content)
                    
                    # if the file exists with the username or email then they are in use (until they expire)
                    if account_obj["server"] == server:
                        if "username" in account_obj and account_obj["username"] == username:
                           email = account_obj.get("email")
                           security = account_obj.get("security")
                           created = account_obj.get("created")
                           server = account_obj.get("server")
                           if not created:
                                subject = "[Meridian 59] New account verification (action required)"

                                verification_url = util_get_output_value("VerifyAccountApi")
        
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
                                
                                ses_client = boto3.client('ses', region_name=aws_region)
                                response = ses_client.send_email(
                                    Destination={'ToAddresses': [email]},
                                    Message={
                                        'Body': {
                                            #'Html': {'Data': body_text},
                                            'Text': {'Data': body_text},
                                        },
                                        'Subject': {'Data': subject},
                                    },
                                    Source=send_from_email_address,
                                )
                                logger.info("account verification email resent!")

    except Exception as e:
        logger.error(f"Exception during resend: {e}")
	
        
    return {
        'statusCode': 200,
        'body': err
    }
