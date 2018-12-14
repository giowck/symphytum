"""
@package dropbox_client
@author Giorgio Wicklein
@date 04/06/2017
Dropbox sync client.
This client implements the Dropbox API using the Dropbox v2 Python SDK.
"""
import os
from sys import argv, exit
import dropbox
from dropbox import Dropbox, DropboxOAuth2FlowNoRedirect
from dropbox.files import WriteMode

ACCESS_TYPE = 'app_folder'
APP_KEY = '3gef8jrsy599xfg'

# This is a workaround to get unbuffered console output
import sys
class Unbuffered:
   def __init__(self, stream):
       self.stream = stream
   def write(self, data):
       self.stream.write(data)
       self.stream.flush()
   def __getattr__(self, attr):
       return getattr(self.stream, attr)
sys.stdout=Unbuffered(sys.stdout)

# Get the authorization URL to link a Dropbox account
def get_authorize_url(auth_flow):
    authorize_url = auth_flow.start()
    return authorize_url

# Get access token for future API access from auth code string
def get_access_token(auth_flow, auth_token):
    authorize_url = auth_flow.start()
    oauth_result = auth_flow.finish(auth_token)
    dbx = Dropbox(oauth_result.access_token)
    return oauth_result.access_token

# Get user name
def get_user_name(dbx):
    account = dbx.users_get_current_account()
    username = account.name.display_name;
    if sys.version_info < (3, 0): # python2.7 default encoding is ASCII
        username = username.encode('utf-8') # in case user name contains non ASCII characters
    return username
    
# Delete file, returns file metadata
def delete_file(dbx, file_name):
    return dbx.files_delete("/" + file_name)

# Download file
def download_file(dbx, src_file, dest_file):
    try:
        file_metadata = dbx.files_download_to_file(dest_file, str("/" + src_file))
    except Exception as e:
        return str("Error:Download error " + str(e))
    
    return str("Download:OK")

# Upload file
def upload_file(dbx, src_file, dest_file):
    try:
        f = open(src_file, "rb")
    except IOError as e:
        print("Error:Cannot open file" + str(e))
        exit(0)
    dbx.files_upload(f.read(), str("/" + dest_file), mode=WriteMode('overwrite'))
    f.close()
    return str("Upload:OK")

# Chunked upload (for files > 150 MiB)
def upload_file_chunked(dbx, src_file, dest_file):
    CHUNK_SIZE = 1 * 1024 * 1024 #1MB instead of 4MB (4 * 1024 * 1024), workaround since 4MB gives socket timeout error (bug)
    try:
        f = open(src_file, "rb")
    except IOError as e:
        print("Error:Cannot open file" + str(e))
        exit(0)
    file_size = os.path.getsize(src_file)
    print("Uploading:" + str(f.tell()))
    upload_session_start_result = dbx.files_upload_session_start(f.read(CHUNK_SIZE))
    cursor = dropbox.files.UploadSessionCursor(session_id=upload_session_start_result.session_id,
                                                   offset=f.tell())
    commit = dropbox.files.CommitInfo(path=str("/" + dest_file), mode=WriteMode('overwrite'))
        
    while f.tell() < file_size:
        if ((file_size - f.tell()) <= CHUNK_SIZE):
            print("Uploading:" + str(f.tell()))
            dbx.files_upload_session_finish(f.read(CHUNK_SIZE),
                                            cursor,
                                            commit)
            
        else:
            print("Uploading:" + str(f.tell()))
            dbx.files_upload_session_append(f.read(CHUNK_SIZE),
                                            cursor.session_id,
                                            cursor.offset)
            cursor.offset = f.tell()
    f.close()
    return str("Upload:OK")

# Start
def main():
    error = False
    if len(argv) < 4:
        print("Missing arguments. Usage:", argv[0], "<app secret>", \
        "<access token> <command> [<args>]")
        exit(0)
    
    global APP_SECRET, ACCESS_TOKEN
    APP_SECRET = argv[1]
    access_token_string = argv[2]
    command = argv[3]
    
    # if access token has ben specified, set it
    if access_token_string != "none":
        ACCESS_TOKEN = access_token_string
    
    try:
        if command == "authorize_url":
            auth_flow = DropboxOAuth2FlowNoRedirect(APP_KEY, APP_SECRET)
            print(":URL:" + get_authorize_url(auth_flow) + ":")
        elif command == "create_access_token":
            if len(argv) >= 5:
                auth_token_string = argv[4]
            else:
                print("Error:Auth token not specified in get_access_token()")
                exit(0)
            auth_flow = DropboxOAuth2FlowNoRedirect(APP_KEY, APP_SECRET)
            print(":Access token:" + get_access_token(auth_flow, auth_token_string) + ":")
        elif command == "user_name":
            dbx = Dropbox(ACCESS_TOKEN)
            print(":User:" + get_user_name(dbx) + ":")
        elif command == "upload_file":
            if len(argv) < 6:
                print("Error:Arguments missing in upload_file()")
                exit(0)
            src = argv[4]
            dest = argv[5]
            dbx = Dropbox(ACCESS_TOKEN)
            response = upload_file(dbx, src, dest)
            print(response)
        elif command == "upload_file_chunked":
            if len(argv) < 6:
                print("Error:Arguments missing in upload_file_chunked()")
                exit(0)
            src = argv[4]
            dest = argv[5]
            dbx = Dropbox(ACCESS_TOKEN)
            response = upload_file_chunked(dbx, src, dest)
            print(response)
        elif command == "download_file":
            if len(argv) < 6:
                print("Error:Arguments missing in download_file()")
                exit(0)
            src = argv[4]
            dest = argv[5]
            dbx = Dropbox(ACCESS_TOKEN)
            response = download_file(dbx, src, dest)
            print(response)
        elif command == "delete_file":
            if len(argv) < 5:
                print("Error:Arguments missing in delete_file()")
                exit(0)
            dbx = Dropbox(ACCESS_TOKEN)
            delete_file(dbx, argv[4])
            print("Delete:OK")
        else:
            print("Error:Invalid command")
        
    except Exception as e:
            print("Error:" + str(e))
            exit(0)
    
if __name__ == "__main__":
    main()

