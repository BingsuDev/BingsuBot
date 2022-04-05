#include <stdlib.h>
#include "sleepy_discord/sleepy_discord.h"
#include "dotenv.h"

std::queue<std::string> split(const std::string& source) {
    std::stringstream ss(source);
    std::string item;
    std::queue<std::string> target;
    while(std::getline(ss, item, ' '))
        if(!item.empty())
            target.push(item);
    return target;
}

class BingsuBot;

namespace Command {
    using Verb = std::function<void(BingsuBot&, SleepyDiscord::Message&, std::queue<std::string>&)>;
    struct Command {
        std::string name;
        std::vector<std::string> params;
        Verb verb;
    };
    using MappedCommands = std::unordered_map<std::string, Command>;
    using MappedCommand = MappedCommands::value_type;
    static MappedCommands all;
    static void addCommand(Command command) {
        all.emplace(command.name, command);
    }
}

class BingsuBot : public SleepyDiscord::DiscordClient {
    public:
        using SleepyDiscord::DiscordClient::DiscordClient;
        void onReady(SleepyDiscord::Ready ready) override {
            updateStatus("Getting developed!", 0, SleepyDiscord::Status::online, false);
        }

        void onMember(SleepyDiscord::Snowflake<SleepyDiscord::Server> serverID, SleepyDiscord::ServerMember member) override {
            SleepyDiscord::Channel welcomeChannel = getChannel("803931209054027787");
            sendMessage(welcomeChannel, "Welcome " + member.user.username + "!", SleepyDiscord::Async);
        }

        void onMessage(SleepyDiscord::Message message) override {
            if(message.isMentioned(getID())) {
                std::queue<std::string> parameters = split(message.content);
                const std::string mention = "<@" + getID().string() + ">";
                const std::string mentionNick = "<@!" + getID().string() + ">";

                if(parameters.size() <= 1 && (parameters.front() != mention || parameters.front() != mentionNick)) return;

                parameters.pop();
                if(parameters.empty()) return;

                Command::MappedCommands::iterator foundCommand = Command::all.find(parameters.front());
                if(foundCommand == Command::all.end()) {
                    sendMessage(message.channelID, "Error: Command not found!", SleepyDiscord::Async);
                    return;
                }
                parameters.pop();
                if(parameters.size() < foundCommand->second.params.size()) {
                    sendMessage(message.channelID, "Error: Too few parameters!", SleepyDiscord::Async);
                    return;
                }

                foundCommand->second.verb(*this, message, parameters);
            }
        }
};

int main(int argc, char** argv) {
    Command::addCommand({
        "help", {}, [](
            BingsuBot& client,
            SleepyDiscord::Message& message,
            std::queue<std::string>&
        ) {
            constexpr char start[] = "Here's a list of all command:```\n";
            constexpr char theEnd[] = "```";
            std::size_t length = strlen(start) + strlen(theEnd);
            for(Command::MappedCommand& command : Command::all) {
                length += command.first.size();
                length += 2;
                for(std::string& paramName : command.second.params) {
                    length += 2;
                    length += paramName.size();
                }
            }

            std::string output;
            output.reserve(length);
            output += start;
            for(Command::MappedCommand& command : Command::all) {
                output += command.first;
                output += ' ';
                for(std::string& paramName : command.second.params) {
                    output += '<';
                    output += paramName;
                    output += "> ";
                }
                output += '\n';
            }
            output += theEnd;
            client.sendMessage(message.channelID, output, SleepyDiscord::Async);
        }
    });
    Command::addCommand({
        "stop", {}, [](
            BingsuBot& client,
            SleepyDiscord::Message& message,
            std::queue<std::string>& params
        ) {
            exit(0);
        }
    });

    dotenv::env.load_dotenv();

    BingsuBot client(dotenv::env["TOKEN"], SleepyDiscord::USER_CONTROLED_THREADS);
    client.setIntents(SleepyDiscord::Intent::SERVER_MESSAGES, SleepyDiscord::Intent::SERVER_MEMBERS);
    client.run();

    return 0;
}