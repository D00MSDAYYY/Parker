#include "db_interactions.hpp"

#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <exception>
#include <locale>
#include <regex>
#include <string>
#include <tgbot/tgbot.h>

using namespace TgBot;

enum class FSM               // FSM states
{
    USER_NOT_AUTORIZED = 0,  // starting
    USER_AUTORIZED,          // USER_AUTORIZED
    USER_INFO_HELP,          // information and help
    USER_CHNG_BIO,           // change info about user
    USER_RQST_PRKNG,         // requesting for parking
    USER_CNCL_RQST,          // cancelling request for parking
    USER_SBMT_RQST,          // submiting request for parking

    ADMIN,                   // only for admins to show special commands
    ADMIN_ADD_USER,          // admin adds user
    ADMIN_RM_USER            // admin removes user
};

GenericReply::Ptr
keyboard(FSM state)
{
    InlineKeyboardMarkup::Ptr keyboard(new InlineKeyboardMarkup);
    switch(state)
    {
        case FSM::USER_AUTORIZED :
            {
                InlineKeyboardButton::Ptr info_help(new InlineKeyboardButton);
                info_help->text         = "Информация и помощь";
                info_help->callbackData = "info_help";

                InlineKeyboardButton::Ptr chng_bio(new InlineKeyboardButton);
                chng_bio->text         = "Изменить личную информацию";
                chng_bio->callbackData = "chng_bio";

                InlineKeyboardButton::Ptr rqst_prkng(new InlineKeyboardButton);
                rqst_prkng->text         = "Заявка на парковку";
                rqst_prkng->callbackData = "rqst_prkng";

                keyboard->inlineKeyboard.push_back({info_help});
                keyboard->inlineKeyboard.push_back({chng_bio});
                keyboard->inlineKeyboard.push_back({rqst_prkng});
            }
            break;
        case FSM::USER_RQST_PRKNG :
            {
                InlineKeyboardButton::Ptr cncl_rqst(new InlineKeyboardButton);
                cncl_rqst->text         = "Отменить заявку";
                cncl_rqst->callbackData = "cncl_rqst";

                InlineKeyboardButton::Ptr submit_rqst(new InlineKeyboardButton);
                submit_rqst->text         = "Подать заявку";
                submit_rqst->callbackData = "submit_rqst";

                keyboard->inlineKeyboard.push_back({cncl_rqst});
                keyboard->inlineKeyboard.push_back({submit_rqst});
            }
            break;
        case FSM::USER_CNCL_RQST :
            {
                InlineKeyboardButton::Ptr yes(new InlineKeyboardButton);
                yes->text         = "Да";
                yes->callbackData = "cncl_rqst";

                InlineKeyboardButton::Ptr no(new InlineKeyboardButton);
                no->text         = "Нет";
                no->callbackData = "submit_rqst";

                keyboard->inlineKeyboard.push_back({yes});
                keyboard->inlineKeyboard.push_back({no});
            }
            break;
        case FSM::ADMIN :
            {
                InlineKeyboardButton::Ptr add(new InlineKeyboardButton);
                add->text         = "Добавить пользователя";
                add->callbackData = "add_user";

                InlineKeyboardButton::Ptr rm(new InlineKeyboardButton);
                rm->text         = "Удалить пользователя";
                rm->callbackData = "rm_user";

                keyboard->inlineKeyboard.push_back({add});
                keyboard->inlineKeyboard.push_back({rm});
            }
            break;
        default : break;
    }
    return keyboard;
}

std::string
text(FSM state = FSM::USER_NOT_AUTORIZED)
{
    std::string text;
    switch(state)
    {
        case FSM::USER_NOT_AUTORIZED :
            text.assign("Вы не найдены в базе данных, попросите администратора "
                        "создать вам аккаунт");
            break;
        case FSM::USER_AUTORIZED : text.assign("Авторизация прошла успешно"); break;
        case FSM::USER_INFO_HELP :
            text.assign("Это бот-помощник для организации очередности парковки!");
            break;
        case FSM::USER_CHNG_BIO :
            text.assign("Фамилия : (Пример)\n"
                        "Имя : (Примерович)\n"
                        "Отчество : (Примеров)\n"
                        "Марка машины : (Тойота Пример 200)\n"
                        "Номерной знак : (а000аа000)\n");
            break;
        case FSM::USER_RQST_PRKNG :
            text.assign("Выберите, что вы хотите сделать на данный момент:");
            break;
        case FSM::USER_CNCL_RQST :
            text.assign("Ваше место Е3, время - с 14:30 до 16:00. Вы действительно "
                        "хотите отменить бронь?");
            break;
        case FSM::USER_SBMT_RQST :
            text.assign("На данный момент свободно 3 места, время - с 12:40 до "
                        "16:35. Выберите время:");
            break;
        case FSM::ADMIN : text.assign("Настройка бота: "); break;
        case FSM::ADMIN_ADD_USER :
            text.assign("Перешлите сюда любое сообщение от нового пользователя: ");
            break;
        case FSM::ADMIN_RM_USER :
            text.assign("Перешлите сюда любое сообщение от удаляемого пользователя: ");
            break;
        default : break;
    }
    return std::move(text);
}

Query_Args
parseBioStr(std::string& str)
{
    auto firstname_index{str.find("Фамилия : (")};
    auto middlename_index{str.find("Имя : (")};
    auto lastname_index{str.find("Отчество : (")};
    auto car_model_index{str.find("Марка машины : (")};
    auto license_index{str.find("Номерной знак : (")};

    if(firstname_index != std::string::npos && middlename_index != std::string::npos
       && lastname_index != std::string::npos && car_model_index != std::string::npos
       && license_index != std::string::npos)
    {
        std::locale prev_locale{std::locale::global(std::locale("ru_RU.UTF-8"))};
        std::smatch firstname_res{};
        std::smatch middlename_res{};
        std::smatch lastname_res{};
        std::smatch car_model_res{};
        std::smatch license_res{};

        std::regex  name_regex{"[А-ЯЁа-яё]{1,30}", std::regex::collate};
        std::regex  car_model_regex{"[А-ЯЁа-яё0-9 ]{1,30}", std::regex::collate};
        std::regex  license_regex{"([А-ЯЁа-яё]{1}[0-9]{3}(?!000)[А-Я]{2}[0-9]{2})"};
        //! dont use flags -- not working

        bool        firstname_flag{
            std::regex_search({str.begin() + firstname_index + strlen("Фамилия : (")},
                                     {str.begin() + middlename_index},
                              firstname_res,
                              name_regex)};
        bool middlename_flag{
            std::regex_search({str.begin() + middlename_index + strlen("Имя : (")},
                              {str.begin() + lastname_index},
                              middlename_res,
                              name_regex)};
        bool lastname_flag{
            std::regex_search({str.begin() + lastname_index + strlen("Отчество : (")},
                              {str.begin() + car_model_index},
                              lastname_res,
                              name_regex)};
        bool car_model_flag{
            std::regex_search({str.begin() + car_model_index + strlen("Марка машины : (")},
                              {str.begin() + license_index},
                              car_model_res,
                              car_model_regex)};
        bool license_flag{
            std::regex_search({str.begin() + license_index + strlen("Номерной знак : (")},
                              {str.end()},
                              license_res,
                              license_regex)};
        std::locale::global(std::locale(prev_locale));

        if(firstname_flag && middlename_flag && lastname_flag && car_model_flag
           && license_flag)
            return {{},
                    firstname_res.str(),
                    middlename_res.str(),
                    lastname_res.str(),
                    car_model_res.str(),
                    license_res.str()};
    }
    return {};
}

class States
{
private:
    Data_Base&                                         _db;
    std::unordered_map<decltype(TgBot::User::id), FSM> _states;
    FSM                                                _dummy{FSM::USER_NOT_AUTORIZED};

public:
    States(Data_Base& db)
        : _db{db}
    {
    }

    FSM&
    at(decltype(TgBot::User::id) id)
    {
        if(_db.employees().exists({.tg_id = std::to_string(id)}))
        {
            if(_states.contains(id))
                return _states.at(id);
            else
            {
                _states.insert({id, FSM::USER_AUTORIZED});
                return _states.at(id);
            }
        }
        else
        {
            if(_states.contains(id)) _states.erase(id);

            _dummy = FSM::USER_NOT_AUTORIZED;
            return _dummy;
        }
    }
};

int
main(int argc, char** argv)
{
    Bot         bot{argv[1]};
    std::string admin_id{argv[2]};
    Data_Base   db{};
    States      states{db};

    // clean up ###########################################################################
    // ####################################################################################
    bot.getApi().setChatMenuButton(0, MenuButton::Ptr{new MenuButtonCommands});
    bot.getApi().deleteMyCommands();

    // set commands #######################################################################
    // ####################################################################################
    BotCommand::Ptr start_command(new BotCommand{"start", "Start bot"});
    BotCommand::Ptr return_command(new BotCommand{"return", "Return to previous step"});
    BotCommand::Ptr admin_command(new BotCommand{"admin", "Only for admin"});
    bot.getApi().setMyCommands({start_command, return_command, admin_command},
                               BotCommandScope::Ptr{new BotCommandScopeDefault{}});

    // ####################################################################################
    // ####################################################################################

    bot.getEvents().onCommand(start_command->command,
                              [&bot, &states](Message::Ptr message)
                              {
                                  states.at(message->from->id) = FSM::USER_AUTORIZED;
                                  bot.getApi().sendMessage(
                                      message->chat->id,
                                      text(states.at(message->from->id)),
                                      false,
                                      0,
                                      keyboard(states.at(message->from->id)));
                              });
    bot.getEvents().onCommand(return_command->command,
                              [&bot, &states](Message::Ptr message)
                              {
                                  switch(states.at(message->from->id))
                                  {
                                      case FSM::USER_INFO_HELP :
                                          states.at(message->from->id) = FSM::USER_AUTORIZED;
                                          break;
                                      case FSM::USER_RQST_PRKNG :
                                          states.at(message->from->id) = FSM::USER_AUTORIZED;
                                          break;
                                      case FSM::USER_CNCL_RQST :
                                          states.at(message->from->id) = FSM::USER_RQST_PRKNG;
                                          break;
                                      case FSM::USER_SBMT_RQST :
                                          states.at(message->from->id) = FSM::USER_RQST_PRKNG;
                                          break;
                                      default : break;
                                  }
                                  bot.getApi().sendMessage(
                                      message->chat->id,
                                      text(states.at(message->from->id)),
                                      false,
                                      0,
                                      keyboard(states.at(message->from->id)));
                              });
    bot.getEvents().onCommand(
        admin_command->command,
        [&bot, &states, &admin_id, &db](Message::Ptr message)
        {
            if(std::to_string(message->from->id) == admin_id)
            {
                db.employees().add({.tg_id = std::to_string(message->from->id)});
                states.at(message->from->id) = FSM::ADMIN;
            }
            bot.getApi().sendMessage(message->chat->id,
                                     text(states.at(message->from->id)),
                                     false,
                                     0,
                                     keyboard(states.at(message->from->id)));
        });

    // ####################################################################################
    // ####################################################################################
    bot.getEvents().onCallbackQuery(
        [&bot, &states, &db](CallbackQuery::Ptr query)
        {
            switch(states.at(query->from->id))
            {
                case FSM::USER_AUTORIZED :
                    {
                        if(StringTools::startsWith(query->data, "info_help"))
                        {
                            states.at(query->from->id) = FSM::USER_INFO_HELP;
                            bot.getApi().sendMessage(query->message->chat->id,
                                                     text(states.at(query->from->id)),
                                                     false,
                                                     0,
                                                     keyboard(states.at(query->from->id)));
                        }
                        if(StringTools::startsWith(query->data, "rqst_prkng"))
                        {
                            states.at(query->from->id) = FSM::USER_RQST_PRKNG;
                            bot.getApi().sendMessage(query->message->chat->id,
                                                     text(states.at(query->from->id)),
                                                     false,
                                                     0,
                                                     keyboard(states.at(query->from->id)));
                        }
                        if(StringTools::startsWith(query->data, "chng_bio"))
                        {
                            states.at(query->from->id) = FSM::USER_CHNG_BIO;
                            bot.getApi().sendMessage(
                                query->message->chat->id,
                                "Скопируйте следующее сообщение, впишите "
                                "новые данные между скобок и отправьте боту: ");
                            bot.getApi().sendMessage(query->message->chat->id,
                                                     text(states.at(query->from->id)),
                                                     false,
                                                     0,
                                                     keyboard(states.at(query->from->id)));
                        }
                    }
                    break;
                case FSM::USER_RQST_PRKNG :
                    {
                        if(StringTools::startsWith(query->data, "cncl_rqst"))
                        {
                            states.at(query->from->id) = FSM::USER_CNCL_RQST;
                            bot.getApi().sendMessage(query->message->chat->id,
                                                     text(states.at(query->from->id)),
                                                     false,
                                                     0,
                                                     keyboard(states.at(query->from->id)));
                        }
                        if(StringTools::startsWith(query->data, "submit_rqst"))
                        {
                            states.at(query->from->id) = FSM::USER_SBMT_RQST;
                            bot.getApi().sendMessage(query->message->chat->id,
                                                     text(states.at(query->from->id)),
                                                     false,
                                                     0,
                                                     keyboard(states.at(query->from->id)));
                        }
                    }
                    break;
                case FSM::ADMIN :
                    {
                        if(StringTools::startsWith(query->data, "add_user"))
                        {
                            states.at(query->from->id) = FSM::ADMIN_ADD_USER;
                            bot.getApi().sendMessage(query->message->chat->id,
                                                     text(states.at(query->from->id)),
                                                     false,
                                                     0,
                                                     keyboard(states.at(query->from->id)));
                        }
                        if(StringTools::startsWith(query->data, "rm_user"))
                        {
                            states.at(query->from->id) = FSM::ADMIN_RM_USER;
                            bot.getApi().sendMessage(query->message->chat->id,
                                                     text(states.at(query->from->id)),
                                                     false,
                                                     0,
                                                     keyboard(states.at(query->from->id)));
                        }
                    }
                    break;
                case FSM::ADMIN_RM_USER :
                    {
                        // db.employees().remove({"123456",
                        //                        "Зубенко",
                        //                        "Михаил",
                        //                        "Петрович",
                        //                        "Тойота Королла",
                        //                        "АМ777Р32"});
                    }
                    break;
                default : break;
            }
        });

    // ####################################################################################
    // ####################################################################################
    bot.getEvents().onAnyMessage(
        [&bot, &states, &db](Message::Ptr message)
        {
            switch(states.at(message->from->id))
            {
                case FSM::ADMIN_ADD_USER :
                    {
                        if(message->forwardFrom)
                        {
                            db.employees().add(
                                {.tg_id = std::to_string(message->forwardFrom->id)});
                            bot.getApi().sendMessage(message->chat->id,
                                                     "пользователь добавлен",
                                                     false,
                                                     0);
                        }
                        else
                        {
                            bot.getApi().sendMessage(message->chat->id,
                                                     "не получилось добавить пользователя",
                                                     false,
                                                     0);
                        }
                    }
                    break;
                case FSM::ADMIN_RM_USER :
                    {
                        if(message->forwardFrom
                           && db.employees().remove(
                               {.tg_id = std::to_string(message->forwardFrom->id)}))
                        {
                            bot.getApi().sendMessage(message->chat->id,
                                                     "пользователь удален",
                                                     false,
                                                     0);
                        }
                        else
                        {
                            bot.getApi().sendMessage(message->chat->id,
                                                     "не получилось удалить пользователя",
                                                     false,
                                                     0);
                        }
                    }
                    break;
                case FSM::USER_CHNG_BIO :
                    {
                        auto response{parseBioStr(message->text)};
                        if(!response.firstname.empty())
                        {
                            response.tg_id = std::to_string(message->from->id);
                            if(db.employees().add(response))
                                bot.getApi().sendMessage(message->chat->id,
                                                         "информация о пользователе обновлена",
                                                         false,
                                                         0);
                        }
                    }
                    break;
                default : break;
            }
        });

    // ####################################################################################
    // ####################################################################################
    signal(SIGINT,
           [](int s)
           {
               printf("SIGINT got\n");
               exit(0);
           });
    try
    {
        printf("Bot username: %s\n", bot.getApi().getMe()->username.c_str());
        bot.getApi().deleteWebhook();

        TgLongPoll longPoll(bot);
        while(true)
        {
            printf("Long poll started\n");
            longPoll.start();
        }
    }
    catch(std::exception& e)
    {
        printf("error: %s\n", e.what());
    }

    return 0;
}
