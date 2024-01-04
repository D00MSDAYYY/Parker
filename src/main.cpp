#include "FSM_states.hpp"
#include "data_base.hpp"
#include "email.hpp"

#include <atomic>
#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <exception>
#include <locale>
#include <optional>
#include <string>
#include <tgbot/tgbot.h>
#include <utility>

int
main(int argc, char** argv)
{
    TgBot::Bot                bot{argv[1]};
    decltype(TgBot::User::id) admin_id{std::stoll(argv[2])};
    Mail_Args                 mail{};
    Data_Base                 db{};

    auto                      text{
        [&mail, &db](FSM                                      state = FSM::USER_NOT_AUTHORIZED,
                     std::optional<decltype(TgBot::User::id)> id = 0) -> std::string
        {
            std::string text;
            switch(state)
            {
                case FSM::USER_NOT_AUTHORIZED :
                    text.assign("🔴 Вас нет в базе данных, свяжитесь с администратором");
                    break;
                case FSM::USER_AUTHORIZED :
                    text.assign("🟢 Авторизация прошла успешно");
                    break;
                case FSM::USER_INFO_HELP :
                    text.assign("🅿️arker - телеграм-бот-помощник для организации очередности "
                                                     "парковки!");
                    break;
                case FSM::USER_CHNG_BIO :
                    {
                        // TODO! use assert(id != nullopt)
                        auto qargs{
                            db.employees().select_where({.tg_id = std::to_string(*id)})};
                        auto params = (qargs ? (*qargs)[0] : Query_Args{});
                        text.assign("Нажатием на поля ввода скопируйте текст сообщения, "
                                                         "впишите новые данные и отправьте боту:\n\n"
                                                         "❗ Используйте только кириллицу и цифры\n\n"
                                                         "`➡️ Фамилия : "
                                    + params.firstname
                                    + "\n"
                                                           "➡️ Имя : "
                                    + params.middlename
                                    + "\n"
                                                           "➡️ Отчество : "
                                    + params.lastname
                                    + "\n"
                                                           "➡️ Марка машины : "
                                    + params.car_model
                                    + "\n"
                                                           "➡️ Номерной знак : "
                                    + params.license + "\n`");
                    }
                    break;
                case FSM::USER_RQST_PRKNG :
                    text.assign("Выберите, что вы хотите сделать в данный момент:");
                    break;
                case FSM::USER_CNCL_RQST :
                    text.assign("Вы действительно хотите отменить бронь?");
                    break;
                case FSM::USER_SBMT_RQST :
                    text.assign("На данный момент свободно "
                                + std::to_string(db.datetime().curPlaces()) + " места из "
                                + std::to_string(*(db.datetime().maxPlaces())));
                    break;
                case FSM::ADMIN : text.assign("Настройка бота: "); break;
                case FSM::ADMIN_ADD_RM_USER :
                    text.assign("Перешлите сюда любое сообщение пользователя: ");
                    break;
                case FSM::ADMIN_CHNG_MAIL :
                    text.assign("`📥 Получатель : " + mail._receiver_addr
                                + "\n"
                                                       "📤 Отправитель : "
                                + mail._sender_addr
                                + "\n"
                                                       "📤🔐 Пароль : "
                                + mail._sender_pass + "\n`");
                    break;
                case FSM::ADMIN_CHNG_NUM :
                    text.assign("Максимальное колличество мест на парковке: "
                                + std::to_string(*(db.datetime().maxPlaces())));
                case FSM::ADMIN_CHNG_WKDY :
                    text.assign("Выберите дни, в которые бот будет организовывать парковку: ");
                    break;
                default : break;
            }
            return std::move(text);
        }};
    auto keyboard{
        [](FSM state) -> TgBot::GenericReply::Ptr
        {
            TgBot::InlineKeyboardMarkup::Ptr keyboard(new TgBot::InlineKeyboardMarkup);
            switch(state)
            {
                case FSM::USER_AUTHORIZED :
                    {
                        TgBot::InlineKeyboardButton::Ptr info_help(
                            new TgBot::InlineKeyboardButton);
                        info_help->text = "ℹ️ Информация и помощь";
                        info_help->callbackData = "info_help";

                        TgBot::InlineKeyboardButton::Ptr chng_bio(
                            new TgBot::InlineKeyboardButton);
                        chng_bio->text = "👤 Изменить данные пользователя";
                        chng_bio->callbackData = "chng_bio";

                        TgBot::InlineKeyboardButton::Ptr rqst_prkng(
                            new TgBot::InlineKeyboardButton);
                        rqst_prkng->text         = "🅿️ Парковка";
                        rqst_prkng->callbackData = "rqst_prkng";

                        keyboard->inlineKeyboard.push_back({info_help});
                        keyboard->inlineKeyboard.push_back({chng_bio});
                        keyboard->inlineKeyboard.push_back({rqst_prkng});
                    }
                    break;
                case FSM::USER_RQST_PRKNG :
                    {
                        TgBot::InlineKeyboardButton::Ptr prkng_vldtn(
                            new TgBot::InlineKeyboardButton);
                        prkng_vldtn->text = "☑️ Валидировать парковку";
                        prkng_vldtn->callbackData = "prkng_vldtn";

                        TgBot::InlineKeyboardButton::Ptr cncl_rqst(
                            new TgBot::InlineKeyboardButton);
                        cncl_rqst->text         = "⬇️ Отменить заявку";
                        cncl_rqst->callbackData = "cncl_rqst";

                        TgBot::InlineKeyboardButton::Ptr submit_rqst(
                            new TgBot::InlineKeyboardButton);
                        submit_rqst->text         = "⬆️ Подать заявку";
                        submit_rqst->callbackData = "submit_rqst";

                        keyboard->inlineKeyboard.push_back({prkng_vldtn});
                        keyboard->inlineKeyboard.push_back({cncl_rqst});
                        keyboard->inlineKeyboard.push_back({submit_rqst});
                    }
                    break;
                case FSM::USER_CNCL_RQST :
                    {
                        TgBot::InlineKeyboardButton::Ptr yes(new TgBot::InlineKeyboardButton);
                        yes->text         = "Да";
                        yes->callbackData = "cncl_rqst";

                        TgBot::InlineKeyboardButton::Ptr no(new TgBot::InlineKeyboardButton);
                        no->text         = "Нет";
                        no->callbackData = "submit_rqst";

                        keyboard->inlineKeyboard.push_back({yes});
                        keyboard->inlineKeyboard.push_back({no});
                    }
                    break;
                case FSM::ADMIN :
                    {
                        TgBot::InlineKeyboardButton::Ptr add_rm(
                            new TgBot::InlineKeyboardButton);
                        add_rm->text = "👤 Добавить/удалить пользователя";
                        add_rm->callbackData = "add_rm_user";

                        TgBot::InlineKeyboardButton::Ptr num(new TgBot::InlineKeyboardButton);
                        num->text = "🔢 Изменить колличество мест";
                        num->callbackData = "chng_num";

                        TgBot::InlineKeyboardButton::Ptr wdy(new TgBot::InlineKeyboardButton);
                        wdy->text         = "📅 Изменить дни недели";
                        wdy->callbackData = "chng_wkdy";

                        TgBot::InlineKeyboardButton::Ptr mailbox(
                            new TgBot::InlineKeyboardButton);
                        mailbox->text         = "📬 Изменить почту";
                        mailbox->callbackData = "chng_mailbox";

                        TgBot::InlineKeyboardButton::Ptr snd_rprt(
                            new TgBot::InlineKeyboardButton);
                        snd_rprt->text         = "📈 Отправить доклад";
                        snd_rprt->callbackData = "snd_rprt";

                        keyboard->inlineKeyboard.push_back({add_rm});
                        keyboard->inlineKeyboard.push_back({num});
                        keyboard->inlineKeyboard.push_back({wdy});
                        keyboard->inlineKeyboard.push_back({mailbox});
                        keyboard->inlineKeyboard.push_back({snd_rprt});
                    }
                    break;
                case FSM::ADMIN_CHNG_WKDY :
                    {
                        TgBot::InlineKeyboardButton::Ptr mon(new TgBot::InlineKeyboardButton);
                        mon->text         = "Понедельник";
                        mon->callbackData = "monday";
                        TgBot::InlineKeyboardButton::Ptr tue(new TgBot::InlineKeyboardButton);
                        tue->text         = "Вторник";
                        tue->callbackData = "tuesday";
                        TgBot::InlineKeyboardButton::Ptr wed(new TgBot::InlineKeyboardButton);
                        wed->text         = "Среда";
                        wed->callbackData = "wednesday";
                        TgBot::InlineKeyboardButton::Ptr thu(new TgBot::InlineKeyboardButton);
                        thu->text         = "Четверг";
                        thu->callbackData = "thursday";
                        TgBot::InlineKeyboardButton::Ptr fri(new TgBot::InlineKeyboardButton);
                        fri->text         = "Пятница";
                        fri->callbackData = "friday";
                        TgBot::InlineKeyboardButton::Ptr sat(new TgBot::InlineKeyboardButton);
                        sat->text         = "Суббота";
                        sat->callbackData = "saturday";
                        TgBot::InlineKeyboardButton::Ptr sun(new TgBot::InlineKeyboardButton);
                        sun->text         = "Воскресенье";
                        sun->callbackData = "sunday";
                        keyboard->inlineKeyboard.push_back({mon});
                        keyboard->inlineKeyboard.push_back({tue});
                        keyboard->inlineKeyboard.push_back({wed});
                        keyboard->inlineKeyboard.push_back({thu});
                        keyboard->inlineKeyboard.push_back({fri});
                        keyboard->inlineKeyboard.push_back({sat});
                        keyboard->inlineKeyboard.push_back({sun});
                    }
                    break;
                default : break;
            }
            if(state != FSM::USER_AUTHORIZED)
            {
                TgBot::InlineKeyboardButton::Ptr rtrn(new TgBot::InlineKeyboardButton);
                rtrn->text         = "↩️ Назад";
                rtrn->callbackData = "rtrn";
                keyboard->inlineKeyboard.push_back({rtrn});
            }
            return keyboard;
        }};
    // clean up ###########################################################################
    // ####################################################################################
    bot.getApi().setChatMenuButton(0, TgBot::MenuButton::Ptr{new TgBot::MenuButtonCommands});
    bot.getApi().deleteMyCommands();

    // set commands #######################################################################
    // ####################################################################################
    TgBot::BotCommand::Ptr start_command(new TgBot::BotCommand{"start", "Start bot"});
    TgBot::BotCommand::Ptr admin_command(new TgBot::BotCommand{"admin", "Only for admin"});
    bot.getApi().setMyCommands(
        {start_command, admin_command},
        TgBot::BotCommandScope::Ptr{new TgBot::BotCommandScopeDefault{}});

    // ####################################################################################
    // ####################################################################################

    bot.getEvents().onCommand(start_command->command,
                              [&bot, &db, &text, &keyboard](TgBot::Message::Ptr message)
                              {
                                  db.states().at(message->from->id) = FSM::USER_AUTHORIZED;
                                  bot.getApi().sendMessage(
                                      message->chat->id,
                                      text(db.states().at(message->from->id)),
                                      false,
                                      0,
                                      keyboard(db.states().at(message->from->id)));
                              });
    bot.getEvents().onCommand(
        admin_command->command,
        [&bot, &db, &admin_id, &text, &keyboard](TgBot::Message::Ptr message)
        {
            if(message->from->id == admin_id)
            {
                db.employees().add({.tg_id = std::to_string(message->from->id)});
                db.states().at(message->from->id) = FSM::ADMIN;
            }
            bot.getApi().sendMessage(message->chat->id,
                                     text(db.states().at(message->from->id)),
                                     false,
                                     0,
                                     keyboard(db.states().at(message->from->id)));
        });

    // ####################################################################################
    // ####################################################################################
    bot.getEvents().onCallbackQuery(
        [&bot, &db, &text, &keyboard, &mail](TgBot::CallbackQuery::Ptr query)
        {
            try
            {
                if(StringTools::startsWith(query->data, "rtrn"))
                {
                    switch(db.states().at(query->from->id))
                    {
                        case FSM::USER_INFO_HELP :
                            db.states().at(query->from->id) = FSM::USER_AUTHORIZED;
                            break;
                        case FSM::USER_CHNG_BIO :
                            db.states().at(query->from->id) = FSM::USER_AUTHORIZED;
                            break;
                        case FSM::USER_RQST_PRKNG :
                            db.states().at(query->from->id) = FSM::USER_AUTHORIZED;
                            break;
                        case FSM::USER_CNCL_RQST :
                            db.states().at(query->from->id) = FSM::USER_RQST_PRKNG;
                            break;
                        case FSM::USER_SBMT_RQST :
                            db.states().at(query->from->id) = FSM::USER_RQST_PRKNG;
                            break;
                        case FSM::ADMIN_CHNG_MAIL :
                            db.states().at(query->from->id) = FSM::ADMIN;
                            break;
                        case FSM::ADMIN_ADD_RM_USER :
                            db.states().at(query->from->id) = FSM::ADMIN;
                            break;
                        case FSM::ADMIN_CHNG_NUM :
                            db.states().at(query->from->id) = FSM::ADMIN;
                            break;
                        default :
                            db.states().at(query->from->id) = FSM::USER_AUTHORIZED;
                            break;
                    }
                    bot.getApi().editMessageText(text(db.states().at(query->from->id)),
                                                 query->message->chat->id,
                                                 query->message->messageId,
                                                 "",
                                                 "",
                                                 false,
                                                 keyboard(db.states().at(query->from->id)));
                }
                else
                {
                    switch(db.states().at(query->from->id))
                    {
                        case FSM::USER_AUTHORIZED :
                            {
                                if(StringTools::startsWith(query->data, "info_help"))
                                {
                                    db.states().at(query->from->id) = FSM::USER_INFO_HELP;
                                    bot.getApi().editMessageText(
                                        text(db.states().at(query->from->id)),
                                        query->message->chat->id,
                                        query->message->messageId,
                                        "",
                                        "",
                                        false,
                                        keyboard(db.states().at(query->from->id)));
                                }
                                if(StringTools::startsWith(query->data, "rqst_prkng"))
                                {
                                    db.states().at(query->from->id) = FSM::USER_RQST_PRKNG;
                                    bot.getApi().editMessageText(
                                        text(db.states().at(query->from->id)),
                                        query->message->chat->id,
                                        query->message->messageId,
                                        "",
                                        "",
                                        false,
                                        keyboard(db.states().at(query->from->id)));
                                }
                                if(StringTools::startsWith(query->data, "chng_bio"))
                                {
                                    db.states().at(query->from->id) = FSM::USER_CHNG_BIO;
                                    bot.getApi().editMessageText(
                                        text(db.states().at(query->from->id), query->from->id),
                                        query->message->chat->id,
                                        query->message->messageId,
                                        "",
                                        "Markdown",
                                        false,
                                        keyboard(db.states().at(query->from->id)));
                                }
                            }
                            break;
                        case FSM::USER_RQST_PRKNG :
                            {
                                if(StringTools::startsWith(query->data, "cncl_rqst"))
                                {
                                    db.states().at(query->from->id) = FSM::USER_CNCL_RQST;
                                    bot.getApi().editMessageText(
                                        text(db.states().at(query->from->id)),
                                        query->message->chat->id,
                                        query->message->messageId,
                                        "",
                                        "",
                                        false,
                                        keyboard(db.states().at(query->from->id)));
                                }
                                if(StringTools::startsWith(query->data, "submit_rqst"))
                                {
                                    db.states().at(query->from->id) = FSM::USER_SBMT_RQST;
                                    bot.getApi().editMessageText(
                                        text(db.states().at(query->from->id)),
                                        query->message->chat->id,
                                        query->message->messageId,
                                        "",
                                        "",
                                        false,
                                        keyboard(db.states().at(query->from->id)));
                                }
                            }
                            break;
                        case FSM::ADMIN :
                            {
                                if(StringTools::startsWith(query->data, "add_rm_user"))
                                {
                                    db.states().at(query->from->id) = FSM::ADMIN_ADD_RM_USER;
                                    bot.getApi().editMessageText(
                                        text(db.states().at(query->from->id)),
                                        query->message->chat->id,
                                        query->message->messageId,
                                        "",
                                        "",
                                        false,
                                        keyboard(db.states().at(query->from->id)));
                                }
                                if(StringTools::startsWith(query->data, "chng_mailbox"))
                                {
                                    db.states().at(query->from->id) = FSM::ADMIN_CHNG_MAIL;
                                    bot.getApi().editMessageText(
                                        text(db.states().at(query->from->id)),
                                        query->message->chat->id,
                                        query->message->messageId,
                                        "",
                                        "Markdown",
                                        false,
                                        keyboard(db.states().at(query->from->id)));
                                }
                                if(StringTools::startsWith(query->data, "chng_num"))
                                {
                                    db.states().at(query->from->id) = FSM::ADMIN_CHNG_NUM;
                                    bot.getApi().editMessageText(
                                        text(db.states().at(query->from->id)),
                                        query->message->chat->id,
                                        query->message->messageId,
                                        "",
                                        "",
                                        false,
                                        keyboard(db.states().at(query->from->id)));
                                }
                                if(StringTools::startsWith(query->data, "snd_rprt"))
                                {
                                    if(sendReport(mail))
                                        bot.getApi().sendMessage(
                                            query->message->chat->id,
                                            "🟢 Отчет отправлен",
                                            false,
                                            0,
                                            keyboard(
                                                db.states().at(query->message->from->id)));
                                    else
                                        bot.getApi().sendMessage(
                                            query->message->chat->id,
                                            "🔴 Отчет не отправлен",
                                            false,
                                            0,
                                            keyboard(
                                                db.states().at(query->message->from->id)));
                                }
                                if(StringTools::startsWith(query->data, "chng_wkdy"))
                                {
                                    db.states().at(query->from->id) = FSM::ADMIN_CHNG_WKDY;
                                    bot.getApi().editMessageText(
                                        text(db.states().at(query->from->id)),
                                        query->message->chat->id,
                                        query->message->messageId,
                                        "",
                                        "",
                                        false,
                                        keyboard(db.states().at(query->from->id)));
                                }
                            }
                            break;
                        case FSM::ADMIN_CHNG_WKDY :
                            {
                                (StringTools::startsWith(query->data, "cncl_rqst")) ? ;





                                db.states().at(query->from->id) = FSM::USER_CNCL_RQST;
                                bot.getApi().editMessageText(
                                    text(db.states().at(query->from->id)),
                                    query->message->chat->id,
                                    query->message->messageId,
                                    "",
                                    "",
                                    false,
                                    keyboard(db.states().at(query->from->id)));
                               
                                
                            }
                            break;
                        default : break;
                    }
                }
            }
            catch(TgBot::TgException& e)
            {
            }
        });

    // ####################################################################################
    // ####################################################################################
    bot.getEvents().onAnyMessage(
        [&bot, &db, &keyboard, &mail](TgBot::Message::Ptr message)
        {
            switch(db.states().at(message->from->id))
            {
                case FSM::USER_CHNG_BIO :
                    {
                        auto        new_bio{parseBioStr(message->text)};
                        std::string fail_message{};

                        if(new_bio.firstname.empty())
                            fail_message += "❌ Не удалось распознать фамилию\n";
                        if(new_bio.middlename.empty())
                            fail_message += "❌ Не удалось распознать имя\n";
                        if(new_bio.lastname.empty())
                            fail_message += "❌ Не удалось распознать отчество\n";
                        if(new_bio.car_model.empty())
                            fail_message += "❌ Не удалось распознать модель автомобиля\n";
                        if(new_bio.license.empty())
                            fail_message += "❌ Не удалось распознать номерной знак\n";

                        new_bio.tg_id = std::to_string(message->from->id);
                        db.employees().update(new_bio);

                        if(fail_message.empty())
                        {
                            bot.getApi().sendMessage(
                                message->chat->id,
                                "🟢 Данные о пользователе обновлены",
                                false,
                                0,
                                keyboard(db.states().at(message->from->id)));
                        }
                        else
                        {
                            bot.getApi().sendMessage(
                                message->chat->id,
                                fail_message + "🔴 Не все данные о пользователе обновлены",
                                false,
                                0,
                                keyboard(db.states().at(message->from->id)));
                        }
                    }
                    break;
                case FSM::ADMIN_ADD_RM_USER :
                    {
                        if(message->forwardFrom)
                        {
                            if(db.employees().exists(
                                   {.tg_id = std::to_string(message->forwardFrom->id)}))
                            {
                                bot.getApi().sendMessage(
                                    message->chat->id,
                                    ((db.employees().remove(
                                        {.tg_id = std::to_string(message->forwardFrom->id)})))
                                        ? "🟢 Пользователь удален"
                                        : "🔴 Пользователь не удален",
                                    false,
                                    0,
                                    keyboard(db.states().at(message->from->id)));
                            }
                            else
                            {
                                bot.getApi().sendMessage(
                                    message->chat->id,
                                    ((db.employees().add(
                                        {.tg_id = std::to_string(message->forwardFrom->id)})))
                                        ? "🟢 Пользователь добавлен"
                                        : "🔴 Пользователь не добавлен",
                                    false,
                                    0,
                                    keyboard(db.states().at(message->from->id)));
                            }
                        }
                    }
                    break;
                case FSM::ADMIN_CHNG_MAIL :
                    {
                        auto        new_mail{parseMailStr(message->text)};

                        std::string fail_message{};

                        if(new_mail._receiver_addr.empty())
                            fail_message += "❌ Не удалось распознать получателя\n";
                        if(new_mail._sender_addr.empty())
                            fail_message += "❌ Не удалось распознать отправителя\n";
                        if(new_mail._sender_pass.empty())
                            fail_message += "❌ Не удалось распознать пароль\n";

                        if(fail_message.empty())
                        {
                            mail = new_mail;

                            bot.getApi().sendMessage(
                                message->chat->id,
                                "🟢 Данные почты обновлены",
                                false,
                                0,
                                keyboard(db.states().at(message->from->id)));
                        }
                        else
                        {
                            bot.getApi().sendMessage(
                                message->chat->id,
                                fail_message + "🔴 Не все данные почты обновлены",
                                false,
                                0,
                                keyboard(db.states().at(message->from->id)));
                        }
                    }
                    break;
                case FSM::ADMIN_CHNG_NUM :
                    {
                        int input{0};
                        try
                        {
                            input = std::stoi(message->text.c_str());
                        }
                        catch(...)
                        {
                            break;
                        }
                        if(db.datetime().maxPlaces(input))
                        {
                            bot.getApi().sendMessage(
                                message->chat->id,
                                "🟢 Колличество мест изменено на " + std::to_string(input),
                                false,
                                0,
                                keyboard(db.states().at(message->from->id)));
                        }
                        else
                            bot.getApi().sendMessage(
                                message->chat->id,
                                "🔴 Колличество мест, уже забронированных на "
                                "следующий день превышает "
                                "новое максимальное число мест. Не удалось изменить "
                                "колличество мест на "
                                    + std::to_string(input),
                                false,
                                0,
                                keyboard(db.states().at(message->from->id)));
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

        TgBot::TgLongPoll longPoll(bot);
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
