#include "digipot.h"
#include "utils.h"



void DigiPot::SetPot1Step() {
	int wip1 = 0;
	int wip1val = GetInt(Param::SOC);
	
	wip1 = utils::change(wip1val, 0, 100, 0, 26);

	Param::SetInt(Param::DigiPot1Step, wip1);
    DigIo::pot1_cs.Clear();
    spi_xfer(SPI3, wip1);
	DigIo::pot1_cs.Set();
}

void DigiPot::SetPot2Step() {
	int wip2 = 0;
	int wip2val = GetInt(Param::SOC);
	
	wip2 = utils::change(wip2val, 0, 100, 0, 26);
	
	Param::SetInt(Param::DigiPot2Step, wip2);
    DigIo::pot2_cs.Clear();
    spi_xfer(SPI3, wip2);
	DigIo::pot2_cs.Set();
}