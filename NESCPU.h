#ifndef NESCPU_HEADER
#define NESCPU_HEADER

enum NESRegister {CARRY,ZERO,INTERRUPT,DECIMAL,BREAK,ALWAYS_ONE,OVERFLOW_REG,NEGATIVE};
enum AddressingMode {INDIRECT, RELATIVEA, ABSOLUTEA, ZEROPAGE, IMMEDIATE, ACUMULATOR, IMPLICIT, INDIRECT_INDEXED, INDEXED_INDIRECT,
					  ABSOLUTEA_INDEXEDY, ABSOLUTEA_INDEXEDX, ZEROPAGE_INDEXEDY, ZEROPAGE_INDEXEDX, INVALID};


enum IRQTable {NMI = 0xfffa, RESET = 0xfffc, IRQ = 0xfffe};

class NESCPU;
class MMC;

struct OPCode {

	AddressingMode _Addressingmode;
	int length;
	int cycles;
	char * OPName;
	void(NESCPU::*opcode_handler)(int, bool);

};

class NESCPU{

	

public:
	static NESCPU * getInstance(MMC * _MMC);
	//void run();

	int execute();

private:



	NESCPU(MMC * _MMC);
	void NESCPU::defineOPCode(unsigned char _code, AddressingMode _mode, int cycles, int length, char * OPName, void(NESCPU::*handler)(int, bool));
	void NESCPU::ORA_Handler(int operand, bool writeback);
	void NESCPU::LDA_Handler(int operand, bool writeback);
	void NESCPU::LDX_Handler(int operand, bool writeback);
	void NESCPU::LDY_Handler(int operand, bool writeback);
	void NESCPU::ASL_Handler(int operand, bool writeback);
	void NESCPU::LSR_Handler(int operand, bool writeback);
	void NESCPU::AND_Handler(int operand, bool writeback);
	void NESCPU::ADC_Handler(int operand, bool writeback);
	void NESCPU::TAX_Handler(int operand, bool writeback);
	void NESCPU::INX_Handler(int operand, bool writeback);
	void NESCPU::INY_Handler(int operand, bool writeback);
	void NESCPU::DEX_Handler(int operand, bool writeback);
	void NESCPU::DEY_Handler(int operand, bool writeback);
	void NESCPU::CMP_Handler(int operand, bool writeback);
	void NESCPU::CPX_Handler(int operand, bool writeback);
	void NESCPU::CPY_Handler(int operand, bool writeback);
	void NESCPU::EOR_Handler(int operand, bool writeback);
	void NESCPU::ROL_Handler(int operand, bool writeback);
	void NESCPU::ROR_Handler(int operand, bool writeback);
	void NESCPU::SBC_Handler(int operand, bool writeback);
	void NESCPU::TYA_Handler(int operand, bool writeback);
	void NESCPU::TXA_Handler(int operand, bool writeback);
	void NESCPU::TAY_Handler(int operand, bool writeback);
	void NESCPU::CLC_Handler(int operand, bool writeback);
	void NESCPU::CLD_Handler(int operand, bool writeback);
	void NESCPU::CLI_Handler(int operand, bool writeback);
	void NESCPU::CLV_Handler(int operand, bool writeback);
	void NESCPU::SEC_Handler(int operand, bool writeback);
	void NESCPU::SED_Handler(int operand, bool writeback);
	void NESCPU::SEI_Handler(int operand, bool writeback);
	void NESCPU::STX_Handler(int operand, bool writeback);
	void NESCPU::STY_Handler(int operand, bool writeback);
	void NESCPU::BNE_Handler(int operand, bool writeback);
	void NESCPU::BEQ_Handler(int operand, bool writeback);
	void NESCPU::BCS_Handler(int operand, bool writeback);
	void NESCPU::BCC_Handler(int operand, bool writeback);
	void NESCPU::BVS_Handler(int operand, bool writeback);
	void NESCPU::BVC_Handler(int operand, bool writeback);
	void NESCPU::BMI_Handler(int operand, bool writeback);
	void NESCPU::BPL_Handler(int operand, bool writeback);
	void NESCPU::STA_Handler(int operand, bool writeback);
	void NESCPU::JMP_Handler(int operand, bool writeback);
	void NESCPU::PLA_Handler(int operand, bool writeback);
	void NESCPU::PHA_Handler(int operand, bool writeback);
	void NESCPU::TXS_Handler(int operand, bool writeback);
	void NESCPU::TSX_Handler(int operand, bool writeback);
	void NESCPU::BIT_Handler(int operand, bool writeback);
	void NESCPU::PHP_Handler(int operand, bool writeback);
	void NESCPU::PLP_Handler(int operand, bool writeback);
	void NESCPU::INC_Handler(int operand, bool writeback);
	void NESCPU::DEC_Handler(int operand, bool writeback);
	void NESCPU::JSR_Handler(int operand, bool writeback);
	void NESCPU::RTS_Handler(int operand, bool writeback);
	void NESCPU::BRK_Handler(int operand, bool writeback);
	void NESCPU::RTI_Handler(int operand, bool writeback);
	void NESCPU::CheckNZ(int operand);
	int NESCPU::getActualMemAddress(AddressingMode _mode, int operand);
	void setStatusRegister(NESRegister _reg);
	void unsetStatusRegister(NESRegister _reg);
	bool NESCPU::getStatusRegister(NESRegister _reg);

	MMC * mMMC;
	void loadOpCodes();
	void NESCPU::interruptHandler(IRQTable _IRQType);
	inline int NESCPU::getIRQAddress(IRQTable _vector);
	OPCode _OPCodes[255];
	static NESCPU * _NESCPU;
	unsigned char reg_A;
	unsigned char reg_X;
	unsigned char reg_Y;
	int reg_PC;
	unsigned char reg_SP;
	unsigned char reg_STATUS;
	bool IRQline;
	bool NMIline;
};



#endif