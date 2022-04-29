#include "Commands.h"


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

#define NO_PID (-1)
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

/**
* Creates and returns a pointer to Command class which matches the given command line (cmd_line)
*/
Command *SmallShell::CreateCommand(const char *new_cmd) {
    // For example:
    const char* cmd_line = (char *) malloc(sizeof(char) * strlen(new_cmd));
    strcpy((char*)cmd_line, new_cmd);
    string cmd_s = _trim(string(cmd_line));
    string firstWord = cmd_s.substr(0, cmd_s.find_first_of(" \n"));

    if (firstWord == "chprompt") {

        saveChangePrompt(cmd_line);

    } else if (firstWord == "jobs") {
        return new JobsCommand(cmd_line, &extra_jobs);

    } else if (firstWord == "fg") {
        char *args[COMMAND_MAX_ARGS];
        int job_number = checkSyntax(cmd_line, args);
        if (job_number < LAST_JOB) {
            cerr << "smash error: fg: invalid arguments" << endl;
            throw exception(); //? or return nullptr
        }
        return new ForegroundCommand(cmd_line, &extra_jobs, job_number);

    } else if (firstWord == "bg") {
        char *args[COMMAND_MAX_ARGS];
        int job_number = checkSyntax(cmd_line, args);
        if (job_number < LAST_JOB) {
            cerr << "smash error: bg: invalid arguments" << endl;
            throw exception(); //? or return nullptr
        }
        return new BackgroundCommand(cmd_line, &extra_jobs, job_number);
    } else if (firstWord == "showpid") {
        //? check if needed to check in if the &
        return new ShowPidCommand(cmd_line);
    } else if (firstWord == "pwd") {
        return new GetCurrDirCommand(cmd_line);
    } else if (firstWord == "cd") {
        return new ChangeDirCommand(cmd_line, pLastPwd);
    } else if (firstWord == "kill") {
        return new KillCommand(cmd_line, &extra_jobs);
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

    Command *cmd = CreateCommand(cmd_line);
    if(cmdIsChprompt(cmd_line)){
        return;
    }
    //! if this is a external command, we need to make sure that we are forking and execv ing the process (and something about setpgrp)
    cmd->execute();
}


void JobsCommand::execute() {
    //the function needs to print out all the jobs in the bg or the ones that stopped.
    extra_jobs->printJobsList();
}

void ForegroundCommand::execute() {
//the function brings either a bg process or a stopped process to fg
    if (this->jobId != LAST_JOB && !jobs_list_fg->jobExists(this->jobId)) {

        cerr << "smash error: fg: job-id " << this->jobId <<" does not exist" << endl;
        return;
    } else if (this->jobId == LAST_JOB && jobs_list_fg->isEmpty()) {
        cerr << "smash error: fg: jobs list is empty" << endl;
        return;
    }
    JobsList::JobEntry *chosen_job;

    if (this->jobId == LAST_JOB) {
        chosen_job = jobs_list_fg->getLastJob(nullptr);
        this->jobId = chosen_job->getJobId();
    } else {
        chosen_job = jobs_list_fg->getJobById(this->jobId);
    }

    if (chosen_job->getStopped()) {
        int ret = kill(chosen_job->getJobPid(), SIGCONT);
        if(ret < 0){
            // * failed to send sigcont to prcess
            perror("smash error: kill failed");
        }

    }
    cout << chosen_job->getCmdInput() << " : " << chosen_job->getJobPid() << "\n";
    waitpid(chosen_job->getJobPid(), nullptr, 0);
    jobs_list_fg->removeJobById(jobId);


}


void BackgroundCommand::execute() {
    //the function brings a stopped process to bg
    if (this->jobIdBackground != LAST_JOB && !jobs_list_background->jobExists(this->jobIdBackground)) {
        cerr << "smash error: bg: job-id " << this->jobIdBackground <<" does not exist" << endl;
        return;
    } else if (this->jobIdBackground == LAST_JOB && jobs_list_background->isEmpty()) {
        cerr << "smash error: bg: jobs list is empty" << endl;
        return;
    }
    JobsList::JobEntry* stopped_job;

    if(jobIdBackground == LAST_JOB) {
        try {
            stopped_job = jobs_list_background->getLastStoppedJob();
            this->jobIdBackground = stopped_job->getJobId();
        }
        catch(exception&){
            cerr << "smash error: bg: there is no stopped jobs to resume" << endl;
            return;
        }
    }
    else{
        stopped_job = jobs_list_background->getJobById(this->jobIdBackground);
    }
    cout << stopped_job->getCmdInput() << ": " << this->jobIdBackground;
     stopped_job->ChangeStopped();
    int ret = kill(stopped_job->getJobPid(), SIGCONT);
    if(ret < 0){
        // * failed to send sigcont to prcess
        perror("smash error: kill failed");
    }

}

void ExternalCommand::execute() {
    char* args[COMMAND_MAX_ARGS];
    _parseCommandLine(cmd_line, args);
    pid_t c_pid = fork();
    char* run_in_bash = (char*) malloc(sizeof(char*) * strlen(cmd_line));
    strcpy(run_in_bash, cmd_line);
    _removeBackgroundSign(run_in_bash);

    if(c_pid == 0){
        setpgrp();
        //we're in the child process
        char* const args_for_bash[4] = {(char*) "/bin/bash",(char*)"-c", run_in_bash, nullptr};
        execv(args_for_bash[0], args_for_bash);
        //if this returned, something wrong happened
        throw exception();
    }
    else{
        //we're in the parent process
        if(_isBackgroundCommand(cmd_line)){
            //runs in background

            SmallShell& smash = SmallShell::getInstance();
          pid = int(c_pid); //changing the default pid to relevant pid
            smash.extra_jobs.addJob(this, false);
        }
        else{
            //runs in the foreground
            waitpid(c_pid, nullptr, 0); //? should I save the status? Also, should I save ethe return value?
        }
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
    size_t len = strlen(string);
    for (size_t i = 0; i < len; i++) {
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
    max_jobId++;
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
        cout << "[" << curr_job_id << "] " << input << " :" << curr_pid << " " << difftime(status_time, job->time_entered);

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
    if (job_ids.find(jobId) == job_ids.end())
        return false;

    return true;
}

int Command::getCommandPid() const {
    return pid;
}

const char *Command::getLine() {
    return cmd_line;
}

Command::Command(const char *cmd_line): cmd_line(cmd_line) {
        pid = NO_PID;
}

void Command::setPid(int new_pid) {
    this->pid = new_pid;
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
                                                                                             jobs_list_fg(jobs)
                                                                                              {}

BackgroundCommand::BackgroundCommand(const char *cmd_line, JobsList *jobs, int job_number) :
        BuiltInCommand(cmd_line),
        jobIdBackground(job_number), jobs_list_background(jobs) {}

ExternalCommand::ExternalCommand(const char *cmd_line) : Command(cmd_line) {

}

/**                           showpid Function                                  **/
ShowPidCommand::ShowPidCommand(const char *cmd_line) : BuiltInCommand(cmd_line) {}

void ShowPidCommand::execute()
{
    cout << "smash pid is " << getpid() << endl;
}

GetCurrDirCommand::GetCurrDirCommand(const char *cmd_line) : BuiltInCommand(cmd_line) {}

void GetCurrDirCommand::execute()
{
    char buf[PATH_MAX];
    if (getcwd(buf, sizeof(buf)) == NULL)
    {
        perror("smash error: getcwd failed");
        return;
    }
    cout << buf << endl;

}

ChangeDirCommand::ChangeDirCommand(const char *cmd_line, string &plastPwd) : BuiltInCommand(cmd_line), plastPwd(plastPwd) {}

void ChangeDirCommand::execute()
{

    char *args[COMMAND_MAX_ARGS];
    int args_amount = _parseCommandLine(cmd_line, args);
    if (args_amount != 2)
    {
        cerr << "smash error: cd: too many arguments" << endl;
        return;
    }

    //saving my current dir
    char buf[PATH_MAX];
    if (getcwd(buf, sizeof(buf)) == NULL)
    {
        perror("smash error: getcwd failed");
        return;
    }

    if (!strcmp(args[1], "-"))
    {
        if (plastPwd == "")
        {
            cerr << "smash error: cd: OLDPWD not set" << endl;
            return;
        }
        if(chdir(plastPwd.c_str()) == -1)
        {
            perror("smash error: chdir failed");
            return;
        }
    }
    else
    {
        if(chdir(args[1]) == -1)
        {
            perror("smash error: chdir failed");
            return;
        }
    }
    plastPwd = string(buf);
}

KillCommand::KillCommand(const char *cmd_line, JobsList *jobs): BuiltInCommand(cmd_line), extra_jobs(jobs) {}

void KillCommand::execute()
{
    char *args[COMMAND_MAX_ARGS];
    JobsList::JobEntry* job = nullptr;
    int args_amount = _parseCommandLine(cmd_line, args);
    if (args_amount != 3)
    {
        cerr << "smash error: kill: invalid arguments" << endl;
        return;
    }

    try {
        job = extra_jobs->getJobById(atoi(args[2]));
    } catch (...) {
        cerr << "smash error: kill: job-id " << args[2] << " does not exist" << endl;
        return;
    }

    if (kill(job->getJobPid(), abs(atoi(args[1]))) == -1)
    {
        perror("smash error: kill failed");
    }
}


/// TODO quit !!!!!