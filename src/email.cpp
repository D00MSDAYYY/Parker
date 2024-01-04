#include "email.hpp"

#include <mailio/message.hpp>
#include <mailio/smtp.hpp>
#include <regex>
#include <string>

Mail_Args
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

bool
sendReport(Mail_Args& mail_args)
{
    mailio::message msg;
    msg.from(mailio::mail_address(
        "",
        mail_args._sender_addr));    // set the correct sender name and address
    msg.add_recipient(mailio::mail_address(
        "",
        mail_args._receiver_addr));  // set the correct recipent name and address
    msg.subject("Отчет по парковке");
    msg.content_transfer_encoding(mailio::mime::content_transfer_encoding_t::QUOTED_PRINTABLE);
    msg.content_type(mailio::message::media_type_t::TEXT, "plain", "utf-8");
    msg.content(
        "Это отчет по парковке, автоматически сгенерированный телеграм-ботом 🅿️arker.\n "
        "Не надо отвечать на это письмо");

    mailio::smtps conn("smtp.gmail.com", 465);
    bool          is_sent{true};
    try
    {
        // modify username/password to use real credentials
        conn.authenticate(mail_args._sender_addr,
                          mail_args._sender_pass,
                          mailio::smtps::auth_method_t::LOGIN);
        conn.submit(msg);
    }
    catch(...)
    {
        is_sent = false;
    }
    return is_sent;
}
