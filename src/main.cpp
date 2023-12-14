#include <tgbot/tgbot.h>

#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <exception>
#include <string>

using namespace std;
using namespace TgBot;

enum class FSM_STATE // FSM states
{
    START,              // starting
    AUTORIZATION,       // autorizing
    AUTORIZATION_CMPLT, // autorizing complete
    INFO_HELP,          // information and help
    RQST_PRKNG,         // requesting for parking
    SUBMIT_RQST,        // submiting request for parking
    CNCL_RQST,          // cancelling request for parking
    TIME_SLCTR          // selecting time for parking
};

TgBot::GenericReply::Ptr keyboard(FSM_STATE state)
{
    InlineKeyboardMarkup::Ptr keyboard(new InlineKeyboardMarkup);

    switch (state)
    {
    case FSM_STATE::AUTORIZATION_CMPLT: {
        vector<InlineKeyboardButton::Ptr> row0;
        InlineKeyboardButton::Ptr info_help(new InlineKeyboardButton);
        info_help->text = "Информация и помощь";
        info_help->callbackData = "info_help";
        row0.push_back(info_help);
        keyboard->inlineKeyboard.push_back(row0);

        vector<InlineKeyboardButton::Ptr> row1;
        InlineKeyboardButton::Ptr rqst_prkng(new InlineKeyboardButton);
        rqst_prkng->text = "Заявка на парковку";
        rqst_prkng->callbackData = "rqst_prkng";
        row1.push_back(rqst_prkng);
        keyboard->inlineKeyboard.push_back(row1);
    }
    break;
    case FSM_STATE::RQST_PRKNG: {
        vector<InlineKeyboardButton::Ptr> row0;
        InlineKeyboardButton::Ptr submit_rqst(new InlineKeyboardButton);
        submit_rqst->text = "Подать заявку";
        submit_rqst->callbackData = "submit_rqst";
        row0.push_back(submit_rqst);
        keyboard->inlineKeyboard.push_back(row0);

        vector<InlineKeyboardButton::Ptr> row1;
        InlineKeyboardButton::Ptr cncl_rqst(new InlineKeyboardButton);
        cncl_rqst->text = "Отменить заявку";
        cncl_rqst->callbackData = "cncl_rqst";
        row1.push_back(cncl_rqst);
        keyboard->inlineKeyboard.push_back(row1);
    }
    break;
    case FSM_STATE::CNCL_RQST: {
        vector<InlineKeyboardButton::Ptr> row0;
        InlineKeyboardButton::Ptr yes(new InlineKeyboardButton);
        yes->text = "Да";
        yes->callbackData = "yes";
        row0.push_back(yes);
       
        InlineKeyboardButton::Ptr no(new InlineKeyboardButton);
        no->text = "Нет";
        no->callbackData = "no";
        row0.push_back(no);

        keyboard->inlineKeyboard.push_back(row0);
    }
    break;

    default:
        break;
    }
    return keyboard;
}

int main(int argc, char **argv)
{
    string token(argv[1]);
    Bot bot(token);
    FSM_STATE state{FSM_STATE::START};

    // bot.getEvents().onCommand("start", [&bot, &keyboard](Message::Ptr message) {
    //     bot.getApi().sendMessage(message->chat->id, "Hi!", false, 0, keyboard);
    // });
    // bot.getEvents().onCommand("check", [&bot, &keyboard](Message::Ptr message) {
    //     string response = "ok";
    //     bot.getApi().sendMessage(message->chat->id, response, false, 0, keyboard, "Markdown");
    // });
    // bot.getEvents().onCallbackQuery([&bot, &keyboard](CallbackQuery::Ptr query) {
    //     if (StringTools::startsWith(query->data, "check"))
    //     {
    //         string response = "ok";
    //         bot.getApi().sendMessage(query->message->chat->id, response, false, 0, keyboard, "Markdown");
    //     }
    // });
    // ###########################################################

    bot.getEvents().onCommand("start", [&bot, &state](Message::Ptr message) {
        if (state == FSM_STATE::START)
        {
            state = FSM_STATE::AUTORIZATION;
            bot.getApi().sendMessage(message->chat->id, "Авторизация. \nВведите логин и пароль через пробел: ");
        }
    });

    bot.getEvents().onAnyMessage([&bot, &state](Message::Ptr message) {
        switch (state)
        {
        case FSM_STATE::AUTORIZATION: {
            // обработка логина и пароля
            if (message->text == "login password")
            {
                state = FSM_STATE::AUTORIZATION_CMPLT;
                bot.getApi().sendMessage(message->chat->id, "Авторизация прошла успешно", false, 0, keyboard(state));
            }
            else
                bot.getApi().sendMessage(message->chat->id, "Пароль или логин неверны");
        }
        break;

        default:
            break;
        }
    });

    bot.getEvents().onCallbackQuery([&bot, &state](CallbackQuery::Ptr query) {
        if (StringTools::startsWith(query->data, "info_help") && state == FSM_STATE::AUTORIZATION_CMPLT)
        {
            state = FSM_STATE::INFO_HELP;
            string response = "Это бот для организации парковки. По вопросам пишите @TEST";
            bot.getApi().sendMessage(query->message->chat->id, response);
        }
        if (StringTools::startsWith(query->data, "rqst_prkng") && state == FSM_STATE::AUTORIZATION_CMPLT)
        {
            state = FSM_STATE::RQST_PRKNG;
            bot.getApi().sendMessage(query->message->chat->id, "?", false, 0, keyboard(state));
        }
        if (StringTools::startsWith(query->data, "submit_rqst") && state == FSM_STATE::RQST_PRKNG)
        {
            state = FSM_STATE::SUBMIT_RQST;
            bot.getApi().sendMessage(query->message->chat->id, "Список свободных мест: \nE1\nE2\nE3\nE4");
        }
        if (StringTools::startsWith(query->data, "cncl_rqst") && state == FSM_STATE::RQST_PRKNG)
        {
            state = FSM_STATE::CNCL_RQST;
            bot.getApi().sendMessage(query->message->chat->id, "Твое место - E3 c 14:12 по 18:19, отменить заявку?", false, 0, keyboard(state));
        }
    });

    signal(SIGINT, [](int s) {
        printf("SIGINT got\n");
        exit(0);
    });

    try
    {
        printf("Bot username: %s\n", bot.getApi().getMe()->username.c_str());
        bot.getApi().deleteWebhook();

        TgLongPoll longPoll(bot);
        while (true)
        {
            printf("Long poll started\n");
            longPoll.start();
        }
    }
    catch (exception &e)
    {
        printf("error: %s\n", e.what());
    }

    return 0;
}
