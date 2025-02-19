#ifndef __CHAT_H__
#define __CHAT_H__
#include <iostream>
#include <string>
#include <vector>
#include <csignal>
#include <filesystem>
#include <algorithm>

#include "sjson.hpp" 
#include "completion.hpp"
#include "colors.h"
#include "logging.hpp"
#include "utils.hpp"

#define DEFAULT_FILE_EXTENSION ".json"
#define USER_PROMPT_FILE "prompts.json"
#define DEFAULT_SAVE_FOLDER "saved_chats/"

//////////////////////////////////////////////
struct actor_t {
    std::string name;
    std::string role;
    std::string tag_color;
    std::string msg_preffix;
    std::string icon;
};

struct message_entry_t {
    message_entry_t(int64_t id, actor_t *actor_info, std::string content) : id(id), actor_info(actor_info), content(content) {}
    int64_t id;
    actor_t *actor_info;
    std::string content;
};

typedef std::vector<message_entry_t> messages_t;

typedef std::map<std::string, actor_t> actors_t;


/* All related about the chat prompt is on this calss */
class Chat : public Completion {
public:
    Chat(){};

    void addNewMessage(std::string_view actor_name, std::string_view content);

    bool removeLastMessage(int nmessages);

    void updateMessageContent(int number, std::string_view newContent);

    void resetChatHistory();

    void draw();

    bool loadUserPrompt(std::string_view prompt_name);

    void setupSystemPrompt(std::string prompt);

    void updateSystemPrompt(std::string new_system_prompt);

    void setupDefaultActors();

    bool loadSavedConversation(std::string file_path);

    bool saveConversation(std::string filename);

    void printActorChaTag(std::string_view actor_name);
    
    bool addActor(std::string name, const char* role, const char* tag_color, std::string preffix = "", std::string icon = "");

    std::string& getUserName();

    std::string& getAssistantName();

    void removeAllActors();

    void cureCompletionForChat();

    void setupChatStopWords();

    int messagesCount();

    void listCurrentActors();

    std::string applyChatTemplate();

    yyjson_mut_doc* getPromptJSON();

    yyjson_mut_doc* getOAIPrompt();

    bool setChatMode(bool value);

    bool isChatMode();

    bool setChatGuards(bool value);
    
    bool hiddeThinkTokens(bool value);

    yyjson_mut_doc* getCurrentPrompt();

private:
    messages_t messages;

    bool chat_guards = true;
    bool chat_mode = true;
    bool using_system_prompt = false;
    bool hidde_think_tokens = true;
    std::string user_name;
    std::string assistant_name;
    actors_t actors;

};
#endif
