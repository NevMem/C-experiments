#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

int parent_event_checker = -1;

void parent_listener_sigrtmin(int sig) {
	fprintf(stderr, "Parent received SIGRTMIN request\n");
	parent_event_checker = 0;
}

void parent_listener_sigrtmax(int sig) {
	fprintf(stderr, "Parent received SIGRTMAX request\n");
	parent_event_checker = 1;
}

void register_parent_listener() {
	signal(SIGRTMIN, parent_listener_sigrtmin);
	signal(SIGRTMAX, parent_listener_sigrtmax);	
}

void proc_parent() {
	while (1) {
		if (parent_event_checker == -1) continue;
		int cur = parent_event_checker;
		if (cur == 0) {
			fprintf(stderr, "Processing SIGRTMIN request\n");

		} else if (cur == 1) {
			fprintf(stderr, "Processing SIGRTMAX request\n");

		} else {
			fprintf(stderr, "Unknown command\n");
		}
		parent_event_checker = -1;
	}
}

int main() {
	printf("Start to perform our job\n");

	size_t n, m;
	scanf("%lld %lld", &n, &m);

	pid_t parent = getpid();
	pid_t first_child = fork();
	if (first_child > 0) {
		// Parent
		fprintf(stderr, "Parent process is ready to create second child\n");
		pid_t second_child = fork();
		if (second_child < 0) {
			fprintf(stderr, "Error occurred while forking second child\n");
			exit(1);
		} else if (second_child > 0) {
			// Parent
			fprintf(stderr, "Both children was successfully created (pids: %d %d)", first_child, second_child);
			register_parent_listener();
			proc_parent();
		} else {
			// Second child
			fprintf(stderr, "Second child is ready to perform his job\n");
		}
	} else if (first_child == 0) {
		// First child
		fprintf(stderr, "First child is ready to perform his job\n");
	} else {
		fprintf(stderr, "Error coccured while forking first child\n");
	}
}
