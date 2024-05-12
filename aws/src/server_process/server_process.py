import boto3
import json
import time
import re
import base64
import socket
import os

sqs = boto3.client('sqs', region_name=os.environ['aws_region'])

game_server_ip      = os.environ['aws_game_server_ip']
game_server_port    = int(os.environ['aws_game_server_port'])
incoming_queue_url  = os.environ['aws_incoming_queue_url']
outgoing_queue_url  = os.environ['aws_outgoing_queue_url']
aws_region          = os.environ["aws_region"]
aws_queue_group_id  = os.environ["aws_queue_group_id"]
key_id_or_alias     = os.environ['aws_kms_alias']

def decrypt_password(ciphertext_base64):
    kms_client = boto3.client('kms', region_name=aws_region)

    # Decrypt the ciphertext using AWS KMS
    ciphertext = base64.b64decode(ciphertext_base64)
    response = kms_client.decrypt(
        KeyId=key_id_or_alias,
        CiphertextBlob=ciphertext,
        EncryptionAlgorithm='RSAES_OAEP_SHA_256'
    )

    # Return the plaintext password
    plaintext_password = response['Plaintext'].decode('utf-8')
    return plaintext_password

# Function to send a command to the server and receive the response
def send_command(command):
    response = ""
    buffer = ""                                                                                                                                                                                                                                                                              
    try:
        with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as blakserv_sock:
            print(f"Sending command [{command}] to {game_server_ip} {game_server_port}")
            blakserv_sock.connect((game_server_ip, game_server_port))
            blakserv_sock.settimeout(10)                                                                                          
            blakserv_sock.sendall(f"{command}\r\n".encode())
            while True:
                try:
                    data = blakserv_sock.recv(1024).decode('utf-8', 'ignore')
                    buffer += data                                                                                                                                                                                                                                                   
                    while "\r\n" in buffer:                                                                                       
                        line, buffer = buffer.split("\r\n", 1)                                                   

                        # Skip lines that start with '>' (echo of the original command)                                                                               
                        if re.match(r"^>", line):
                            continue

                        response += line + "\r\n"  # Append the processed line to the response                                                                        
                    break
                except socket.timeout:
                    print("Socket timeout while receiving data.")
                    break
                except socket.error as e:
                    print(f"Socket error while receiving data: {e}")
    except Exception as e:
        print(f"Error in send_command: {e}")

    return response

def reset_password(username, password):
    if not game_server_ip or not game_server_port:
        raise ValueError("Invalid server or port")

    server_response = send_command(f"show account {username}")
    print(server_response)
    if "Cannot find account" in server_response:
        raise Exception("Failed to find account!")

    # Extract account number from response                        
    sr = re.sub(r'\s+', ' ', server_response)

    # Split the modified string by spaces into a list                                
    results = sr.split(" ")
    account_number = results[6]

    # Create a user                                                                  
    user_response = send_command(f"set account password {account_number} {password}")
    if "Set password" not in user_response:
        raise Exception("Failed to reset password.")

    return account_number

def create_account(username, password):

    if not game_server_ip or not game_server_port:
        raise ValueError("Invalid server or port")

    # Create an account                                      
    server_response = send_command(f"create automated {username} {password}")
    print(server_response)
    if "Created account" not in server_response:
       raise Exception("Failed to create user - please try again later.")

    # Extract account number from response
    x = server_response.split(" ")
    account_number = x[2].replace(".", "").strip()

    # Create a user                                       
    user_response = send_command(f"create user {account_number}")
    if not user_response:
        raise Exception("Failed to create user - please try again later.")

    return account_number, user_response


def process_message(account):
    username = account["username"]
    password_ciphertext = account["password"]          
    password = decrypt_password(password_ciphertext)

    if account["message"] == "create":
        account_number, user_response = create_account(username, password)
        print(f"Account Number: {account_number}")
        print(f"User Response: {user_response}")
        return account_number

    if account["message"] == "reset":
        account_number = reset_password(username, password)
        return account_number

print("Processing script has started")

while True:
    # Poll the incoming queue for messages
    messages = sqs.receive_message(
        QueueUrl=incoming_queue_url,
        MaxNumberOfMessages=1,
        WaitTimeSeconds=5,
    )

    if 'Messages' in messages:
        for message in messages['Messages']:
            try:
                message_body = message['Body']
                account = json.loads(message_body)
                print(f"Received message: {message_body}")

                # Process the message
                account_number = process_message(account)

                message_response = "account created"
                if account["message"] == "reset":
                    message_response = "password reset"

                # Send the processed message to the outgoing queue
                message_data = {
                        "action": message_response,
                        "username": account["username"],
                        "account_number": account_number,
                        "server": account["server"],
                        "email": account["email"]
                }
                sqs.send_message(
                    QueueUrl=outgoing_queue_url,
                    MessageGroupId=aws_queue_group_id,
                    MessageBody=json.dumps(message_data)
                )
                print(f"updating done queue: {message_data}")

                # Delete the message from the incoming queue to prevent reprocessing
                sqs.delete_message(
                    QueueUrl=incoming_queue_url,
                    ReceiptHandle=message['ReceiptHandle']
                )
                print("Message processed and deleted from the incoming queue.")
            except Exception as e:
                print(f"Exception during message processing: {e}")
    else:
        print("No new messages in the incoming queue. Waiting...")
        time.sleep(5)  # Wait before polling again if the queue was empty