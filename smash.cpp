#include <iostream>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <string>
#include "Commands.h"
#include "signals.h"

#define CHPROMPT 7
std::string changePrompt(std::string chprompt_line, bool * cmd_is_chprompt);


int main(int argc, char* argv[]) {
    if(signal(SIGTSTP , ctrlZHandler)==SIG_ERR) {
        perror("smash error: failed to set ctrl-Z handler");
    }
    if(signal(SIGINT , ctrlCHandler)==SIG_ERR) {
        perror("smash error: failed to set ctrl-C handler");
    }

    //TODO: setup sig alarm handler

    SmallShell& smash = SmallShell::getInstance();
    bool cmd_is_chprompt = false;
    std::string new_prompt = NULL;
    while(true) {

        if(cmd_is_chprompt){
            std::cout << new_prompt;
        }

        else{
            std::cout << "smash> ";
        }
        std::string cmd_line;
        std::getline(std::cin, cmd_line);

        std::string cmd_s = _trim(std::string(cmd_line));

        std::string firstWord = cmd_s.substr(0, CHPROMPT); // ? check if includes last character
        if(firstWord.compare("chprompt") == 0){
            new_prompt = changePrompt(cmd_s , &cmd_is_chprompt);
            continue;
        }

        smash.executeCommand(cmd_line.c_str());
    }
    return 0;
}

std::string changePrompt(std::string chprompt_line, bool * cmd_is_chprompt)
{
    int length = chprompt_line.length();
    std::string second_word = nullptr;

    int first_space = chprompt_line.find_first_of(' ');
    if (first_space < length && chprompt_line[CHPROMPT + 1] == ' ')
    {
        int second_word_starts = chprompt_line.find_first_not_of(' ', CHPROMPT + 1);
        second_word = chprompt_line.substr(CHPROMPT + 1, chprompt_line.find_first_of("' '\n", second_word_starts));
        *cmd_is_chprompt = true;
    }
    else{
        cmd_is_chprompt = false;
    }

    return second_word;
}