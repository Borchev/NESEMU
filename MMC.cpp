#include "stdafx.h"
#include "MMC.h"
#include <iostream>
#include <fstream>
#include <algorithm>

using namespace std;

#define TRACE_PPU_ACCESS false

MMC::MMC() {
	memorypool = new unsigned char[0xFFFF]();
};

unsigned char MMC::getMEMByte(int location) {

	unsigned char temp; //used to store mem before destructuve reads
	if ((location & 0xE000) == 0x2000)
	{
		location &= 0x2007;
		if (TRACE_PPU_ACCESS) cout << "PPU REGISTER ADDRESS ACCESSED:" << location << endl;
		
	}
	else if ((location & 0xE000) == 0x0) {
		location &= 0x07FF;
	}
	temp = memorypool[location];
	if (location == 0x2002) memorypool[0x2002] &= 0x7F;
	return temp;

	
};

void MMC::setMEMByte(int location, int value) {
	memorypool[location] = value;

}

void MMC::loadROM(char * filename) {
	streampos size;
	char * memblock;

	ifstream file(filename, ios::in | ios::binary | ios::ate);
	if (file.is_open())
	{
		size = file.tellg();
		memblock = new char[size];
		file.seekg(0, ios::beg);
		file.read(memblock, size);
		file.close();

		cout << "the entire file content is in memory";
		memcpy(&memorypool[0x8000], &memblock[16], 16384 * 2 * sizeof(char));
		delete[] memblock;



		//char temp_memblock[] = { 0x20, 0x09, 0x80, 0x20, 0x0c, 0x80, 0x20, 0x12, 0x80, 0xa2, 0x00, 0x60, 0xe8, 0xe0, 0x05, 0xd0, 0xfb, 0x60, 0x00 };
		//memcpy(&memorypool[0x8000], &temp_memblock[0], 19);
	}
	else cout << "Unable to open file";

}