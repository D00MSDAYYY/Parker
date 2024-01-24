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

#include "email.hpp"

#include "string_trimming.hpp"

#include <mailio/message.hpp>
#include <mailio/smtp.hpp>
#include <regex>
#include <string>

std::optional<Mail_Args>
parseMailStr(std::string& str)
{
    auto receiver_addr_index{str.find("📥 Получатель :")};
    auto sender_addr_index{str.find("📤 Отправитель :")};
    auto sender_pass_index{str.find("🔐 Пароль :")};
    auto server_addr_index{str.find("🌐 Сервер :")};
    auto server_port_index{str.find("🔗 Порт :")};

    if(receiver_addr_index != std::string::npos && sender_addr_index != std::string::npos
       && sender_pass_index != std::string::npos && server_addr_index != std::string::npos
       && server_port_index != std::string::npos)
    {
        std::string receiver_addr_str{str.begin() + receiver_addr_index + strlen("📥 Получатель :"),
                                      str.begin() + sender_addr_index};
        std::string sender_addr_str{str.begin() + sender_addr_index + strlen("📤 Отправитель :"),
                                    str.begin() + sender_pass_index};
        std::string sender_pass_str{str.begin() + sender_pass_index + strlen("🔐 Пароль :"),
                                    str.begin() + server_addr_index};
        std::string server_addr_str{str.begin() + server_addr_index + strlen("🌐 Сервер :"),
                                    str.begin() + server_port_index};
        std::string server_port_str{str.begin() + server_port_index + strlen("🔗 Порт :"),
                                    str.end()};
        Mail_Args   res{};  // default initalization with std::nullopt
        if(!receiver_addr_str.empty()) res.receiver_addr = trim(receiver_addr_str);
        if(!sender_addr_str.empty()) res.sender_addr = trim(sender_addr_str);
        if(!sender_pass_str.empty()) res.sender_pass = trim(sender_pass_str);
        if(!server_addr_str.empty()) res.server_addr = trim(server_addr_str);
        if(!server_port_str.empty()) res.server_port = trim(server_port_str);

        return res;
    }
    else
        return {};
}

bool
sendReport(Mail_Args& mail_args, const std::string& file_name)
{
    if(mail_args.receiver_addr && mail_args.sender_addr && mail_args.sender_pass
       && mail_args.server_addr && mail_args.server_port)
    {
        using namespace mailio;
        
        message msg;
        msg.from(mailio::mail_address(
            "",
            *mail_args.sender_addr));                 // set the correct sender name and address
        msg.add_recipient(
            mail_address("",
                         *mail_args.receiver_addr));  // set the correct recipent name and address
        msg.subject("Отчет по парковке");
        msg.content_transfer_encoding(mime::content_transfer_encoding_t::QUOTED_PRINTABLE);
        msg.content_type(message::media_type_t::TEXT, "plain", "utf-8");
        msg.content("Это отчет по парковке, автоматически сгенерированный телеграм-ботом 🅿️arker.\n "
                    "Не надо отвечать на это письмо");

        std::ifstream report(file_name, std::ios::binary);
        std::list<std::tuple<std::istream&, std::string, message::content_type_t>> atts;
        atts.push_back(std::tuple{std::ref(report),
                                  std::string("report.xlsx"),
                                  message::content_type_t(message::media_type_t::TEXT, "xlsx")});
        msg.attach(atts);

        bool is_sent{true};
        try
        {
            mailio::smtps conn(*mail_args.server_addr, std::stoi(*mail_args.server_port));
            conn.authenticate(*mail_args.sender_addr,
                              *mail_args.sender_pass,
                              mailio::smtps::auth_method_t::LOGIN);
            conn.submit(msg);
        }
        catch(...)
        {
            is_sent = false;
        }
        return is_sent;
    }
    else
        return false;
}
