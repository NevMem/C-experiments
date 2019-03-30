#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

int parent_event_checker = -1;

void parent_listener_SIGINT(int sig) {
	fprintf(stderr, "Parent received SIGINT request\n");
	parent_event_checker = 0;
}

void parent_listener_sigrtmax(int sig) {
	fprintf(stderr, "Parent received SIGRTMAX request\n");
	parent_event_checker = 1;
}

void register_parent_listener() {
	signal(SIGINT, parent_listener_SIGINT);
	// signal(SIGRTMAX, parent_listener_sigrtmax);	
}

/* Globals */ // FIXME:(bad code style)
size_t n, m;
FILE* out;
pid_t parent;

void proc_parent(int* first_child_pipe, int first_pid, int* second_child_pipe, int second_pid) {
	register_parent_listener();

	fprintf(stderr, "Parent listener was successfully registered\n");

	char buffer[10];

	sprintf(buffer, "%d", second_pid);
	fprintf(stderr, "Second child pid: %d\n", second_pid);
	write(first_child_pipe[1], buffer, strlen(buffer));
	close(first_child_pipe[1]);

	sprintf(buffer, "%d", first_pid);
	fprintf(stderr, "First child pid: %d\n", first_pid);
	write(second_child_pipe[1], buffer, strlen(buffer));
	close(second_child_pipe[1]);

	int iters = 0;
	
	while (1) {
		++iters;
		if (parent_event_checker == -1) continue;
		int cur = parent_event_checker;
		if (cur == 0 && counter == m) {
			fprintf(stderr, "Processing SIGINT request\n");
			fprintf(stderr, "Closing file by parent thread\n");
			fclose(out);
			break;
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

int counter = 0;

int first_brother = 0;
void first_listener(int sig) {
	fprintf(stderr, "First child listner was called\n");
	char* buffer = malloc(m + 1);
	for (size_t i = 0; i != m; ++i)
		buffer[i] = 'A';
	buffer[m] = '\0';
	fprintf(out, "%s", buffer);
	fflush(out);
	free(buffer);
	counter++;
	if (counter == n) {
		kill(parent, SIGINT);
		exit(0);
	}
	kill(first_brother, SIGINT);
}

void proc_first(int* pipe) {
	int brother = readOneInt(pipe[0]);
	fprintf(stderr, "Brother of first child has pid: %d\n", brother);
	first_brother = brother;
	signal(SIGINT, first_listener);
	fprintf(stderr, "First child listener was successfully set\n");
	fprintf(stderr, "Sending SIGINT to brother of first\n");
	kill(first_brother, SIGINT);
	sleep(1);
	while (1) {}
}

int second_brother = 0;
void second_listener(int sig) {
	fprintf(stderr, "Second child listner was called\n");
	char* buffer = malloc(m + 1);
	for (size_t i = 0; i != m; ++i)
		buffer[i] = 'B';
	buffer[m] = '\0';
	fprintf(out, "%s", buffer);
	fflush(out);
	free(buffer);
	kill(second_brother, SIGINT);
}

void proc_second(int* pipe) {
	int brother = readOneInt(pipe[0]);
	fprintf(stderr, "Brother of second child has pid: %d\n", brother);
	second_brother = brother;
	signal(SIGINT, second_listener);
	fprintf(stderr, "Second child listener was successfully set\n");
	sleep(1);
	while (1) {}
}

int main() {
	printf("Start to perform our job\n");

	out = fopen("output.txt", "w");

	scanf("%lld %lld", &n, &m);

	int first_pipe[2], second_pipe[2];
	pipe(first_pipe);
	pipe(second_pipe);

	parent = getpid();
	fprintf(stderr, "Parent pid: %d\n", parent);
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
