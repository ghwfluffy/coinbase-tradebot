from coinbase.rest import rest_base
from market.smooth import SmoothMarket

class Context(rest_base.RESTBase):
    def __init__(self):
        CREDENTIAL="../secrets/coinbase_cloud_api_key.json"
        super().__init__(key_file=CREDENTIAL)

        self.smooth = SmoothMarket()
        self.tranches = []

        self.phases = []
        self.faster_smooth = SmoothMarket(max_change_per_minute=0.001)
