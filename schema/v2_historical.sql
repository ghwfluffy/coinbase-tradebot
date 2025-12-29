CREATE TABLE IF NOT EXISTS btc_price (
    time BIGINT PRIMARY KEY,
    price BIGINT
);

CREATE TABLE IF NOT EXISTS wallet (
    time BIGINT PRIMARY KEY,
    usd BIGINT,
    btc BIGINT,
    value BIGINT
);
