#include <stdio.h>

#include "ChannelUnitForward.h"

int main() {
    ChannelUnitForward* unit = new ChannelUnitForward();
	unit->Init();
	unit->Test();
	unit->Run();
}
