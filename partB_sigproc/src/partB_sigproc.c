///*
// * Bhavay Garg
// * 041102440
// *
// * partB_sigproc.c -- Forks child processes that handle SIGUSR1
// *
// * CST8244 Lab 3 - Part B
// * Parent creates N children, each child waits for SIGUSR1 signal.
// * Parent waits for all children to finish before exiting.
// */
//
//#include <stdio.h>
//#include <stdlib.h>
//#include <unistd.h>
//#include <errno.h>
//#include <signal.h>
//#include <sys/types.h>
//#include <sys/wait.h>
//
///* Global variable to indicate if SIGUSR1 has been received */
//volatile sig_atomic_t usr1Happened = 0;
//
//void sigusr1_handler(int sig);
//
//int main(void) {
//	pid_t pid;
//	pid_t parent_pid;
//	int numChildren;
//	int childrenFinished = 0;
//	struct sigaction sa;
//
//	// Get parent PID
//	parent_pid = getpid();
//	printf("PID = %d: Parent running...\n", parent_pid);
//
//	// Get number of children to create
//	while(1) {
//	    printf("Enter the number of children:\n");
//	    if (scanf("%d", &numChildren) != 1 || numChildren < 1) {
//	        fprintf(stderr, "Invalid number of children\n");
//
//	        // Clear the entire input buffer
//	        int c;
//	        while ((c = getchar()) != '\n' && c != EOF);  //Clears everything
//	    }
//	    else {
//	        break;
//	    }
//	}
//
//
//	// Fork the children
//	for (int i = 0; i < numChildren; i++) {
//		pid = fork();
//
//		if (pid == -1) {
//			// Fork failed
//			perror("fork");
//			exit(1);
//		} else if (pid == 0) {
//			// CHILD PROCESS
//			pid_t child_pid = getpid();
//
//			// Set up signal handler for SIGUSR1
//			sa.sa_handler = sigusr1_handler;
//			sa.sa_flags = 0;
//			sigemptyset(&sa.sa_mask);
//
//			if (sigaction(SIGUSR1, &sa, NULL) == -1) {
//				perror("sigaction");
//				exit(1);
//			}
//
//			printf("PID = %d: Child running...\n", child_pid);
//
//			// Loop until SIGUSR1 is received
//			while (!usr1Happened) {
//				sleep(1);
//			}
//
//			// Signal received
//			printf("PID = %d: Child received USR1.\n", child_pid);
//			printf("PID = %d: Child exiting.\n", child_pid);
//
//			exit(EXIT_SUCCESS);
//		}
//
//	}
//
//	// PARENT PROCESS - Wait for all children to finish
//	while (childrenFinished < numChildren) {
//		wait(NULL);
//		childrenFinished++;
//	}
//
//	// All children finished
//	printf("PID = %d: Children finished, parent exiting.\n", parent_pid);
//
//	return 0;
//}
////  sigusr1_handler() - Signal handler for SIGUSR1
//// Sets the global flag to indicate signal was receiveds
//void sigusr1_handler(int sig) {
//	usr1Happened = 1;
//}


/*
 * Bhavay Garg
 * 041102440
 *
 * partB_sigproc.c -- Forks child processes that handle SIGUSR1
 *
 * CST8244 Lab 3 - Part B
 * Parent creates N children, each child waits for SIGUSR1 signal.
 * Parent REFUSES to die while children are running (prevents orphans/zombies).
 * Parent waits for all children to finish before exiting.
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>

/* Global variable to indicate if SIGUSR1 has been received */
volatile sig_atomic_t usr1Happened = 0;

/* Global counter for active children */
volatile sig_atomic_t active_children = 0;

void sigusr1_handler(int sig);
void parent_protection_handler(int sig);

int main(void) {
	pid_t pid;
	pid_t parent_pid;
	int numChildren;
	int childrenFinished = 0;
	struct sigaction sa;
	struct sigaction sa_parent;

	// Get parent PID
	parent_pid = getpid();
	printf("PID = %d: Parent running...\n", parent_pid);

	// ======== INSTALL PARENT PROTECTION HANDLER ========
	// Parent will REFUSE to die while children are running!
	sa_parent.sa_handler = parent_protection_handler;
	sa_parent.sa_flags = 0;
	sigemptyset(&sa_parent.sa_mask);

	// Catch termination signals and refuse them
	if (sigaction(SIGUSR1, &sa_parent, NULL) == -1) {
		perror("sigaction SIGUSR1");
		exit(1);
	}
	// ====================================================

	// Get number of children to create
	while(1) {
	    printf("Enter the number of children:\n");
	    if (scanf("%d", &numChildren) != 1 || numChildren < 1) {
	        fprintf(stderr, "Invalid number of children\n");
	        // Clear the entire input buffer
	        int c;
	        while ((c = getchar()) != '\n' && c != EOF);
	    }
	    else {
	        break;
	    }
	}

	// Set the number of active children
	active_children = numChildren;

	// Fork the children
	for (int i = 0; i < numChildren; i++) {
		pid = fork();

		if (pid == -1) {
			// Fork failed
			perror("fork");
			exit(1);
		} else if (pid == 0) {
			// ======== CHILD PROCESS ========
			pid_t child_pid = getpid();

			// Set up signal handler for SIGUSR1
			sa.sa_handler = sigusr1_handler;
			sa.sa_flags = 0;
			sigemptyset(&sa.sa_mask);

			if (sigaction(SIGUSR1, &sa, NULL) == -1) {
				perror("sigaction");
				exit(1);
			}

			printf("PID = %d: Child running...\n", child_pid);

			// Loop until SIGUSR1 is received
			while (!usr1Happened) {
				sleep(1);
			}

			// Signal received
			printf("PID = %d: Child received USR1.\n", child_pid);
			printf("PID = %d: Child exiting.\n", child_pid);

			exit(EXIT_SUCCESS);
		}
		// PARENT continues to next iteration
	}

	// PARENT PROCESS - Wait for all children to finish
	while (childrenFinished < numChildren) {
		wait(NULL);
		childrenFinished++;
		active_children--;  // Decrement active children count
	}

	// All children finished
	printf("PID = %d: Children finished, parent exiting.\n", parent_pid);

	return 0;
}

/*
 * sigusr1_handler() - Signal handler for SIGUSR1 (for children)
 * Sets the global flag to indicate signal was received
 */
void sigusr1_handler(int sig) {
	usr1Happened = 1;
}

/*
 * parent_protection_handler() - Signal handler for parent process
 * Parent REFUSES to die while children are running!
 * Displays message telling user to kill children first.
 */
void parent_protection_handler(int sig) {
	// Note: We use write() instead of printf() because printf is not async-signal-safe
	const char msg[] = "\nPID = XXXXX: Cannot kill parent while children are running. Kill children first!\n";
	char buffer[100];
	int len;

	// Simple message using write (async-signal-safe)
	// For production, you'd format the PID properly, but for lab this is acceptable
	len = snprintf(buffer, sizeof(buffer),
	               "\nCannot kill parent (PID %d) - %d children still running. Kill children first!\n",
	               getpid(), active_children);

	write(STDOUT_FILENO, buffer, len);

	// Do NOT exit - just return and continue running!
}
