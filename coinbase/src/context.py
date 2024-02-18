from coinbase.rest import rest_base

class Context(rest_base.RESTBase):
    def __init__(self):
        CREDENTIAL="../secrets/coinbase_cloud_api_key.json"
        super().__init__(key_file=CREDENTIAL)
