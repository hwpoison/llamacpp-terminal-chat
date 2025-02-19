#ifndef __COMPLETION_H__
#define __COMPLETION_H__

#include <iostream>
#include <string>

#include "sjson.hpp"
#include "utils.hpp"
#include "minipost.hpp"
#include "terminal.hpp"
#include "colors.h"

#include <csignal>

#define DEFAULT_IP "127.0.0.1"
#define DEFAULT_PORT 8080

#define COMPLETION_ENDPOINT     "/completion"
#define OAI_COMPLETION_ENDPOINT "/v1/chat/completions"

#define PARAMS_FILENAME    "params.json"
#define TEMPLATES_FILENAME "templates.json"

extern bool stopCompletionFlag;
extern bool completionInProgress;
extern bool thinking;

struct chat_template_t {
    std::string bos; // Beging of string
    std::string begin_system;
    std::string end_system;

    std::string begin_user;
    std::string end_user;

    std::string begin_assistant;
    std::string end_assistant;

    std::string begin_think;
    std::string end_think;

    std::string eos; // End of string
};

static void completionSignalHandler(int signum) {
    if(completionInProgress){
        stopCompletionFlag =  true;
        signal(SIGINT, SIG_DFL); // reset the signal
    }
}

class Completion {
public:
    Completion(){
        completionBus.buffer = "";
        completionBus.stream = true;
    };

    bool completionCallback(const std::string &chunk, const CallbackBus *bus);

    bool loadParametersSettings(std::string_view profile_name);

    std::string dumpPayload(yyjson_mut_doc* prompt_json);

    Response requestCompletion(const char* ipaddr, const int16_t port, yyjson_mut_doc* prompt_json);

    chat_template_t& getChatTemplates();

    void addStopWord(std::string word);

    bool loadChatTemplates(std::string_view chat_template_name);

    CallbackBus completionBus;

    bool using_oai_completion = false;

private:
    
    httpRequest Req;

    chat_template_t chat_template = {"", "", "\n", "", "\n", "", "\n", "<think>", "</think>", "\n"};

    Dict<std::string> parameters;

    std::vector<std::string> stop_words;

};
#endif
