#include <iostream>
#include <signal.h>
#include "signals.h"
#include "Commands.h"

using namespace std;
#define KILL_SIGNAL_NUM (9)
#define NO_PID (-1)

void ctrlZHandler(int sig_num) {
	// TODO: Add your implementation
    cout<< "smash: got ctrl-Z" << endl;
    SmallShell& smash = SmallShell::getInstance(b );
    int pid = smash.curr_cmd->getCommandPid();
    if (pid == NO_PID)
    {
        return;
    }
    else
    {
        //Command* command_job = Command(smash.curr_cmd->getLine(), smash.curr_cmd->getCommandPid());
        smash.extra_jobs.addJob(smash.curr_cmd, true);
        kill(pid, 19);
        cout<< "smash: process " << pid << " was stopped" << endl;

    }

}
//! if stopping sleep, the linux will continue afterwards even though it was stopped

void ctrlCHandler(int sig_num) {
  // TODO: Add your implementation
    cout<< "smash: got ctrl-C" << endl;
    SmallShell& smash = SmallShell::getInstance();
    int pid = smash.curr_cmd->getCommandPid();
    if (pid == NO_PID)
    {
        return;
    }
    else
    {
        kill(pid, KILL_SIGNAL_NUM);
        cout<< "smash: process " << pid << " was killed" << endl;

    }
}

void alarmHandler(int sig_num) {
  // TODO: Add your implementation
    cout<< "smash: got an alarm" << endl;
}

