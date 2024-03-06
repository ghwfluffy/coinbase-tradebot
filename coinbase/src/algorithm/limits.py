from context import Context
from utils.math import floor_usd

def check_tranche_funds(ctx: Context, usd: float) -> bool:
    # TODO
    # Loop through all tranches, find associated orders, sum to dollar amount
    # Get wallet
    # Make sure we are within tranche limit
    pass

def get_phased_wager(ctx: Context) -> float:
    # TODO
    # Get current wallet
    # Multiple wallet total by phased amount
    # Check its within available USD
    pass
