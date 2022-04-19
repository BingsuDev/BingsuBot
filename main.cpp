#include <stdlib.h>
#include <iostream>
#include <string>
#include <algorithm>
#include <cstdio>
#include <ctime>
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
        std::string description;
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
            updatePresence();
        }

        void updatePresence() {
            while(1) {
                std::srand(std::time(nullptr));
                std::vector<std::string> statuses = {"Getting developed!", "Learning new commands!", "Helping out Emi!", "Contemplating life...", "Low level code ftw!"};
                int i = rand() % (statuses.size() - 1 + 1);
                int delay = 900;
                delay *= CLOCKS_PER_SEC;
                clock_t now = clock();
                while(clock() - now <delay);
                updateStatus(statuses[i], 0, SleepyDiscord::Status::online, false);
            }
        }

        void onMember(SleepyDiscord::Snowflake<SleepyDiscord::Server> serverID, SleepyDiscord::ServerMember member) override {
            SleepyDiscord::Embed embed;
            SleepyDiscord::SendMessageParams messageParams;
            std::vector<SleepyDiscord::EmbedField> fields;
            SleepyDiscord::EmbedField field [1];
            SleepyDiscord::EmbedImage image;
            SleepyDiscord::EmbedThumbnail thumbnail;
            image.url = "https://giffiles.alphacoders.com/127/127869.gif";
            thumbnail.url = "https://cdn.discordapp.com/avatars/" + member.user.ID + "/" + member.user.avatar + ".png";
            field[0].name = "Welcome!";
            field[0].value = "Welcome to Emi's hideout kind stranger, we hope you will enjoy your stay and make lots of friends!";
            fields.push_back(field[0]);
            embed.title = member.user.username;
            embed.image = image;
            embed.thumbnail = thumbnail;
            embed.fields = fields;

            messageParams.channelID = "965878907339436032";
            messageParams.embed = embed;

            try {
                sendMessage(messageParams, SleepyDiscord::Async);
            } catch(SleepyDiscord::ErrorCode e) {
                std::cout << e << std::endl;
            }
            fields.clear();
        }

        void onMessage(SleepyDiscord::Message message) override {
            auto& dotenv = dotenv::env.load_dotenv();
            std::queue<std::string> parameters = split(message.content);
            const std::string mention = "<@" + getID().string() + ">";
            const std::string mentionNick = "<@!" + getID().string() + ">";
            const char prefix = dotenv["PREFIX"].front();

            if(message.author.bot == true) return;

            if(parameters.empty()) return;

            if(parameters.front()[0] != prefix) return;

            parameters.front().erase(0,1);

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
};

int main(int argc, char** argv) {
    Command::addCommand({
        "help", {}, "This command right here.", [](
            BingsuBot& client,
            SleepyDiscord::Message& message,
            std::queue<std::string>&
        ) {
            SleepyDiscord::Embed embed;
            std::vector<SleepyDiscord::EmbedField> fields;
            SleepyDiscord::SendMessageParams messageParams;
            SleepyDiscord::EmbedField field [10];
            embed.title = "Help!";
            int i = 0;
            for(Command::MappedCommand& command : Command::all) {
                field[i].name = command.second.name;
                field[i].value = command.second.description;
                fields.push_back(field[i]);
                i++;
            }
            embed.fields = fields;

            messageParams.channelID = message.channelID;
            messageParams.embed = embed;

            try {
                client.sendMessage(messageParams, SleepyDiscord::Async);
            } catch(SleepyDiscord::ErrorCode e) {
                std::cout << e << std::endl;
            }
            fields.clear();
        }
    });
    Command::addCommand({
        "avatar", {}, "Request someone's avatar", [](
            BingsuBot& client,
            SleepyDiscord::Message& message,
            std::queue<std::string>&
        ) {
            SleepyDiscord::Embed embed;
            std::string name;
            SleepyDiscord::EmbedImage image;
            SleepyDiscord::SendMessageParams messageParams;
            messageParams.channelID = message.channelID;
            name = message.author.username;
            image.url = "https://cdn.discordapp.com/avatars/" + message.author.ID + "/" + message.author.avatar + ".png";
            if(message.mentions.size() >= 1) {
                name = message.mentions[0].username;
                image.url = "https://cdn.discordapp.com/avatars/" + message.mentions[0].ID + "/" + message.mentions[0].avatar + ".png";
            }

            embed.image = image;
            embed.title = name;

            try {
                messageParams.embed = embed;
                client.sendMessage(messageParams, SleepyDiscord::Async);
            } catch(SleepyDiscord::ErrorCode e) {
                std::cout << e << std::endl;
            }
        }
    });
    /*Command::addCommand({
        "ping", {}, "Test the bots latency!", [](
            BingsuBot& client,
            SleepyDiscord::Message& message,
            std::queue<std::string>& params
        ) {
            std::time_t t = std::time(0);
            std::tm* now = std::localtime(&t);
            std::cout << (now->tm_min) << ":" << (now->tm_sec) << std::endl;
            std::cout << message.timestamp.substr(14).substr(0, message.timestamp.substr(14).find(".")) << std::endl;
            client.sendMessage(message.channelID, (now->tm_min) + ":" + (now->tm_sec), SleepyDiscord::Async);
            client.sendMessage(message.channelID, message.timestamp.substr(14).substr(0, message.timestamp.substr(14).find(".")), SleepyDiscord::Async);
        }
    });*/
    Command::addCommand({
        "stop", {}, "Stops the bot", [](
            BingsuBot& client,
            SleepyDiscord::Message& message,
            std::queue<std::string>& params
        ) {
            client.sendMessage(message.channelID, "Stopping the bot...");
            abort();
        }
    });

    dotenv::env.load_dotenv();

    BingsuBot client(dotenv::env["TOKEN"], SleepyDiscord::USER_CONTROLED_THREADS);
    client.setIntents(SleepyDiscord::Intent::SERVER_MESSAGES, SleepyDiscord::Intent::SERVER_MEMBERS);
    client.run();

    return 0;
}
