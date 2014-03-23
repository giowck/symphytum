"""
@package dropbox_client
@author Giorgio Wicklein - GIOWISYS Software
@date 07/06/2012
Dropbox sync client.
This client implements the Dropbox API using the Dropbox Python SDK.
"""
import os
from sys import argv, exit
from dropbox import client, rest, session

ACCESS_TYPE = 'app_folder'
APP_KEY = 'yhes0ma0ptan2as'
APP_SECRET = ''

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

# Get request token
def get_request_token(sess):
	request_token = sess.obtain_request_token()
	return request_token

# Get the authorization URL to link a Dropbox account
def get_authorize_url(sess, request_token):
	url = sess.build_authorize_url(request_token)
	return url

# Get access token for API access
def get_access_token(sess, request_token):
	access_token = sess.obtain_access_token(request_token)
	return access_token

# Get user name
def get_user_name(dclient):
	dict = dclient.account_info()
	return dict["display_name"];

# Upload file
def upload_file(dclient, src_file, dest_file):
	try:
		f = open(src_file, "rb")
	except IOError, e:
		print "Error:Cannot open file", str(e)
		exit(0)
	metadata = dclient.put_file(dest_file, f, True)
	f.close()
	original_bytes = os.stat(src_file).st_size
	uploaded_bytes = metadata["bytes"]
	if original_bytes == uploaded_bytes:
		return "Upload:OK"
	else:
		return "Error:Upload data corrupted"

# Chunked upload (for files > 150 MiB)
def upload_file_chunked(dclient, src_file, dest_file):
	total_bytes = 0
	try:
		total_bytes = os.stat(src_file).st_size
		f = open(src_file, "rb")
	except IOError, e:
		print "Error:Cannot open file", str(e)
		exit(0)
	uploader = dclient.get_chunked_uploader(f, total_bytes)
	print "Upload:Started"
	error = False
	while uploader.offset < total_bytes:
		try:
			upload = uploader.upload_chunked()
			error = False
		except Exception, e:
			if not error:
				error = True
				print "Warning:Chunk upload failed. Second try..."
			else:
				print "Warning:Chunk upload failed for second time. Aborted."
				print "Error:" + str(e)
				exit(0)
	uploader.finish(dest_file, True)
	print "Upload:OK"

# Download file
def download_file(dclient, src_file, dest_file):
	try:
		out = open(dest_file, "wb")
	except IOError, e:
		print "Error:Cannot open file", str(e)
		exit(0)
	f, metadata = dclient.get_file_and_metadata(src_file)
	out.write(f.read())
	out.close()
	f.close()
	downloaded_bytes = os.stat(dest_file).st_size
	metadata_bytes = metadata["bytes"]
	if downloaded_bytes == metadata_bytes:
		return "Download:OK"
	else:
		return "Error:Download data corrupted"

# Delete file
def delete_file(dclient, file_name):
	dclient.file_delete(file_name)


# Start
def main():
	error = False
	if len(argv) < 4:
		print "Missing arguments. Usage:", argv[0], "<app secret>", \
		"<access token> <command> [<args>]"
		exit(0)
	
	global APP_SECRET, ACCESS_TOKEN
	APP_SECRET = argv[1]
	access_token_string = argv[2]
	command = argv[3]
	
	dsession = session.DropboxSession(APP_KEY, APP_SECRET, ACCESS_TYPE)
	# if access token has ben specified, set it
	if access_token_string != "none":
		l = access_token_string.split('_')
		if len(l) < 2:
			print "Error:Invalid access token"
			exit(0)
		key = l[0]
		secret = l[1]
		dsession.set_token(key, secret)
		
	dclient = client.DropboxClient(dsession)
	
	try:
		if command == "authorize_url":
			request_token = get_request_token(dsession)
			print ":Request token:" + request_token.key + "_" + request_token.secret + ":"
			print ":URL:" + get_authorize_url(dsession, request_token) + ":"
		elif command == "create_access_token":
			if len(argv) >= 5:
				token_string = argv[4]
			else:
				token_string = "invalid_invalid"	
			l = token_string.split('_')
			key = l[0]
			secret = l[1]
			request_token = session.OAuthToken(key, secret)
			access_token = get_access_token(dsession, request_token)
			print ":Access token:" + access_token.key + "_" + access_token.secret + ":"
		elif command == "user_name":
			print ":User:" + get_user_name(dclient) + ":"
		elif command == "upload_file":
			if len(argv) < 6:
				print "Error:Arguments missing in upload_file()"
				exit(0)
			src = argv[4]
			dest = argv[5]
			response = upload_file(dclient, src, dest)
			print response
		elif command == "upload_file_chunked":
			if len(argv) < 6:
				print "Error:Arguments missing in upload_file_chunked()"
				exit(0)
			src = argv[4]
			dest = argv[5]
			upload_file_chunked(dclient, src, dest)
		elif command == "download_file":
			if len(argv) < 6:
				print "Error:Arguments missing in download_file()"
				exit(0)
			src = argv[4]
			dest = argv[5]
			response = download_file(dclient, src, dest)
			print response
		elif command == "delete_file":
			if len(argv) < 5:
				print "Error:Arguments missing in delete_file()"
				exit(0)
			delete_file(dclient, argv[4])
			print "Delete:OK"
		else:
			print "Error:Invalid command"
		
	except Exception, e:
			print "Error:" + str(e)
			exit(0)
	
if __name__ == "__main__":
    main()
