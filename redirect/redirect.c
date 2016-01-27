#include "redirect.h"

void redirectIn(char * newInPath){
	//redirect stdin using newIn
	int fd = open(newInPath, O_CREAT | O_RDONLY, 0777);
	dup2(fd, 0);
}

void redirectOut(char * newOutPath) {
	//redirect stdout using newOut
	int fd = open(newOutPath, O_CREAT | O_TRUNC | O_WRONLY, 0777);
	dup2(fd, 1);
}
