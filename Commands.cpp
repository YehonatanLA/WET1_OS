#include <unistd.h>
#include <string.h>
#include <iostream>
#include <vector>
#include "signal.h"
#include <sstream>
#include <sys/wait.h>
#include <iomanip>
#include "Commands.h"
#include <time.h>
#include <utime.h>
#include <cstring>


using namespace std;

#if 0
#define FUNC_ENTRY()  \
  cout << __PRETTY_FUNCTION__ << " --> " << endl;

#define FUNC_EXIT()  \
  cout << __PRETTY_FUNCTION__ << " <-- " << endl;
#else
#define FUNC_ENTRY()
#define FUNC_EXIT()
#define LAST_JOB -1

#endif


const std::string WHITESPACE = " \n\r\t\f\v";

string _ltrim(const std::string &s) {
    size_t start = s.find_first_not_of(WHITESPACE);
    return (start == std::string::npos) ? "" : s.substr(start);
}

string _rtrim(const std::string &s) {
    size_t end = s.find_last_not_of(WHITESPACE);
    return (end == std::string::npos) ? "" : s.substr(0, end + 1);
}

string _trim(const std::string &s) {
    return _rtrim(_ltrim(s));
}

int _parseCommandLine(const char *cmd_line, char **args) {
    FUNC_ENTRY()
    int i = 0;
    std::istringstream iss(_trim(string(cmd_line)).c_str());
    for (std::string s; iss >> s;) {
        args[i] = (char *) malloc(s.length() + 1);
        memset(args[i], 0, s.length() + 1);
        strcpy(args[i], s.c_str());
        args[++i] = NULL;
    }
    return i;

    FUNC_EXIT()
}

bool _isBackgroundCommand(const char *cmd_line) {
    const string str(cmd_line);
    return str[str.find_last_not_of(WHITESPACE)] == '&';
}

void _removeBackgroundSign(char *cmd_line) {
    const string str(cmd_line);
    // find last character other than spaces
    unsigned int idx = str.find_last_not_of(WHITESPACE);
    // if all characters are spaces then return
    if (idx == string::npos) {
        return;
    }
    // if the command line does not end with & then return
    if (cmd_line[idx] != '&') {
        return;
    }
    // replace the & (background sign) with space and then remove all tailing spaces.
    cmd_line[idx] = ' ';
    // truncate the command line string up to the last non-space character
    cmd_line[str.find_last_not_of(WHITESPACE, idx) + 1] = 0;
}

// TODO: Add your implementation for classes in Commands.h 

SmallShell::SmallShell() {
// TODO: add your implementation
}

SmallShell::~SmallShell() {
// TODO: add your implementation
}

/**
* Creates and returns a pointer to Command class which matches the given command line (cmd_line)
*/
Command *SmallShell::CreateCommand(const char *cmd_line) {
    // For example:

    string cmd_s = _trim(string(cmd_line));
    string firstWord = cmd_s.substr(0, cmd_s.find_first_of(" \n"));

    if (firstWord.compare("chprompt") == 0 || firstWord.compare("chprompt&") == 0) {
        //? check if needed to check in if the &
        saveChangePrompt(cmd_line);

    } else if (firstWord.compare("jobs") == 0 || firstWord.compare("jobs&") == 0) {
        //? check if needed to check in if the &
        return new JobsCommand(cmd_line, &extra_jobs);

    } else if (firstWord.compare("fg") == 0 || firstWord.compare("fg&") == 0) {
        //? check if needed to check in if the &
        char *args[COMMAND_MAX_ARGS];
        int job_number = checkSyntax(cmd_line, args);
        if (job_number < LAST_JOB) {
            std::cout << "smash error: fg: invalid arguments" << std::endl;
            throw std::exception(); //? or return nullptr
        }
        return new ForegroundCommand(cmd_line, &extra_jobs, job_number);

    } else if (firstWord.compare("bg") == 0 || firstWord.compare("bg&") == 0) {
        //? check if needed to check in if the &
        char *args[COMMAND_MAX_ARGS];
        int job_number = checkSyntax(cmd_line, args);
        if (job_number < LAST_JOB) {
            std::cout << "smash error: bg: invalid arguments" << std::endl;
            throw std::exception(); //? or return nullptr
        }
        return new BackgroundCommand(cmd_line, &extra_jobs, job_number);

    }
    /*
    if (firstWord.compare("pwd") == 0) {
      return new GetCurrDirCommand(cmd_line);
    }
    else if (firstWord.compare("showpid") == 0) {
      return new ShowPidCommand(cmd_line);
    }
    else if ...
    .....
    else {
      return new ExternalCommand(cmd_line);
    }
    */
    return nullptr;
}

void SmallShell::executeCommand(const char *cmd_line) {
    // TODO: Add your implementation here
    // for example:
    // Command* cmd = CreateCommand(cmd_line);
    // cmd->execute();
    // Please note that you must fork smash process for some commands (e.g., external commands....)

    Command *cmd = CreateCommand(cmd_line);
    //! if this is a external command, we need to make sure that we are forking and execv ing the process (and something about setpgrp)
    cmd->execute();
}


void JobsCommand::execute() {
    //the function needs to print out all the jobs in the bg or the ones that stopped.
    extra_jobs->printJobsList();
}

void ForegroundCommand::execute() {
//the function brings either a bg process or a stopped process to fg
    if (this->jobId != LAST_JOB && !jobs_list->jobExists(this->jobId)) {
        std::cout << "smash error: fg: job-id " << this->jobId <<" does not exist" << endl;
        return;
    } else if (this->jobId == LAST_JOB && jobs_list->isEmpty()) {
        cout << "smash error: fg: jobs list is empty" << endl;
        return;
    }
    JobsList::JobEntry *chosen_job;

    if (this->jobId == LAST_JOB) {
        chosen_job = jobs_list->getLastJob(nullptr);
        this->jobId = chosen_job->getJobId();
    } else {
        chosen_job = jobs_list->getJobById(this->jobId);
    }

    if (chosen_job->getStopped()) {
        // ? send SIGCONT, not sure how to do

    }
    std::cout << chosen_job->getCmdInput() << " : " << chosen_job->getPid() << "\n";
    waitpid(chosen_job->getPid(), nullptr); //? should I care about the status?
    jobs_list->removeJobById(jobId);


}


void BackgroundCommand::execute() {
    //the function brings a stopped process to bg
    if (this->jobIdBackground != LAST_JOB && !jobs_list_background->jobExists(this->jobIdBackground)) {
        std::cout << "smash error: bg: job-id " << this->jobIdBackground <<" does not exist" << endl;
        return;
    } else if (this->jobIdBackground == LAST_JOB && jobs_list_background->isEmpty()) {
        cout << "smash error: bg: jobs list is empty" << endl;
        return;
    }
    JobsList::JobEntry* stopped_job;

    if(jobIdBackground == LAST_JOB) {
        try {
            stopped_job = jobs_list_background->getLastStoppedJob();
            this->jobIdBackground = stopped_job->getJobId();
        }
        catch(std::exception&){
            cout << "smash error: bg: there is no stopped jobs to resume" << endl;
            return;
        }
    }
    else{
        stopped_job = jobs_list_background->getJobById(this->jobIdBackground);
    }
    cout << stopped_job->getCmdInput() << ": " << this->jobIdBackground;
     stopped_job->ChangeStopped();
    // ? send SIGCONT, not sure how to do

}

const std::string &SmallShell::getCurrPrompt() {
    return curr_prompt;
}

void SmallShell::setCurrPrompt(const string &s) {
    this->curr_prompt = s;
}

void SmallShell::saveChangePrompt(const char *cmd) {
    char *args[COMMAND_MAX_ARGS];
    int num_args = _parseCommandLine(cmd, args);
    SmallShell &smash = SmallShell::getInstance();

    if (num_args == 1) {
        smash.setCurrPrompt(smash.default_prompt);
    } else {
        std::string new_prompt = string(args[1]).append(string("> ")); //? check if works
        smash.setCurrPrompt(new_prompt);
    }
}

bool SmallShell::isNumber(char *string) {
    size_t len = strlen(string);
    for (int i = 0; i < len; i++) {
        if (!isdigit(string[i])) {
            return false;
        }
    }
    return true;
}


int SmallShell::checkSyntax(const char *line, char **args) {
    int job;
    int args_amount = _parseCommandLine(line, args);

    if (args_amount == 2 && isNumber(args[1])) {
        job = int(args[1]);
    } else if (args_amount == 1) {
        job = LAST_JOB;
    } else {
        job = LAST_JOB - 1; // * not a valid job number
    }
    return job;
}

void JobsList::addJob(Command *cmd, bool isStopped) {
    //the function receives a command and if the proccess stopped and puts it in the jobs list
    removeFinishedJobs();
    if (jobs.size() == 100) {
        //! problem, not sure what to do
        throw std::exception();
    }
    //? should I check if the job is added again? in page 7 it says to reset timer, but how do I check if it was added in the first place?
    // * unless of course it is meant that I add the same job while it is already in job list, somehow
    max_jobId++;
    // * check if needed to copy string
    JobEntry new_job = JobEntry(cmd->getPid(), max_jobId, cmd->getLine(), isStopped, time(nullptr));
    job_ids[max_jobId] = jobs.insert(jobs.end(), &new_job); // hash[jobid] = pointer to the new element in linked list


}

void JobsList::printJobsList() {
    //the function prints all the jobs in bg or that stopped based on the
    removeFinishedJobs();
    for (auto &job: jobs) {
        int curr_job_id = job->job_id;
        const char *input = job->cmd_input;
        int curr_pid = job->pid;
        time_t status_time = time(nullptr);
        cout << "[" << curr_job_id << "] " << input << " :" << curr_pid << difftime(status_time, job->time_entered);

        if (job->stopped) {
            cout << " (stopped)";
        }
        cout << "\n";
    }
}

void JobsList::removeFinishedJobs() {
    //The function removes all jobs from linked list and hash that finished

    auto it = jobs.begin();
    int kid_pid = waitpid(-1, nullptr, WNHOANG);
    while (kid_pid > 0) {

        // * two ways of going with this - either choose a way to convert pid to jobid (let's say pid->jobid hash) and use the hash,
        // * or delete by naive way
        int job_id = findJobId(kid_pid);
        removeJobById(job_id);
        kid_pid = waitpid(-1, nullptr, WNHOANG);
    }
}

JobsList::JobEntry *JobsList::getJobById(int jobId) {
    //! maybe need to handle errors like non-existing jobId or empty list
    if (job_ids.find(jobId) == job_ids.end()) {

        throw std::exception(); //! error, not a valid job id
    }
    std::list<JobEntry *>::iterator it = job_ids[jobId];
    return *it;

}

void JobsList::removeJobById(int jobId) {
    //! maybe need to handle errors like non-existing jobId or empty list
    if (job_ids.find(jobId) == job_ids.end()) {
        throw std::exception(); //! error, not a valid job id
    }
    //? should I free the data before erasing?
    std::list<JobEntry *>::iterator it = job_ids[jobId];
    jobs.erase(it);
    job_ids.erase(jobId);

}

JobsList::JobEntry *JobsList::getLastJob(int *lastJobId) {
    //! maybe need to handle errors like non-existing jobId or empty list
    JobEntry *last_entry = jobs.back();
    return last_entry;
}

JobsList::JobEntry *JobsList::getLastStoppedJob() {
    for (auto rit = jobs.rbegin(); rit != jobs.rend(); ++rit) { //going through list backwards
        if ((*rit)->stopped) {
            return *rit;
        }
    }
    //! no job has stopped, throw error
    throw std::exception();
}

int JobsList::findJobId(int pid) {
    for (auto &job: jobs) {
        if (job->pid == pid) {
            return job->job_id;
        }
    }
    //! should not get here, must mean pid not a process running in bg or stopped
    throw std::exception();
}

JobsList::JobsList() {
    max_jobId = 0;
    jobs = list<JobEntry *>();
    job_ids = unordered_map<int, std::list<JobEntry *>::iterator>();
}

bool JobsList::isEmpty() const {
    return jobs.empty();
}

bool JobsList::jobExists(int jobId) {
    if (job_ids.find(jobId) == job_ids.end())
        return false;

    return true;
}

int Command::getPid() const {
    return pid;
}

const char *Command::getLine() {
    return cmd_line;
}

JobsList::JobEntry::JobEntry(int pid, int job_id, const char *cmd_input, bool stopped, time_t start_time) : pid(pid),
                                                                                                            job_id(job_id),
                                                                                                            cmd_input(
                                                                                                                    cmd_input),
                                                                                                            stopped(stopped),
                                                                                                            time_entered(
                                                                                                                    start_time) {}

int JobsList::JobEntry::getPid() const {
    return pid;
}

bool JobsList::JobEntry::getStopped() const {
    return stopped;
}

const char *JobsList::JobEntry::getCmdInput() {
    return cmd_input;
}

int JobsList::JobEntry::getJobId() const {
    return job_id;
}

void JobsList::JobEntry::ChangeStopped() {
    stopped = 1 - stopped;
}

BuiltInCommand::BuiltInCommand(const char *cmd_line) : Command(cmd_line) {}

JobsCommand::JobsCommand(const char *cmdLine, JobsList *jobs) : BuiltInCommand(cmdLine), extra_jobs(jobs) {

}

ForegroundCommand::ForegroundCommand(const char *cmd_line, JobsList *jobs, int job_number) : BuiltInCommand(cmd_line),
                                                                                             jobs_list(jobs),
                                                                                             jobId(job_number) {}

BackgroundCommand::BackgroundCommand(const char *cmd_line, JobsList *jobs, int job_number) :
        BuiltInCommand(cmd_line),
        jobs_list_background(jobs), jobIdBackground(job_number) {}
