/*
 * Bhavay Garg
 * 041102440
 *
 * partA_sighandler.c -- Catches SIGUSR1 signal
 *
 * CST8244 Lab 3 - Part A
 * This program demonstrates handling of SIGUSR1 signal.
 * The program loops until it receives SIGUSR1, then exits gracefully.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>

/* Global variable to indicate if SIGUSR1 has been received
 * Using sig_atomic_t ensures atomic access (single instruction)
 * Using volatile prevents compiler optimization issues
 */
volatile sig_atomic_t usr1Happened = 0;  // 0 = false

void sigusr1_handler(int sig);

int main(void) {
	struct sigaction sa;
	pid_t pid;

	// Setting up the sig handeler
	sa.sa_handler = sigusr1_handler;
	sa.sa_flags = 0;
	sigemptyset(&sa.sa_mask);

//  installing the sigusr1 using the sigaction
	if (sigaction(SIGUSR1, &sa, NULL) == -1) {
		perror("sigaction");
		exit(1);
	}

	// print the process id to terminate the process
	pid = getpid();
	printf("PID = %d: Running...\n", pid);

	/* Loop until signal is received */
	while (!usr1Happened) {
		sleep(1); /* Sleep to reduce CPU usage */
	}

	/* Signal received, print message and exit */
	printf("PID = %d: Received USR1.\n", pid);
	printf("PID = %d: Exiting.\n", pid);

	return 0;
}

//  sigusr1_handler() - Signal handler for SIGUSR1
// Sets the global flag to indicate signal was received

void sigusr1_handler(int sig) {
	usr1Happened = 1;
}
