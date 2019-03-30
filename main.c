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

void proc_parent(int* first_child_pipe, int first_pid, int* second_child_pipe, int second_pid) {
	register_parent_listener();

	fprintf(stderr, "Parent listener was successfully registered\n");

	char buffer[10];

	sprintf(buffer, "%d", second_pid);
	fprintf(stderr, "Second child pid: %d and it's len is: %d", second_pid, strlen(buffer));
	write(first_child_pipe[1], buffer, strlen(buffer));
	close(first_child_pipe[1]);

	sprintf(buffer, "%d", first_pid);
	fprintf(stderr, "First child pid: %d and it's len is: %d", first_pid, strlen(buffer));
	write(second_child_pipe[1], buffer, strlen(buffer));
	close(second_child_pipe[1]);
	
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

int readOneInt(int* in) {
	char buffer[20];
	read(in, buffer, 20);
	return atoi(buffer);
}

void proc_first(int* pipe) {
	int brother = readOneInt(pipe[0]);
	fprintf(stderr, "Brother of first child has pid: %d", brother);
}

void proc_second(int* pipe) {
	int brother = readOneInt(pipe[0]);
	fprintf(stderr, "Brother of second child has pid: %d", brother);
}

FILE* out;

int main() {
	printf("Start to perform our job\n");

	out = fopen("output.txt", "w");

	size_t n, m;
	scanf("%lld %lld", &n, &m);

	int first_pipe[2], second_pipe[2];
	pipe(first_pipe);
	pipe(second_pipe);

	pid_t parent = getpid();
	fprintf("Parent pid: %d\n", parent);
	pid_t first_child = fork(), second_child;
	if (first_child > 0) {
		// Parent
		fprintf(stderr, "Parent process is ready to create second child\n");
		second_child = fork();
		if (second_child < 0) {
			fprintf(stderr, "Error occurred while forking second child\n");
			exit(1);
		} else if (second_child > 0) {
			// Parent
			fprintf(stderr, "Children were successfully created (pids: %d %d)\n", first_child, second_child);
			proc_parent(first_pipe, first_child, second_pipe, second_child);
		} else {
			// Second child
			fprintf(stderr, "Second child is ready to perform his job\n");
			proc_second(second_pipe);
		}
	} else if (first_child == 0) {
		// First child
		fprintf(stderr, "First child is ready to perform his job\n");
		proc_first(first_pipe);
	} else {
		fprintf(stderr, "Error coccured while forking first child\n");
	}
}
