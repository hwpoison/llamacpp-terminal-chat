# 🦙 llama.cpp/server Terminal Client

This is a lightweight terminal interface alternative for llama.cpp server following an outline of a chat script.

* Many commands for manipulate the conversation flow and also debug it.
* Chat templates and personal prompts can be created and customized.
* Load/Save conversation system.
* Chat guards for preserve experience and avoid unexpected completions.
* Open AI completion support
* Compilable for Windows and Linux.

*link to llama.cpp project server:* https://github.com/ggerganov/llama.cpp/tree/master/examples/server

![Chat style view](/screenshots/screenshot.png)

Chat templates can be added and customized.

![Chat style view](/screenshots/prompt_templates.png)
![Chat style view](/screenshots/screenshot2.png)

## How to Use
#### Configuration files:
* **prompts.json**  Contains all personal prompts definitions. Supports system prompt and an actor system.
* **params.json**  All prompt configurations profiles.
* **templates.json**  All chat templates.

#### Execultable Arguments:
-  --prompt *prompt name*            (default: default)
-  --param-profile *profile name*       (default: default)
-  --chat-template *prompt template*  (default: None)
-  --no-chat-tags      Disable chat style actors tags (ex:  'User:' 'Someone:')
-  --no-chat-guards: Disable the chat guards (default: false)
-  --ip <ip address>                    (default: 127.0.0.1)
-  --port <port>                        (default: 8080)
-  --debug Debug messages into log file (default: false)

#### Command support:
To input a command, simply insert `/command` followed by the desired command.

#### Command List:
 📝Conversation manipulation:
- **narrator**: Lets to narrator generate a narration.
- **actor** or **now** Choice who will talk now. If doesn't exists it will be created. (ie: /now  Einstein)
- **as**: Pretend to be an actor and prompt it. (ex: /as Einstein)
- **talkto**: Talk to a determinated character. It will swtich the current talking actor. (ie: /talkto Monica)
- **insert** or **i**: Multiline mode mode, write "EOL" or "eol" then enter to submit the content. Also if you prefer, can specify a file name to read and insert the content directly.
- **retry** or **r**: Retry the last completion.
- **continue**: Continue the completion without intervention. (The assistant will continue talking)
- **edit**: Edit the assistant last message to re-submit it.
- **undolast**: Undo only the last completion.
- **undo** or **u**: Undo the last completion and user input.

🗣️Conversation mode:
- **chat on/off:** Turn on/off chat tags.
- **pthink on/off:** Show/hidde thoughts during model reasoning (default: hidden).

💾Conversation saving:
- **save (chatname):** Save the chat. (without extension)
- **load (chatname):** Load a previous saved chat.

⚙️Manage configurations:
- **help** Get commands help (this page)
- **redraw:** Redraw the chat content.
- **reset:** Reset the entire chat.
- **quit** or **q**: Exit the program.
- **lprompt:** Print the current prompt that will be send.
- **lactors:** Print current actors.
- **lparams:** Print the current parameters.
- **rparams** Reload current parameter profile.
- **rtemplate** Reload current template profile.
- **sparam (parameter profile name)** Load and set param profile in runtime from param.json.
- **stemplate (template name)** Load and set prompt template in runtime from template.json.
- **ssystem (input new line)** Set new system prompt (from begin).
- **sprompt (prompt name)** Load and set custom prompt in runtime from prompt.json.

#### 💂About the chat guards:
The chat guards adds the prompt template tokens into stop words array.

#### 🖋️About OpenAI Completion style
With this mode, chat tags are not supported also messages preffix. The chat template is choiced by the server based on built in model specifications.

#### ✂️Shortcut:
You can stop the completion using CTRL+C signal.

#### 🧱Instructions to build

    git submodule init
    git submodule update
    make static

## 🧾**Tested on**

<table><tbody><tr><td>Windows</td><td>GCC</td><td>g++ (x86_64-win32-seh-rev0, Built by MinGW-Builds project) 14.2.0</td></tr><tr><td>Linux</td><td>GCC</td><td>gcc (Ubuntu 9.4.0-1ubuntu1~20.04.1) 9.4.0</td></tr></tbody></table>
