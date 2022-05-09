#include "Commands.h"
#include <fcntl.h>

using namespace std;

#if 0
#define FUNC_ENTRY()  \
  cout << __PRETTY_FUNCTION__ << " --> " << endl;

#define FUNC_EXIT()  \
  cout << __PRETTY_FUNCTION__ << " <-- " << endl;
#else
#define FUNC_ENTRY()
#define FUNC_EXIT()
#define LAST_JOB (-1)
#define MAX_PROCESSES_AMOUNT (100)
#define KILL_SIGNAL_NUM (9)

#define NO_PID (-1)
#define TAIL_MAX_AMOUNT (3)
#define TAIL_MIN_AMOUNT (2)
#define DEFAULT_LINE_AMOUNT (10)


#endif


const string WHITESPACE = " \n\r\t\f\v";

string _ltrim(const string &s) {
    size_t start = s.find_first_not_of(WHITESPACE);
    return (start == string::npos) ? "" : s.substr(start);
}

string _rtrim(const string &s) {
    size_t end = s.find_last_not_of(WHITESPACE);
    return (end == string::npos) ? "" : s.substr(0, end + 1);
}

string _trim(const string &s) {
    return _rtrim(_ltrim(s));
}

int _parseCommandLine(const char *cmd_line, char **args) {
    FUNC_ENTRY()
    int i = 0;
    istringstream iss(_trim(string(cmd_line)).c_str());
    for (string s; iss >> s;) {
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

int findSign(const char *cmd_line, string sign) {
    char *args[COMMAND_MAX_ARGS];
    int size = _parseCommandLine(cmd_line, args);

    for (int i = 0; i < size; i++) {
        if (!strcmp(args[i], sign.c_str()))
            return i;
    }
    return -1;
}

/**
* Creates and returns a pointer to Command class which matches the given command line (cmd_line)
*/
Command *SmallShell::CreateCommand(const char *new_cmd) {
    // For example:
    const char *cmd_line = (char *) malloc(sizeof(char) * strlen(new_cmd));
    strcpy((char *) cmd_line, new_cmd);
    string cmd_s = _trim(string(cmd_line));
    string firstWord = cmd_s.substr(0, cmd_s.find_first_of(" \n"));

    if (findSign(cmd_line, ">") >= 0 || findSign(cmd_line, ">>") >= 0) {
        return new RedirectionCommand(cmd_line);
    } else if (findSign(cmd_line, "|") >= 0 || findSign(cmd_line, "|&") >= 0) {
        return new PipeCommand(cmd_line);
    } else if (firstWord == "chprompt") {

        saveChangePrompt(cmd_line);

    } else if (firstWord == "jobs") {
        return new JobsCommand(cmd_line, &extra_jobs);

    } else if (firstWord == "fg") {
        return new ForegroundCommand(cmd_line, &extra_jobs, LAST_JOB);

    } else if (firstWord == "bg") {
        return new BackgroundCommand(cmd_line, &extra_jobs);

    } else if (firstWord == "showpid") {
        return new ShowPidCommand(cmd_line);
    } else if (firstWord == "pwd") {
        return new GetCurrDirCommand(cmd_line);
    } else if (firstWord == "cd") {
        return new ChangeDirCommand(cmd_line, pLastPwd);
    } else if (firstWord == "kill") {
        return new KillCommand(cmd_line, &extra_jobs);
    } else if (firstWord == "quit") {
        return new QuitCommand(cmd_line, &extra_jobs);
    } else if (firstWord == "tail") {
        return new TailCommand(cmd_line);
    } else if (firstWord == "touch") {
        return new TouchCommand(cmd_line);
    }  else if (firstWord == "") {
        return new EmptyCommand(cmd_line);
    } else {
        //must be an externalCommand
        return new ExternalCommand(cmd_line);
    }

    return nullptr;
}

void SmallShell::executeCommand(const char *cmd_line) {
    // TODO: Add your implementation here
    // for example:
    // Command* cmd = CreateCommand(cmd_line);
    // cmd->execute();
    // Please note that you must fork smash process for some commands (e.g., external commands....)

    curr_cmd = CreateCommand(cmd_line);
    if (cmdIsChprompt(cmd_line)) {
        return;
    }
    //! if this is a external command, we need to make sure that we are forking and execv ing the process (and something about setpgrp)
    curr_cmd->execute();
    curr_cmd = nullptr;
}



void JobsCommand::execute() {
    //the function needs to print out all the jobs in the bg or the ones that stopped.
    extra_jobs->printJobsList();
}

void ForegroundCommand::execute() {
//the function brings either a bg process or a stopped process to fg

    char *args[COMMAND_MAX_ARGS];
    int job_number = SmallShell::checkSyntaxForeGroundBackground(cmd_line, args);
    if (job_number < LAST_JOB) {
        cerr << "smash error: fg: invalid arguments" << endl;
        return;
    }
    this->jobId = job_number;
    int args_amount = _parseCommandLine(cmd_line, args);
    bool is_last = (args_amount == 1);

    if (!is_last && !jobs_list_fg->jobExists(this->jobId)) {

        cerr << "smash error: fg: job-id " << this->jobId << " does not exist" << endl;
        return;
    } else if (is_last && jobs_list_fg->isEmpty()) {
        cerr << "smash error: fg: jobs list is empty" << endl;
        return;
    }
    JobsList::JobEntry *chosen_job;

    if (is_last) {
        chosen_job = jobs_list_fg->getLastJob(nullptr);
        this->jobId = chosen_job->getJobId();
    } else {
        chosen_job = jobs_list_fg->getJobById(this->jobId);
    }

    if (chosen_job->getStopped()) {
        int ret = kill(chosen_job->getJobPid(), SIGCONT);
        if (ret < 0) {
            // * failed to send sigcont to prcess
            perror("smash error: kill failed");
        }

    }
    SmallShell& smash = SmallShell::getInstance();
    smash.curr_cmd->setPid(chosen_job->getJobPid());
    smash.curr_cmd->setLine(chosen_job->getCmdInput());
    cout << chosen_job->getCmdInput() << " : " << chosen_job->getJobPid() << "\n";
    waitpid(chosen_job->getJobPid(), nullptr, WUNTRACED);
    jobs_list_fg->removeJobById(jobId);


}


void BackgroundCommand::execute() {
    char *args[COMMAND_MAX_ARGS];
    int job_number = SmallShell::checkSyntaxForeGroundBackground(cmd_line, args);
    if (job_number < LAST_JOB) {
        cerr << "smash error: bg: invalid arguments" << endl;
        return; //? or return nullptr
    }
    int args_amount = _parseCommandLine(cmd_line, args);
    bool is_last = (args_amount == 1);
    //the function brings a stopped process to bg
    if (!is_last && !jobs_list_background->jobExists(job_number)) {
        cerr << "smash error: bg: job-id " << job_number << " does not exist" << endl;
        return;
    } else if (is_last && jobs_list_background->isEmpty()) {
        cerr << "smash error: bg: there is no stopped jobs to resume" << endl;
        return;
    }
    JobsList::JobEntry *stopped_job;

    if (job_number == LAST_JOB) {
        try {
            stopped_job = jobs_list_background->getLastStoppedJob();
        }
        catch (exception &) {
            cerr << "smash error: bg: there is no stopped jobs to resume" << endl;
            return;
        }
    } else {
        stopped_job = jobs_list_background->getJobById(job_number);
        if (!stopped_job->getStopped()) {
            cerr << "smash error: bg: job-id " << stopped_job->getJobId() << " is already running in the background"
                 << endl;
            return;
        }
    }
    cout << stopped_job->getCmdInput() << " : " << stopped_job->getJobPid() << endl;
    stopped_job->ChangeStopped();
    int ret = kill(stopped_job->getJobPid(), SIGCONT);
    if (ret < 0) {
        // * failed to send sigcont to prcess
        perror("smash error: kill failed");
    }

}

void ExternalCommand::execute() {
    char *args[COMMAND_MAX_ARGS];
    _parseCommandLine(cmd_line, args);
    pid_t c_pid = fork();
    if (c_pid == -1) {
        perror("smash error: fork failed");
    }

    char *run_in_bash = (char *) malloc(strlen(cmd_line));
    strcpy(run_in_bash, cmd_line);
    _removeBackgroundSign(run_in_bash);

    if (c_pid == 0) {
        setpgrp();
        //we're in the child process
        char *const args_for_bash[4] = {(char *) "/bin/bash", (char *) "-c", run_in_bash, nullptr};
        execv(args_for_bash[0], args_for_bash);
        //if this returned, something wrong happened
        throw exception();
    } else {
        SmallShell &smash = SmallShell::getInstance();
        //we're in the parent process
        if (_isBackgroundCommand(cmd_line)) {
            //runs in background
            pid = int(c_pid); //changing the default pid to relevant pid
            setPid(pid);
            smash.extra_jobs.addJob(this, false);
        } else {
            //runs in the foreground
            pid = int(c_pid);
            waitpid(c_pid, nullptr, WUNTRACED); //? should I save the status? Also, should I save ethe return value?
        }
    }
}

void TailCommand::execute() {
    char *args[COMMAND_MAX_ARGS];
    bool valid;
    int args_num;
    size_t lines_from_end = checkSyntaxTail(cmd_line, args, &valid);
    args_num = _parseCommandLine(cmd_line, args);
    if (!valid) {
        cerr << "smash error: tail: invalid arguments" << endl;
        return;
    }

    size_t i, count;
    string file_name = _trim(args[args_num - 1]).c_str();
    try {
        count = count_lines((char *) file_name.c_str());
    }
    catch (std::exception &) {
        return; //? already printed the error, so is there something to do here?
    }

    if (count < lines_from_end) {
        lines_from_end = count;
    }
    string line;
    int fd;
    if((fd = open(file_name.c_str(), O_RDONLY)) < 0){
        perror("smash error: open failed");
        return;
    }
    for (i = 0; i < count - lines_from_end; ++i) {
        readLine(fd, line); /* read and discard: skip line */
        line.clear();
    }

    while (readLine(fd, line)) {
        cout << line << endl;
        line.clear();
    }
    if(close(fd) < 0){
        perror("smash error: close failed");
        return;
    }

}

void TouchCommand::execute() {
    char *args[COMMAND_MAX_ARGS];
    bool valid;
    int args_num;
    valid = checkSyntaxTouch(cmd_line, args);
    args_num = _parseCommandLine(cmd_line, args);

    if (!valid) {
        cerr << "smash error: touch: invalid arguments" << endl;
        return;
    }
    string time_str = _trim(args[args_num - 1]);
    const char *file_str = _trim(args[1]).c_str();
    std::string delimiter = ":";
    size_t curr_index;
    int time_arr[6];
    for (int &i: time_arr) {
        curr_index = time_str.find(delimiter);
        i = stoi(time_str.substr(0, curr_index));
        time_str = time_str.substr(curr_index + 1);

    }
    struct tm new_file_time = {
            .tm_sec = time_arr[0],
            .tm_min = time_arr[1],
            .tm_hour = time_arr[2],
            .tm_mday = time_arr[3],
            .tm_mon = time_arr[4] - 1,
            .tm_year = time_arr[5] - 1900,
            .tm_wday = 0,
            .tm_yday = 0,
            .tm_isdst = 0
    };

    tm *time_info = &new_file_time;
    time_t time_ret = mktime(time_info);
    if (time_ret == -1) {
        perror("smash error: mktime failed");
        return;
    }
    const struct utimbuf time_buf = {time_ret, time_ret};
    int ret_utime = utime(file_str, &time_buf);
    if (ret_utime == -1) {
        perror("smash error: utime failed");
        return;
    }


}

const string &SmallShell::getCurrPrompt() {
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
        string new_prompt = string(args[1]).append(string("> ")); //? check if works
        smash.setCurrPrompt(new_prompt);
    }
}

bool SmallShell::isNumber(char *string) {
    if(string[0] != '-' && !isdigit(string[0])){
        return false;
    }
    size_t len = strlen(string);
    for (size_t i = 1; i < len; i++) {
        if (!isdigit(string[i])) {
            return false;
        }
    }
    return true;
}

int SmallShell::checkSyntaxForeGroundBackground(const char *line, char **args) {
    int job;
    int args_amount = _parseCommandLine(line, args);

    if (args_amount == 2 && isNumber((char*)_trim(args[1]).c_str())) {
        job = atoi(args[1]);
    } else if (args_amount == 1) {
        job = LAST_JOB;
    } else {
        job = LAST_JOB - 1; // * not a valid job number
    }
    return job;
}

bool SmallShell::cmdIsChprompt(const char *line) {
    string cmd_s = _trim(string(line));
    string firstWord = cmd_s.substr(0, cmd_s.find_first_of(" \n"));
    return firstWord == "chprompt";

}

void JobsList::addJob(Command *cmd, bool isStopped) {
    //the function receives a command and if the proccess stopped and puts it in the jobs list
    removeFinishedJobs();
    if (jobs.size() == MAX_PROCESSES_AMOUNT) {
        //! problem, not sure what to do
        throw exception();
    }
    //? should I check if the job is added again? in page 7 it says to reset timer, but how do I check if it was added in the first place?
    // * unless of course it is meant that I add the same job while it is already in job list, somehow
    max_jobId = findMax();
    // * check if needed to copy string
    JobEntry *new_job = new JobEntry(cmd->getCommandPid(), max_jobId, cmd->getLine(), isStopped, time(nullptr));
    //! need to delete the jobEntry in destructor for clearing memory
    job_ids[max_jobId] = jobs.insert(jobs.end(), new_job); // hash[jobid] = pointer to the new element in linked list


}

void JobsList::printJobsList() {
    //the function prints all the jobs in bg or that stopped based on the
    removeFinishedJobs();
    for (auto &job: jobs) {
        int curr_job_id = job->job_id;
        const char *input = job->cmd_input;
        int curr_pid = job->pid;
        time_t status_time = time(nullptr);
        cout << "[" << curr_job_id << "] " << input << " : " << curr_pid << " "
             << difftime(status_time, job->time_entered) << " secs";

        if (job->stopped) {
            cout << " (stopped)";
        }
        cout << "\n";
    }
}

void JobsList::removeFinishedJobs() {
    //The function removes all jobs from linked list and hash that finished

    int kid_pid = waitpid(-1, nullptr, WNOHANG);
    while (kid_pid > 0) {
        int job_id = findJobId(kid_pid);
        removeJobById(job_id);
        kid_pid = waitpid(-1, nullptr, WNOHANG);
    }
}

JobsList::JobEntry *JobsList::getJobById(int jobId) {
    //! maybe need to handle errors like non-existing jobId or empty list
    removeFinishedJobs();
    if (job_ids.find(jobId) == job_ids.end()) {

        throw exception(); //! error, not a valid job id
    }
    list<JobEntry *>::iterator it = job_ids[jobId];
    return *it;

}

void JobsList::removeJobById(int jobId) {
    //! maybe need to handle errors like non-existing jobId or empty list
    if (job_ids.find(jobId) == job_ids.end()) {
        throw exception(); //! error, not a valid job id
    }
    //? should I free the data before erasing?
    list<JobEntry *>::iterator it = job_ids[jobId];
    jobs.erase(it);
    job_ids.erase(jobId);

}

JobsList::JobEntry *JobsList::getLastJob(int *lastJobId) {
    //! maybe need to handle errors like non-existing jobId or empty list
    removeFinishedJobs();
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
    throw exception();
}

int JobsList::findJobId(int pid) {
    for (auto &job: jobs) {
        if (job->pid == pid) {
            return job->job_id;
        }
    }
    //! should not get here, must mean pid not a process running in bg or stopped
    throw exception();
}

JobsList::JobsList() {
    max_jobId = 0;
    jobs = list<JobEntry *>();
    job_ids = unordered_map<int, list<JobEntry *>::iterator>();
}

bool JobsList::isEmpty() const {
    return jobs.empty();
}

bool JobsList::jobExists(int jobId) {
    removeFinishedJobs();
    if (job_ids.find(jobId) == job_ids.end())
        return false;

    return true;
}

int JobsList::findMax() {
    int max = 0;
    if(!jobs.empty()) {
        for (auto &job: jobs) {
            if (job->job_id > max) {
                max = job->job_id;
            }

        }
    }
    return (max + 1);
}

int Command::getCommandPid() const {
    return pid;
}

const char *Command::getLine() {
    return cmd_line;
}

Command::Command(const char *cmd_line) : cmd_line(cmd_line) {
    pid = NO_PID;
}

Command::Command(int pid_copy, const char *cmd_line_copy) {
    pid = pid_copy;
    cmd_line = cmd_line_copy;
}


void Command::setPid(int new_pid) {
    this->pid = new_pid;
}

void Command::setLine(const char* new_line) {
    this->cmd_line = new_line;
}

JobsList::JobEntry::JobEntry(int pid, int job_id, const char *cmd_input, bool stopped, time_t start_time) : pid(pid),
                                                                                                            job_id(job_id),
                                                                                                            cmd_input(
                                                                                                                    cmd_input),
                                                                                                            stopped(stopped),
                                                                                                            time_entered(
                                                                                                                    start_time) {}

int JobsList::JobEntry::getJobPid() const {
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
                                                                                             jobId(job_number),
                                                                                             jobs_list_fg(jobs) {}

BackgroundCommand::BackgroundCommand(const char *cmd_line, JobsList *jobs) :
        BuiltInCommand(cmd_line),
         jobs_list_background(jobs) {}

ExternalCommand::ExternalCommand(const char *cmd_line) : Command(cmd_line) {

}

/**                           showpid Function                                  **/
ShowPidCommand::ShowPidCommand(const char *cmd_line) : BuiltInCommand(cmd_line) {}

void ShowPidCommand::execute() {
    SmallShell &smash = SmallShell::getInstance();
    int pid_smash = smash.smash_pid;
    cout << "smash pid is " << pid_smash << endl;
}

GetCurrDirCommand::GetCurrDirCommand(const char *cmd_line) : BuiltInCommand(cmd_line) {}

void GetCurrDirCommand::execute() {
    char buf[PATH_MAX];
    if (getcwd(buf, sizeof(buf)) == NULL) {
        perror("smash error: getcwd failed");
        return;
    }
    cout << buf << endl;

}

ChangeDirCommand::ChangeDirCommand(const char *cmd_line, string &plastPwd) : BuiltInCommand(cmd_line),
                                                                             plastPwd(plastPwd) {}

void ChangeDirCommand::execute() {

    char *args[COMMAND_MAX_ARGS];
    int args_amount = _parseCommandLine(cmd_line, args);
    if (args_amount != 2) {
        cerr << "smash error: cd: too many arguments" << endl;
        return;
    }

    //saving my current dir
    char buf[PATH_MAX];
    if (getcwd(buf, sizeof(buf)) == NULL) {
        perror("smash error: getcwd failed");
        return;
    }

    if (!strcmp(args[1], "-")) {
        if (plastPwd == "") {
            cerr << "smash error: cd: OLDPWD not set" << endl;
            return;
        }
        if (chdir(plastPwd.c_str()) == -1) {
            perror("smash error: chdir failed");
            return;
        }
    } else {
        if (chdir(args[1]) == -1) {
            perror("smash error: chdir failed");
            return;
        }
    }
    plastPwd = string(buf);
}

KillCommand::KillCommand(const char *cmd_line, JobsList *jobs) : BuiltInCommand(cmd_line), extra_jobs(jobs) {}

void KillCommand::execute() {
    char *args[COMMAND_MAX_ARGS];
    JobsList::JobEntry *job = nullptr;
    int args_amount = _parseCommandLine(cmd_line, args);
    if (args_amount != 3) {
        cerr << "smash error: kill: invalid arguments" << endl;
        return;
    }

    try {
        job = extra_jobs->getJobById(atoi(args[2]));
    } catch (...) {
        cerr << "smash error: kill: job-id " << args[2] << " does not exist" << endl;
        return;
    }

    if (kill(job->getJobPid(), abs(atoi(args[1]))) == -1) {
        perror("smash error: kill failed");
    }
}

TailCommand::TailCommand(const char *cmd_line) : BuiltInCommand(cmd_line) {
}

size_t TailCommand::checkSyntaxTail(const char *line, char **args, bool *valid) {
    int arg_amount = _parseCommandLine(line, args);
    if (arg_amount > TAIL_MAX_AMOUNT || arg_amount < TAIL_MIN_AMOUNT) {
        *valid = false;
        return 0;
    }
    if (arg_amount == TAIL_MIN_AMOUNT) {
        *valid = true;
        return DEFAULT_LINE_AMOUNT;
    } else {
        // * three parameters and the second one is a number
        std::string number_str = args[1];
        std::string sub = number_str.substr(1);
        const char *c_sub = sub.c_str();
        // moving to char* the number without the negative sign

        if (!SmallShell::isNumber((char *) c_sub) || number_str.at(0) != '-') {
            *valid = false;
            return 0;
        }
        *valid = true;
        return std::stoul(number_str.substr(1));
    }
}

size_t TailCommand::count_lines(char *buf) {
    int fd = -1;
    if((fd = open(buf, O_RDONLY)) < 0){
        perror("smash error: open failed");
        throw std::exception();
    }
    string line;
    size_t counter = 0;

    while (readLine(fd, line)) {
        line.clear();
        counter++;
    }

    if(close(fd) < 0){
        perror("smash error: close failed");
        throw std::exception();
    }
    return counter;
}

bool TailCommand::readLine(int fd, std::string& line){
    int read_rt;
    char curr_ch[1];

    while(true) {
        //assuming file is open
        read_rt = read(fd, curr_ch, 1);

        if (read_rt == 0) {
            // finished reading
            return false;

        } else if (curr_ch[0] == '\n') {
            return true;

        } else {
            line.append(curr_ch);
            continue;
        }
    }
}

TouchCommand::TouchCommand(const char *cmd_line) : BuiltInCommand(cmd_line) {

}

bool TouchCommand::checkSyntaxTouch(const char *line, char **args) {
    int args_len = _parseCommandLine(line, args);
    if (args_len != 3) {
        return false;
    }
    return true;
}

QuitCommand::QuitCommand(const char *cmd_line, JobsList *jobs) : BuiltInCommand(cmd_line), extra_jobs(jobs) {}

void QuitCommand::execute() {
    char *args[COMMAND_MAX_ARGS];
    int args_amount = _parseCommandLine(cmd_line, args);
    if (args_amount > 1 && !strcmp(args[1], "kill")) {
        list<JobsList::JobEntry *>::iterator it;
        cout << "smash: sending SIGKILL signal to " << extra_jobs->jobs.size() << " jobs:" << endl;
        for (it = (extra_jobs->jobs).begin(); it != (extra_jobs->jobs).end(); ++it) {
            cout << (*it)->getJobPid() << ": " << (*it)->getCmdInput() << endl;
            if (kill((*it)->getJobPid(), KILL_SIGNAL_NUM) == -1) {
                perror("smash error: kill failed");
            }
        }
    }
    exit(EXIT_SUCCESS);
}

RedirectionCommand::RedirectionCommand(const char *cmd_line) : Command(cmd_line) {}

void RedirectionCommand::execute() {

    char *args[COMMAND_MAX_ARGS];
    _parseCommandLine(cmd_line, args);
    char executed_cmd[strlen(cmd_line) + 1];
    strcpy(executed_cmd, cmd_line);

    int sign_place_one = findSign(cmd_line, ">");
    int sign_place_two = findSign(cmd_line, ">>");
    if (sign_place_one != -1) {
        pid_t pid = fork();
        if (pid == 0) {
            close(1);
            open(args[sign_place_one + 1], O_CREAT | O_RDWR | O_TRUNC  , 0655);
            char *sym_pos = strstr(executed_cmd, ">");
            *sym_pos = '\0';
            (SmallShell::getInstance()).executeCommand(executed_cmd);
            sync();
            close(1);
            exit(0);
        } else {
            waitpid(pid, nullptr, WUNTRACED);
        }
        return;
    }
    if (sign_place_two != -1) {
        pid_t pid = fork();
        if (pid == 0) {
            close(1);
            open(args[sign_place_two + 1], O_CREAT | O_APPEND | O_RDWR, 0655);
            char *sym_pos = strstr(executed_cmd, ">");
            *sym_pos = '\0';
            (SmallShell::getInstance()).executeCommand(executed_cmd);
            sync();
            close(1);
            exit(0);
        } else {
            waitpid(pid, nullptr, WUNTRACED);
        }
        return;

    }
    return;

}

PipeCommand::PipeCommand(const char *cmd_line) : Command(cmd_line) {}

void PipeCommand::execute() {

    char *args[COMMAND_MAX_ARGS];
    _parseCommandLine(cmd_line, args);
    char executed_cmd[strlen(cmd_line) + 1];
    strcpy(executed_cmd, cmd_line);


    int my_pipe[2];
    pipe(my_pipe);

    int sign_place_one = findSign(cmd_line, "|");
    int sign_place_two = findSign(cmd_line, "|&");
    if (sign_place_one != -1) {
        if (fork() == 0) {
            if (fork() != 0) {
                dup2(my_pipe[0], 0);
                close(my_pipe[0]);
                close(my_pipe[1]);
                char *sym_pos = strstr(executed_cmd, "|");
                wait(NULL);
                (SmallShell::getInstance()).executeCommand(sym_pos + 1);
                exit(EXIT_SUCCESS);
            } else {
                dup2(my_pipe[1], 1);
                close(my_pipe[0]);
                close(my_pipe[1]);
                char *sym_pos = strstr(executed_cmd, "|");
                *sym_pos = '\0';
                (SmallShell::getInstance()).executeCommand(executed_cmd);
                exit(EXIT_SUCCESS);
            }
        } else {
            close(my_pipe[0]);
            close(my_pipe[1]);
            wait(NULL);
        }
        return;
    }
    if (sign_place_two != -1) {
        if (fork() == 0) {
            if (fork() != 0) {
                dup2(my_pipe[0], 0);
                close(my_pipe[0]);
                close(my_pipe[1]);
                char *sym_pos = strstr(executed_cmd, "|");
                wait(NULL);
                (SmallShell::getInstance()).executeCommand(sym_pos + 1);
                exit(EXIT_SUCCESS);
            } else {
                dup2(my_pipe[1], 2);
                close(my_pipe[0]);
                close(my_pipe[1]);
                char *sym_pos = strstr(executed_cmd, "|");
                *sym_pos = '\0';
                (SmallShell::getInstance()).executeCommand(executed_cmd);
                exit(EXIT_SUCCESS);
            }
        } else {
            close(my_pipe[0]);
            close(my_pipe[1]);
            wait(NULL);
        }
    }

}



EmptyCommand::EmptyCommand(const char *cmd_line) : BuiltInCommand(cmd_line) {}

void EmptyCommand::execute()
{
    return;
}


