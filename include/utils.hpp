#ifndef __UTILS_H__
#define __UTILS_H__
#include <iostream>
#include <algorithm>
#include <cstring>
#include <any>
#include <map> 
#include <ctime>
#include <cstdint>
#include <fstream>

std::string normalizeText(std::string_view text);

std::string toUpperCase(std::string_view input);

std::string toLowerCase(std::string_view input);

void removeBreakLinesAtStart(std::string& str);

const std::string getCurrentDate();

int64_t getTimeStamp();

const char *get_arg_value(int argc, char **argv, const char *target_arg);

std::string highLightText(std::string text, std::string color, std::string start, std::string end);

std::string readFileContent(const std::string& file_name);

void printCmdHelp();

void printChatHelp();

// a dict like python
template<typename keyT>
class Dict {
public:
    template<typename valueT>
    void set(keyT key, valueT value){
        vals[key] = value;
    }
    
    template<typename valueT>
    valueT get(keyT key){
        return std::any_cast<valueT>(vals[key]);
    }

    template<typename Function>
    void forEach(Function func){
        for (auto& pair : vals) {
            func(pair.first, pair.second);
        }
    }

    void clear(){
        vals.clear();
    }

private:
    std::map<keyT, std::any> vals;
};

#endif
