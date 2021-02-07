#include <stdio.h>
#include <stdlib.h>

/** instructions references */
// http://www.obelisk.me.uk/6502
// http://www.6502.org/tutorials/6502opcodes.html

/** learning tool */
// https://skilldrick.github.io/easy6502/

using Byte = unsigned char;
using Word = unsigned short;

using u32 = unsigned int;
using s32 = signed int;

struct Mem
{
	static constexpr u32 MAX_MEM = 1024 * 64;
	Byte Data[MAX_MEM];

	void Initialize()
	{
		for (u32 i = 0; i < MAX_MEM; i++)
		{
			Data[i] = 0;
		}
	}

	/** read 1 byte */
	Byte operator[] (u32 Address) const
	{
		return Data[Address];
	}
	/** write 1 byte */
	Byte& operator[] (u32 Address)
	{
		return Data[Address];
	}

	/** write 2 bytes */
	void WriteWord(u32& cycles, Word value, u32 address)
	{
		Data[address] = value & 0xFF;
		Data[address + 1] = value >> 8;
		cycles -= 2;
	}
};

struct CPU
{
	Word PC; // program counter
	Byte SP; // stack pointer

	Byte A, X, Y; // registers

	Byte C : 1; // status flag CARRY
	Byte Z : 1; // status flag ZERO
	Byte I : 1; // status flag INTERRUPT
	Byte D : 1; // status flag DECIMAL
	Byte B : 1; // status flag BREAK
	Byte V : 1; // status flag OVERFLOW
	Byte N : 1; // status flag NEGATIVE

	void Reset(Mem& memory)
	{
		PC = 0xFFFC;
		SP = 0x0100;
		C = Z = I = D = B = V = N = 0;
		A = X = Y = 0;
		memory.Initialize();
	}

	Byte FetchByte(u32& cycles, Mem& memory)
	{
		Byte data = memory[PC];
		PC++;
		cycles--;
		return data;
	}

	Byte ReadByte(u32& cycles, Word address, Mem& memory)
	{
		Byte data = memory[address];
		cycles--;
		return data;
	}

	Word FetchWord(u32& cycles, Mem& memory)
	{
		// 6502 is little endian
		Word data = memory[PC];
		PC++;
		cycles--;

		data |= (memory[PC] << 8);
		PC++;
		cycles--;

		// TODO: if BIG_ENDIAN, swap bytes

		return data;
	}

	Word ReadWord(u32& cycles, Word address, Mem& memory)
	{
		Byte leastByte = ReadByte(cycles, address, memory);
		Byte mostByte = ReadByte(cycles, address + 1, memory);
		return leastByte | (mostByte << 8);
	}

	// opcodes
	static constexpr Byte
		// Load		
		INS_LDA_IM = 0xA9,
		INS_LDA_ZP = 0xA5,
		INS_LDA_ZPX = 0xB5,
		INS_LDA_ABS = 0xAD,
		INS_LDA_ABSX = 0xBD,
		INS_LDA_ABSY = 0xB9,
		INS_LDA_INDX = 0xA1,
		INS_LDA_INDY = 0xB1,
		//INS_LDX_IM = 0xA2,
		//INS_LDX_ZP = 0xA6,
		//INS_LDX_ZPY = 0xB6,
		//INS_LDX_ABS = 0xAE,
		//INS_LDX_ABSY = 0xBE,
		//INS_LDY_IM = 0xA0,
		//INS_LDY_ZP = 0xA4,
		//INS_LDY_ZPX = 0xB4,
		//INS_LDY_ABS = 0xAC,
		//INS_LDY_ABSX = 0xBC,
		// Store
		INS_STA_ABS = 0x8D,
		//INS_STX_ZP = 0x86,
		//INS_STX_ZPY = 0x96,
		//INS_STX_ABS = 0x8E,
		// Add/Subtract
		//INS_ADC_IM = 0x69,
		//INS_ADC_ZP = 0x65,
		//INS_ADC_ZPX = 0x75,
		//INS_ADC_ABS = 0x6D,
		//INS_ADC_ABSX = 0x7D,
		//INS_ADC_ABSY = 0x79,
		//INS_ADC_INDX = 0x61,
		//INS_ADC_INDY = 0x71,
		//INS_SBC_IM = 0xE9,
		//INS_SBC_ZP = 0xE5,
		//INS_SBC_ZPX = 0xF5,
		//INS_SBC_ABS = 0xED,
		//INS_SBC_ABSX = 0xFD,
		//INS_SBC_ABSY = 0xF9,
		//INS_SBC_INDX = 0xE1,
		//INS_SBC_INDY = 0xF1,
		// Transfer
		INS_TAX = 0xAA,
		INS_TAY = 0xA8,
		INS_TXA = 0x8A,
		INS_TYA = 0x98,
		INS_TSX = 0xBA,
		INS_TXS = 0x9A,
		// Increments/Decrements
		INS_INX = 0xE8,
		INS_INY = 0xC8,
		INS_DEX = 0xCA,
		INS_DEY = 0x88,
		// Compare
		//INS_CMP_IM = 0xC9,
		//INS_CMP_ZP = 0xC5,
		//INS_CMP_ZPX = 0xD5,
		//INS_CMP_ABS = 0xCD,
		//INS_CMP_ABSX = 0xDD,
		//INS_CMP_ABSY = 0xD9,
		//INS_CMP_INDX = 0xC1,
		//INS_CMP_INDY = 0xD1,
		//INS_CPX_IM = 0xE0,
		//INS_CPX_ZP = 0xE4,
		//INS_CPX_ABS = 0xEC,
		//INS_CPY_IM = 0xC0,
		//INS_CPY_ZP = 0xC4,
		//INS_CPY_ABS = 0xCC,
		// Branch
		INS_BMI = 0x30,
		INS_BNE = 0xD0,
		INS_BEQ = 0xF0,
		INS_BCC = 0x90,
		INS_BCS = 0xB0,
		INS_BPL = 0x10,
		INS_BVC = 0x50,
		INS_BVS = 0x70,
		// Flags
		INS_SEC = 0x38,
		INS_SED = 0xF8,
		INS_SEI = 0x78,
		INS_CLC = 0x18,
		INS_CLD = 0xD8,
		INS_CLI = 0x58,
		INS_CLV = 0xB8,
		// Jump/Break
		INS_JMP_ABS = 0x4C,
		INS_JMP_IND = 0x6C,
		INS_JSR = 0x20,
		INS_RTS = 0x60,
		INS_BRK = 0x00;


	/** @return the number of cycles that were used */
	s32 Execute(u32 cycles, Mem& memory)
	{
		s32 numOfCycles = cycles;
		while (cycles > 0)
		{
			Byte inst = FetchByte(cycles, memory);
			switch (inst)
			{
			case INS_LDA_IM:
			{
				Byte value = FetchByte(cycles, memory);
				A = value;
				LDASetStatus();
			} break;
			case INS_LDA_ZP:
			{
				Byte zeroPageAddress = FetchByte(cycles, memory);
				A = ReadByte(cycles, zeroPageAddress, memory);
				LDASetStatus();
			} break;
			case INS_LDA_ZPX:
			{
				Byte zeroPageAddress = FetchByte(cycles, memory);
				zeroPageAddress += X;
				cycles--;
				A = ReadByte(cycles, zeroPageAddress, memory);
				LDASetStatus();
			} break;
			case INS_LDA_ABS:
			{
				Word address = FetchWord(cycles, memory);
				A = ReadByte(cycles, address, memory);
				LDASetStatus();
			} break;
			case INS_LDA_ABSX:
			{
				Word address = FetchWord(cycles, memory);
				address += X;
				A = ReadByte(cycles, address, memory);
				LDASetStatus();
			} break;
			case INS_LDA_ABSY:
			{
				Word address = FetchWord(cycles, memory);
				Word addressY = address + Y;
				A = ReadByte(cycles, address, memory);
				if (addressY - address >= 0xFF) cycles--;
				LDASetStatus();
			} break;
			case INS_LDA_INDX:
			{
				Byte zeroPageAddress = FetchByte(cycles, memory);
				zeroPageAddress += X;
				cycles--;
				Word targetAddress = ReadWord(cycles, zeroPageAddress, memory);
				A = ReadByte(cycles, targetAddress, memory);
				LDASetStatus();
			} break;
			case INS_LDA_INDY:
			{
				Byte zeroPageAddress = FetchByte(cycles, memory);
				Word targetAddress = ReadWord(cycles, zeroPageAddress, memory);
				Word targetAddressY = targetAddress + Y;
				cycles--;
				A = ReadByte(cycles, targetAddressY, memory);
				if (targetAddressY - targetAddress >= 0xFF) cycles--;
				LDASetStatus();
			} break;
			case INS_STA_ABS:
			{
				Word address = FetchWord(cycles, memory);
				memory.WriteWord(cycles, A, address);
				LDASetStatus();
			} break;
			case INS_TAX:
			{
				X = A;
				cycles -= 2;
				LDASetStatus();
			} break;
			case INS_TAY:
			{
				Y = A;
				cycles -= 2;
				LDASetStatus();
			} break;
			case INS_TSX:
			{
				X = SP;
				cycles -= 2;
				LDASetStatus();
			} break;
			case INS_TXA:
			{
				A = X;
				cycles -= 2;
				LDASetStatus();
			} break;
			case INS_TXS:
			{
				SP = X;
				cycles -= 2;
			} break;
			case INS_TYA:
			{
				A = Y;
				cycles -= 2;
				LDASetStatus();
			} break;
			case INS_INX:
			{
				X += 1;
				cycles -= 2;
				LDASetStatus();
			} break;
			case INS_INY:
			{
				Y += 1;
				cycles -= 2;
				LDASetStatus();
			} break;
			case INS_DEX:
			{
				X -= 1;
				cycles -= 2;
				LDASetStatus();
			} break;
			case INS_DEY:
			{
				Y -= 1;
				cycles -= 2;
				LDASetStatus();
			} break;
			case INS_BNE:
			{
				if (Z == 0)
				{
					Byte relativeAddr = FetchByte(cycles, memory);
					PC += relativeAddr;
				}
				//TODO: takes +2 cycles if to a new page
				cycles -= 2;
			} break;
			case INS_BEQ:
			{
				if (Z != 0)
				{
					Byte relativeAddr = FetchByte(cycles, memory);
					PC += relativeAddr;
				}
				//TODO: takes +2 cycles if to a new page
				cycles -= 2;
			} break;
			case INS_BCC:
			{
				if (C == 0)
				{
					Byte relativeAddr = FetchByte(cycles, memory);
					PC += relativeAddr;
				}
				//TODO: takes +2 cycles if to a new page
				cycles -= 2;
			} break;
			case INS_BCS:
			{
				if (C != 0)
				{
					Byte relativeAddr = FetchByte(cycles, memory);
					PC += relativeAddr;
				}
				//TODO: takes +2 cycles if to a new page
				cycles -= 2;
			} break;
			case INS_BMI:
			{
				if (N != 0)
				{
					Byte relativeAddr = FetchByte(cycles, memory);
					PC += relativeAddr;
				}
				//TODO: takes +2 cycles if to a new page
				cycles -= 2;
			} break;
			case INS_BPL:
			{
				if (N == 0)
				{
					Byte relativeAddr = FetchByte(cycles, memory);
					PC += relativeAddr;
				}
				//TODO: takes +2 cycles if to a new page
				cycles -= 2;
			} break;
			case INS_BVC:
			{
				if (V == 0)
				{
					Byte relativeAddr = FetchByte(cycles, memory);
					PC += relativeAddr;
				}
				//TODO: takes +2 cycles if to a new page
				cycles -= 2;
			} break;
			case INS_BVS:
			{
				if (V != 0)
				{
					Byte relativeAddr = FetchByte(cycles, memory);
					PC += relativeAddr;
				}
				//TODO: takes +2 cycles if to a new page
				cycles -= 2;
			} break;
			case INS_SEC:
			{
				C = 1;
				cycles -= 2;
			} break;
			case INS_SED:
			{
				D = 1;
				cycles -= 2;
			} break;
			case INS_SEI:
			{
				I = 1;
				cycles -= 2;
			} break;
			case INS_CLC:
			{
				C = 0;
				cycles -= 2;
			} break;
			case INS_CLD:
			{
				D = 0;
				cycles -= 2;
			} break;
			case INS_CLI:
			{
				I = 0;
				cycles -= 2;
			} break;
			case INS_CLV:
			{
				V = 0;
				cycles -= 2;
			} break;
			case INS_JMP_ABS:
			{
				Word address = FetchWord(cycles, memory);
				PC = address;
				cycles--;
			} break;
			case INS_JMP_IND:
			{
				Word initialAddr = FetchWord(cycles, memory);
				Word targetAddr = ReadWord(cycles, initialAddr, memory);
				PC = targetAddr;
				cycles--;
			} break;
			case INS_JSR:
			{
				Word subAddress = FetchWord(cycles, memory);
				memory.WriteWord(cycles, PC - 1, SP);
				cycles--;
				PC = subAddress;
				cycles--;
				SP++;
			} break;
			case INS_BRK:
			{
				//TODO:
				// push PC to Stack
				// push ProcessorStatus to Stack
				// Load IRQ (interrupt vector) at 0xFFFE/F into PC
				// set Break flag to 1
				B = 1;
				cycles -= 7;
			} break;
			default:
			{
				printf("Instruction not handled %d\n", inst);
			} break;
			}
		}

		return numOfCycles - cycles;
	}

	Word LoadProgram(Byte* program, int numOfBytes, Mem& memory)
	{
		Word startAddr = 0x00;
		for (u32 i = 0; i < numOfBytes; i++)
		{
			memory[startAddr + i] = program[i];
		}
		return startAddr;
	}

	void LDASetStatus()
	{
		Z = (A == 0); // set if A == 0
		N = (A & 0b10000000) > 0; // set if bit 7 of A is set
	}
};


// CPU: 6507 (6502 w/ reduced instruction set, 8-bit addr, clock: 1.19MHz)
// RAM: 128 bytes
// ROM: 2K or 4K