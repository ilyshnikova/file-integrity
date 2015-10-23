#include <iostream>
#include "daemon.h"


int main() {
	TerminalClient("127.0.0.1", "8081").Process();
	return 0;
}
