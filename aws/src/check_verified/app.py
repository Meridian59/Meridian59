import logging
import json
import boto3
import os

from m59utils import util_decode_request_body, util_get_output_value

logger = logging.getLogger()
logger.setLevel(logging.INFO)

VERIFIED_YES    = 0
VERIFIED_NO     = 1 << 0
VERIFIED_ERROR  = 1 << 1

def lambda_handler(event, context):
    
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
    
    logger.info(f"Checking verified status for account:{username} on server:{server}")
	
    # Unverified accounts will never be in the master database and only unique s3 files
    try:
        s3_client = boto3.client('s3')
        process_bucket_name = util_get_output_value("NewM59AccountsToProcessBucketName")
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
                            verified = 0
                            if "verified" in account_obj:
                                verified = account_obj["verified"]
                            #logger.info(f" ---> found verification status: {verified}")
                            if verified == 0:
                                err = err | VERIFIED_NO
                            else:
                                err = err | VERIFIED_YES
               
    except Exception as e:
        logger.error(f"Exception during verification check: {e}")
        err = err | VERIFIED_ERROR
        
    return {
        'statusCode': 200,
        'body': err
    }
