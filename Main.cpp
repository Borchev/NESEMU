// ConsoleApplication17.cpp : Defines the entry point for the console application.
//
#include "stdafx.h"
#include "NESCPU.h"
#include "NESPPU.h"


#include "MMC.h"

#include <iostream>
#include <SDL.h>

int initEmu();
NESCPU * main_cpu;
NESPPU * main_ppu;

using namespace std;


int main(int argx, char* argv[]) {
	
	initEmu();
	bool running = true;

	while (running) {
		running = main_ppu->execute();
		running = main_ppu->execute();
		running = main_ppu->execute();
		main_cpu->execute();

	}
	
	return 0;
}

int initEmu()
{


	MMC * _MMC = new MMC();

	_MMC->loadROM("./donkey.nes");

	main_cpu = NESCPU::getInstance(_MMC);
	main_ppu = NESPPU::getInstance(_MMC);
	

		return 0;
}

