#include <string>

struct Mail
{
    std::string _receiver_addr{};
    std::string _sender_addr{};
    std::string _sender_pass{};
};

Mail
parseMailStr(std::string& str);
