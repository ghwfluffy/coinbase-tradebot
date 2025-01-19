CREATE TABLE IF NOT EXISTS order_pairs (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    algorithm VARCHAR,
    buy_order_uuid VARCHAR,
    sell_order_uuid VARCHAR,
    buy_price INTEGER,
    sell_price INTEGER,
    quantity INTEGER,
    created VARCHAR
);
