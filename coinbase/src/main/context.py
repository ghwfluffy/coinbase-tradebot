from typing import List

from coinbase.rest import rest_base
from market.smooth import SmoothMarket
# TODO: Split ctx into 'state' and 'coinbase_ctx'
#from algorithm.hodl import HodlHistory
#from algorithm.tranche import Tranche
#from algorithm.phase import Phase
from typing import Any, Callable

class Context(rest_base.RESTBase):
    def __init__(self) -> None:
        CREDENTIAL="../secrets/coinbase_cloud_api_key.json"
        super().__init__(key_file=CREDENTIAL)

        self.orderbook: Any = None #: OrderBook = OrderBook.read_fs("orderbook.json")

        # Version 3 algorithm: Tranched
        self.smooth: SmoothMarket = SmoothMarket()
        #self.tranches: List[Tranche] = []
        self.tranches: List[Any] = []

        # Version 4 algorithm: Phasic
        #self.phases: List[Phase] = []
        self.phases: List[Any] = []

        # Version 0 algorithm: HODL
        #self.hodl_history: HodlHistory = HodlHistory()
        self.hodl_history: Any = None

        self.check_tranche_funds: Callable = lambda ctx, usd: None
        self.get_phased_wager: Callable = lambda ctx: None
