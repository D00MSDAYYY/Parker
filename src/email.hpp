
#include <string>

struct Mail_Args
{
    std::string _receiver_addr{};
    std::string _sender_addr{};
    std::string _sender_pass{};
};

Mail_Args
parseMailStr(std::string& str);

bool
sendReport(Mail_Args& mail_args);
