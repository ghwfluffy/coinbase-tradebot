#include <gtb/OrderPairDb.h>
#include <gtb/Uuid.h>
#include <gtb/Log.h>

#include <sstream>

using namespace gtb;

namespace
{

constexpr const char *SCHEMA_FILE = "./schema/spread_trader.sql";
constexpr const char *PROD_DB_FILE = "spread_trader.sqlite";
std::string dbFile = PROD_DB_FILE;

constexpr const char *COLUMNS =
    "uuid, algorithm, state, buy_order_uuid, sell_order_uuid, bet, buy_price, sell_price, quantity, created, "
        "final_purchased, final_buy_fees, final_sold, final_sell_fees";

std::string getActiveStates()
{
    return "'None', 'Pending', 'BuyActive', 'Holding', 'SellActive'";
}

std::string getBoughtStates()
{
    return "'Holding', 'SellActive'";
}

bool selectQuery(
    Database &db,
    const std::string &query,
    std::list<OrderPair> &pairs)
{
    DatabaseResult result = db.getConn().query(query);
    if (!result)
    {
        log::error("Failed to query order pairs.");
        return false;
    }

    while (result.next())
    {
        size_t col = 0;
        OrderPair pair;
        pair.uuid = result[col++].getString();
        pair.algo = result[col++].getString();
        from_string(result[col++].getString(), pair.state);
        pair.buyOrder = result[col++].getString();
        pair.sellOrder = result[col++].getString();
        pair.betCents = result[col++].getUInt32();
        pair.buyPrice = result[col++].getUInt32();
        pair.sellPrice = result[col++].getUInt32();
        pair.quantity = result[col++].getUInt64();
        pair.created = result[col++].getUInt64();
        pair.profit.purchased = result[col++].getUInt64();
        pair.profit.buyFees = result[col++].getUInt64();
        pair.profit.sold = result[col++].getUInt64();
        pair.profit.sellFees = result[col++].getUInt64();

        pairs.push_back(std::move(pair));
    }

    return true;
}

}

void OrderPairDb::setDbFile(
    std::string str)
{
    dbFile = std::move(str);
}

void OrderPairDb::initDb(
    Database &db)
{
    db.init(dbFile, SCHEMA_FILE);
}

std::list<OrderPair> OrderPairDb::select(
    Database &db,
    const std::string &algorithm,
    bool activeOnly)
{
    std::string query;
    query += "SELECT ";
    query += COLUMNS;
    query += " FROM order_pairs";
    query += " WHERE 1=1";
    if (!algorithm.empty())
        query += " AND algorithm='" + algorithm + "'";
    if (activeOnly)
        query += " AND state IN (" + getActiveStates() + ")";

    std::list<OrderPair> orders;
    if (!selectQuery(db, query, orders))
        log::error("Failed to query order pairs for algorithm '%s'.", algorithm.c_str());

    return orders;
}

bool OrderPairDb::selectBought(
    Database &db,
    std::list<OrderPair> &orders)
{
    std::string query;
    query += "SELECT ";
    query += COLUMNS;
    query += " FROM order_pairs";
    query += " WHERE state IN (" + getBoughtStates() + ")";
    return selectQuery(db, query, orders);
}

bool OrderPairDb::remove(
    Database &db,
    const std::string &uuid)
{
    std::string query;
    query += "DELETE FROM order_pairs WHERE uuid='" + uuid + "'";
    if (!db.getConn().execute(query))
    {
        log::error("Failed to remove order pair '%s' from DB.", uuid.c_str());
        return false;
    }

    return true;
}

bool OrderPairDb::insert(
    Database &db,
    OrderPair &pair)
{
    // Create UUID
    if (pair.uuid.empty())
        pair.uuid = Uuid::generate();

    std::ostringstream oss;
    oss << "INSERT INTO order_pairs (" << COLUMNS << ")"
        << "VALUES ("
        << "'" << pair.uuid << "',"
        << "'" << pair.algo << "',"
        << "'" << to_string(pair.state) << "',"
        << "'" << pair.buyOrder << "',"
        << "'" << pair.sellOrder << "',"
        << pair.betCents << ","
        << pair.buyPrice << ","
        << pair.sellPrice << ","
        << pair.quantity << ","
        << pair.created << ","
        << pair.profit.purchased << ","
        << pair.profit.buyFees << ","
        << pair.profit.sold << ","
        << pair.profit.sellFees << ")";
    if (!db.getConn().execute(oss.str()))
    {
        log::error("Failed to insert order pair '%s' in DB.", pair.uuid.c_str());
        return false;
    }

    return true;
}

bool OrderPairDb::update(
    Database &db,
    const OrderPair &pair)
{
    std::ostringstream oss;
    oss << "UPDATE order_pairs SET "
        << "algorithm='" << pair.algo << "',"
        << "state='" << to_string(pair.state) << "',"
        << "buy_order_uuid='" << pair.buyOrder << "',"
        << "sell_order_uuid='" << pair.sellOrder << "',"
        << "bet=" << pair.betCents << ","
        << "buy_price=" << pair.buyPrice << ","
        << "sell_price=" << pair.sellPrice << ","
        << "quantity=" << pair.quantity << ","
        << "created=" << pair.created << ","
        << "final_purchased=" << pair.profit.purchased << ","
        << "final_buy_fees=" << pair.profit.buyFees << ","
        << "final_sold=" << pair.profit.sold << ","
        << "final_sell_fees=" << pair.profit.sellFees << " "
        << "WHERE uuid='" << pair.uuid << "'";
    if (!db.getConn().execute(oss.str()))
    {
        log::error("Failed to update order pair '%s' in DB.", pair.uuid.c_str());
        return false;
    }

    return true;
}
