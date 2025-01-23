#include <gtb/OrderPairDb.h>
#include <gtb/Uuid.h>
#include <gtb/Log.h>

#include <sstream>

using namespace gtb;

namespace
{

constexpr const char *COLUMNS =
    "uuid, algorithm, state, buy_order_uuid, sell_order_uuid, buy_price, sell_price, quantity, created";

std::string getActiveStates()
{
    return "'None', 'Pending', 'BuyActive', 'Holding', 'SellActive'";
}

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

    DatabaseResult result = db.getConn().query(query);
    if (!result)
        log::error("Failed to query order pairs for algorithm '%s'.", algorithm.c_str());

    std::list<OrderPair> pairs;
    while (result.next())
    {
        size_t col = 0;
        OrderPair pair;
        pair.uuid = result[col++].getString();
        pair.algo = result[col++].getString();
        from_string(result[col++].getString(), pair.state);
        pair.buyOrder = result[col++].getString();
        pair.sellOrder = result[col++].getString();
        pair.buyPrice = result[col++].getUInt32();
        pair.sellPrice = result[col++].getUInt32();
        pair.quantity = result[col++].getUInt64();
        pair.created = result[col++].getUInt64();

        pairs.push_back(std::move(pair));
    }

    return pairs;
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
        << pair.buyPrice << ","
        << pair.sellPrice << ","
        << pair.quantity << ","
        << pair.created << ")";
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
        << "buy_price=" << pair.buyPrice << ","
        << "sell_price=" << pair.sellPrice << ","
        << "quantity=" << pair.quantity << ","
        << "created=" << pair.created << " "
        << "WHERE uuid='" << pair.uuid << "'";
    if (!db.getConn().execute(oss.str()))
    {
        log::error("Failed to update order pair '%s' in DB.", pair.uuid.c_str());
        return false;
    }

    return true;
}
