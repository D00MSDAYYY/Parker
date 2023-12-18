#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <exception>
#include <string>
#include <tgbot/tgbot.h>

using namespace std;
using namespace TgBot;

enum class FSM               // FSM states
{
    USER_NOT_AUTORIZED = 0,  // starting
    USER_AUTORIZED,          // USER_AUTORIZED
    USER_INFO_HELP,          // information and help
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

                InlineKeyboardButton::Ptr rqst_prkng(new InlineKeyboardButton);
                rqst_prkng->text         = "Заявка на парковку";
                rqst_prkng->callbackData = "rqst_prkng";

                keyboard->inlineKeyboard.push_back({info_help});
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
            {
                text.assign("Вы не найдены в базе данных, попросите администратора "
                            "создать вам аккаунт");
            }
            break;
        case FSM::USER_AUTORIZED :
            {
                text.assign("Авторизация прошла успешно");
            }
            break;
        case FSM::USER_INFO_HELP :
            {
                text.assign("Это бот-помощник для организации очередности парковки!");
            }
            break;
        case FSM::USER_RQST_PRKNG :
            {
                text.assign("Выберите, что вы хотите сделать на данный момент:");
            }
            break;
        case FSM::USER_CNCL_RQST :
            {
                text.assign("Ваше место Е3, время - с 14:30 до 16:00. Вы действительно "
                            "хотите отменить бронь?");
            }
            break;
        case FSM::USER_SBMT_RQST :
            {
                text.assign("На данный момент свободно 3 места, время - с 12:40 до "
                            "16:35. Выберите время:");
            }
            break;
        case FSM::ADMIN :
            {
                text.assign("Настройка бота: ");
            }
            break;
        case FSM::ADMIN_ADD_USER :
            {
                text.assign("Перешлите сюда любое сообщение от нового пользователя: ");
            }
            break;
        case FSM::ADMIN_RM_USER :
            {
                text.assign("Перешлите сюда любое сообщение от удаляемого пользователя: ");
            }
            break;
        default : break;
    }
    return std::move(text);
}

int
main(int argc, char** argv)
{
    Bot         bot{argv[1]};
    std::string admin_id{argv[2]};

    // clean up ###########################################################################
    // ####################################################################################
    bot.getApi().setChatMenuButton(0, MenuButton::Ptr{new MenuButtonCommands});
    bot.getApi().deleteMyCommands();

    // set commands #######################################################################
    // ####################################################################################
    BotCommand::Ptr start_command(new BotCommand{"start", "Start bot"});
    BotCommand::Ptr return_command(new BotCommand{"return", "Return to prev step"});
    BotCommand::Ptr admin_command(new BotCommand{"admin", "Only for admin"});
    bot.getApi().setMyCommands({start_command, return_command, admin_command},
                               BotCommandScope::Ptr{new BotCommandScopeDefault{}});

    // ####################################################################################
    // ####################################################################################
    std::unordered_map<decltype(User::id), FSM> states;  //! check for concurrency

    bot.getEvents().onCommand(
        start_command->command,
        [&bot, &states](Message::Ptr message)
        {
            if(states.contains(message->from->id))
            {
                states.at(message->from->id) = FSM::USER_AUTORIZED;
                bot.getApi().sendMessage(message->chat->id,
                                         text(states.at(message->from->id)),
                                         false,
                                         0,
                                         keyboard(states.at(message->from->id)));
            }
            else
            {
                bot.getApi().sendMessage(message->chat->id,
                                         text(FSM::USER_NOT_AUTORIZED),
                                         false,
                                         0,
                                         keyboard(FSM::USER_NOT_AUTORIZED));
            }
        });
    bot.getEvents().onCommand(
        return_command->command,
        [&bot, &states](Message::Ptr message)
        {
            if(states.contains(message->from->id))
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
                bot.getApi().sendMessage(message->chat->id,
                                         text(states.at(message->from->id)),
                                         false,
                                         0,
                                         keyboard(states.at(message->from->id)));
            }
        });
    bot.getEvents().onCommand(admin_command->command,
                              [&bot, &states, &admin_id](Message::Ptr message)
                              {
                                  if(std::to_string(message->from->id) == admin_id)
                                  {
                                      states[message->from->id] = FSM::ADMIN;
                                      bot.getApi().sendMessage(
                                          message->chat->id,
                                          text(states.at(message->from->id)),
                                          false,
                                          0,
                                          keyboard(states.at(message->from->id)));
                                  }
                              });

    // ####################################################################################
    // ####################################################################################
    bot.getEvents().onCallbackQuery(
        [&bot, &states](CallbackQuery::Ptr query)
        {
            if(states.contains(query->from->id))
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
                    default : break;
                }
            }
        });

    // ####################################################################################
    // ####################################################################################
    bot.getEvents().onAnyMessage(
        [&bot, &states](Message::Ptr message)
        {
            if(states.contains(message->from->id))
            {
                switch(states.at(message->from->id))
                {
                    case FSM::ADMIN_ADD_USER :
                        {
                            if(message->forwardFrom)
                            {
                                states.insert(
                                    {message->forwardFrom->id, FSM::USER_AUTORIZED});
                                // update database info
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
                            if(message->forwardFrom && states.contains(message->forwardFrom->id))
                            {
                                states.erase(message->forwardFrom->id);
                                //! update database info()
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
                    default : break;
                }
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
    catch(exception& e)
    {
        printf("error: %s\n", e.what());
    }

    return 0;
}
