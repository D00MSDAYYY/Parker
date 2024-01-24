//      888                                               888
//      888                                               888
//      888                                               888
//  .d88888  .d88b.   .d88b.  88888b.d88b.  .d8888b   .d88888  8888b.  888  888
// d88" 888 d88""88b d88""88b 888 "888 "88b 88K      d88" 888     "88b 888  888
// 888  888 888  888 888  888 888  888  888 "Y8888b. 888  888 .d888888 888  888
// Y88b 888 Y88..88P Y88..88P 888  888  888      X88 Y88b 888 888  888 Y88b 888
//  "Y88888  "Y88P"   "Y88P"  888  888  888  88888P'  "Y88888 "Y888888  "Y88888
//                                                                          888
//                                                                     Y8b d88P
//                                                                      "Y88P"

#pragma once
#include <optional>
#include <string>

struct Mail_Args
{
    std::optional<std::string> receiver_addr{std::nullopt};
    std::optional<std::string> sender_addr{std::nullopt};
    std::optional<std::string> sender_pass{std::nullopt};

    std::optional<std::string> server_addr{std::nullopt};
    std::optional<std::string> server_port{std::nullopt};
};

std::optional<Mail_Args>
parseMailStr(std::string& str);

bool
sendReport(Mail_Args& mail_args, const std::string& file_name);
