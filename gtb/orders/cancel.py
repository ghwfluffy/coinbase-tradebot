from datetime import datetime

from gtb.core.context import Context
from gtb.orders.order import Order, OrderInfo
from gtb.utils.logging import Log

from coinbase.rest import orders as order_api

def cancel_order(ctx: Context, algorithm: str, order: Order, reason: str) -> bool:
    # Already complete
    if order.status == Order.Status.Complete:
        return False
    # Already canceled
    if order.status == Order.Status.Canceled:
        return True
    # Not registered with coinbase
    if order.status == Order.Status.Pending or order.status == Order.Status.OnHold:
        order.status = Order.Status.Canceled
        return True

    try:
        assert order.info is not None
        result = order_api.cancel_orders(
            ctx.api,
            order_ids=[order.info.order_id],
        )
        ok = result['results'][0]['success']
        if ok:
            order.status = Order.Status.Canceled
            order.info.final_time = datetime.now()
            order.info.cancel_reason = reason
            order.info.cancel_time = datetime.now()
            Log.info("Canceled ${:.2f} {} for {}: {}".format(
                order.usd,
                order.order_type.name,
                algorithm,
                reason))
        return ok
    except Exception as e:
        Log.exception("Failed to cancel ${:.2f} {} for {} ({})".format(
            order.usd,
            order.order_type.name,
            algorithm,
            reason,
        ), e)

    return False
