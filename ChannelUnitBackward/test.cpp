#include <stdio.h>

#include "ChannelUnitBackward.h"

int main() {
    ChannelUnitBackward* unit = new ChannelUnitBackward();
	unit->Init();
	unit->Test();
	unit->Run();
}
