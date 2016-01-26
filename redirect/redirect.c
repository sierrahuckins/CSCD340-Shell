#include "redirect.h"

void redirectIn(char * newInPath){
	//redirect stdin using newIn
	int fd = open(newInPath, O_CREAT | O_APPEND | O_RDONLY, 0777);
	close(0);
	dup(fd);
}

void redirectOut(char * newOutPath) {
	//redirect stdout using newOut
	int fd = open(newOutPath, O_CREAT | O_APPEND | O_WRONLY, 0777);
	close(1);
	dup(fd);
}
