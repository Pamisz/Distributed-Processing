#include <stdio.h>
#include <unistd.h>

int main() {
	int depth;
	printf("Enter depth: ");
	if (scanf("%d", &depth) == 1) {
		printf("Genering tree with depth: %d\n", depth);
	}
	else {
		printf("Bad data, try again.\n");
	}

	for (int i = 1; i < 2*depth; i++) {
		int pid = fork();
		if (pid == -1) {
			perror("Fork error");
			return 2;
		}
		if ((pid == 0 && i%2 == 1) || (!(pid == 0) && i%2 == 0)) {
			sleep(3);
			return 0;
		}
	}
	execlp("ps", "-u student", "--forest", NULL);
	return 0;
    
	return 0;
}