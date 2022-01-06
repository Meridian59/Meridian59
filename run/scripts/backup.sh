# Backup last day's saved games to S3, then delete old files from local disk
# The SERVER environment variable must be set to the number of the server.

if [ -z "$SERVER" ]; then
  echo "SERVER environment variable not specified"
  exit 1
fi

BASE_DIR=/home/ec2-user/Meridian59/run/server

# Delete old saved games
nice -n 19 find $BASE_DIR/savegame/* -mmin +1440 -delete
nice -n 19 find /tmp/Meridian* -mmin +1440 -delete

DATE=`date +%F-%T`
BACKUP_FILENAME=/tmp/MeridianBackup-$SERVER-$DATE.zip
nice -n 19 zip -9 $BACKUP_FILENAME $BASE_DIR/savegame/*

# Copy to S3
nice -n 19 aws s3 cp $BACKUP_FILENAME s3://meridian-backup

# Delete old log files
nice -n 19 find $BASE_DIR/channel/* -mmin +14400 -delete
