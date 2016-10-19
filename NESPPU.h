#ifndef NESPPU_HEADER
#define NESPPU_HEADER
#include <SDL.h>
class NESPPU{

	enum PPURegister { PPUCTRL, PPUMASK, PPUSTATUS, OAMADDR, OAMDATA, PPUSCROLL, PPUADDR, PPUDATA, OAMDMA };
	enum PPUCtrlElement {NMI, PPUMS, SPRITEHEIGHT, BACKGRNDTILESELECT, SPRITETILESELECT, INCREMENTMODE,
							NAMETABLESELECT};
	enum PPUMaskElement {BLUEE, GREENE, REDE, SPRITEENABLE, BACKGROUNDENABLE, SPRITELCOLUMNENABLE, 
						BACKGROUNDLCOLUMNENABLE, GREYSCALE};
	enum PPUStatusElement {VBLANK, SPRITEZEROHIT, SPRITEOVERFLOW};

public:
	static NESPPU * getInstance(MMC * _MMC);

	int execute();

private:

	static NESPPU * _NESPPU;
	NESPPU::NESPPU(MMC * _MMC);
	NESPPU::~NESPPU();
	void NESPPU::drawPixel(int x, int y, int red, int green, int blue);
	int xlocation;
	int ylocation;
	MMC * mMMC;
	bool evenframe;
	
	void NESPPU::setPPUReg(PPURegister _reg, unsigned char _value, bool set);

	SDL_Event mEvent;
	SDL_Renderer *mRenderer;
	SDL_Window *mWindow;
};

#endif