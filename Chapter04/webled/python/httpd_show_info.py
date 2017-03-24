from BaseHTTPServer import BaseHTTPRequestHandler
import urlparse

# 1st part - Global defines & functions
addr = ('192.168.7.2', 8080)

class GetHandler(BaseHTTPRequestHandler):
	# 2nd part - Define the HTTPServer's GET handler

	def do_GET(self):
		parsed_path = urlparse.urlparse(self.path)

		# 3rd part - Build the answering message
		message_parts = [
			'CLIENT VALUES',
			'client_address -> %s (%s)' % (self.client_address,
						self.address_string()),
			'command -> %s' % self.command,
			'path -> %s' % self.path,
			'real path -> t%s' % parsed_path.path,
			'query -> %s' % parsed_path.query,
			'request_version -> %s' % self.request_version,
			'',
			'SERVER VALUES',
			'server_version -> %s' % self.server_version,
			'sys_version -> %s' % self.sys_version,
			'protocol_version -> %s' % self.protocol_version,
			'',
			'HEADERS RECEIVED',
		]

		for name, value in sorted(self.headers.items()):
			message_parts.append('%s -> %s' % (name,
				value.rstrip()))
		message_parts.append('')
		message = '\r\n'.join(message_parts)

		# 4th part - Send the answer
		self.send_response(200)
		self.end_headers()
		self.wfile.write(message)

		return

# 5th part - Setup the web server
if __name__ == '__main__':
	from BaseHTTPServer import HTTPServer
	server = HTTPServer(addr, GetHandler)
	print 'Starting server at %s:%s, use <Ctrl-C> to stop' % addr
	server.serve_forever()
