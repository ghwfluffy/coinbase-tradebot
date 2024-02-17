import json
import time
import requests

import hashlib
import base64
import hmac

class RestIfx():
    def __init__(self):
        self.init_api_secret()

    def init_api_secret(self):
        with open("../secrets/cex.json") as fp:
            data = fp.read()
            data = json.loads(data)
            self.api_key = data['api_key']
            self.api_secret = data['api_secret'].encode('utf-8')

    def post(self, endpoint, params = None):
        return self.request('POST', endpoint, params)

    def request(self, method, endpoint, params = None):
        if method == "GET":
            func = requests.get
        elif method == "POST":
            func = requests.post

        timestamp = str(time.time())
        auth_msg = str(endpoint + timestamp + json.dumps(params)).encode('utf-8')
        signature = base64.b64encode(hmac.new(self.api_secret, auth_msg, hashlib.sha256).digest())

        headers = {
            'X-AGGR-KEY': self.api_key,
            'X-AGGR-TIMESTAMP': timestamp,
            'X-AGGR-SIGNATURE': signature,
            'Content-Type': 'application/json',
        }

        url = 'https://trade.cex.io/api/spot/rest-public/' + endpoint

        try:
            response = func(url, data=json.dumps(params), headers=headers)
            if response.status_code == 200:
                ret = response.json()['data']
                ret['status_code'] = response.status_code
                ret['success'] = True
                ret['error'] = 'Success'
            else:
                ret['success'] = False
        except Exception as e:
            ret = {
                'success': False,
                'status_code': 0,
                'error': str(e),
            }

        return ret
