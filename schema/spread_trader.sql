CREATE TABLE IF NOT EXISTS order_pairs (
    uuid VARCHAR PRIMARY KEY,
    algorithm VARCHAR,
    state VARCHAR,
    buy_order_uuid VARCHAR,
    sell_order_uuid VARCHAR,
    bet INTEGER,
    buy_price INTEGER,
    sell_price INTEGER,
    quantity INTEGER,
    created INTEGER,
    final_purchased INTEGER,
    final_buy_fees INTEGER,
    final_sold INTEGER,
    final_sell_fees INTEGER
);
