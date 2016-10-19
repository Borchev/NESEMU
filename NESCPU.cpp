#include "stdafx.h"

#include "NESCPU.h" 
#include "MMC.h"
#include <iostream>
#include <iomanip>

NESCPU * NESCPU::_NESCPU;

#define TRACE_CPU false

NESCPU* NESCPU::getInstance(MMC * _MMC) {
	if (_NESCPU == NULL) {
		_NESCPU = new NESCPU(_MMC);		
	}
	return _NESCPU;
};

NESCPU::NESCPU(MMC * _MMC) {
	//_OPCodes = new OPCode[255];
	loadOpCodes();
	mMMC = _MMC;
	reg_A = 0;
	reg_PC = getIRQAddress(RESET);
	reg_SP = 0xFF;
	reg_STATUS = 0;
	reg_X = 0;
	reg_Y = 0;
	IRQline = false;
};

inline int NESCPU::getIRQAddress(IRQTable _vector) {

	return ((mMMC->getMEMByte(_vector+1) << 8) | mMMC->getMEMByte(_vector)) & 0xFFFF;

}

int NESCPU::execute() {

	bool oldIFlag = getStatusRegister(INTERRUPT);
	int operand = 0x00;
	int oldReg_PC = reg_PC;
	unsigned char opcode = mMMC->getMEMByte(reg_PC);

	
	if (TRACE_CPU) {
		std::cout << "Opcode: " << _OPCodes[opcode].OPName << std::endl;
		std::cout << "Address: ";

		for (int i = 1; i < _OPCodes[opcode].length; i++) std::cout << std::setfill('0') << std::setw(2) << mMMC->getMEMByte(reg_PC + i);
		std::cout << std::endl;

		std::cout << "Addressing mode: " << _OPCodes[opcode]._Addressingmode << std::endl;
		std::cout << "Operation length: " << _OPCodes[opcode].length << std::endl;
		std::cout << "Operation cycles: " << _OPCodes[opcode].cycles << std::endl;
	}

	if (_OPCodes[opcode].length > 2) operand = mMMC->getMEMByte(reg_PC + 2) << 8;
	if (_OPCodes[opcode].length > 1) operand |= mMMC->getMEMByte(reg_PC + 1);
	else operand = 0;

	if (_OPCodes[opcode].opcode_handler != NULL) {

		operand = getActualMemAddress(_OPCodes[opcode]._Addressingmode, operand);

		if ((_OPCodes[opcode]._Addressingmode != ACUMULATOR) && (_OPCodes[opcode]._Addressingmode != IMMEDIATE)) {
			
			(this->*_OPCodes[opcode].opcode_handler)(operand, false);
		}
		else {

			(this->*_OPCodes[opcode].opcode_handler)(operand, true);
		}
	}
	reg_PC += _OPCodes[mMMC->getMEMByte(oldReg_PC)].length; 


	
	if (TRACE_CPU) {
		std::cout << "After operation A reg: " << (int)reg_A << std::endl;
		std::cout << "After operation X reg: " << (int)reg_X << std::endl;
		std::cout << "After operation Y reg: " << (int)reg_Y << std::endl;
		std::cout << "After operation PC reg: " << (int)reg_PC << std::endl;
		std::cout << "After operation SP reg: " << (int)reg_SP << std::endl;
		std::cout << "Flags After operation" << (int)reg_STATUS << std::endl;
	}


	//use previous if SEI, PLP, CLI else use current state 
	if ((mMMC->getMEMByte(oldReg_PC) != 0x78) && (mMMC->getMEMByte(oldReg_PC) != 0x58) && (mMMC->getMEMByte(oldReg_PC) != 0x28)) oldIFlag = getStatusRegister(INTERRUPT);


	if (NMIline) {

		interruptHandler(NMI);

	}
	else if (oldIFlag && IRQline) { //IRQ 

		interruptHandler(IRQ);
	}


	

	return true;
};

void NESCPU::interruptHandler(IRQTable _IRQType) {

	//duplicated code ---fix, just PHP and JSR

	mMMC->setMEMByte(reg_SP + 256, (reg_PC + 2) >> 8);
	reg_SP--;
	mMMC->setMEMByte(reg_SP + 256, (reg_PC + 2) & 0x00FF);
	reg_SP--;

	mMMC->setMEMByte(reg_SP + 256, reg_STATUS);
	setStatusRegister(ALWAYS_ONE);
	setStatusRegister(BREAK);
	reg_SP--;
	reg_PC = getIRQAddress(_IRQType);

}


void NESCPU::loadOpCodes() {
	defineOPCode(0x00, IMPLICIT, 7, 2, " BRK", &NESCPU::BRK_Handler);
	defineOPCode(0x01, INDEXED_INDIRECT, 6, 2, "ORA", &NESCPU::ORA_Handler);
	defineOPCode(0x02, INVALID, 0, 0, "NULL", NULL);
	defineOPCode(0x03, INVALID, 0, 0, "NULL", NULL);
	defineOPCode(0x04, INVALID, 0, 0, "NULL", NULL);
	defineOPCode(0x05, ZEROPAGE, 3, 2, "ORA", &NESCPU::ORA_Handler);
	defineOPCode(0x06, ZEROPAGE, 5, 2, "ASL",  &NESCPU::ASL_Handler);
	defineOPCode(0x07, INVALID, 0, 0, "NULL", NULL);
	defineOPCode(0x08, IMPLICIT, 3, 1, "PHP", &NESCPU::PHP_Handler);
	defineOPCode(0x09, IMMEDIATE, 2, 2, "ORA", &NESCPU::ORA_Handler);
	defineOPCode(0x0A, ACUMULATOR, 2, 1, "ASL", &NESCPU::ASL_Handler);
	defineOPCode(0x0B, INVALID, 0, 0, "NULL",NULL);
	defineOPCode(0x0C, INVALID, 0, 0, "NULL",NULL);
	defineOPCode(0x0D, ABSOLUTEA, 4, 3, "ORA", &NESCPU::ORA_Handler);
	defineOPCode(0x0E, ABSOLUTEA, 6, 3, "ASL", &NESCPU::ASL_Handler);
	defineOPCode(0x0F, INVALID, 0, 0, "NULL",NULL);
	defineOPCode(0x10, RELATIVEA, 255, 2, "BPL", &NESCPU::BPL_Handler);
	defineOPCode(0x11, INDIRECT_INDEXED, 5, 2, "ORA", &NESCPU::ORA_Handler);
	defineOPCode(0x12, INVALID, 0, 0, "NULL",NULL);
	defineOPCode(0x13, INVALID, 0, 0, "NULL",NULL);
	defineOPCode(0x14, INVALID, 0, 0, "NULL",NULL);
	defineOPCode(0x15, ZEROPAGE_INDEXEDX, 4, 2, "ORA", &NESCPU::ORA_Handler);
	defineOPCode(0x16, ZEROPAGE_INDEXEDX, 6, 2, "ASL", &NESCPU::ASL_Handler);
	defineOPCode(0x17, INVALID, 0, 0, "NULL",NULL);
	defineOPCode(0x18, IMPLICIT, 2, 1, "CLC", &NESCPU::CLC_Handler);
	defineOPCode(0x19, ABSOLUTEA_INDEXEDY, 4, 3, "ORA", &NESCPU::ORA_Handler);
	defineOPCode(0x1A, INVALID, 0, 0, "NULL",NULL);
	defineOPCode(0x1B, INVALID, 0, 0, "NULL",NULL);
	defineOPCode(0x1C, INVALID, 0, 0, "NULL",NULL);
	defineOPCode(0x1D, ABSOLUTEA_INDEXEDX, 4, 3, "ORA", &NESCPU::ORA_Handler);
	defineOPCode(0x1E, ABSOLUTEA_INDEXEDX, 7, 3, "ASL", &NESCPU::ASL_Handler);
	defineOPCode(0x1F, INVALID, 0, 0, "NULL",NULL);
	defineOPCode(0x20, ABSOLUTEA, 6, 3, "JSR", &NESCPU::JSR_Handler);
	defineOPCode(0x21, INDEXED_INDIRECT, 6, 2, "AND", &NESCPU::AND_Handler);
	defineOPCode(0x22, INVALID, 0, 0, "NULL",NULL);
	defineOPCode(0x23, INVALID, 0, 0, "NULL",NULL);
	defineOPCode(0x24, ZEROPAGE, 3, 2, "BIT", &NESCPU::BIT_Handler);
	defineOPCode(0x25, ZEROPAGE, 3, 2, "AND", &NESCPU::AND_Handler);
	defineOPCode(0x26, ZEROPAGE, 5, 2, "ROL", &NESCPU::ROL_Handler);
	defineOPCode(0x27, INVALID, 0, 0, "NULL",NULL);
	defineOPCode(0x28, IMPLICIT, 4, 1, "PLP", &NESCPU::PLP_Handler);
	defineOPCode(0x29, IMMEDIATE, 2, 2, "AND", &NESCPU::AND_Handler);
	defineOPCode(0x2A, ACUMULATOR, 2, 1, "ROL", &NESCPU::ROL_Handler);
	defineOPCode(0x2B, INVALID, 0, 0, "NULL",NULL);
	defineOPCode(0x2C, ABSOLUTEA, 4, 3, "BIT", &NESCPU::BIT_Handler);
	defineOPCode(0x2D, ABSOLUTEA, 4, 3, "AND", &NESCPU::AND_Handler);
	defineOPCode(0x2E, ABSOLUTEA, 6, 3, "ROL", &NESCPU::ROL_Handler);
	defineOPCode(0x2F, INVALID, 0, 0, "NULL",NULL);
	defineOPCode(0x30, RELATIVEA, 255, 2, "BMI", &NESCPU::BMI_Handler);
	defineOPCode(0x31, INDIRECT_INDEXED, 5, 2, "AND", &NESCPU::AND_Handler);
	defineOPCode(0x32, INVALID, 0, 0, "NULL",NULL);
	defineOPCode(0x33, INVALID, 0, 0, "NULL",NULL);
	defineOPCode(0x34, INVALID, 0, 0, "NULL",NULL);
	defineOPCode(0x35, ZEROPAGE_INDEXEDX, 4, 2, "AND", &NESCPU::AND_Handler);
	defineOPCode(0x36, ZEROPAGE_INDEXEDX, 6, 2, "ROL", &NESCPU::ROL_Handler);
	defineOPCode(0x37, INVALID, 0, 0, "NULL",NULL);
	defineOPCode(0x38, IMPLICIT, 2, 1, "SEC", &NESCPU::SEC_Handler);
	defineOPCode(0x39, ABSOLUTEA_INDEXEDY, 4, 3, "AND", &NESCPU::AND_Handler);
	defineOPCode(0x3A, INVALID, 0, 0, "NULL",NULL);
	defineOPCode(0x3B, INVALID, 0, 0, "NULL",NULL);
	defineOPCode(0x3C, INVALID, 0, 0, "NULL",NULL);
	defineOPCode(0x3D, ABSOLUTEA_INDEXEDX, 4, 3, "AND", &NESCPU::AND_Handler);
	defineOPCode(0x3E, ABSOLUTEA_INDEXEDX, 7, 3, "ROL", &NESCPU::ROL_Handler);
	defineOPCode(0x3F, INVALID, 0, 0, "NULL",NULL);
	defineOPCode(0x40, IMPLICIT, 6, 1, "RTI", &NESCPU::RTI_Handler);
	defineOPCode(0x41, INDEXED_INDIRECT, 6, 2, "EOR", &NESCPU::EOR_Handler);
	defineOPCode(0x42, INVALID, 0, 0, "NULL",NULL);
	defineOPCode(0x43, INVALID, 0, 0, "NULL",NULL);
	defineOPCode(0x44, ZEROPAGE, 5, 2, "NULL",NULL);
	defineOPCode(0x45, ZEROPAGE, 3, 2, "EOR", &NESCPU::EOR_Handler);
	defineOPCode(0x46, ZEROPAGE, 5, 2, "LSR", &NESCPU::LSR_Handler);
	defineOPCode(0x47, INVALID, 0, 0, "NULL",NULL);
	defineOPCode(0x48, IMPLICIT, 3, 1, "PHA", &NESCPU::PHA_Handler);
	defineOPCode(0x49, IMMEDIATE, 2, 2, "EOR", &NESCPU::EOR_Handler);
	defineOPCode(0x4A, ACUMULATOR, 2, 1, "LSR", &NESCPU::LSR_Handler);
	defineOPCode(0x4B, INVALID, 0, 0, "NULL",NULL);
	defineOPCode(0x4C, ABSOLUTEA, 3, 3, "JMP", &NESCPU::JMP_Handler);
	defineOPCode(0x4D, ABSOLUTEA, 4, 3, "EOR", &NESCPU::EOR_Handler);
	defineOPCode(0x4E, ABSOLUTEA, 6, 3, "LSR", &NESCPU::LSR_Handler);
	defineOPCode(0x4F, INVALID, 0, 0, "NULL",NULL);
	defineOPCode(0x50, RELATIVEA, 255, 2, "BVC", &NESCPU::BVC_Handler);
	defineOPCode(0x51, INDIRECT_INDEXED, 5, 2, "EOR", &NESCPU::EOR_Handler);
	defineOPCode(0x52, INVALID, 0, 0, "NULL",NULL);
	defineOPCode(0x53, INVALID, 0, 0, "NULL",NULL);
	defineOPCode(0x54, INVALID, 0, 0, "NULL",NULL);
	defineOPCode(0x55, ZEROPAGE_INDEXEDX, 4, 2, "EOR", &NESCPU::EOR_Handler);
	defineOPCode(0x56, ZEROPAGE_INDEXEDX, 6, 2, "LSR", &NESCPU::LSR_Handler);
	defineOPCode(0x57, INVALID, 0, 0, "NULL",NULL);
	defineOPCode(0x58, IMPLICIT, 2, 1, "CLI", &NESCPU::CLI_Handler);
	defineOPCode(0x59, ABSOLUTEA_INDEXEDY, 4, 3, "EOR", &NESCPU::EOR_Handler);
	defineOPCode(0x5A, INVALID, 0, 0, "NULL",NULL);
	defineOPCode(0x5B, INVALID, 0, 0, "NULL",NULL);
	defineOPCode(0x5C, INVALID, 0, 0, "NULL",NULL);
	defineOPCode(0x5D, ABSOLUTEA_INDEXEDX, 4, 3, "EOR", &NESCPU::EOR_Handler);
	defineOPCode(0x5E, ABSOLUTEA_INDEXEDX, 7, 3, "LSR", &NESCPU::LSR_Handler);
	defineOPCode(0x5F, INVALID, 0, 0, "NULL",NULL);
	defineOPCode(0x60, IMPLICIT, 6, 1, "RTS", &NESCPU::RTS_Handler);
	defineOPCode(0x61, INDEXED_INDIRECT, 6, 2, "ADC", &NESCPU::ADC_Handler);
	defineOPCode(0x62, INVALID, 0, 0, "NULL",NULL);
	defineOPCode(0x63, INVALID, 0, 0, "NULL",NULL);
	defineOPCode(0x64, INVALID, 0, 0, "NULL",NULL);
	defineOPCode(0x65, ZEROPAGE, 3, 2, "ADC", &NESCPU::ADC_Handler);
	defineOPCode(0x66, ZEROPAGE, 5, 2, "ROR", &NESCPU::ROR_Handler);
	defineOPCode(0x67, INVALID, 0, 0, "NULL",NULL);
	defineOPCode(0x68, IMPLICIT, 4, 1, "PLA", &NESCPU::PLA_Handler);
	defineOPCode(0x69, IMMEDIATE, 2, 2, "ADC", &NESCPU::ADC_Handler);
	defineOPCode(0x6A, ACUMULATOR, 2, 1, "ROR", &NESCPU::ROR_Handler);
	defineOPCode(0x6B, INVALID, 0, 0, "NULL",NULL);
	defineOPCode(0x6C, INDIRECT, 5, 3, "JMP", &NESCPU::JMP_Handler);
	defineOPCode(0x6D, ABSOLUTEA, 4, 3, "ADC", &NESCPU::ADC_Handler);
	defineOPCode(0x6E, ABSOLUTEA, 6, 3, "ROR", &NESCPU::ROR_Handler);
	defineOPCode(0x6F, INVALID, 0, 0, "NULL",NULL);
	defineOPCode(0x70, RELATIVEA, 255, 2, "BVS", &NESCPU::BVS_Handler);
	defineOPCode(0x71, INDIRECT_INDEXED, 5, 2, "ADC", &NESCPU::ADC_Handler);
	defineOPCode(0x72, INVALID, 0, 0, "NULL",NULL);
	defineOPCode(0x73, INVALID, 0, 0, "NULL",NULL);
	defineOPCode(0x74, INVALID, 0, 0, "NULL",NULL);
	defineOPCode(0x75, ZEROPAGE_INDEXEDX, 4, 2, "ADC", &NESCPU::ADC_Handler);
	defineOPCode(0x76, ZEROPAGE_INDEXEDX, 6, 2, "ROR", &NESCPU::ROR_Handler);
	defineOPCode(0x77, INVALID, 0, 0, "NULL",NULL);
	defineOPCode(0x78, IMPLICIT, 2, 1, "SEI", &NESCPU::SEI_Handler);
	defineOPCode(0x79, ABSOLUTEA_INDEXEDY, 4, 2, "ADC", &NESCPU::ADC_Handler);
	defineOPCode(0x7A, INVALID, 0, 0, "NULL",NULL);
	defineOPCode(0x7B, INVALID, 0, 0, "NULL",NULL);
	defineOPCode(0x7C, INVALID, 0, 0, "NULL",NULL);
	defineOPCode(0x7D, ABSOLUTEA_INDEXEDX, 4, 3, "ADC", &NESCPU::ADC_Handler);
	defineOPCode(0x7E, ABSOLUTEA_INDEXEDX, 7, 3, "ROR", &NESCPU::ROR_Handler);
	defineOPCode(0x7F, INVALID, 0, 0, "NULL",NULL);
	defineOPCode(0x80, INVALID, 0, 0, "NULL",NULL);
	defineOPCode(0x81, INDEXED_INDIRECT, 6, 2, "STA", &NESCPU::STA_Handler);
	defineOPCode(0x82, INVALID, 0, 0, "NULL",NULL);
	defineOPCode(0x83, INVALID, 0, 0, "NULL",NULL);
	defineOPCode(0x84, ZEROPAGE, 3, 2, "STY", &NESCPU::STY_Handler);
	defineOPCode(0x85, ZEROPAGE, 3, 2, "STA", &NESCPU::STA_Handler);
	defineOPCode(0x86, ZEROPAGE, 3, 2, "STX", &NESCPU::STX_Handler);
	defineOPCode(0x87, INVALID, 0, 0, "NULL",NULL);
	defineOPCode(0x88, IMPLICIT, 2, 1, "DEY", &NESCPU::DEY_Handler);
	defineOPCode(0x89, INVALID, 0, 0, "NULL",NULL);
	defineOPCode(0x8A, IMPLICIT, 2, 1, "TXA", &NESCPU::TXA_Handler);
	defineOPCode(0x8B, INVALID, 0, 0, "NULL",NULL);
	defineOPCode(0x8C, ABSOLUTEA, 4, 3, "STY", &NESCPU::STY_Handler);
	defineOPCode(0x8D, ABSOLUTEA, 4, 3, "STA", &NESCPU::STA_Handler);
	defineOPCode(0x8E, ABSOLUTEA, 4, 3, "STX", &NESCPU::STX_Handler);
	defineOPCode(0x8F, INVALID, 0, 0, "NULL",NULL);
	defineOPCode(0x90, RELATIVEA, 255, 2, "BCC", &NESCPU::BCC_Handler);
	defineOPCode(0x91, INDIRECT_INDEXED, 6, 2, "STA", &NESCPU::STA_Handler);
	defineOPCode(0x92, INVALID, 0, 0, "NULL",NULL);
	defineOPCode(0x93, INVALID, 0, 0, "NULL",NULL);
	defineOPCode(0x94, ZEROPAGE_INDEXEDX, 4, 2, "STY", &NESCPU::STY_Handler);
	defineOPCode(0x95, ZEROPAGE_INDEXEDX, 4, 2, "STA", &NESCPU::STA_Handler);
	defineOPCode(0x96, ZEROPAGE_INDEXEDY, 4, 2, "STX", &NESCPU::STX_Handler);
	defineOPCode(0x97, INVALID, 0, 0, "NULL",NULL);
	defineOPCode(0x98, IMPLICIT, 2, 1, "TYA", &NESCPU::TYA_Handler);
	defineOPCode(0x99, ABSOLUTEA_INDEXEDY, 5, 3, "STA", &NESCPU::STA_Handler);
	defineOPCode(0x9A, IMPLICIT, 2, 1, "TXS", &NESCPU::TXS_Handler);
	defineOPCode(0x9B, INVALID, 0, 0, "NULL",NULL);
	defineOPCode(0x9C, INVALID, 0, 0, "NULL",NULL);
	defineOPCode(0x9D, ABSOLUTEA_INDEXEDX, 5, 3, "STA", &NESCPU::STA_Handler);
	defineOPCode(0x9E, INVALID, 0, 0, "NULL",NULL);
	defineOPCode(0x9F, INVALID, 0, 0, "NULL",NULL);

	defineOPCode(0xA0, IMMEDIATE, 2, 2, "LDY", &NESCPU::LDY_Handler);
	defineOPCode(0xA1, INDEXED_INDIRECT, 6, 2, "LDA", &NESCPU::LDA_Handler);
	defineOPCode(0xA2, IMMEDIATE, 2, 2, "LDX", &NESCPU::LDX_Handler);
	defineOPCode(0xA3, INVALID, 0, 0, "NULL",NULL);
	defineOPCode(0xA4, ZEROPAGE, 3, 2, "LDY", &NESCPU::LDY_Handler);
	defineOPCode(0xA5, ZEROPAGE, 3, 2, "LDA", &NESCPU::LDA_Handler);
	defineOPCode(0xA6, ZEROPAGE, 3, 2, "LDX", &NESCPU::LDX_Handler);
	defineOPCode(0xA7, INVALID, 0, 0, "NULL",NULL);
	defineOPCode(0xA8, IMPLICIT, 2, 1, "TAY", &NESCPU::TAY_Handler);
	defineOPCode(0xA9, IMMEDIATE, 2, 2, "LDA", &NESCPU::LDA_Handler);
	defineOPCode(0xAA, IMPLICIT, 2, 1, "TAX", &NESCPU::TAX_Handler);
	defineOPCode(0xAB, INVALID, 0, 0, "NULL",NULL);
	defineOPCode(0xAC, ABSOLUTEA, 4, 3, "LDY", &NESCPU::LDY_Handler);
	defineOPCode(0xAD, ABSOLUTEA, 4, 3, "LDA", &NESCPU::LDA_Handler);
	defineOPCode(0xAE, ABSOLUTEA, 4, 3, "LDX", &NESCPU::LDX_Handler);
	defineOPCode(0xAF, INVALID, 0, 0, "NULL",NULL);
	defineOPCode(0xB0, RELATIVEA, 255, 2, "BCS", &NESCPU::BCS_Handler);
	defineOPCode(0xB1, INDIRECT_INDEXED, 5, 2, "LDA", &NESCPU::LDA_Handler);
	defineOPCode(0xB2, INVALID, 0, 0, "NULL",NULL);
	defineOPCode(0xB3, INVALID, 0, 0, "NULL",NULL);
	defineOPCode(0xB4, ZEROPAGE_INDEXEDX, 4, 2, "LDY", &NESCPU::LDY_Handler);
	defineOPCode(0xB5, ZEROPAGE_INDEXEDX, 4, 2, "LDA", &NESCPU::LDA_Handler);
	defineOPCode(0xB6, ZEROPAGE_INDEXEDY, 4, 2, "LDX", &NESCPU::LDX_Handler);
	defineOPCode(0xB7, INVALID, 0, 0, "NULL",NULL);
	defineOPCode(0xB8, IMPLICIT, 2, 1, "CLV", &NESCPU::CLV_Handler);
	defineOPCode(0xB9, ABSOLUTEA_INDEXEDY, 4, 3, "LDA", &NESCPU::LDA_Handler);
	defineOPCode(0xBA, IMPLICIT, 2, 1, "TSX", &NESCPU::TSX_Handler);
	defineOPCode(0xBB, INVALID, 0, 0, "NULL",NULL);
	defineOPCode(0xBC, ABSOLUTEA_INDEXEDX, 4, 3, "LDY", &NESCPU::LDY_Handler);
	defineOPCode(0xBD, ABSOLUTEA_INDEXEDX, 4, 3, "LDA", &NESCPU::LDA_Handler);
	defineOPCode(0xBE, ABSOLUTEA_INDEXEDY, 4, 3, "LDX", &NESCPU::LDX_Handler);
	defineOPCode(0xBF, INVALID, 0, 0, "NULL",NULL);
	defineOPCode(0xC0, IMMEDIATE, 2, 2, "CPY", &NESCPU::CPY_Handler);
	defineOPCode(0xC1, INDEXED_INDIRECT, 6, 2, "CMP", &NESCPU::CMP_Handler);
	defineOPCode(0xC2, INVALID, 0, 0, "NULL",NULL);
	defineOPCode(0xC3, INVALID, 0, 0, "NULL",NULL);
	defineOPCode(0xC4, ZEROPAGE, 3, 2, "CPY", &NESCPU::CPY_Handler);
	defineOPCode(0xC5, ZEROPAGE, 3, 2, "CMP", &NESCPU::CMP_Handler);
	defineOPCode(0xC6, ZEROPAGE, 5, 2, "DEC", &NESCPU::DEC_Handler);
	defineOPCode(0xC7, INVALID, 0, 0, "NULL",NULL);
	defineOPCode(0xC8, IMPLICIT, 2, 1, "INY", &NESCPU::INY_Handler);
	defineOPCode(0xC9, IMMEDIATE, 2, 2, "CMP", &NESCPU::CMP_Handler);
	defineOPCode(0xCA, IMPLICIT, 2, 1, "DEX", &NESCPU::DEX_Handler);
	defineOPCode(0xCB, INVALID, 0, 0, "NULL",NULL);
	defineOPCode(0xCC, ABSOLUTEA, 4, 3, "CPY", &NESCPU::CPY_Handler);
	defineOPCode(0xCD, ABSOLUTEA, 4, 3, "CMP", &NESCPU::CMP_Handler);
	defineOPCode(0xCE, ABSOLUTEA, 6, 3, "DEC", &NESCPU::DEC_Handler);
	defineOPCode(0xCF, INVALID, 0, 0, "NULL",NULL);
	defineOPCode(0xD0, RELATIVEA, 2, 2, "BNE", &NESCPU::BNE_Handler);
	defineOPCode(0xD1, INDIRECT_INDEXED, 5, 2, "CMP", &NESCPU::CMP_Handler);
	defineOPCode(0xD2, INVALID, 0, 0, "NULL",NULL);
	defineOPCode(0xD3, INVALID, 0, 0, "NULL",NULL);
	defineOPCode(0xD4, INVALID, 0, 0, "NULL",NULL);
	defineOPCode(0xD5, ZEROPAGE_INDEXEDX, 4, 2, "CMP", &NESCPU::CMP_Handler);
	defineOPCode(0xD6, ZEROPAGE_INDEXEDX, 6, 2, "DEC", &NESCPU::DEC_Handler);
	defineOPCode(0xD7, INVALID, 0, 0, "NULL",NULL);
	defineOPCode(0xD8, IMPLICIT, 2, 1, "CLD", &NESCPU::CLD_Handler);
	defineOPCode(0xD9, ABSOLUTEA_INDEXEDY, 4, 3, "CMP", &NESCPU::CMP_Handler);
	defineOPCode(0xDA, INVALID, 0, 0, "NULL",NULL);
	defineOPCode(0xDB, INVALID, 0, 0, "NULL",NULL);
	defineOPCode(0xDC, INVALID, 0, 0, "NULL",NULL);
	defineOPCode(0xDD, ABSOLUTEA_INDEXEDX, 4, 3, "CMP", &NESCPU::CMP_Handler);
	defineOPCode(0xDE, ABSOLUTEA_INDEXEDX, 7, 3, "DEC", &NESCPU::DEC_Handler);
	defineOPCode(0xDF, INVALID, 0, 0, "NULL",NULL);
	defineOPCode(0xE0, IMMEDIATE, 2, 2, "CPX", &NESCPU::CPX_Handler);
	defineOPCode(0xE1, INDEXED_INDIRECT, 6, 2, "SBC", &NESCPU::SBC_Handler);
	defineOPCode(0xE2, INVALID, 0, 0, "NULL",NULL);
	defineOPCode(0xE3, INVALID, 0, 0, "NULL",NULL);
	defineOPCode(0xE4, ZEROPAGE, 3, 2, "CPX", &NESCPU::CPX_Handler);
	defineOPCode(0xE5, ZEROPAGE, 3, 2, "SBC", &NESCPU::SBC_Handler);
	defineOPCode(0xE6, ZEROPAGE, 5, 2, "INC", &NESCPU::INC_Handler);
	defineOPCode(0xE7, INVALID, 0, 0, "NULL",NULL);
	defineOPCode(0xE8, IMPLICIT, 2, 1, "INX", &NESCPU::INX_Handler);
	defineOPCode(0xE9, IMMEDIATE, 2, 2, "SBC", &NESCPU::SBC_Handler);
	defineOPCode(0xEA, IMPLICIT, 2, 1, "NOP",NULL);
	defineOPCode(0xEB, INVALID, 0, 0, "NULL",NULL);
	defineOPCode(0xEC, ABSOLUTEA, 4, 3, "CPX", &NESCPU::CPX_Handler);
	defineOPCode(0xED, ABSOLUTEA, 4, 3, "SBC", &NESCPU::SBC_Handler);
	defineOPCode(0xEE, ABSOLUTEA, 6, 3, "INC", &NESCPU::INC_Handler);
	defineOPCode(0xEF, INVALID, 0, 0, "NULL",NULL);
	defineOPCode(0xF0, RELATIVEA, 255, 2, "BEQ", &NESCPU::BEQ_Handler);
	defineOPCode(0xF1, INDIRECT_INDEXED, 5, 2, "CPX", &NESCPU::CPX_Handler);
	defineOPCode(0xF2, INVALID, 0, 0, "NULL",NULL);
	defineOPCode(0xF3, INVALID, 0, 0, "NULL",NULL);
	defineOPCode(0xF4, INVALID, 0, 0, "NULL",NULL);
	defineOPCode(0xF5, ZEROPAGE_INDEXEDX, 4, 2, "SBC", &NESCPU::SBC_Handler);
	defineOPCode(0xF6, ZEROPAGE_INDEXEDX, 6, 2, "INC", &NESCPU::INC_Handler);
	defineOPCode(0xF7, INVALID, 0, 0, "NULL",NULL);
	defineOPCode(0xF8, IMPLICIT, 2, 1, "SED", &NESCPU::SED_Handler);
	defineOPCode(0xF9, ABSOLUTEA_INDEXEDY, 4, 3, "SBC", &NESCPU::SBC_Handler);
	defineOPCode(0xFA, INVALID, 0, 0, "NULL",NULL);
	defineOPCode(0xFB, INVALID, 0, 0, "NULL",NULL);
	defineOPCode(0xFC, INVALID, 0, 0, "NULL",NULL);
	defineOPCode(0xFD, ABSOLUTEA_INDEXEDX, 4, 3, "SBC", &NESCPU::SBC_Handler);
	defineOPCode(0xFE, ABSOLUTEA_INDEXEDX, 7, 3, "INC", &NESCPU::INC_Handler);
	defineOPCode(0xFF, INVALID, 0, 0, "NULL",NULL);

};

void NESCPU::defineOPCode(unsigned char _code, AddressingMode _mode, int cycles, int length, char * OPName, void(NESCPU::*handler)(int, bool)) {
	_OPCodes[_code]._Addressingmode = _mode;
	_OPCodes[_code].cycles = cycles;
	_OPCodes[_code].length = length;
	_OPCodes[_code].OPName = OPName;
	_OPCodes[_code].opcode_handler = handler;
};

void NESCPU::setStatusRegister(NESRegister _reg) {
	reg_STATUS |= 1 << _reg;
}

void NESCPU::unsetStatusRegister(NESRegister _reg) {
	reg_STATUS &= ~(1 << _reg);
}

bool NESCPU::getStatusRegister(NESRegister _reg) {
	return ((reg_STATUS & (1 << _reg)) != 0);
}


int NESCPU::getActualMemAddress(AddressingMode _mode, int operand) {

	switch (_mode) {
	case RELATIVEA:
	case IMPLICIT:
		return operand;
		break;
	case ACUMULATOR:
		return reg_A & 0xFF;
		break;
	case IMMEDIATE:
		return operand & 0xFF;
		break;
	case ABSOLUTEA:
		return operand & 0xFFFF;
		break;
	case ZEROPAGE:
		return operand & 0xFF;
		break;
	case ZEROPAGE_INDEXEDX:
		return ((operand + reg_X) & 0xFF);
		break;
	case ZEROPAGE_INDEXEDY:
		return ((operand + reg_Y) & 0xFF);
		break;
	case ABSOLUTEA_INDEXEDX:
		return (operand & 0xFFFF) + reg_X;
		break;
	case ABSOLUTEA_INDEXEDY:
		return (operand & 0xFFFF) + reg_Y;
		break;
	case INDIRECT:
		return (mMMC->getMEMByte(operand) | (mMMC->getMEMByte(operand + 1) << 8)) & 0xFFFF; //special addressing for jmp only
		break;
	case INDEXED_INDIRECT:
		return (mMMC->getMEMByte(operand + reg_X) | (mMMC->getMEMByte(operand + reg_X + 1) << 8)) & 0xFFFF;
		break;
	case INDIRECT_INDEXED:
		return ((mMMC->getMEMByte(operand) | (mMMC->getMEMByte(operand + 1) << 8)) + reg_Y) & 0xFFFF;
		break;

	}

}

void NESCPU::CheckNZ(int operand){

	if (((operand >> 7) & 0x1) == 1) setStatusRegister(NEGATIVE);
	else unsetStatusRegister(NEGATIVE);
	if (operand == 0) setStatusRegister(ZERO);
	else unsetStatusRegister(ZERO);

}


void NESCPU::ORA_Handler(int operand, bool writeback) {

	if (!writeback) operand = mMMC->getMEMByte(operand);

	operand |= reg_A;

	CheckNZ(operand);
	
	reg_A = operand;

};

void NESCPU::LDA_Handler(int operand, bool writeback) {

	if (!writeback) operand = mMMC->getMEMByte(operand);

	CheckNZ(operand);
	reg_A = operand;

};

void NESCPU::LDX_Handler(int operand, bool writeback) {

	if (!writeback) operand = mMMC->getMEMByte(operand);

	CheckNZ(operand);
	reg_X = operand;

};

void NESCPU::LDY_Handler(int operand, bool writeback) {

	if (!writeback) operand = mMMC->getMEMByte(operand);

	CheckNZ(operand);
	reg_Y = operand;

};

void NESCPU::ASL_Handler(int operand, bool writeback) {

	int tempAddress;
	if (writeback) {
		tempAddress = operand;
		operand = mMMC->getMEMByte(operand);
	}

	if (((operand >> 7) & 0x1) == 1) setStatusRegister(CARRY);
	else unsetStatusRegister(CARRY);
	operand <<= 1;

	CheckNZ(operand);

	if (writeback) { mMMC->setMEMByte(tempAddress, operand); } //memwrite here
	else  reg_A = operand; 

};

void NESCPU::ROL_Handler(int operand, bool writeback) {

	int tempAddress;
	if (writeback) {
		tempAddress = operand;
		operand = mMMC->getMEMByte(operand);
	}

	bool temp = getStatusRegister(CARRY);
	if (((operand >> 7) & 0x1) == 1) setStatusRegister(CARRY);
	else unsetStatusRegister(CARRY);
	operand <<= 1;
	if (temp) operand += 1;

	CheckNZ(operand);

	if (writeback) { mMMC->setMEMByte(tempAddress,operand); } //memwrite here
	else  reg_A = operand;

};

void NESCPU::ROR_Handler(int operand, bool writeback) {

	int tempAddress;
	if (writeback) {
		tempAddress = operand;
		operand = mMMC->getMEMByte(operand);
	}

	bool temp = getStatusRegister(CARRY);
	if ((operand & 0x1) == 1) setStatusRegister(CARRY);
	else unsetStatusRegister(CARRY);
	operand >>= 1;
	if (temp) operand += (1 << 7);

	CheckNZ(operand);

	if (writeback)  { mMMC->setMEMByte(tempAddress, operand); }//memwrite here
	else reg_A = operand;

};


void NESCPU::LSR_Handler(int operand, bool writeback) {


	int tempAddress;
	if (writeback) {
		 tempAddress = operand;
		operand = mMMC->getMEMByte(operand);
	}

	if ((operand & 0x1) == 1) setStatusRegister(CARRY);
	else unsetStatusRegister(CARRY);
	operand >>= 1;

	CheckNZ(operand);

	if (writeback)  { mMMC->setMEMByte(tempAddress, operand); }//memwrite here
	else reg_A = operand;

};


void NESCPU::AND_Handler(int operand, bool writeback) {

	if (!writeback) operand = mMMC->getMEMByte(operand);

	operand &= reg_A;
	CheckNZ(operand);

	reg_A = operand; //memwrite here
	

};


void NESCPU::ADC_Handler(int operand, bool writeback) {

	if (!writeback) operand = mMMC->getMEMByte(operand);


	int temp = operand + reg_A + (getStatusRegister(CARRY) ? 1 : 0);
	if (temp > 0xFF) setStatusRegister(CARRY);
	else unsetStatusRegister(OVERFLOW_REG);

	

	if ((reg_A ^ temp) & (operand ^ temp) & 0x80) setStatusRegister(OVERFLOW_REG);
	else unsetStatusRegister(OVERFLOW_REG);
	operand = temp & 0xFF;

	CheckNZ(operand);

	reg_A = operand; //memwrite here


};


void NESCPU::TAX_Handler(int operand, bool writeback) {

	operand = reg_A;


	CheckNZ(operand);

	reg_X = operand; //memwrite here


};

void NESCPU::TYA_Handler(int operand, bool writeback) {

	operand = reg_Y;


	CheckNZ(operand);

	reg_A = operand; //memwrite here


};

void NESCPU::TAY_Handler(int operand, bool writeback) {

	operand = reg_A;


	CheckNZ(operand);

	reg_Y = operand; //memwrite here


};

void NESCPU::TXA_Handler(int operand, bool writeback) {

	operand = reg_X;


	CheckNZ(operand);

	reg_A = operand; //memwrite here


};


void NESCPU::INX_Handler(int operand, bool writeback) {

	operand = reg_X;
	operand++;

	CheckNZ(operand);

	reg_X = operand; //memwrite here


};

void NESCPU::INY_Handler(int operand, bool writeback) {

	operand = reg_Y;
	operand++;

	CheckNZ(operand);

	reg_Y = operand; //memwrite here


};

void NESCPU::DEX_Handler(int operand, bool writeback) {



	operand = reg_X;
	operand--;

	CheckNZ(operand);

	reg_X = operand; //memwrite here

};

void NESCPU::DEY_Handler(int operand, bool writeback) {

	

	operand = reg_Y;
	operand--;

	CheckNZ(operand);

	reg_Y = operand; //memwrite here

};

void NESCPU::CMP_Handler(int operand, bool writeback) {

	if (!writeback) operand = mMMC->getMEMByte(operand);

	operand = reg_A - operand;

	CheckNZ(operand);
	if (!getStatusRegister(NEGATIVE)) setStatusRegister(CARRY);
	else unsetStatusRegister(CARRY);
	
};

void NESCPU::CPX_Handler(int operand, bool writeback) {

	if (!writeback) operand = mMMC->getMEMByte(operand);

	operand = reg_X - operand;

	CheckNZ(operand);
	if (!getStatusRegister(NEGATIVE)) setStatusRegister(CARRY);
	else unsetStatusRegister(CARRY);

};

void NESCPU::CPY_Handler(int operand, bool writeback) {

	if (!writeback) operand = mMMC->getMEMByte(operand);

	operand = reg_Y - operand;

	CheckNZ(operand);
	if (!getStatusRegister(NEGATIVE)) setStatusRegister(CARRY);
	else unsetStatusRegister(CARRY);

};

void NESCPU::EOR_Handler(int operand, bool writeback) {

	if (!writeback) operand = mMMC->getMEMByte(operand);

	operand = reg_A ^ operand;

	CheckNZ(operand);
	
	reg_A = operand;

};

void NESCPU::SBC_Handler(int operand, bool writeback) {

	if (!writeback) operand = mMMC->getMEMByte(operand);


	operand = ~operand;

	int temp = operand + reg_A + (getStatusRegister(CARRY) ? 1 : 0);
	if (temp > 0xFF) setStatusRegister(CARRY);
	else unsetStatusRegister(OVERFLOW_REG);



	if ((reg_A ^ temp) & (operand ^ temp) & 0x80) setStatusRegister(OVERFLOW_REG);
	else unsetStatusRegister(OVERFLOW_REG);

	operand = temp & 0xFF;

	CheckNZ(operand);

	reg_A = operand; //memwrite here


};

void NESCPU::CLC_Handler(int operand, bool writeback) {
	unsetStatusRegister(CARRY);
};

void NESCPU::CLD_Handler(int operand, bool writeback) {
	unsetStatusRegister(DECIMAL);
};

void NESCPU::CLI_Handler(int operand, bool writeback) {
	unsetStatusRegister(INTERRUPT);
};

void NESCPU::CLV_Handler(int operand, bool writeback) {
	unsetStatusRegister(OVERFLOW_REG);
};


void NESCPU::SEC_Handler(int operand, bool writeback) {
	setStatusRegister(CARRY);
};

void NESCPU::SED_Handler(int operand, bool writeback) {
	setStatusRegister(DECIMAL);
};

void NESCPU::SEI_Handler(int operand, bool writeback) {
	setStatusRegister(INTERRUPT);
};

void NESCPU::STX_Handler(int operand, bool writeback) {
	mMMC->setMEMByte(operand, reg_X);
};

void NESCPU::STY_Handler(int operand, bool writeback) {
	mMMC->setMEMByte(operand, reg_Y);
};

void NESCPU::STA_Handler(int operand, bool writeback) {
	mMMC->setMEMByte(operand, reg_A);
};

void NESCPU::BNE_Handler(int operand, bool writeback) {
	if (!getStatusRegister(ZERO)) reg_PC += (char)operand;
}

void NESCPU::BEQ_Handler(int operand, bool writeback) {
	if (getStatusRegister(ZERO)) reg_PC += (char)operand;
}

void NESCPU::BCS_Handler(int operand, bool writeback) {
	if (getStatusRegister(CARRY)) reg_PC += (char)operand;
}

void NESCPU::BCC_Handler(int operand, bool writeback) {
	if (!getStatusRegister(CARRY)) reg_PC += (char)operand;
}

void NESCPU::BVS_Handler(int operand, bool writeback) {
	if (getStatusRegister(OVERFLOW_REG)) reg_PC += (char)operand;
}

void NESCPU::BVC_Handler(int operand, bool writeback) {
	if (!getStatusRegister(OVERFLOW_REG)) reg_PC += (char)operand;
}

void NESCPU::BMI_Handler(int operand, bool writeback) {
	if (getStatusRegister(NEGATIVE)) reg_PC += (char)operand;
}

void NESCPU::BPL_Handler(int operand, bool writeback) {
	if (!getStatusRegister(NEGATIVE)) reg_PC += (char)operand;
}

void NESCPU::JMP_Handler(int operand, bool writeback) {
	reg_PC = operand - 3; //save a few cycles by removing the 
}

void NESCPU::JSR_Handler(int operand, bool writeback) {
	mMMC->setMEMByte(reg_SP + 256, (reg_PC + 2) >> 8);
	reg_SP--;
	mMMC->setMEMByte(reg_SP + 256, (reg_PC + 2) & 0x00FF);
	reg_SP--;
	reg_PC = operand - 3;
}

void NESCPU::RTS_Handler(int operand, bool writeback) {

	reg_PC = mMMC->getMEMByte(reg_SP + 256 + 1) | (mMMC->getMEMByte(reg_SP + 256 + 2) << 8);
	reg_SP += 2;
}

void NESCPU::PHA_Handler(int operand, bool writeback) {
	mMMC->setMEMByte(reg_SP + 256, reg_A);
	reg_SP--;
};

void NESCPU::PLA_Handler(int operand, bool writeback) {
	reg_SP++;
	reg_A = mMMC->getMEMByte(reg_SP + 256);
	
};

void NESCPU::TXS_Handler(int operand, bool writeback) {
	
	reg_SP = reg_X & 0xFF;

};

void NESCPU::TSX_Handler(int operand, bool writeback) {

	reg_X = reg_SP & 0xFF;

};

void NESCPU::PHP_Handler(int operand, bool writeback) {

	mMMC->setMEMByte(reg_SP + 256, reg_STATUS);
	setStatusRegister(ALWAYS_ONE);
	setStatusRegister(BREAK);
	reg_SP--;

};



void NESCPU::BRK_Handler(int operand, bool writeback) {

	setStatusRegister(ALWAYS_ONE);
	setStatusRegister(BREAK);
	IRQline = true;
//	reg_PC++;
};

void NESCPU::PLP_Handler(int operand, bool writeback) {

	reg_SP++;
	reg_STATUS = mMMC->getMEMByte(reg_SP + 256);

};

void NESCPU::BIT_Handler(int operand, bool writeback) {
	operand = mMMC->getMEMByte(operand) & 0xFF;
	if ((operand & reg_A) == 0x00) setStatusRegister(ZERO);
	reg_STATUS = ((reg_STATUS & 0x3F) | (operand & 0xC0));

};

void NESCPU::INC_Handler(int operand, bool writeback) {
	int value = mMMC->getMEMByte(operand) & 0xFF;
	value++;
	CheckNZ(value);
	mMMC->setMEMByte(operand, value & 0xFF);

}

void NESCPU::DEC_Handler(int operand, bool writeback) {
	int value = mMMC->getMEMByte(operand) & 0xFF;
	value--;
	CheckNZ(value);
	mMMC->setMEMByte(operand, value & 0xFF);
}

void NESCPU::RTI_Handler(int operand, bool writeback) {

	reg_SP++;
	reg_STATUS = mMMC->getMEMByte(reg_SP + 256);

	reg_PC = mMMC->getMEMByte(reg_SP + 256 + 1) | (mMMC->getMEMByte(reg_SP + 256 + 2) << 8);
	reg_SP += 2;
	

};
