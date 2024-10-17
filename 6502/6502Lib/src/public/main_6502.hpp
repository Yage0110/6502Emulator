#pragma once
#include <stdio.h>
#include <stdlib.h>

using Byte = unsigned char;
using Word = unsigned short;

using u32 = unsigned int;
using s32 = signed int;

struct Memo{
	static constexpr u32 MAX_MEMO = 1024 * 64;
	Byte Data[MAX_MEMO];

	void Initialize(){
		for (u32 i = 0; i < MAX_MEMO; i++){
			Data[i] = 0;
		}
	}

	//read 1 byte
	Byte operator[](u32 Address) const{
		// assert here Address is < MAX_MEMO
		return Data[Address];
	}

	// write 1 byte 
	Byte& operator[](u32 Address){
		// assert here Address is < MAX_MEMO
		return Data[Address];
	}

	//write 2 bytes 
	void WriteWord(Word Value, u32 Address, s32& Cycles){
		Data[Address] = Value & 0xFF;
		Data[Address + 1] = (Value >> 8);
		Cycles -= 2;
	}
};

struct CPU{

	Word PC;		//program counter
	Word SP;		//stack pointer

	Byte A, X, Y;	//registers

	Byte C : 1;	//carry status flag
	Byte Z : 1;	//zero status flag
	Byte I : 1; //int status flag
	Byte D : 1; //decimal status flag
	Byte B : 1; //break status flag
	Byte V : 1; //overflow status flag
	Byte N : 1; //negative status flag

	void Reset(Memo& memory){
		Reset(0xFFFC, memory);
	}

	void Reset(Word ResetVector, Memo& memory){
		PC = ResetVector;
		SP = 0x0100;
		C = Z = I = D = B = V = N = 0;
		A = X = Y = 0;
		memory.Initialize();
	}

	Byte FetchByte(s32& Cycles, Memo& memory){
		Byte Data = memory[PC];
		PC++;
		Cycles--;
		return Data;
	}

	Word FetchWord(s32& Cycles, Memo& memory){
		// 6502 is little endian
		Word Data = memory[PC];
		PC++;
		
		Data |= (memory[PC] << 8 );
		PC++;

		Cycles -= 2;

		return Data;
	}

	Byte ReadByte(s32& Cycles, Word Address,Memo& memory){
		Byte Data = memory[Address];
		Cycles--;
		return Data;
	}

	Word ReadWord(s32& Cycles, Word Address, Memo& memory){
		Byte LoByte = ReadByte(Cycles, Address, memory);
		Byte HiByte = ReadByte(Cycles, Address + 1, memory);
		return LoByte | (HiByte << 8);
	}

	// opcodes
	static constexpr Byte
		INSTR_LDA_IM = 0xA9,
		INSTR_LDA_ZP = 0xA5,
		INSTR_LDA_ZPX = 0xB5,
		INSTR_LDA_ABS = 0xAD,
		INSTR_LDA_ABSX = 0xBD,
		INSTR_LDA_ABSY = 0xB9,
		INSTR_LDA_INDX = 0xA1,
		INSTR_LDA_INDY = 0xB1,
		INSTR_JSR = 0x20;

	void LDASetStatus(){
		Z = (A == 0);
		N = (A & 0b10000000) > 0;
	}

	//return the number of cycles that were used 
	s32 Execute(s32 Cycles, Memo& memory){
		const s32 CyclesRequested = Cycles;
		while (Cycles > 0){
			Byte INSTR = FetchByte(Cycles, memory);			
			switch (INSTR)
			{
			case INSTR_LDA_IM:{
				Byte Value = FetchByte( Cycles, memory );
				A = Value;
				LDASetStatus();
			} break;
			case INSTR_LDA_ZP:{
				Byte ZeroPageAddr = FetchByte(Cycles, memory );
				A = ReadByte(Cycles, ZeroPageAddr, memory);
				LDASetStatus();
			} break;
			case INSTR_LDA_ZPX:{
				Byte ZeroPageAddr = FetchByte(Cycles, memory);
				ZeroPageAddr += X;
				Cycles--;
				A = ReadByte(Cycles, ZeroPageAddr, memory);
				LDASetStatus();
			} break;
			case INSTR_LDA_ABS:{
				Word AbsAddress = FetchWord(Cycles, memory);
				A = ReadByte(Cycles, AbsAddress, memory);
			} break;
			case INSTR_LDA_ABSX:{
				Word AbsAddress = FetchWord(Cycles, memory);
				Word AbsAddressX = AbsAddress + X;
				A = ReadByte(Cycles, AbsAddressX, memory);
				if (AbsAddressX - AbsAddress >= 0xFF){
					Cycles--;
				}
			} break;
			case INSTR_LDA_ABSY:{
				Word AbsAddress = FetchWord(Cycles, memory);
				Word AbsAddressY = AbsAddress + Y;
				A = ReadByte(Cycles, AbsAddressY, memory);
				if (AbsAddressY - AbsAddress >= 0xFF){
					Cycles--;
				}
			} break;
			case INSTR_LDA_INDX:{
				Byte ZPAddress = FetchByte(Cycles, memory);
				ZPAddress += X;
				Cycles--;
				Word EffectiveAddr = ReadWord(Cycles, ZPAddress, memory);
				A = ReadByte(Cycles, EffectiveAddr, memory);
			} break;
			case INSTR_LDA_INDY:{
				Byte ZPAddress = FetchByte(Cycles, memory);
				Word EffectiveAddr = ReadWord(Cycles, ZPAddress, memory);
				Word EffectiveAddrY = EffectiveAddr + Y;
				A = ReadByte(Cycles, EffectiveAddrY, memory);
				if (EffectiveAddrY - EffectiveAddr >= 0xFF){
					Cycles--;
				}
			} break;
			case INSTR_JSR:{
				Word SubAddr = 
					FetchWord(Cycles, memory);
				memory.WriteWord(PC - 1, SP, Cycles);
				SP += 2;
				PC = SubAddr;
				Cycles--;
			} break;
			default:{
				printf("Instruction %d not handled\n", INSTR);
				throw -1;
			} break;
			}
		}

		const s32 NumCyclesUsed = CyclesRequested - Cycles;
		return NumCyclesUsed;
	}
};