#include <tgbot/tgbot.h>

#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <exception>
#include <string>

using namespace std;
using namespace TgBot;

enum class FSM
{               // FSM states
    START = 0,  // starting
    AUTORIZED = 1 ,  // autorized
    INFO_HELP = 2,  // information and help
    RQST_PRKNG = 3, // requesting for parking
    CNCL_RQST = 4,  // cancelling request for parking
    SUBMIT_RQST = 5 // submiting request for parking
};

TgBot::GenericReply::Ptr keyboard(FSM state)
{
    InlineKeyboardMarkup::Ptr keyboard(new InlineKeyboardMarkup);
    switch (state)
    {
    case FSM::AUTORIZED: {
        InlineKeyboardButton::Ptr info_help(new InlineKeyboardButton);
        info_help->text = "Информация и помощь";
        info_help->callbackData = "info_help";

        InlineKeyboardButton::Ptr rqst_prkng(new InlineKeyboardButton);
        rqst_prkng->text = "Заявка на парковку";
        rqst_prkng->callbackData = "rqst_prkng";

        keyboard->inlineKeyboard.push_back({info_help});
        keyboard->inlineKeyboard.push_back({rqst_prkng});
    }
    break;
    case FSM::RQST_PRKNG: {
        InlineKeyboardButton::Ptr cncl_rqst(new InlineKeyboardButton);
        cncl_rqst->text = "Отменить заявку";
        cncl_rqst->callbackData = "cncl_rqst";

        InlineKeyboardButton::Ptr submit_rqst(new InlineKeyboardButton);
        submit_rqst->text = "Подать заявку";
        submit_rqst->callbackData = "submit_rqst";

        keyboard->inlineKeyboard.push_back({cncl_rqst});
        keyboard->inlineKeyboard.push_back({submit_rqst});
    }
    break;
    case FSM::CNCL_RQST: {
        InlineKeyboardButton::Ptr yes(new InlineKeyboardButton);
        yes->text = "Да";
        yes->callbackData = "cncl_rqst";

        InlineKeyboardButton::Ptr no(new InlineKeyboardButton);
        no->text = "Нет";
        no->callbackData = "submit_rqst";

        keyboard->inlineKeyboard.push_back({yes});
        keyboard->inlineKeyboard.push_back({no});
    }
    break;
    default:
        break;
    }
    return keyboard;
}

std::string text(FSM state)
{
    std::string text;
    switch (state)
    {
    case FSM::START: {
        text.assign("Введите логин и пароль через пробел:");
    }
    break;
    case FSM::AUTORIZED: {
        text.assign("Авторизация прошла успешно");
    }
    break;
    case FSM::INFO_HELP: {
        text.assign("Это бот-помощник для организации очередности парковки!");
    }
    break;
    case FSM::RQST_PRKNG: {
        text.assign("Выберите, что вы хотите сделать на данный момент:");
    }
    break;
    case FSM::CNCL_RQST: {
        text.assign("Ваше место Е3, время - с 14:30 до 16:00. Вы действительно хотите отменить бронь?");
    }
    break;
    case FSM::SUBMIT_RQST: {
        text.assign("На данный момент свободно 3 места, время - с 12:40 до 16:35. Выберите время:");
    }
    break;

    default:
        break;
    }
    return std::move(text);
}

int main(int argc, char **argv)
{
    string token(argv[1]);
    Bot bot(token);
    MenuButton::Ptr menu_start{new MenuButtonCommands};
    bot.getApi().setChatMenuButton(0, menu_start);
    FSM state{FSM::START};

    // ####################################################################################
    // ####################################################################################
    bot.getEvents().onCommand("start", [&bot, &state](Message::Ptr message) {
        state = FSM::START;
        ReplyKeyboardRemove::Ptr rm{new ReplyKeyboardRemove};
        bot.getApi().sendMessage(message->chat->id, text(state), false, 0, keyboard(state));
    });

    bot.getEvents().onCommand("step_back", [&bot, &state](Message::Ptr message) {
        switch (state)
        {
        case FSM::AUTORIZED: {
            state = FSM::START;
            // TODO ! move  bot.getApi().sendMessage just after the end of switch (mb no)
            bot.getApi().sendMessage(message->chat->id, text(state), false, 0, keyboard(state));
        }
        break;
        case FSM::INFO_HELP: {
            state = FSM::AUTORIZED;
            bot.getApi().sendMessage(message->chat->id, text(state), false, 0, keyboard(state));
        }
        break;
        case FSM::RQST_PRKNG: {
            state = FSM::AUTORIZED;
            bot.getApi().sendMessage(message->chat->id, text(state), false, 0, keyboard(state));
        }
        break;
        case FSM::CNCL_RQST: {
            state = FSM::RQST_PRKNG;
            bot.getApi().sendMessage(message->chat->id, text(state), false, 0, keyboard(state));
        }
        break;
        case FSM::SUBMIT_RQST: {
            state = FSM::RQST_PRKNG;
            bot.getApi().sendMessage(message->chat->id, text(state), false, 0, keyboard(state));
        }
        break;

        default:
            break;
        }
    });
    // ####################################################################################
    // ####################################################################################
    bot.getEvents().onAnyMessage([&bot, &state](Message::Ptr message) {
        switch (state)
        {
        case FSM::START: {
            if (message->text == "log pass")
            {
                state = FSM::AUTORIZED;
                bot.getApi().sendMessage(message->chat->id, text(state), false, 0, keyboard(state));
            }
        }
        break;

        default:
            break;
        }
    });
    // ####################################################################################
    // ####################################################################################
    bot.getEvents().onCallbackQuery([&bot, &state](CallbackQuery::Ptr query) {
        switch (state)
        {
        case FSM::AUTORIZED: {
            if (StringTools::startsWith(query->data, "info_help"))
            {
                state = FSM::INFO_HELP;
                bot.getApi().sendMessage(query->message->chat->id, text(state), false, 0, keyboard(state));
            }
            if (StringTools::startsWith(query->data, "rqst_prkng"))
            {
                state = FSM::RQST_PRKNG;
                bot.getApi().sendMessage(query->message->chat->id, text(state), false, 0, keyboard(state));
            }
        }
        break;
        case FSM::RQST_PRKNG: {
            if (StringTools::startsWith(query->data, "cncl_rqst"))
            {
                state = FSM::CNCL_RQST;
                bot.getApi().sendMessage(query->message->chat->id, text(state), false, 0, keyboard(state));
            }
            if (StringTools::startsWith(query->data, "submit_rqst"))
            {
                state = FSM::SUBMIT_RQST;
                bot.getApi().sendMessage(query->message->chat->id, text(state), false, 0, keyboard(state));
            }
        }
        break;
        default:
            break;
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
