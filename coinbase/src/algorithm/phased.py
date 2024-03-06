from context import Context
from algorithm.phase import Phase
from orders.order_book import OrderBook

def update_phases(ctx: Context, orderbook: OrderBook):
    # Get most recent phase
    if len(ctx.phases) == 0:
        ctx.phases.append(Phase())
    phase: Phase = ctx.phases[-1]

    # Get current market value
    market: CurrentMarket = CurrentMarket.get(ctx)
    if market == None:
        return None

    # Update smooth
    ctx.faster_smooth.update_market(market)

    # 
    phase.add_point(ctx.faster_smooth)

    # TODO
    pass
