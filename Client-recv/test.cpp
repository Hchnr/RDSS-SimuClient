#include <stdio.h>

#include "Client.h"

int main() {
    Client* unit = new Client();
	unit->Init();
	unit->Test();
	unit->Run();
}
