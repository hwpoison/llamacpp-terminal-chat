#include "chat.hpp"

int main(int argc, char *argv[]) {
    std::string user_prompt   =     "";         // read from prompts.json
    std::string chat_template =     "";         //           tempaltes.json
    std::string param_profile =     "creative"; //           params.json
    bool chat_mode            =     true;
    bool chat_guards          =     true;
    bool debug                =     false;
    const char *ipaddr        =     DEFAULT_IP;
    int16_t port              =     DEFAULT_PORT;
    std::string save_folder_path =  DEFAULT_SAVE_FOLDER;

    const char *input_arg = nullptr;
    // Handle CLI args
    if ((input_arg = get_arg_value(argc, argv, "--prompt")) != NULL)
        user_prompt = input_arg;

    if ((input_arg = get_arg_value(argc, argv, "--params-profile")) != NULL)
        param_profile = input_arg;

    if ((input_arg = get_arg_value(argc, argv, "--chat-template")) != NULL)
        chat_template = input_arg;

    if ((input_arg = get_arg_value(argc, argv, "--no-chat-tags")) != NULL)
        chat_mode = false;

    if ((input_arg = get_arg_value(argc, argv, "--no-chat-guards")) != NULL)
        chat_guards = false;

    if ((input_arg = get_arg_value(argc, argv, "--ip")) != NULL)
        ipaddr = input_arg;

    if ((input_arg = get_arg_value(argc, argv, "--port")) != NULL)
        port = std::stoi(input_arg);

    if ((input_arg = get_arg_value(argc, argv, "--debug")) != NULL)
        debug = true;

    if ((input_arg = get_arg_value(argc, argv, "--help")) != NULL) {
        printCmdHelp();
        exit(0);
    }
    
    // Setup terminal behaviours
    Terminal::setupEncoding();
    Terminal::setTitle("LLaMA Chat");

    // Initialize Readline
    rl_bind_key('\t', rl_insert);  // Optional: bind Tab to insert
//    history_preserve_point(1);

    // Initialize History
    using_history();

    // Init chat context
    Chat chatContext;
    chatContext.setChatGuards(chat_guards);
    chatContext.setChatMode(chat_mode);

    // Enable/disable debug
    if(!debug)
        Logging::disable_file_logging();

    std::string userInput, director_input;

    // Load prompt template
    if(chat_template.empty()){
        chatContext.using_oai_completion = true;
    }else{
        if(!chatContext.loadChatTemplates(chat_template)){
            Logging::error("Failed to load the custom chat template '%s' ! Using default empty.", chat_template.c_str());
            Terminal::pause();
        }
    }

    // Load user prompt
    if(user_prompt.empty()){
        chatContext.setupSystemPrompt("You are a very helpful assistant.");
        chatContext.setupDefaultActors();
    }else{
        if(!chatContext.loadUserPrompt(user_prompt))
            exit(1);   
    }

    // Load params profile
    if(!chatContext.loadParametersSettings(param_profile)){
        Logging::error("Failed to load the param profile from '%s' ! Please check it.", param_profile.c_str());
        Terminal::pause();
        exit(1);
    }

    chatContext.setupChatStopWords();

    // Control actor talk turn
    bool singleTurn = false;
    std::string previousActingActor;
    std::string currentActor = chatContext.getAssistantName();
    
    Terminal::resetAll();
    

    // Chat loop
    while (true) {
        chatContext.completionBus.buffer.clear();
        chatContext.draw();

        // Control who is talking currently
        if(singleTurn){
            currentActor = previousActingActor;
            singleTurn = false;
        }else{
            previousActingActor = currentActor;
        }

        // No longer print user chat tag. Now, it is printed as part of the readline prompt
        // chatContext.printActorChaTag(chatContext.getUserName());
        // Terminal::resetColor();
        
        // Prepare CTRL+C stop completion signal
        signal(SIGINT, completionSignalHandler);

        std::string actorTag = chatContext.returnActorChaTag(chatContext.getUserName());

	rl_set_prompt(actorTag.c_str());

        char* rl_input = readline(actorTag.c_str());
        if (!rl_input) break;  // Break loop when Ctrl+D encountered
        if (*rl_input) {
            add_history(rl_input);
        }

        userInput = rl_input;

        // Reset cin state during ctrl+c
        if (std::cin.fail() || std::cin.eof()) {
            std::cin.clear(); 
            continue;
        }

        // Start command processing
        if (userInput[0] == '/') {
            size_t space = userInput.find(" ");
            std::string cmd = "";
            std::string arg = "";
            if (space != std::string::npos) {
                cmd = userInput.substr(0, space);
                arg = userInput.substr(space + 1);
            } else {
                cmd = userInput;
            }


            /* ----------------------------------------------------------------- */
            /*                            MISC                                   */
            /* ----------------------------------------------------------------- */
            // help page
            if (cmd == "/help" || cmd == "/h") {
                printChatHelp();
                Terminal::pause();
                continue;

            // redraw terminal content
            }else if (cmd == "/redraw") {
                Terminal::clear();
                continue;

            // exit from program
            } else if (cmd == "/quit" || cmd == "/q") {
                exit(0);

            /* ----------------------------------------------------------------- */
            /*                            FOR ROLEPLAY                           */
            /* ----------------------------------------------------------------- */
            // create and talk with a new actor
            } else if (cmd == "/actor" || cmd == "/now" || cmd == "/a") {
                if(chatContext.using_oai_completion){
                    Logging::error("Not supported with OAI Completion Style");
                    Terminal::pause();
                    continue;
                }
                if(arg.empty()){
                    Logging::error("An actor name is required.");
                    Terminal::pause();
                    continue;
                }
                chatContext.addActor(arg, "assistant", ANSIColors::getRandColor());
                currentActor = arg;

            // act like an actor
            } else if (cmd == "/as") {
                if(chatContext.using_oai_completion){
                    Logging::error("Not supported with OAI Completion Style");
                    Terminal::pause();
                    continue;
                }
                if(arg.empty()){
                    Logging::error("An actor name is required.");
                    Terminal::pause();
                    continue;
                }
                chatContext.addActor(arg, "actor", ANSIColors::getRandColor());
                chatContext.printActorChaTag(arg);
                std::getline(std::cin, director_input);
                chatContext.addNewMessage(arg, director_input);
                continue;

            // talk to an specific actor
            } else if (cmd == "/talkto") {
                if(chatContext.using_oai_completion){
                    Logging::error("Not supported with OAI Completion Style");
                    Terminal::pause();
                    continue;
                }
                chatContext.addActor(arg, "actor", ANSIColors::getRandColor());
                chatContext.printActorChaTag(chatContext.getUserName());
                std::getline(std::cin, director_input);
                chatContext.addNewMessage(chatContext.getUserName(), director_input);
                currentActor = arg;

            // lets narrator describes the current context
            } else if (cmd == "/narrator") {
                if(chatContext.using_oai_completion){
                    Logging::error("Not supported with OAI Completion Style");
                    Terminal::pause();
                    continue;
                }
                std::string narration = "*make a short narration in third person describing the current situation and characters in the chat*";
                chatContext.addActor("Narrator", "system", "yellow", narration);
                currentActor = "Narrator";
                singleTurn = true;

            // view actors 
            } else if (cmd == "/lactors") {
                if(chatContext.using_oai_completion){
                    Logging::error("Not supported with OAI Completion Style");
                    Terminal::pause();
                    continue;
                }
                std::cout << "> Current actors:\n";
                chatContext.listCurrentActors();
                Terminal::pause();
                continue;

            /* ----------------------------------------------------------------- */
            /*                            SAVE & LOAD CONVERSATIONS              */
            /* ----------------------------------------------------------------- */
            // save chat in a file
            } else if (cmd == "/save") {
                if(!std::filesystem::exists(save_folder_path))
                    std::filesystem::create_directory(save_folder_path);

                std::string filename = arg.empty()? "/chat_" + getCurrentDate(): arg;
                if(arg.find(DEFAULT_FILE_EXTENSION)==std::string::npos) filename+=DEFAULT_FILE_EXTENSION;

                if(chatContext.saveConversation(save_folder_path + filename))
                    Logging::success("Conversation saved as '%s'.", filename.c_str());
                else
                    Logging::error("Problem saving the conversation.");       
                Terminal::pause();
                continue;


            // load from file
            } else if (cmd == "/load") {
                std::string filename = arg;
                if(arg.find(DEFAULT_FILE_EXTENSION)==std::string::npos) 
                    filename+=DEFAULT_FILE_EXTENSION;
                if (chatContext.loadSavedConversation(save_folder_path + filename))
                    Logging::success("Conversation from '%s' loaded.", filename.c_str());
                else
                    Logging::error("Failed to load conversation!");
                currentActor = chatContext.getAssistantName();
                Terminal::pause();
                continue;


            /* ----------------------------------------------------------------- */
            /*                            RELOAD CONFIGURATIONS                  */
            /* ----------------------------------------------------------------- */
            // reload params from file
            } else if (cmd == "/rparams") {
                chatContext.loadParametersSettings(param_profile);
                chatContext.setupChatStopWords(); 
                Logging::success("Params '%s' reloaded!", param_profile.c_str());Terminal::pause();
                continue;

            // reload template
            } else if (cmd == "/rtemplate") {
                chatContext.loadChatTemplates(chat_template);
                Logging::success("Template '%s' reloaded", chat_template.c_str());Terminal::pause();
                continue;


            /* ----------------------------------------------------------------- */
            /*                           CHANGE SOME THINGS IN RUNTIME           */
            /* ----------------------------------------------------------------- */
            // load prompt template in runtime
            } else if (cmd == "/stemplate") {
                if(chatContext.loadChatTemplates(arg)){
                    Logging::success("Prompt template '%s' loaded.", arg.c_str());
                    chat_template = arg;
                    chatContext.using_oai_completion = false;
                }else{
                    Logging::error("Failed to load template!");
                }
                Terminal::pause();
                continue;

            // load prompt template in runtime
            } else if (cmd == "/sprompt") {
                if(chatContext.loadUserPrompt(arg)){
                    Logging::success("User prompt '%s' loaded.", arg.c_str());
                    user_prompt = arg;
                }else{
                    Logging::error("Failed to load '%s' user prompt!", arg.c_str());
                }
                currentActor = chatContext.getAssistantName();
                Terminal::pause();
                continue;

            // set new initial system prompt
            } else if (cmd == "/ssystem") {
                chatContext.printActorChaTag("System");
                std::getline(std::cin, userInput);
                chatContext.updateSystemPrompt(userInput);
                continue;

            // toggle show think tokens
            } else if (cmd == "/pthink") {
                if(arg == "on"){
                    chatContext.hiddeThinkTokens(false);
                }
                else if(arg == "off"){
                    chatContext.hiddeThinkTokens(true);
                }else{
                    Logging::warn("Invalid argument: '%s'. Expected 'on' or 'off'.", arg.c_str());
                    Terminal::pause();
                }
                continue;

            // load param profile in runtime
            } else if (cmd == "/sparam") {
                if(chatContext.loadParametersSettings(arg)){
                    chatContext.setupChatStopWords(); 
                    Logging::success("Param profile '%s' loaded", arg.c_str());
                    param_profile = arg;
                }else{
                    Logging::error("Failed to load '%s' param profile!", arg.c_str());
                }
                Terminal::pause();
                continue;

            // switch chat mode (just add tags name for the conversation actors under prompt level, not gui)
            } else if (cmd == "/chat") {
                if(arg == "on"){
                    chatContext.setChatMode(true);
                }
                else if(arg == "off"){
                    chatContext.setChatMode(false);
                }else{
                    Logging::warn("Invalid argument: '%s'. Expected 'on' or 'off'.", arg.c_str());
                    Terminal::pause();
                    continue;
                }
                Logging::success("Chat mode %s", chatContext.isChatMode()?"ON":"OFF");
                Terminal::pause();
                continue;


            /* ----------------------------------------------------------------- */
            /*                           VIEW CONFIGURATION                      */
            /* ----------------------------------------------------------------- */
            // show current prompt
            } else if (cmd == "/lprompt") {
                std::cout << "[Current user prompt]    : " << (user_prompt.empty()?"None":user_prompt) << std::endl;
                std::cout << "[Current params profile] : " << param_profile << std::endl;
                std::cout << "[Current chat template]  : " << (chat_template.empty() ? "OAI mode": chat_template) << std::endl;
                std::cout << "\n+Current prompt content:\n\n```\n" << chatContext.applyChatTemplate() << "\n```\n";
                Terminal::pause();
                continue;
                
            // list current parameters
            } else if (cmd == "/lparams") {
                std::cout << "[Current params profile] : " << param_profile << std::endl;
                std::cout << std::endl << chatContext.dumpPayload(chatContext.getCurrentPrompt()) << "\n\n";
                Terminal::pause();
                continue;

            /* ----------------------------------------------------------------- */
            /*                       MANIPULATE THE CONVERSATION                 */
            /* ----------------------------------------------------------------- */
            // multiline insertion mode
            } else if (cmd == "/insert" || cmd == "/i") {
                // read content from a file
                if(!arg.empty()) {
                    std::string newText = readFileContent(arg);
                    if(newText.empty()){
                        Logging::warn("Error reading the file content");
                        Terminal::pause();
                        continue;
                    }
                    userInput=newText;
                }else{ // enable multiline input mod
                    userInput.clear();
                    Terminal::setTitle("Multiline mode");
                    std::string line;
                    while(1){
                        std::getline(std::cin, line);
                        if(line == "EOL" or line == "eol") break;
                        userInput+= line + "\n";
                    };
                }
                chatContext.addNewMessage(chatContext.getUserName(), userInput);
                chatContext.draw();
                
            // edit assistant previos message
            } else if (cmd == "/edit") {
                chatContext.removeLastMessage(1);
                chatContext.draw();
                chatContext.printActorChaTag(currentActor);
                std::getline(std::cin, userInput);
                chatContext.addNewMessage(currentActor, userInput);
                continue;

            // undo only last message
            } else if (cmd == "/undolast" || cmd == "/ul") {
                chatContext.removeLastMessage(1);
                continue;

            // undo last completion and user message
            } else if (cmd == "/undo" || cmd == "/u") {
                chatContext.removeLastMessage(2);
                continue;

            // reset all chat historial
            } else if (cmd == "/reset" || cmd == "/clear" || cmd == "/cls") {
                chatContext.resetChatHistory();
                currentActor = chatContext.getAssistantName();
                continue;

            // retry last completion
            } else if (cmd == "/retry" || cmd == "/r") {
                chatContext.removeLastMessage(1);
                chatContext.draw();

            // just continue the completation witht the current actor
            } else if (cmd == "/continue" || cmd == "/c") {
            
            // ...
            } else {
                Logging::error("Wrong command! Use '/help' for chat commands usage.");
                Terminal::pause();
                continue;
            }

        } else {
            chatContext.addNewMessage(chatContext.getUserName(), userInput);
        }

        // Print current actor chat tag
        chatContext.printActorChaTag(currentActor);

        // Append the message to the prompt
        if(!chatContext.using_oai_completion) chatContext.addNewMessage(currentActor, ""); 

        // Send for completion
        Terminal::setTitle("Thinking...");
        Response res = chatContext.requestCompletion(ipaddr, port, chatContext.getCurrentPrompt());
        if (res.Status != HTTP_OK) {
            if (res.Status == HTTP_INTERNAL_ERROR)
                Logging::critical("500 Internal server error!");
            if (res.Status == -1)
                Logging::error("Error to connect, please check that server is running and try again.");
            if(!res.body.empty()) Logging::error(std::string("\nServer response body:" + res.body).c_str());
            Terminal::setTitle("Completion in error.");
            Terminal::pause();
            chatContext.removeLastMessage(2);
        }else{
            chatContext.cureCompletionForChat();
            if(chatContext.using_oai_completion) chatContext.addNewMessage(currentActor, "");
            chatContext.updateMessageContent(chatContext.messagesCount()-1, chatContext.completionBus.buffer);
        }
    }

    atexit(Terminal::resetColor);
    return 0;
}
