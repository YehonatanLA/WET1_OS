#ifndef SMASH_COMMAND_H_
#define SMASH_COMMAND_H_

#include <ctime>
#include <list>
#include <unordered_map>
#include <unistd.h>
#include <cstring>
#include <iostream>
#include <vector>
#include <sstream>
#include <sys/wait.h>
#include <iomanip>
#include <ctime>
#include <utime.h>
#include <cstring>
#include "signal.h"
#include <fstream>

#define COMMAND_ARGS_MAX_LENGTH (200)
#define COMMAND_MAX_ARGS (20)
#define PATH_MAX (4096)
class Command {
// TODO: Add your data members
protected:
    int pid;
    const char* cmd_line;
// ? pid
public:
    explicit Command(const char *cmd_line);

    virtual ~Command() = default;

    virtual void execute() = 0;
    //virtual void prepare();
    //virtual void cleanup();

    // TODO: Add your extra methods if needed
    int getCommandPid() const;
    const char* getLine();
    void setPid(int new_pid);
};

class BuiltInCommand : public Command {
public:
    explicit BuiltInCommand(const char *cmd_line);

    virtual ~BuiltInCommand() {}
};

class ExternalCommand : public Command {
public:
    ExternalCommand(const char *cmd_line);

    virtual ~ExternalCommand() {}

    void execute() override;
};

class PipeCommand : public Command {
    // TODO: Add your data members
public:
    PipeCommand(const char *cmd_line);

    virtual ~PipeCommand() {}

    void execute() override;
};

class RedirectionCommand : public Command {
    // TODO: Add your data members
public:
    explicit RedirectionCommand(const char *cmd_line);

    virtual ~RedirectionCommand() {}

    void execute() override;
    //void prepare() override;
    //void cleanup() override;
};

class ChangeDirCommand : public BuiltInCommand {
// TODO: Add your data members
public:
    std::string &plastPwd;

    ChangeDirCommand(const char *cmd_line, std::string &plastPwd);

    virtual ~ChangeDirCommand() {}

    void execute() override;
};

class GetCurrDirCommand : public BuiltInCommand {
public:
    GetCurrDirCommand(const char *cmd_line);

    virtual ~GetCurrDirCommand() {}

    void execute() override;
};

class ShowPidCommand : public BuiltInCommand {
public:
    ShowPidCommand(const char *cmd_line);

    virtual ~ShowPidCommand() {}

    void execute() override;
};

class JobsList;

class QuitCommand : public BuiltInCommand {
// TODO: Add your data members
public:
    JobsList *extra_jobs;

    QuitCommand(const char *cmd_line, JobsList *jobs);

    virtual ~QuitCommand() {}

    void execute() override;
};


class JobsList {
public:
    class JobEntry {
        int pid;
        int job_id;
        const char* cmd_input;
        bool stopped;
        time_t time_entered;

        // TODO: Add your data members
        friend JobsList;
    public:
        JobEntry(int pid, int job_id, const char *cmd_input, bool stopped, time_t start_time);

        int getJobPid() const;

        bool getStopped() const;

        const char *getCmdInput();

        int getJobId() const;

        void ChangeStopped();
    };
    // TODO: Add your data members
    int max_jobId;

    std::list<JobEntry*> jobs;
    std::unordered_map<int, std::list<JobEntry*>::iterator> job_ids; //points to an item in the job list


public:
    JobsList();

    ~JobsList() = default; //? default or need to destruct the data structures and data inside

    void addJob(Command *cmd, bool isStopped = false);

    void printJobsList();

    void killAllJobs();

    void removeFinishedJobs();

    JobEntry *getJobById(int jobId);

    void removeJobById(int jobId);

    JobEntry *getLastJob(int *lastJobId);

    JobsList::JobEntry *getLastStoppedJob();
    // TODO: Add extra methods or modify exisitng ones as needed
    int findJobId(int pid);

    bool isEmpty() const;

    bool jobExists(int jobId);
};

class JobsCommand : public BuiltInCommand {
    // TODO: Add your data members
    JobsList *extra_jobs;
public:
    JobsCommand(const char *cmdLine, JobsList *jobs);

    virtual ~JobsCommand() {}

    void execute() override;
};

class KillCommand : public BuiltInCommand {
    // TODO: Add your data members
    JobsList *extra_jobs;
public:
    KillCommand(const char *cmd_line, JobsList *jobs);

    virtual ~KillCommand() {}

    void execute() override;
};

class ForegroundCommand : public BuiltInCommand {

    // TODO: Add your data members
    int jobId;
    JobsList* jobs_list_fg;
public:
    ForegroundCommand(const char *cmd_line, JobsList *jobs, int job_number);

    virtual ~ForegroundCommand() {}

    void execute() override;
};

class BackgroundCommand : public BuiltInCommand {
    // TODO: Add your data members
    int jobIdBackground;
    JobsList* jobs_list_background;
public:
    BackgroundCommand(const char *cmd_line, JobsList *jobs, int job_number);

    virtual ~BackgroundCommand() {}

    void execute() override;
};

class TailCommand : public BuiltInCommand {
public:
    explicit TailCommand(const char *cmd_line);

    static size_t checkSyntaxTail(const char *line, char **args, bool *valid);

    virtual ~TailCommand() {}

    void execute() override;

    static size_t count_lines(char buf[4096]);
};

class TouchCommand : public BuiltInCommand {
public:
    TouchCommand(const char *cmd_line);

    virtual ~TouchCommand() {}

    void execute() override;

    static bool checkSyntaxTouch(const char *line, char *pString[20]);
};


class SmallShell {
private:
    // TODO: Add your data members
    std::string curr_prompt = "smash> ";
    SmallShell();
public:
    JobsList extra_jobs;
    const std::string default_prompt = "smash> ";
    std::string pLastPwd = "";

    const std::string& getCurrPrompt();
    void setCurrPrompt(const std::string& s);

    Command *CreateCommand(const char *cmd_line);

    SmallShell(SmallShell const &) = delete; // disable copy ctor
    void operator=(SmallShell const &) = delete; // disable = operator
    static SmallShell &getInstance() // make SmallShell singleton
    {
        static SmallShell instance; // Guaranteed to be destroyed.
        // Instantiated on first use.
        return instance;
    }

    ~SmallShell();
    void executeCommand(const char *cmd_line);

    // TODO: add extra methods as needed
    static void saveChangePrompt(const char* cmd);

    static bool isNumber(char *string);

    static int checkSyntaxForeGroundBackground(const char *line, char **args);

    bool cmdIsChprompt(const char *line);
};

#endif //SMASH_COMMAND_H_
