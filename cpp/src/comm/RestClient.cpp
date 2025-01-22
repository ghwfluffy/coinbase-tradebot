#include <gtb/RestClient.h>
#include <gtb/Log.h>

#include <sstream>

using namespace gtb;

namespace
{

HttpResponse parseResponse(
    cpr::Response response)
{
    HttpResponse ret;
    ret.code = response.status_code < 0 ? 0 : static_cast<unsigned int>(response.status_code);
    if (response.status_code == 0)
        log::error("Failed HTTP query: %s", response.error.message.c_str());

    try {
        ret.data = nlohmann::json::parse(response.text);
    } catch (const std::exception &e) {
        // Ignore
    }

    return ret;
}

}

HttpResponse::operator bool() const
{
    return code == 200;
}

std::string HttpResponse::text() const
{
    std::stringstream ss;
    ss << "Code: " << code;
    if (data != nullptr)
        ss << " | " << data.dump();
    return ss.str();
}

RestClient::RestClient(
    std::string baseUrl)
        : baseUrl(std::move(baseUrl))
{
}

HttpResponse RestClient::post(
    const std::string &path,
    const std::string &jwt,
    const nlohmann::json &data)
{
    // Setup request
    Connection conn = getConn();
    conn.session->SetUrl(baseUrl + path);
    conn.setJwt(jwt);

    if (data != nullptr)
        conn.session->SetBody(data.dump());

    // Send
    cpr::Response response = conn.session->Post();
    saveConn(std::move(conn));

    return parseResponse(response);
}

HttpResponse RestClient::get(
    const std::string &path,
    const std::string &jwt,
    const nlohmann::json &data,
    bool query)
{
    // Setup request
    Connection conn = getConn();
    conn.session->SetUrl(baseUrl + path);
    conn.setJwt(jwt);

    if (data != nullptr)
    {
        if (!query)
            conn.session->SetBody(data.dump());
        else
        {
            cpr::Parameters params;
            for (auto it = data.begin(); it != data.end(); ++it)
                params.Add({it.key(), it.value().dump()});
            conn.session->SetParameters(params);
        }
    }

    // Send
    cpr::Response response = conn.session->Get();
    saveConn(std::move(conn));

    return parseResponse(response);
}

HttpResponse RestClient::del(
    const std::string &path,
    const std::string &jwt)
{
    // Setup request
    Connection conn = getConn();
    conn.session->SetUrl(baseUrl + path);
    conn.setJwt(jwt);

    // Send
    cpr::Response response = conn.session->Delete();
    saveConn(std::move(conn));

    return parseResponse(response);
}

RestClient::Connection RestClient::getConn()
{
    RestClient::Connection conn;

    // Try to find a live connection in the pool
    bool found = false;
    {
        auto now = std::chrono::steady_clock::now();

        std::lock_guard<std::mutex> lock(mtx);
        auto iter = sessionPool.begin();
        while (!found && iter != sessionPool.end())
        {
            conn = std::move(*iter);
            iter = sessionPool.erase(iter);
            if (conn.timeout > now)
                found = true;
        }
    }

    if (!found)
        conn = newConn();

    return conn;
}

RestClient::Connection RestClient::newConn() const
{
    Connection conn;
    conn.session = std::make_unique<cpr::Session>();
    conn.setHeader("Connection", "keep-alive");
    conn.setHeader("Keep-Alive", "timeout=30");
    conn.setHeader("CB-VERSION", "2016-02-18");
    conn.setHeader("User-Agent", "Gtb");
    conn.setHeader("Accept", "application/json");
    conn.setHeader("Content-Type", "application/json");
    conn.session->SetTimeout(cpr::Timeout{30'000});
    conn.session->SetOption(cpr::HttpVersion(cpr::HttpVersionCode::VERSION_1_1));
    //conn.session->SetOption(cpr::Verbose{true});
    return conn;
}

void RestClient::saveConn(Connection conn)
{
    constexpr const unsigned int MAX_USES = 999;
    if (++conn.uses >= MAX_USES)
        return;

    std::lock_guard<std::mutex> lock(mtx);
    conn.timeout = std::chrono::steady_clock::now() + std::chrono::seconds(20);
    sessionPool.push_back(std::move(conn));
}

void RestClient::Connection::setHeader(
    const std::string &key,
    std::string value)
{
    headers[key] = std::move(value);
    session->SetHeader(headers);
}

void RestClient::Connection::clearHeader(
    const std::string &key)
{
    auto iter = headers.find(key);
    if (iter != headers.end())
    {
        headers.erase(key);
        session->SetHeader(headers);
    }
}

void RestClient::Connection::setJwt(
    const std::string &jwt)
{
    if (jwt.empty())
        clearHeader("Authorization");
    else
        setHeader("Authorization", "Bearer " + jwt);
}
