import logging
import json
import boto3
import os
import time
from datetime import datetime

from m59utils import util_get_output_value, meridian_domain_name

logger = logging.getLogger()
logger.setLevel(logging.INFO)


def lambda_handler(event, context):
    start = time.time()
    master_database_key = os.environ["database_file"]
    bucket_name = util_get_output_value("59AccountsDatabaseBucketName")
    object_key_read = os.environ['database_lock_file']
    s3_client = boto3.client('s3')

    send_from_email_address = os.environ["send_from_email_address"]
    aws_region = os.environ["region_name"]
    domain_name = os.environ["domain_name"] or meridian_domain_name
    
    accounts_to_add = []
    accounts_to_delete = []
    process_bucket_name = util_get_output_value("NewM59AccountsToProcessBucketName")
    database_size = 0
    
    try:
        paginator = s3_client.get_paginator('list_objects_v2')
        pages = paginator.paginate(Bucket=process_bucket_name, Prefix="")
        
        for page in pages:
            for obj in page['Contents']:
                #logger.info(obj)
            
                file_obj = s3_client.get_object(Bucket=process_bucket_name, Key=obj['Key'])
                file_content = file_obj['Body'].read().decode('utf-8')
                
                #logger.info(file_content)
                #logger.info("------")
                
                account_obj = json.loads(file_content)
                stamp = datetime.now()
                if "timestamp" in account_obj and "email" in account_obj:
                    stamp = datetime.fromisoformat(account_obj["timestamp"])
                    created = account_obj["created"]
                    if created == 1:
                        accounts_to_add.append(account_obj)
                    elif datetime.now() - stamp > datetime.timedelta(days=1) and "verified" not in account_obj:
                        accounts_to_delete.append(account_obj)
                else:
                    # bad data remove it!
                    logger.info(f"Removing bad data from S3: {file_content}")
                    accounts_to_delete.append(account_obj)

    except Exception as e:
        logger.error(f"Exception during clean: {e}")
        
    if len(accounts_to_add) > 0 or len(accounts_to_delete) > 0 :
        account_object = ""
        try:
            logger.info(f"Attempting to open S3 {bucket_name}:{master_database_key}")
            file_obj = s3_client.get_object(Bucket=bucket_name, Key=master_database_key)
            file_content = file_obj['Body'].read().decode('utf-8')
            
            for account in accounts_to_add:
                file_content += f"\"{account["username"]}\",\"{account["email"]}\",\"{account["server"]}\",\"{account["account_number"]}\"\n"
                account_file = f"{account["email"]}_{account["server"]}.json"
                s3_client.delete_object(Bucket=process_bucket_name, Key=account_file)
                logger.info(f"Deleted {account_file} as merging to main database !")
                
            for account in accounts_to_delete:
                account_file = f"{account["email"]}_{account["server"]}.json"
                s3_client.delete_object(Bucket=process_bucket_name, Key=account_file)
                logger.info(f"Deleted stale {account_file} !")
                
            # update master database
            if len(accounts_to_add) > 0:
                try:
                    s3_client.put_object(Bucket=bucket_name, Key=master_database_key, Body=file_content)
                    database_size = len(file_content.splitlines())
                    logger.info(f"Master database written to {bucket_name}/{object_key_read}")
                except Exception as e:
                    logger.error(f"Error writing content to S3: {e}")
                    raise Exception("Error writing content to S3")
            
        except Exception as e:
            logger.error(f"Failed to load update accounts and main database {e}")
    else:
        logger.info(f"Attempting to open S3 {bucket_name}:{master_database_key}")
        file_obj = s3_client.get_object(Bucket=bucket_name, Key=master_database_key)
        file_content = file_obj['Body'].read().decode('utf-8')
        database_size = len(file_content.splitlines())
        

    subject = "[Meridian 59] Clean up job ran!"
    send_to = send_from_email_address
        
    body_text = f"""
Dear Maintainer,

Cleaning job ran!

 - {len(accounts_to_add)} accounts merged to main database.
 - {len(accounts_to_delete)} stale accounts purged.
 - Master database now contains {database_size:,} accounts.
 - Took {round(time.time() - start, 2)} to complete!

Have fun in the game!
The Meridian 59 Team
{domain_name}
        """
        
    ses_client = boto3.client('ses', region_name=aws_region)
    response = ses_client.send_email(
        Destination={'ToAddresses': [send_to]},
        Message={
            'Body': {
                'Text': {'Data': body_text},
            },
            'Subject': {'Data': subject},
        },
        Source=send_from_email_address
    )
    
    
    return {
        'statusCode': 200,
        'body': json.dumps('Clear complete!')
    }
