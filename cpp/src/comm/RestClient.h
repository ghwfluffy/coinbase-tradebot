#pragma once

#include <cpr/cpr.h>
#include <nlohmann/json.hpp>

#include <memory>

namespace gtb
{

struct HttpResponse
{
    operator bool() const;
    std::string text() const;

    nlohmann::json data;
    unsigned int code = 0;
};

class RestClient
{
    public:
        RestClient(
            std::string baseUrl,
            bool verbose = false);
        RestClient(RestClient &&) = delete;
        RestClient(const RestClient &) = delete;
        RestClient &operator=(RestClient &&) = delete;
        RestClient &operator=(const RestClient &) = delete;
        ~RestClient() = default;

        HttpResponse post(
            const std::string &path,
            const std::string &jwt,
            const nlohmann::json &data);

        HttpResponse get(
            const std::string &path,
            const std::string &jwt,
            const nlohmann::json &data = nlohmann::json(),
            bool query = true);

        HttpResponse del(
            const std::string &path,
            const std::string &jwt);

    private:
        struct Connection
        {
            unsigned int uses = 0;
            std::chrono::steady_clock::time_point timeout;

            std::unique_ptr<cpr::Session> session;

            // HTTP headers
            void setHeader(
                const std::string &key,
                std::string value);

            void clearHeader(
                const std::string &key);

            void setJwt(
                const std::string &jwt);

            cpr::Header headers;
        };

        Connection getConn();
        Connection newConn() const;
        void saveConn(Connection conn);

        std::mutex mtx;
        std::string baseUrl;
        bool verbose;
        std::list<Connection> sessionPool;
};

}
