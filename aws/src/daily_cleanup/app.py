import logging
import json
import boto3
import os
import time
from datetime import datetime

from m59utils import util_get_output_value, meridian_domain_name

logger = logging.getLogger()
logger.setLevel(logging.INFO)

s3_client = boto3.client('s3')

process_bucket_name = util_get_output_value("NewM59AccountsToProcessBucketName")
audit_bucket_name = util_get_output_value("M59AccountsAuditBucketName")

def move_to_audit(accounts_to_move):
    for account in accounts_to_move:
        account_file = f"{account['email']}_{account['server']}.json"
        copy_source = {
            'Bucket': process_bucket_name,
            'Key': account_file
        }

        try:
            # Copy the object to the destination bucket
            s3_client.copy_object(
                Bucket=audit_bucket_name,
                CopySource=copy_source,
                Key=account_file
            )
            logger.info(f"Copied {account_file} to {audit_bucket_name} !")

            # Delete the object from the source bucket
            s3_client.delete_object(Bucket=process_bucket_name,Key=account_file)
            logger.info(f"Deleted {account_file} from {process_bucket_name} !")

        except Exception as e:
            logger.error(f"Error moving {account_file}: {str(e)}")


def lambda_handler(event, context):
    start = time.time()
    master_database_key = os.environ["database_file"]
    bucket_name = util_get_output_value("59AccountsDatabaseBucketName")
    object_key_read = os.environ['database_lock_file']

    send_from_email_address = os.environ["send_from_email_address"]
    aws_region = os.environ["region_name"]
    domain_name = os.environ["domain_name"] or meridian_domain_name
    
    logger.info(f"Attempting to open S3 {bucket_name}:{master_database_key}")
    db_obj = s3_client.get_object(Bucket=bucket_name, Key=master_database_key)
    db_content = db_obj['Body'].read().decode('utf-8')
    current_database_size = len(db_content.splitlines())
    
    accounts_to_add = []
    new_database_size = current_database_size
    
    try:
        paginator = s3_client.get_paginator('list_objects_v2')
        pages = paginator.paginate(Bucket=process_bucket_name, Prefix="")
        
        for page in pages:
            for obj in page['Contents']:
            
                file_obj = s3_client.get_object(Bucket=process_bucket_name, Key=obj['Key'])
                file_content = file_obj['Body'].read().decode('utf-8')
                
                account_obj = json.loads(file_content)
                if "timestamp" in account_obj and "email" in account_obj:
                    created = account_obj.get("created")
                    if created is True:
                        accounts_to_add.append(account_obj)
                else:
                    logger.info(f"Bad data found in: {obj['Key']} - {file_content}")

    except Exception as e:
        logger.error(f"Exception during clean: {e}")
        
    if len(accounts_to_add) > 0:

        # update master database
        for account in accounts_to_add:
            db_content += f"\"{account["username"]}\",\"{account["email"]}\",\"{account["server"]}\",\"{account["account_number"]}\"\n"
        
        try:
            s3_client.put_object(Bucket=bucket_name, Key=master_database_key, Body=db_content)
            new_database_size = len(db_content.splitlines())
            logger.info(f"Master database written to {bucket_name}/{object_key_read}")
        except Exception as e:
            logger.error(f"Error writing content to S3: {e}")

        # Move to audit bucket
        move_to_audit(accounts_to_add)

    subject = "[Meridian 59] Clean up job ran!"
    send_to = send_from_email_address
        
    body_text = f"""
Dear Maintainer,

Cleaning job ran!

 - {len(accounts_to_add)} accounts merged to main database.
 - Master database now contains {new_database_size:,} accounts (previously: {current_database_size:,}).
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
