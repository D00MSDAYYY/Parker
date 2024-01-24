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

#include "FSM_states.hpp"
#include "data_base.hpp"
#include "email.hpp"
#include "excel.hpp"

#include <chrono>
#include <condition_variable>
#include <csignal>
#include <cstdlib>
#include <ctime>
#include <exception>
#include <mutex>
#include <optional>
#include <string>
#include <tgbot/tgbot.h>
#include <thread>
#include <utility>

using namespace std::chrono_literals;
using namespace std::chrono;

seconds
parseTime(const std::string& datetimeString, const std::string& format = "%H:%M:%S")
{
    std::tm            raw{};
    std::istringstream ss{datetimeString};
    ss >> std::get_time(&raw, format.c_str());

    return {hours{raw.tm_hour} + minutes{raw.tm_min} + seconds{raw.tm_sec}};
}

int
main(int argc, char** argv)
{
    // main information ###################################################################
    // ####################################################################################
    TgBot::Bot              bot{argv[1]};
    int64_t                 admin_id{std::stoll(argv[2])};
    Mail_Args               mail{};
    Data_Base               db{};
    std::atomic<bool>       working_flag{true};
    seconds                 start_time{1h + 1min};    // time then bot starts working
    seconds                 stop_time{23h + 1min};    // time theb bot stops working
    seconds                 report_time{20h + 1min};  // time then bot sends report

    // timing_thread ######################################################################
    // working time managing ##############################################################
    std::condition_variable cv_timing{};
    std::mutex              mtx_timing{};
    std::jthread            timing_thread{
        [&working_flag, &cv_timing, &mtx_timing, &start_time, &stop_time]()
        {
            while(true)
            {
                using namespace std::chrono;

                zoned_time curr_tp{current_zone(), system_clock::now()};
                auto       local_t{curr_tp.get_local_time()};
                auto       local_d{floor<days>(local_t)};
                auto       is_reversed{(start_time > stop_time) ? true : false};

                if((local_t > local_d + start_time && local_t < local_d + stop_time)
                   || (local_t > local_d && local_t < local_d + stop_time
                       || local_t < local_d + 24h && local_t > local_d + start_time)
                          && is_reversed)
                {
                    // if local_t is in working time range :
                    std::unique_lock u_lock{mtx_timing};
                    working_flag = true;
                    zoned_time sleep_to_stop{current_zone(),
                                             local_d + ((local_t < local_d + stop_time) ? 0h : 24h)
                                                 + stop_time};
                    cv_timing.wait_until(u_lock, sleep_to_stop.get_sys_time());
                }
                else
                {
                    // if local_t is out of working time range :
                    std::unique_lock u_lock{mtx_timing};
                    working_flag = false;
                    zoned_time sleep_to_start{
                        current_zone(),
                        local_d + ((local_t < local_d + start_time) ? 0h : 24h) + start_time};
                    std::cerr << "sleep_to_start until sys = " << sleep_to_start.get_sys_time()
                              << std::endl;
                    cv_timing.wait_until(u_lock, sleep_to_start.get_sys_time());
                }
            }
        }

    };

    // report_thread ######################################################################
    // report sending managing ############################################################
    std::condition_variable cv_email{};
    std::mutex              mtx_email{};
    std::jthread            report_thread{
        [&db, &cv_email, &mtx_email, &report_time, &mail]()
        {
            while(true)
            {
                using namespace std::chrono;

                zoned_time curr_tp{current_zone(), system_clock::now()};
                auto       local_t{curr_tp.get_local_time()};
                auto       local_d{floor<days>(local_t)};
                auto       is_reversed{(local_t > local_d + report_time) ? true : false};
                zoned_time sleep_to_report{current_zone(),
                                           local_d + report_time + ((is_reversed) ? 24h : 0h)};

                if(system_clock::now()
                   > zoned_time{current_zone(), local_d + report_time}.get_sys_time())
                {
                    year_month_weekday curr_wkdy{floor<days>(local_d)};

                    if(db.datetime().allowedWeekdays().contains(++curr_wkdy.weekday()))
                    {
                        auto report{createReport(
                            db,
                            year_month_day{floor<days>(curr_tp.get_local_time() + 24h)})};
                        if(report) sendReport(mail, *report);
                    }
                }
                std::unique_lock u_lock{mtx_email};
                cv_email.wait_until(u_lock, sleep_to_report.get_sys_time());
            }
        }};

    // text ###############################################################################
    // this lambda fully responsible for message's text editing ###########################
    auto text{
        [&mail, &db, &working_flag, &stop_time, &start_time, &report_time](
            FSM                    state,
            std::optional<int64_t> id = std::nullopt) -> std::string
        {
            std::string text{};
            switch(state)
            {
                case FSM::USER_BOT_NOT_WORKING :
                    text.assign("Рабочее время бота :"
                                "\n🕛 начало : "
                                + std::format("{:%R}", start_time)
                                + "\n🕛 конец : " + std::format("{:%R}", stop_time));
                    break;
                case FSM::USER_NOT_AUTHORIZED :
                    text.assign("🔴 Вас нет в базе данных, свяжитесь с администратором");
                    break;
                case FSM::USER_AUTHORIZED : text.assign("🟢 Авторизация прошла успешно"); break;
                case FSM::USER_INFO_HELP :
                    text.assign("🅿️arker - телеграм-бот для организации очередности парковки!");
                    break;
                case FSM::USER_CHNG_BIO :
                    {
                        if(id)
                        {
                            auto dbdata{db.employees().selectWhere({.tg_id = std::to_string(*id)})};
                            auto user_info = (dbdata ? (*dbdata)[0] : Query_Args{});
                            text.assign(
                                "Нажатием на поля ввода скопируйте текст сообщения, "
                                "впишите новые данные и отправьте боту:\n\n"
                                "❗ Используйте только кириллицу и цифры\n\n"
                                "`➡️ Фамилия : "
                                + ((user_info.fname) ? *user_info.fname : "")  //
                                + "\n➡️ Имя : " + (user_info.midname ? *user_info.midname : "")
                                + "\n➡️ Отчество : " + (user_info.lname ? *user_info.lname : "")
                                + "\n➡️ Марка машины : " + (user_info.car ? *user_info.car : "")
                                + "\n➡️ Номерной знак : "
                                + (user_info.license ? *user_info.license : "") + "\n`");
                        }
                    }
                    break;
                case FSM::USER_RQST_PRKNG : text.assign("Выберите день: "); break;
                case FSM::ADMIN : text.assign("Настройка бота: "); break;
                case FSM::ADMIN_ADD_RM_USER :
                    text.assign("Перешлите сюда любое сообщение пользователя: ");
                    break;
                case FSM::ADMIN_CHNG_MAIL :
                    text.assign(
                        "`📥 Получатель : " + ((mail.receiver_addr) ? *mail.receiver_addr : "")  //
                        + "\n📤 Отправитель : " + ((mail.sender_addr) ? *mail.sender_addr : "")
                        + "\n🔐 Пароль : " + ((mail.sender_pass) ? *mail.sender_pass : "")
                        + "\n🌐 Сервер : " + ((mail.server_addr) ? *mail.server_addr : "")
                        + "\n🔗 Порт : " + ((mail.server_port) ? *mail.server_port : "") + "`");
                    break;
                case FSM::ADMIN_CHNG_NUM :
                    text.assign("Максимальное колличество мест на парковке : "
                                + std::to_string(*db.datetime().maxPlaces()));
                    break;
                case FSM::ADMIN_CHNG_WKDY : text.assign("Парковка организуется по дням : "); break;
                case FSM::ADMIN_CHNG_WRKNG_TIME :
                    text.assign("`Рабочее время бота : "
                                "\n🕛 начало : "
                                + std::format("{:%R}", start_time)
                                + "\n🕛 конец : " + std::format("{:%R}", stop_time) + "`");
                    break;
                case FSM::ADMIN_CHNG_EMAIL_TIME :
                    text.assign("`🕛 Время оправки доклада : " + std::format("{:%R}", report_time)
                                + "`");
                    break;
                default : break;
            }
            return text;
        }};

    // keyboard ###########################################################################
    // this lambda fully responsible for message's keyboard editing #######################
    auto keyboard{
        [&db, &cv_timing](FSM                    state,
                          std::optional<int64_t> id = std::nullopt) -> TgBot::GenericReply::Ptr
        {
            TgBot::InlineKeyboardMarkup::Ptr keyboard{new TgBot::InlineKeyboardMarkup};
            switch(state)
            {
                case FSM::USER_AUTHORIZED :
                    {
                        TgBot::InlineKeyboardButton::Ptr info_help(new TgBot::InlineKeyboardButton);
                        info_help->text         = "ℹ️ Информация и помощь";
                        info_help->callbackData = "info_help";

                        TgBot::InlineKeyboardButton::Ptr chng_bio(new TgBot::InlineKeyboardButton);
                        chng_bio->text         = "👤 Изменить личные данные ";
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
                        using namespace std::chrono;

                        if(id)
                        {
                            zoned_time         local_tp{current_zone(), system_clock::now()};
                            year_month_weekday y_m_wd_local{floor<days>(local_tp.get_local_time())};
                            weekday            curr_weekday_local{y_m_wd_local.weekday()};

                            Weekdays           aval_wkdys{};  // wdays allowed by admin
                            Weekdays chosen_wkdys{};  // wdays which user already successfully chose

                            for(int i{1}; i < 7; ++i)
                            {
                                auto next_wkdy{curr_weekday_local + days{i}};

                                if(db.datetime().allowedWeekdays().contains(next_wkdy))
                                {
                                    aval_wkdys.insert(next_wkdy);
                                    auto next_wkdy_date{local_tp.get_local_time()
                                                        + (next_wkdy - curr_weekday_local)};
                                    if(db.datetime().exists(
                                           {.tg_id = std::to_string(*id),
                                            .date  = std::format(
                                                "{:%F}",
                                                year_month_day{floor<days>(next_wkdy_date)})}))
                                        chosen_wkdys.insert(next_wkdy);
                                }
                            }
                            if(aval_wkdys.contains(Monday))
                            {
                                TgBot::InlineKeyboardButton::Ptr mon(
                                    new TgBot::InlineKeyboardButton);
                                mon->text = chosen_wkdys.contains(Monday) ? "✅ Понедельник"
                                                                          : "Понедельник";
                                mon->callbackData = "mon";
                                keyboard->inlineKeyboard.push_back({mon});
                            }
                            if(aval_wkdys.contains(Tuesday))
                            {
                                TgBot::InlineKeyboardButton::Ptr tue(
                                    new TgBot::InlineKeyboardButton);
                                tue->text =
                                    chosen_wkdys.contains(Tuesday) ? "✅ Вторник" : "Вторник";
                                tue->callbackData = "tue";
                                keyboard->inlineKeyboard.push_back({tue});
                            }
                            if(aval_wkdys.contains(Wednesday))
                            {
                                TgBot::InlineKeyboardButton::Ptr wed(
                                    new TgBot::InlineKeyboardButton);
                                wed->text = chosen_wkdys.contains(Wednesday) ? "✅ Среда" : "Среда";
                                wed->callbackData = "wed";
                                keyboard->inlineKeyboard.push_back({wed});
                            }
                            if(aval_wkdys.contains(Thursday))
                            {
                                TgBot::InlineKeyboardButton::Ptr thu(
                                    new TgBot::InlineKeyboardButton);
                                thu->text =
                                    chosen_wkdys.contains(Thursday) ? "✅ Четверг" : "Четверг";
                                thu->callbackData = "thu";
                                keyboard->inlineKeyboard.push_back({thu});
                            }
                            if(aval_wkdys.contains(Friday))
                            {
                                TgBot::InlineKeyboardButton::Ptr fri(
                                    new TgBot::InlineKeyboardButton);
                                fri->text =
                                    chosen_wkdys.contains(Friday) ? "✅ Пятница" : "Пятница";
                                fri->callbackData = "fri";
                                keyboard->inlineKeyboard.push_back({fri});
                            }
                            if(aval_wkdys.contains(Saturday))
                            {
                                TgBot::InlineKeyboardButton::Ptr sat(
                                    new TgBot::InlineKeyboardButton);
                                sat->text =
                                    chosen_wkdys.contains(Saturday) ? "✅ Суббота" : "Суббота";
                                sat->callbackData = "sat";
                                keyboard->inlineKeyboard.push_back({sat});
                            }
                            if(aval_wkdys.contains(Sunday))
                            {
                                TgBot::InlineKeyboardButton::Ptr sat(
                                    new TgBot::InlineKeyboardButton);
                                sat->text = chosen_wkdys.contains(Sunday) ? "✅ Воскресенье"
                                                                          : "Воскресенье";
                                sat->callbackData = "sun";
                                keyboard->inlineKeyboard.push_back({sat});
                            }
                        }
                    }
                    break;
                case FSM::ADMIN :
                    {
                        TgBot::InlineKeyboardButton::Ptr add_rm_user(
                            new TgBot::InlineKeyboardButton);
                        add_rm_user->text = "👤 Добавить/удалить пользователя";
                        add_rm_user->callbackData = "add_rm_user";

                        TgBot::InlineKeyboardButton::Ptr chng_num(new TgBot::InlineKeyboardButton);
                        chng_num->text = "🔢 Изменить колличество мест";
                        chng_num->callbackData = "chng_num";

                        TgBot::InlineKeyboardButton::Ptr chng_wkdy(new TgBot::InlineKeyboardButton);
                        chng_wkdy->text         = "📅 Изменить дни недели";
                        chng_wkdy->callbackData = "chng_wkdy";

                        TgBot::InlineKeyboardButton::Ptr chng_email(
                            new TgBot::InlineKeyboardButton);
                        chng_email->text         = "📬 Изменить почту";
                        chng_email->callbackData = "chng_mailbox";

                        TgBot::InlineKeyboardButton::Ptr chng_email_time(
                            new TgBot::InlineKeyboardButton);
                        chng_email_time->text = "🕛 Изменить время отправки доклада";
                        chng_email_time->callbackData = "chng_email_time";

                        TgBot::InlineKeyboardButton::Ptr snd_email(new TgBot::InlineKeyboardButton);
                        snd_email->text         = "📈 Отправить доклад";
                        snd_email->callbackData = "snd_email";

                        TgBot::InlineKeyboardButton::Ptr chng_working_time(
                            new TgBot::InlineKeyboardButton);
                        chng_working_time->text = "🕛 Изменить время работы бота";
                        chng_working_time->callbackData = "chng_working_time";

                        keyboard->inlineKeyboard.push_back({add_rm_user});
                        keyboard->inlineKeyboard.push_back({chng_num});
                        keyboard->inlineKeyboard.push_back({chng_wkdy});
                        keyboard->inlineKeyboard.push_back({chng_working_time});
                        keyboard->inlineKeyboard.push_back({chng_email});
                        keyboard->inlineKeyboard.push_back({snd_email});
                        keyboard->inlineKeyboard.push_back({chng_email_time});
                    }
                    break;
                case FSM::ADMIN_CHNG_WKDY :
                    {
                        using namespace std::chrono;

                        TgBot::InlineKeyboardButton::Ptr mon(new TgBot::InlineKeyboardButton);
                        mon->text         = db.datetime().allowedWeekdays().contains(Monday)
                                              ? "✅ Понедельник"
                                              : "Понедельник";
                        mon->callbackData = "mon";
                        TgBot::InlineKeyboardButton::Ptr tue(new TgBot::InlineKeyboardButton);
                        tue->text = db.datetime().allowedWeekdays().contains(Tuesday) ? "✅ Вторник"
                                                                                      : "Вторник";
                        tue->callbackData = "tue";
                        TgBot::InlineKeyboardButton::Ptr wed(new TgBot::InlineKeyboardButton);
                        wed->text = db.datetime().allowedWeekdays().contains(Wednesday) ? "✅ Среда"
                                                                                        : "Среда";
                        wed->callbackData = "wed";
                        TgBot::InlineKeyboardButton::Ptr thu(new TgBot::InlineKeyboardButton);
                        thu->text         = db.datetime().allowedWeekdays().contains(Thursday)
                                              ? "✅ Четверг"
                                              : "Четверг";
                        thu->callbackData = "thu";
                        TgBot::InlineKeyboardButton::Ptr fri(new TgBot::InlineKeyboardButton);
                        fri->text = db.datetime().allowedWeekdays().contains(Friday) ? "✅ Пятница"
                                                                                     : "Пятница";
                        fri->callbackData = "fri";
                        TgBot::InlineKeyboardButton::Ptr sat(new TgBot::InlineKeyboardButton);
                        sat->text         = db.datetime().allowedWeekdays().contains(Saturday)
                                              ? "✅ Суббота"
                                              : "Суббота";
                        sat->callbackData = "sat";
                        TgBot::InlineKeyboardButton::Ptr sun(new TgBot::InlineKeyboardButton);
                        sun->text         = db.datetime().allowedWeekdays().contains(Sunday)
                                              ? "✅ Воскресенье"
                                              : "Воскресенье";
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
            if(state != FSM::USER_AUTHORIZED && state != FSM::ADMIN)
            {
                TgBot::InlineKeyboardButton::Ptr rtrn(new TgBot::InlineKeyboardButton);
                rtrn->text         = "↩️ Назад";
                rtrn->callbackData = "rtrn";
                keyboard->inlineKeyboard.push_back({rtrn});
            }
            if(state == FSM::USER_BOT_NOT_WORKING) keyboard = nullptr;

            return keyboard;
        }};

    // clean up and set commands ##########################################################
    // ####################################################################################
    bot.getApi().setChatMenuButton(0, TgBot::MenuButton::Ptr{new TgBot::MenuButtonCommands});
    bot.getApi().deleteMyCommands();

    TgBot::BotCommand::Ptr start_command(new TgBot::BotCommand{});
    start_command->description = "Start bot";
    start_command->command     = "start";
    TgBot::BotCommand::Ptr admin_command(new TgBot::BotCommand{});
    admin_command->description = "Only for admin";
    admin_command->command     = "admin";
    bot.getApi().setMyCommands({start_command, admin_command},
                               TgBot::BotCommandScope::Ptr{new TgBot::BotCommandScopeDefault{}});

    // ####################################################################################
    // ####################################################################################

    bot.getEvents().onCommand(
        start_command->command,
        [&bot, &db, &text, &keyboard, &working_flag](TgBot::Message::Ptr message)
        {
            if(working_flag)
                db.states().at(message->from->id) = FSM::USER_AUTHORIZED;
            else
                db.states().at(message->from->id) = FSM::USER_BOT_NOT_WORKING;

            bot.getApi().sendMessage(message->chat->id,
                                     text(db.states().at(message->from->id)),
                                     false,
                                     0,
                                     keyboard(db.states().at(message->from->id)),
                                     "Markdown");
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
                                     keyboard(db.states().at(message->from->id)),
                                     "Markdown");
        });

    // ####################################################################################
    // ####################################################################################

    bot.getEvents().onCallbackQuery(
        [&bot, &db, &text, &keyboard, &mail, &working_flag, &cv_timing, &admin_id](
            TgBot::CallbackQuery::Ptr query)
        {
            std::string response{};

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
                    case FSM::ADMIN_ADD_RM_USER :
                        db.states().at(query->from->id) = FSM::ADMIN;
                        break;
                    case FSM::ADMIN_CHNG_MAIL : db.states().at(query->from->id) = FSM::ADMIN; break;
                    case FSM::ADMIN_CHNG_NUM : db.states().at(query->from->id) = FSM::ADMIN; break;
                    case FSM::ADMIN_CHNG_WKDY : db.states().at(query->from->id) = FSM::ADMIN; break;
                    case FSM::ADMIN_CHNG_WRKNG_TIME :
                        db.states().at(query->from->id) = FSM::ADMIN;
                        break;
                    case FSM::ADMIN_CHNG_EMAIL_TIME :
                        db.states().at(query->from->id) = FSM::ADMIN;
                        break;
                    default : db.states().at(query->from->id) = FSM::USER_AUTHORIZED; break;
                }
            }
            if(working_flag || query->from->id == admin_id)
            {
                switch(db.states().at(query->from->id))
                {
                    case FSM::USER_AUTHORIZED :
                        if(StringTools::startsWith(query->data, "info_help"))
                            db.states().at(query->from->id) = FSM::USER_INFO_HELP;
                        if(StringTools::startsWith(query->data, "rqst_prkng"))
                            db.states().at(query->from->id) = FSM::USER_RQST_PRKNG;
                        if(StringTools::startsWith(query->data, "chng_bio"))
                            db.states().at(query->from->id) = FSM::USER_CHNG_BIO;
                        break;
                    case FSM::USER_RQST_PRKNG :
                        {
                            using namespace std::chrono;

                            std::optional<weekday> wkdy_chosen{};
                            if(StringTools::startsWith(query->data, "mon")) wkdy_chosen = Monday;
                            if(StringTools::startsWith(query->data, "tue")) wkdy_chosen = Tuesday;
                            if(StringTools::startsWith(query->data, "wed")) wkdy_chosen = Wednesday;
                            if(StringTools::startsWith(query->data, "thu")) wkdy_chosen = Thursday;
                            if(StringTools::startsWith(query->data, "fri")) wkdy_chosen = Friday;
                            if(StringTools::startsWith(query->data, "sat")) wkdy_chosen = Saturday;
                            if(StringTools::startsWith(query->data, "sun")) wkdy_chosen = Sunday;
                            if(wkdy_chosen)
                            {
                                zoned_time         local_tp{current_zone(), system_clock::now()};
                                year_month_weekday y_m_wd_local{
                                    floor<days>(local_tp.get_local_time())};
                                weekday curr_weekday_local{y_m_wd_local.weekday()};

                                auto    wkdy_chosen_date{local_tp.get_local_time()
                                                      + (*wkdy_chosen - curr_weekday_local)};

                                if(db.datetime().exists(
                                       {.tg_id = std::to_string(query->from->id),
                                        .date  = std::format(
                                            "{:%F}",
                                            year_month_day{floor<days>(wkdy_chosen_date)})}))
                                {
                                    // if user clicks on already chosen wkday - he cancels the
                                    // request
                                    db.datetime().remove(
                                        {.tg_id = std::to_string(query->from->id),
                                         .date  = std::format(
                                             "{:%F}",
                                             year_month_day{floor<days>(wkdy_chosen_date)})});
                                    response =
                                        "\n🟡 Заявка на "
                                        + std::format("{:%F}",
                                                      year_month_day{floor<days>(wkdy_chosen_date)})
                                        + " отменена";
                                }
                                else
                                {
                                    std::string fail_message{};

                                    auto        dbdata{db.employees().selectWhere(
                                        {.tg_id = std::to_string(query->from->id)})};
                                    if(dbdata)
                                    {
                                        auto tmp{std::move((*dbdata)[0])};

                                        if(!tmp.fname) fail_message += "\n❌ Вы не указали фамилию";
                                        if(!tmp.midname) fail_message += "\n❌ Вы не указали имя";
                                        if(!tmp.lname)
                                            fail_message += "\n❌ Вы не указали отчество";
                                        if(!tmp.car)
                                            fail_message += "\n❌ Вы не указали модель машины";
                                        if(!tmp.license)
                                            fail_message += "\n❌ Вы не указали номерной знак";

                                        if(fail_message.empty())
                                        {
                                            zoned_time         local_tp{current_zone(),
                                                                system_clock::now()};
                                            year_month_weekday y_m_wd_local_now{
                                                floor<days>(local_tp.get_local_time())};
                                            auto weekday_now{y_m_wd_local_now.weekday()};
                                            auto future_day{local_tp.get_local_time()
                                                            + (*wkdy_chosen - weekday_now)};
                                            response =
                                                (db.datetime().add(
                                                    {.tg_id = std::to_string(query->from->id),
                                                     .date  = std::format(
                                                         "{:%F}",
                                                         year_month_day{floor<days>(future_day)})}))
                                                    ? "\n🟢 Заявка на "
                                                          + std::format("{:%F}",
                                                                        year_month_day{floor<days>(
                                                                            wkdy_chosen_date)})
                                                          + " подана"
                                                    : "\n🔴 Заявка не подана";
                                        }
                                        else
                                            response = fail_message
                                                     + "\n🔴 Заявка не подана из-за ошибок "
                                                       "распознавания полей ввода ";
                                    }
                                    else
                                        response = "\n🔴 Пользователь не найден в базе данных";
                                }
                            }
                        }
                    case FSM::ADMIN :
                        if(StringTools::startsWith(query->data, "add_rm_user"))
                            db.states().at(query->from->id) = FSM::ADMIN_ADD_RM_USER;
                        if(StringTools::startsWith(query->data, "chng_mailbox"))
                            db.states().at(query->from->id) = FSM::ADMIN_CHNG_MAIL;
                        if(StringTools::startsWith(query->data, "chng_num"))
                            db.states().at(query->from->id) = FSM::ADMIN_CHNG_NUM;
                        if(StringTools::startsWith(query->data, "chng_wkdy"))
                            db.states().at(query->from->id) = FSM::ADMIN_CHNG_WKDY;
                        if(StringTools::startsWith(query->data, "chng_working_time"))
                            db.states().at(query->from->id) = FSM::ADMIN_CHNG_WRKNG_TIME;
                        if(StringTools::startsWith(query->data, "chng_email_time"))
                            db.states().at(query->from->id) = FSM::ADMIN_CHNG_EMAIL_TIME;
                        if(StringTools::startsWith(query->data, "snd_email"))
                        {
                            using namespace std::chrono;

                            zoned_time     local_tp{current_zone(), system_clock::now()};
                            year_month_day y_m_d_local_next{
                                floor<days>(local_tp.get_local_time() + days{1})};

                            auto file_path{createReport(db, y_m_d_local_next)};
                            if(file_path)
                            {
                                response = (sendReport(mail, *file_path))
                                             ? "\n🟢 Отчет отправлен"
                                             : "\n🔴 Отчет не отправлен";
                            }
                            else
                                response = "\n🔴 Не получилось создать отчет, возможно, на "
                                         + std::format("{:%F}", y_m_d_local_next)
                                         + "нет заявок на парковку";
                        }

                        break;
                    case FSM::ADMIN_CHNG_WKDY :
                        using namespace std::chrono;
                        if(StringTools::startsWith(query->data, "mon"))
                            if(db.datetime().allowedWeekdays().contains(Monday))
                                db.datetime().allowedWeekdays().erase(Monday);
                            else
                                db.datetime().allowedWeekdays().insert(Monday);
                        if(StringTools::startsWith(query->data, "tue"))
                            if(db.datetime().allowedWeekdays().contains(Tuesday))
                                db.datetime().allowedWeekdays().erase(Tuesday);
                            else
                                db.datetime().allowedWeekdays().insert(Tuesday);
                        if(StringTools::startsWith(query->data, "wed"))
                            if(db.datetime().allowedWeekdays().contains(Wednesday))
                                db.datetime().allowedWeekdays().erase(Wednesday);
                            else
                                db.datetime().allowedWeekdays().insert(Wednesday);
                        if(StringTools::startsWith(query->data, "thu"))
                            if(db.datetime().allowedWeekdays().contains(Thursday))
                                db.datetime().allowedWeekdays().erase(Thursday);
                            else
                                db.datetime().allowedWeekdays().insert(Thursday);
                        if(StringTools::startsWith(query->data, "fri"))
                            if(db.datetime().allowedWeekdays().contains(Friday))
                                db.datetime().allowedWeekdays().erase(Friday);
                            else
                                db.datetime().allowedWeekdays().insert(Friday);
                        if(StringTools::startsWith(query->data, "sat"))
                            if(db.datetime().allowedWeekdays().contains(Saturday))
                                db.datetime().allowedWeekdays().erase(Saturday);
                            else
                                db.datetime().allowedWeekdays().insert(Saturday);
                        if(StringTools::startsWith(query->data, "sun"))
                            if(db.datetime().allowedWeekdays().contains(Sunday))
                                db.datetime().allowedWeekdays().erase(Sunday);
                            else
                                db.datetime().allowedWeekdays().insert(Sunday);
                        break;
                    default : break;
                }
            }
            else
                db.states().at(query->from->id) = FSM::USER_BOT_NOT_WORKING;
            try
            {
                bot.getApi().editMessageText(
                    text(db.states().at(query->from->id), query->from->id) + response,
                    query->message->chat->id,
                    query->message->messageId,
                    "",
                    "Markdown",
                    false,
                    keyboard(db.states().at(query->from->id), query->from->id));
            }
            catch(...)
            {
            }
        });
    // ####################################################################################
    // ####################################################################################
    bot.getEvents().onAnyMessage(
        [&bot,
         &db,
         &keyboard,
         &mail,
         &working_flag,
         &text,
         &start_time,
         &stop_time,
         &admin_id,
         &cv_timing,
         &report_time,
         &cv_email](TgBot::Message::Ptr message)
        {
            std::string response{};

            if(working_flag || message->from->id == admin_id)
            {
                switch(db.states().at(message->from->id))
                {
                    case FSM::USER_CHNG_BIO :
                        {
                            auto new_bio_str{parseBioStr(message->text)};
                            new_bio_str->tg_id = std::to_string(message->from->id);

                            std::string fail_message{};
                            if(!new_bio_str->fname)
                                fail_message += "\n❌ Не удалось распознать фамилию";
                            if(!new_bio_str->midname)
                                fail_message += "\n❌ Не удалось распознать имя";
                            if(!new_bio_str->lname)
                                fail_message += "\n❌ Не удалось распознать отчество";
                            if(!new_bio_str->car)
                                fail_message += "\n❌ Не удалось распознать модель автомобиля";
                            if(!new_bio_str->license)
                                fail_message += "\n❌ Не удалось распознать номерной знак";

                            if(fail_message.empty())
                                response = (db.employees().update(*new_bio_str))
                                             ? "\n🟢 Данные пользователя обновлены"
                                             : "\n🔴 Данные пользователя не обновлены, возможно, "
                                               "пользователя нет в базе данных";
                            else
                                response = fail_message
                                         + "\n🔴 Данные пользователя не обновлены из-за ошибок "
                                           "распознавания полей ввода";
                        }
                        break;
                    case FSM::ADMIN_ADD_RM_USER :
                        {
                            if(message->forwardFrom)
                            {
                                if(db.employees().exists(
                                       {.tg_id = std::to_string(message->forwardFrom->id)}))
                                    response =
                                        ((db.employees().remove(
                                            {.tg_id = std::to_string(message->forwardFrom->id)})))
                                            ? "\n🟢 Пользователь удален"
                                            : "\n🔴 Пользователь не удален";
                                else
                                    response =
                                        ((db.employees().add(
                                            {.tg_id = std::to_string(message->forwardFrom->id)})))
                                            ? "\n🟢 Пользователь добавлен"
                                            : "\n🔴 Пользователь не добавлен";
                            }
                        }
                        break;
                    case FSM::ADMIN_CHNG_MAIL :
                        {
                            auto        new_mail{parseMailStr(message->text)};
                            std::string fail_message{};
                            if(!new_mail->receiver_addr)
                                fail_message += "\n❌ Не удалось распознать получателя";
                            if(!new_mail->sender_addr)
                                fail_message += "\n❌ Не удалось распознать отправителя";
                            if(!new_mail->sender_pass)
                                fail_message += "\n❌ Не удалось распознать пароль";
                            if(!new_mail->server_addr)
                                fail_message += "\n❌ Не удалось распознать сервер";
                            if(!new_mail->server_port)
                                fail_message += "\n❌ Не удалось распознать порт";
                            if(fail_message.empty())
                            {
                                mail     = *new_mail;
                                response = "\n🟢 Данные почты обновлены :";
                            }
                            else
                                response = fail_message
                                         + "\n🔴 Данные почты не обновлены из-за ошибок "
                                           "распознавания полей ввода";
                        }
                        break;
                    case FSM::ADMIN_CHNG_NUM :
                        {
                            int input{0};
                            // clang-format off
                            try { input = std::stoi(message->text.c_str()); }
                            catch(...) { break; }
                            // clang-format off
                            response = (db.datetime().maxPlaces(input))
                                         ? "\n🟢 Колличество мест изменено "
                                         : "\n🔴 Колличество мест, уже забронированных на "
                                           "следующий день превышает новое максимальное число мест";
                        }
                        break;
                    case FSM::ADMIN_CHNG_WRKNG_TIME :
                        {
                            if(message->text.contains("🕛 начало :")
                               && message->text.contains("🕛 конец :"))
                            {
                                auto it1 = message->text.begin() + message->text.find("🕛 начало :")
                                         + std::strlen("🕛 начало :");
                                auto it2 = message->text.begin() + message->text.find("🕛 конец :")
                                         + std::strlen("🕛 конец :");
                                auto stop_tm  = parseTime(std::string{it2, it2 + 8});
                                auto start_tm = parseTime(std::string{it1, it1 + 8});

                                if(start_tm == stop_tm) stop_tm += 1min;
                                stop_time  = stop_tm;
                                start_time = start_tm;
                                cv_timing.notify_one();
                                response = "\n🟢 Время работы изменено";
                            }
                            else
                                response = "\n🔴 Время работы не изменено ";
                        }
                        break;
                    case FSM::ADMIN_CHNG_EMAIL_TIME :
                        {
                            if(message->text.contains("доклада :"))
                            {
                                auto it1{message->text.begin() + message->text.find("доклада :")
                                     + std::strlen("доклада :")};
                                auto time{parseTime(std::string{it1, it1 + 8})};
                                report_time = time;
                                cv_email.notify_one();
                                response = "\n🟢 Время отправки изменено";
                            }
                            else
                                response = "\n🔴 Время отправки не изменено";
                        }
                        break;

                    default : break;
                }
            }
            else
                db.states().at(message->from->id) = FSM::USER_BOT_NOT_WORKING;

            if(!response.empty())
                bot.getApi().sendMessage(message->chat->id,
                                         text(db.states().at(message->from->id),message->from->id) + response,
                                         false,
                                         0,
                                         keyboard(db.states().at(message->from->id)),
                                         "Markdown");
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
