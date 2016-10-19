#include "stdafx.h"

#include "MMC.h"

#include "NESPPU.h"
#include <iostream>


const int WINDOW_WIDTH = 256;
const int WINDOW_HEIGHT = 240;

NESPPU * NESPPU::_NESPPU;

NESPPU* NESPPU::getInstance(MMC * _MMC) {
	if (_NESPPU == NULL) {
		_NESPPU = new NESPPU(_MMC);
	}
	return _NESPPU;
};

NESPPU::NESPPU(MMC * _MMC) {

	xlocation = 0;
	ylocation = 0;
	mMMC = _MMC;
	evenframe = false;



	SDL_Init(SDL_INIT_VIDEO);
	SDL_CreateWindowAndRenderer(WINDOW_WIDTH, WINDOW_WIDTH, 0, &mWindow, &mRenderer);
	SDL_SetRenderDrawColor(mRenderer, 0, 0, 0, 0);
	SDL_RenderClear(mRenderer);
	

};

NESPPU::~NESPPU() {
	SDL_DestroyRenderer(mRenderer);
	SDL_DestroyWindow(mWindow);
	SDL_Quit();
}

void NESPPU::drawPixel(int x, int y, int red, int green, int blue) {


		SDL_SetRenderDrawColor(mRenderer, red, green, blue, 255);
		SDL_RenderDrawPoint(mRenderer,x,y);

}

void NESPPU::setPPUReg(PPURegister _reg, unsigned char _value, bool set) {
	unsigned char temp_reg;
	int address;
	switch (_reg) {
	case PPUSTATUS:
		address = 0x2002;
		break;
	}

	temp_reg = mMMC->getMEMByte(address);

	if (set) {
		mMMC->setMEMByte(address, temp_reg | _value);
	}
	else {
		mMMC->setMEMByte(address, temp_reg & ~(_value));
	}


}

int NESPPU::execute() {
	if (SDL_PollEvent(&mEvent) && mEvent.type == SDL_QUIT) return 0;

	SDL_SetRenderDrawColor(mRenderer, xlocation, ylocation, 128, 255);
	SDL_RenderDrawPoint(mRenderer,xlocation,ylocation);


	evenframe = !evenframe;

	if (xlocation == 341) {
		ylocation++;
		xlocation = 0;
	}

	ylocation %= 262;

	if ((ylocation == 241) && (xlocation == 1)) {
		setPPUReg(PPUSTATUS, 0x80, true); //Set Vblank
		SDL_RenderPresent(mRenderer);
		SDL_SetRenderDrawColor(mRenderer, 0, 0, 0, 0);
		SDL_RenderClear(mRenderer);
		std::cout << "VBLANK" << std::endl;
	}
	if ((ylocation == 261) && (xlocation == 1)) setPPUReg(PPUSTATUS, 0x80, false); //Clear Vblank




	xlocation++;

	return 1;
};