CREATE TABLE IF NOT EXISTS btc_price (
    time INTEGER PRIMARY KEY,
    price INTEGER
);

CREATE TABLE IF NOT EXISTS profits_and_losses (
    time INTEGER PRIMARY KEY,
    purchased INTEGER,
    sold INTEGER,
    buy_fees INTEGER,
    sell_fees INTEGER,
    profit INTEGER
);
