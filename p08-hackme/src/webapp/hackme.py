#!/usr/bin/env python

import pickle
from base64 import b64decode,b64encode
from os import popen

from flask import Flask, request, make_response


app = Flask(__name__)


def rp(command):
    return popen(command).read()


@app.route('/')
def index():
    return """
    <html>
    <head><title>Hack Me!</title></head>
    <body>
        <p><h3>Functions</h3></p>
        <a href="/lookup">DNS lookup on address</a><br>
        <a href="/evaluate">Evaluate expression</a><br>
    </body>
    </html>
    """


@app.route('/evaluate', methods = ['POST', 'GET'])
def evaluate():
    expression = None
    if request.method == 'POST':
        expression = request.form['expression']
    return """
    <html>
       <body>""" + "Result: " + (str(eval(expression)).replace('\n', '\n<br>')  if expression else "") + """
          <form action = "/evaluate" method = "POST">
             <p><h3>Enter expression to evaluate</h3></p>
             <p><input type = 'text' name = 'expression'/></p>
             <p><input type = 'submit' value = 'Evaluate'/></p>
          </form>
       </body>
    </html>
    """


@app.route('/lookup', methods = ['POST', 'GET'])
def lookup():
    address = None
    if request.method == 'POST':
        address = request.form['address']
    return """
    <html>
       <body>""" + "Result:\n<br>\n" + (rp("nslookup " + address).replace('\n', '\n<br>')  if address else "") + """
          <form action = "/lookup" method = "POST">
             <p><h3>Enter address to lookup</h3></p>
             <p><input type = 'text' name = 'address'/></p>
             <p><input type = 'submit' value = 'Lookup'/></p>
          </form>
       </body>
    </html>
    """


@app.route('/web_cookie_setting', methods = ['POST', 'GET'])
def cookie():
    cookieValue = None
    value = None
    
    if request.method == 'POST':
        cookieValue = request.form['value']
        value = cookieValue
    elif 'cookie_value' in request.cookies:
        cookieValue = pickle.loads(b64decode(request.cookies['cookie_value'])) 
    
        
    form = """
    <html>
       <body>Cookie value: """ + str(cookieValue) +"""
          <form action = "/web_cookie_setting" method = "POST">
             <p><h3>Enter value to be stored in cookie</h3></p>
             <p><input type = 'text' name = 'value'/></p>
             <p><input type = 'submit' value = 'Set Cookie'/></p>
          </form>
       </body>
    </html>
    """
    resp = make_response(form)
    
    if value:
        resp.set_cookie('cookie_value', b64encode(pickle.dumps(value)))

    return resp


if __name__ == '__main__':
    app.run()
