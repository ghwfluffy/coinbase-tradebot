#include <gtb/CoinbaseRestClient.h>

#include <stdio.h>

using namespace gtb;

int main(int argc, const char *argv[])
{
    if (argc != 2)
    {
        fprintf(stderr, "Retrieve order information.\n");
        fprintf(stderr, "Usage: %s <Order UUID>\n", argv[0]);
        return 1;
    }

    std::string uuid(argv[1]);

    CoinbaseRestClient client(true);
    CoinbaseOrder order = client.getOrder(uuid);
    return order ? 0 : 1;
}
