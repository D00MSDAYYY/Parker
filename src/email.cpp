#include "email.hpp"

#include <regex>
#include <string>
#include <iostream>

Mail
parseMailStr(std::string& str)
{
    auto receiver_addr_index{str.find("📥 Получатель :")};
    auto sender_addr_index{str.find("📤 Отправитель :")};
    auto sender_pass_index{str.find("📤🔐 Пароль :")};

    if(receiver_addr_index != std::string::npos && sender_addr_index != std::string::npos
       && sender_pass_index != std::string::npos)
    {
        std::smatch receiver_addr_res{};
        std::smatch sender_addr_res{};
        std::smatch sender_pass_res{};

        std::regex  email_regex{"(\\S+@\\S+\\.\\S+)"};
        std::regex  pass_regex{"(\\w.*)"};
        std::regex_search({str.begin() + receiver_addr_index + strlen("📥 Получатель :")},
                          {str.begin() + sender_addr_index},
                          receiver_addr_res,
                          email_regex);

        std::regex_search({str.begin() + sender_addr_index + strlen("📤 Отправитель :")},
                          {str.begin() + sender_pass_index},
                          sender_addr_res,
                          email_regex);
        std::regex_search({str.begin() + sender_pass_index + strlen("📤🔐 Пароль :")},
                          {str.end()},
                          sender_pass_res,
                          pass_regex);
        std::cerr << receiver_addr_res.str() << std::endl;
        std::cerr << sender_addr_res.str() << std::endl;
        std::cerr << sender_pass_res.str() << std::endl;

        return {receiver_addr_res.str(), sender_addr_res.str(), sender_pass_res.str()};
    }
    else
        return {};
}
