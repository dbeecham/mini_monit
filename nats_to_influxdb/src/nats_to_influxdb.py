import asyncio
import json
from nats.aio.client import Client as NATS
from influxdb import InfluxDBClient as InfluxDB
from datetime import datetime

endpoints = [
        "s.http.pluspole.se",
        "s.http.parans.pluspole.se"
]

async def run(loop):
    nc = NATS()
    db = InfluxDB('localhost', 8086, '', '', 'overwatch')

    await nc.connect(io_loop=loop)

    def message_handler(endpoint):
        async def _message_handler(msg):
            data = msg.data.decode()
            j = json.loads(data)

            print("j: {}".format(j))

            point = {
                "measurement": endpoint,
                "time": datetime.utcnow().isoformat(),
                "fields": j
            }

            try:
                db.write_points([point])
            except Exception as e:
                print("except: {}".format(e))

            print("msg: {}".format(point))

        return _message_handler

    for endpoint in endpoints:
        await nc.subscribe(endpoint, cb=message_handler(endpoint))

    while True:
        await asyncio.sleep(10, loop=loop)

if __name__ == '__main__':
    loop = asyncio.get_event_loop()

    try:
        loop.run_until_complete(run(loop))
    finally:
        loop.close()
