from BaseHTTPServer import BaseHTTPRequestHandler
import urlparse

# 1st part - Global defines & functions
value_f  = "/sys/class/gpio/gpio66/value"
addr = ('192.168.7.2', 8080)

def pr_str(val):
	return "on" if val else "off"

def put_data(file, data):
	f = open(file, "w")
	f.write(data)
	f.close()

def get_data(file):
	f = open(file, "r")
	data = f.read()
	f.close()
	return data

class GetHandler(BaseHTTPRequestHandler):
	# 2nd part - Define the HTTPServer's GET handler
	def do_GET(self):
		parsed_path = urlparse.urlparse(self.path)

		# 1st part - Set the new led status as requested
		query = parsed_path.query
		data = urlparse.parse_qs(query)
		if ("led" in data):
			led_new_status = data["led"][0]
			put_data(value_f, led_new_status)

		# 3rd part - Get the current led status
		led_status = int(get_data(value_f))

		# 4th part - Get the current led status
		led_new_status = 1 - led_status

		# 5th part - Get the current led status
		message = '''
<html>
  <head>
    <title>Turing a led on/off using Python</title>
  </head>

  <body>
    <h1>Turing a led on/off using Python</h1>
    Current led status is: %s
    <p>

    Press the button to turn the led %s
    <p>

    <form method="get" action="/">
      <button type="submit" value="%s" name="led">Turn %s</button>
    </form>
  </body>
</html>
''' % (pr_str(led_status), pr_str(led_new_status),
	led_new_status, pr_str(led_new_status))
		# 6th part - Send the answer
		self.send_response(200)
		self.end_headers()
		self.wfile.write(message)

		return

# 7th part - Setup the web server
if __name__ == '__main__':
	from BaseHTTPServer import HTTPServer
	server = HTTPServer(addr, GetHandler)
	print 'Starting server at %s:%s, use <Ctrl-C> to stop' % addr
	server.serve_forever()
