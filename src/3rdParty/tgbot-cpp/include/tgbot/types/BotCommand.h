#ifndef TGBOT_BOTCOMMAND_H
#define TGBOT_BOTCOMMAND_H

#include <cstdint>
#include <memory>
#include <string>

namespace TgBot
{

/**
 * @brief This object represents a bot command.
 *
 * https://core.telegram.org/bots/api#botcommand
 * @ingroup types
 */
class BotCommand
{
public:
    typedef std::shared_ptr<BotCommand> Ptr;

    BotCommand() {}

    //! this is my extension to TgBot-cpp to provide more compact code :
    BotCommand(const std::string& comm, const std::string& descr)
        : command{comm},
          description{descr} {};
    //! end of extension
    virtual ~BotCommand() {}

    /**
     * @brief command label.
     */
    std::string command;

    /**
     * @brief description label.
     */
    std::string description;
};
}  // namespace TgBot

#endif  // TGBOT_BOTCOMMAND_H
