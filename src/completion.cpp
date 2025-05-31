#include "completion.hpp"
bool stopCompletionFlag = false;
bool completionInProgress = false;
bool thinkInProgress = false;

inline std::string safeGetStr(yyjson_val* val) {
    const char* str = yyjson_get_str(val);
    return str ? str : "";
}

bool Completion::completionCallback(const std::string &chunk, const CallbackBus *bus) {
    if (stopCompletionFlag) {
        Terminal::setTitle("Interrupted by user");
        stopCompletionFlag = false;
        completionInProgress = false;
        thinkInProgress = false;
        return false; //callback will return False so streaming will stop.
    }

    // Extract json fragment from chunk
    std::string completionData = chunk;
    if (const_cast<CallbackBus*>(bus)->stream) {
        size_t dataKeyPos = chunk.find("data: ");
        if (dataKeyPos == std::string::npos) return true;
        completionData = chunk.substr(dataKeyPos + 6);
    } else {
        size_t contentKeyPos = chunk.find("\"content\":");
        if (contentKeyPos == std::string::npos) return true;
    }
    
    Logging::debug("Received data: %s", completionData.c_str());
    
    // Parse the data
    yyjson_doc *doc = yyjson_read(completionData.c_str(), strlen(completionData.c_str()), 0);

    // Just ignore bad chunks
    if(doc == NULL){
        yyjson_doc_free(doc);
        return false;
    }

    yyjson_val *root = yyjson_doc_get_root(doc);

    std::string token = "";
    bool endOfCompletation = false;
    double tokenPerSeconds = 0.0;
   
    // Read incoming json data 
    if(using_oai_completion){
        yyjson_val *choices = yyjson_obj_get(root, "choices");
        yyjson_val *content_array = yyjson_arr_get(choices, 0);
        yyjson_val *finish_reason = yyjson_obj_get(content_array, "finish_reason");

        if(!yyjson_is_null(finish_reason)) {
            yyjson_val *timings = yyjson_obj_get(root, "timings");
            yyjson_val *predicted_per_second = yyjson_obj_get(timings, "predicted_per_second");
            tokenPerSeconds = yyjson_get_real(predicted_per_second);
            endOfCompletation = true;
        }else{
            yyjson_val *delta = yyjson_obj_get(content_array, "delta");
            yyjson_val *content = yyjson_obj_get(delta, "content");
            token = safeGetStr(content);
        }
    }else{
        yyjson_val *content = yyjson_obj_get(root, "content");
        
        token = safeGetStr(content);
        yyjson_val *timings = yyjson_obj_get(root, "timings");
        yyjson_val *predicted_per_second = yyjson_obj_get(timings, "predicted_per_second");

        tokenPerSeconds = yyjson_get_real(predicted_per_second);
        yyjson_val *stop = yyjson_obj_get(root, "stop");
        endOfCompletation = yyjson_get_bool(stop);
    }

    yyjson_doc_free(doc);
    
    // Check end of completation
    if (endOfCompletation) {
        if (!const_cast<CallbackBus*>(bus)->stream){
            std::cout << token;
            const_cast<CallbackBus*>(bus)->buffer+=token;
        }

        // once ends, show token/s in the terminal title
        char windowTitle[30];
        std::snprintf(windowTitle, sizeof(windowTitle), "%.1f t/s\n",
                      tokenPerSeconds);
        Terminal::setTitle(windowTitle);
        completionInProgress = false;
        return false;
    }

    // Think in progress
    if(token=="<think>" && !thinkInProgress){
        thinkInProgress = true;
        Terminal::setTitle("Thinking...");
        std::cout << std::endl << ANSIColors::getColorCode("green_bc");
    }

    // Print the token in terminal, here is where the text stream print happens.
    std::cout << token << std::flush;
    const_cast<CallbackBus*>(bus)->buffer+=token;

    if(token=="</think>" && thinkInProgress){
        thinkInProgress = false;
        Terminal::setTitle("Done");
        std::cout << ANSI_COLOR_RESET;
    }

    return true;
}

// Read parameters from params.json file
bool Completion::loadParametersSettings(std::string_view profile_name) {
    sjson parameters_file = sjson(PARAMS_FILENAME);
    if(!parameters_file.is_opened())
        return false;

    // Choice the profile entry and work with it
    const char *PARAMETER_PROFILE_[] = {"params_profiles", profile_name.data(), "\0"};
    yyjson_val *parameter_profile = parameters_file.get_value(PARAMETER_PROFILE_);
    if(parameter_profile == NULL) return false;
    
    yyjson_val *key, *val;
    yyjson_obj_iter iter = yyjson_obj_iter_with(parameter_profile);

    // reset current parameters
    parameters.clear();

    // Map the json content to parameters
    while ((key = yyjson_obj_iter_next(&iter))) {
        const char* param_key = yyjson_get_str(key);
        yyjson_val *param_value = yyjson_obj_iter_get_val(key);
        std::string value_type = yyjson_get_type_desc(param_value);

        if(value_type == "uint"){
            parameters.set<int>(param_key, yyjson_get_int(param_value));
        }else if(value_type == "real"){
            parameters.set<double>(param_key, yyjson_get_real(param_value));
        }else if(value_type == "string"){
            parameters.set<std::string>(param_key, "\"" + std::string(yyjson_get_str(param_value)) + "\"");
        }else if(value_type == "true"){
            parameters.set<bool>(param_key, true);
        }else if(value_type == "false"){
            parameters.set<bool>(param_key, false);
        }else if(value_type == "array"){
            if(!strcmp(param_key, "stop")){
                size_t idx, max;
                yyjson_val *hit;
                yyjson_arr_foreach(param_value, idx, max, hit)
                        stop_words.push_back(std::string(yyjson_get_str(hit)));
            }
        }
    }

    if(this->parameters.get<bool>("stream") == false)
        completionBus.stream = false;

    return true;
}

// load parameters from params.json
bool Completion::loadChatTemplates(std::string_view chat_template_name) {
    sjson templates_fp = sjson(TEMPLATES_FILENAME);
    if(!templates_fp.is_opened()) return false;

    const char *main_key[] = {"chat_templates", chat_template_name.data(), "\0"};
    
    if(templates_fp.get_value(main_key) == NULL) return false;

    const auto loadSeqToken = [&](const char *key, const char *field_key) {
        const char *keys[] = {"chat_templates", chat_template_name.data(), key, field_key, "\0"};
        const char* key_value = templates_fp.get_str(keys);
        return key_value?key_value:"";
    };

    chat_template.bos =                loadSeqToken("SEQ", "BOS");
    chat_template.begin_system =       loadSeqToken("SEQ", "B_SYS");
    chat_template.end_system =         loadSeqToken("SEQ", "E_SYS");
    chat_template.begin_user =         loadSeqToken("SEQ", "B_USER");
    chat_template.end_user =           loadSeqToken("SEQ", "E_USER");
    chat_template.begin_assistant =    loadSeqToken("SEQ", "B_ASSISTANT");
    chat_template.end_assistant =      loadSeqToken("SEQ", "E_ASSISTANT");
    chat_template.begin_think =        loadSeqToken("SEQ", "B_THINK");
    chat_template.end_think =          loadSeqToken("SEQ", "E_THINK");
    chat_template.eos =                loadSeqToken("SEQ", "EOS");

    return true;
}

// compose json to send
std::string Completion::dumpPayload(yyjson_mut_doc* prompt_json) {
    yyjson_mut_doc *doc = yyjson_mut_doc_new(NULL);
    yyjson_mut_val *root = yyjson_mut_obj(doc);
    yyjson_mut_doc_set_root(doc, root);

    yyjson_mut_val *prompt_root = yyjson_mut_doc_get_root(prompt_json);

    // add the prompt_json to the main json root
    size_t idx, max;
    yyjson_mut_val *key, *val;
    yyjson_mut_obj_foreach(prompt_root, idx, max, key, val) {
        const char* keyStr = yyjson_mut_get_str(key);
        yyjson_mut_obj_add_val(doc, root, keyStr, yyjson_mut_val_mut_copy(doc, val));
    }

    // stop word list
    const char **stop_word_arr = new const char *[stop_words.size()];
    for(int i = 0; i < stop_words.size(); i++)
        stop_word_arr[i] = stop_words[i].c_str();
    yyjson_mut_val *hits = yyjson_mut_arr_with_str(doc, stop_word_arr, stop_words.size());
    yyjson_mut_obj_add_val(doc, root, "stop", hits);

    // others params
    parameters.forEach([&doc, &root](const std::string& key, const std::any& value) {
        if (value.type() == typeid(int)) {
            yyjson_mut_obj_add_int(doc, root, key.c_str(), std::any_cast<int>(value));
        } else if (value.type() == typeid(double)) {
            yyjson_mut_obj_add_real(doc, root, key.c_str(), std::any_cast<double>(value));
        } else if (value.type() == typeid(bool)) {
            yyjson_mut_obj_add_bool(doc, root, key.c_str(), std::any_cast<bool>(value));
        } else if (value.type() == typeid(std::string)) {
            yyjson_mut_obj_add_str(doc, root, key.c_str(), std::any_cast<std::string>(value).c_str());
        }
    }); 

    const char *json = yyjson_mut_write(doc, 0, NULL);
    std::string result(json);
    
    delete[] stop_word_arr;
    yyjson_mut_doc_free(doc);
    free(const_cast<char*>(json));

    return result;
}

Response Completion::requestCompletion(
    const char* ipaddr, const int16_t port, yyjson_mut_doc* prompt_json) 
{
    completionInProgress = true;
    std::string send_json = dumpPayload(prompt_json);
    Logging::debug("Sending: %s", send_json.c_str());

    if(completionInProgress){
        Terminal::setTitle("Completing...");
    }
    return Req.post(
        ipaddr, 
        port, 
        using_oai_completion?OAI_COMPLETION_ENDPOINT:COMPLETION_ENDPOINT, 
        send_json,
        [this](std::string response, const CallbackBus* bus) -> bool {
            return this->completionCallback(response, bus);
        }, 
        &completionBus
    );
}

chat_template_t& Completion::getChatTemplates(){
    return chat_template;
}

void Completion::addStopWord(std::string word){
    stop_words.push_back(word);
}
