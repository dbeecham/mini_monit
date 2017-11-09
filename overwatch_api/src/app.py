import json
from flask import Flask
from influxdb import InfluxDBClient as InfluxDB
import re

app = Flask(__name__)

valid_endpoint = re.compile('[a-zA-Z]+(\.[a-zA-Z]+)*')

@app.route("/")
def hello():
    return "yes"

@app.route("/api/v1/endpoint/<endpoint>")
def endpoint(endpoint):
    if valid_endpoint.fullmatch(endpoint) != None:
        c = InfluxDB('localhost', 8086, '', '', 'overwatch')
        res = c.query('select * from "{}" where time > now() - 20m;'.format(endpoint))
        return json.dumps(list(res.get_points()))
