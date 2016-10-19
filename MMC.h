#ifndef MMC_HEADER
#define MMC_HEADER
class MMC {


public:
	MMC();
	unsigned char getMEMByte(int location);
	void setMEMByte(int location, int value);
	void loadROM(char * filename);

private:
	unsigned char * memorypool;

};


#endif 