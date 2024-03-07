from coinbase.rest import rest_base
from market.smooth import SmoothMarket

class Context(rest_base.RESTBase):
    def __init__(self):
        CREDENTIAL="../secrets/coinbase_cloud_api_key.json"
        super().__init__(key_file=CREDENTIAL)

        # Version 3 algorithm: Tranched
        self.smooth = SmoothMarket()
        self.tranches = []

        # Version 4 algorithm: Phasic
        self.phases = []
