from context import Context
from settings import Settings
from utils.maths import floor_usd
from utils.wallet import get_wallet
from market.current import CurrentMarket
from orders.order_pair import OrderPair
from orders.order_book import OrderBook

def get_hodl_usd(ctx: Context, wallet) -> float | None:
    market: CurrentMarket | None = CurrentMarket.get(ctx)
    if not market:
        return None

    wallet_btc = wallet['BTC']['total']
    pending_sell_btc: float = 0
    for order in ctx.orderbook:
        if order.status == OrderPair.Status.PendingSell and order.sell:
            pending_sell_btc += order.sell.btc

    orphaned = wallet_btc - pending_sell_btc
    if orphaned > 0:
        return orphaned * market.split
    return 0.01

def check_tranche_funds(ctx: Context, usd: float) -> bool:
    # Loop through all tranches, find associated orders, sum to dollar amount
    total_active: float = 0
    for order in ctx.orderbook:
        # Only care about active orders
        if not order.status in [
            OrderPair.Status.Active,
            OrderPair.Status.PendingSell,
            ]:
            continue

        total_active += order.buy.usd

    # Get wallet
    wallet = get_wallet(ctx)
    if not wallet:
        return False

    # Get HODL amount
    hodl_usd: float | None = get_hodl_usd(ctx, wallet)
    if not hodl_usd:
        return False
    assert hodl_usd is not None

    # Total all USD minus HODL
    total_wallet: float = wallet['all']['total']
    tranched_amount: float = (total_wallet - hodl_usd) * Settings.TRANCHED_WALLET_POOL
    if tranched_amount <= 0:
        return False

    # Remaining funds to use for tranches
    remaining_funds: float = tranched_amount - total_active
    if remaining_funds <= 0:
        return False

    # Do we have 'usd' amount available for this purchase?
    if remaining_funds < usd:
        return False

    return True

def get_phased_wager(ctx: Context) -> float | None:
    # Get wallet
    wallet = get_wallet(ctx)
    if not wallet:
        return None

    # Get HODL amount
    hodl_usd: float | None = get_hodl_usd(ctx, wallet)
    if not hodl_usd:
        return None
    assert hodl_usd is not None

    # Total all USD minus HODL
    total_wallet: float = wallet['all']['total']
    phased_amount: float = (total_wallet - hodl_usd) * Settings.PHASED_WALLET_POOL
    if phased_amount <= 20.0:
        return None

    if phased_amount > 20.0: # TODO: Testing
        return 20.0

    return phased_amount
