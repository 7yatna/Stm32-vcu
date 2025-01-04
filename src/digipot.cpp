/*
 * This file is part of the ZombieVeter project.
 *
 * Copyright (C) 2020 Johannes Huebner <dev@johanneshuebner.com>
 *               2021-2022 Damien Maguire <info@evbmw.com>
 * Yes I'm really writing software now........run.....run away.......
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

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