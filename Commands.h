#ifndef SMASH_COMMAND_H_
#define SMASH_COMMAND_H_

#include <ctime>
#include <list>
#include <unordered_map>

#define COMMAND_ARGS_MAX_LENGTH (200)
#define COMMAND_MAX_ARGS (20)

class Command {
// TODO: Add your data members
    int pid;
    const char* cmd_line;
// ? pid
public:
    Command(const char *cmd_line);

    virtual ~Command();

    virtual void execute() = 0;
    //virtual void prepare();
    //virtual void cleanup();

    // TODO: Add your extra methods if needed
    int getPid() const;
    const char* getLine();
};

class BuiltInCommand : public Command {
public:
    BuiltInCommand(const char *cmd_line);

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
// TODO: Add your data members public:
// ? add last directory
    ChangeDirCommand(const char *cmd_line, char **plastPwd);

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
// TODO: Add your data members public:
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

        int getPid() const;

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

    ~JobsList() = default; //? defualt or need to destruct the data structures and data inside

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
public:
    KillCommand(const char *cmd_line, JobsList *jobs);

    virtual ~KillCommand() {}

    void execute() override;
};

class ForegroundCommand : public BuiltInCommand {

    // TODO: Add your data members
    int jobId;
    JobsList* jobs_list;
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
    TailCommand(const char *cmd_line);

    virtual ~TailCommand() {}

    void execute() override;
};

class TouchCommand : public BuiltInCommand {
public:
    TouchCommand(const char *cmd_line);

    virtual ~TouchCommand() {}

    void execute() override;
};


class SmallShell {
private:
    // TODO: Add your data members
    std::string curr_prompt = default_prompt;
    SmallShell();

public:
    JobsList extra_jobs;
    const std::string default_prompt = "smash> ";

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

    static int checkSyntax(const char *line, char **args);
};

#endif //SMASH_COMMAND_H_
