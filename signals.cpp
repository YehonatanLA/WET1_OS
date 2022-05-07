#include <iostream>
#include <signal.h>
#include "signals.h"
#include "Commands.h"
#include <fcntl.h>

using namespace std;
#define KILL_SIGNAL_NUM (9)


void ctrlZHandler(int sig_num) {
	// TODO: Add your implementation
    cout<< "smash: got ctrl-Z" << endl;
    //add to jobs list
    kill(getpid(), sig_num);


    // ? HOW TO SEND SIGSTOP
}
//! if stopping sleep, the linux will continue afterwards even though it was stopped

void ctrlCHandler(int sig_num) {
  // TODO: Add your implementation
    cout<< "smash: got ctrl-C" << endl;
    kill(getpid(), sig_num);
}

void alarmHandler(int sig_num) {
  // TODO: Add your implementation
    cout<< "smash: got an alarm" << endl;
}

