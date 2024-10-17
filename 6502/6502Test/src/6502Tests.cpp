#include <gtest/gtest.h>
#include "main_6502.hpp"

class M6502Test1 : public testing::Test{
public:
	Memo memo;
	CPU cpu;

	virtual void SetUp(){
		cpu.Reset(memo);
	}

	virtual void TearDown(){
	}
};

static void VerifyUnmodifiedFlagsFromLDA(const CPU& cpu, const CPU& CPUCopy){
	EXPECT_EQ(cpu.C, CPUCopy.C);
	EXPECT_EQ(cpu.I, CPUCopy.I);
	EXPECT_EQ(cpu.D, CPUCopy.D);
	EXPECT_EQ(cpu.B, CPUCopy.B);
	EXPECT_EQ(cpu.V, CPUCopy.V);
}

TEST_F(M6502Test1,TheCPUDoesNothingWhenWeExecuteZeroCycles){
	//given:
	constexpr s32 NUM_CYCLES = 0;

	//when:
	s32 CyclesUsed = cpu.Execute(NUM_CYCLES, memo);

	//then:
	EXPECT_EQ(CyclesUsed, 0);
}

TEST_F(M6502Test1,CPUCanExecuteMoreCyclesThanRequestedIfRequiredByTheInstruction){
	// given:
	memo[0xFFFC] = CPU::INSTR_LDA_IM;
	memo[0xFFFD] = 0x84;
	CPU CPUCopy = cpu;
	constexpr s32 NUM_CYCLES = 1;

	//when:
	s32 CyclesUsed = cpu.Execute(NUM_CYCLES, memo);

	//then:
	EXPECT_EQ(CyclesUsed, 2);
}

TEST_F(M6502Test1, LDAImmediateCanLoadAValueIntoTheARegister){
	// given:
	memo[0xFFFC] = CPU::INSTR_LDA_IM;
	memo[0xFFFD] = 0x84;

	//when:
	CPU CPUCopy = cpu;
	s32 CyclesUsed = cpu.Execute(2, memo);

	//then:
	EXPECT_EQ(cpu.A, 0x84);
	EXPECT_EQ(CyclesUsed, 2);
	EXPECT_FALSE(cpu.Z);
	EXPECT_TRUE(cpu.N);
	VerifyUnmodifiedFlagsFromLDA(cpu, CPUCopy);
}

TEST_F(M6502Test1, LDAImmediateCanAffectTheZeroFlag){
	// given:
	cpu.A = 0x44;
	memo[0xFFFC] = CPU::INSTR_LDA_IM;
	memo[0xFFFD] = 0x0;
	CPU CPUCopy = cpu;

	//when:
	cpu.Execute(2, memo);

	//then:
	EXPECT_TRUE(cpu.Z);
	EXPECT_FALSE(cpu.N);
	VerifyUnmodifiedFlagsFromLDA(cpu, CPUCopy);
}

TEST_F(M6502Test1, LDAZeroPageCanLoadAValueIntoTheARegister){
	// given:
	// start - inline a little program
	memo[0xFFFC] = CPU::INSTR_LDA_ZP;
	memo[0xFFFD] = 0x42;
	memo[0x0042] = 0x37;
	// end - inline a little program

	//when:
	CPU CPUCopy = cpu;
	s32 CyclesUsed = cpu.Execute(3, memo);

	//then:
	EXPECT_EQ(cpu.A, 0x37);
	EXPECT_EQ(CyclesUsed, 3);
	EXPECT_FALSE(cpu.Z);
	EXPECT_FALSE(cpu.N);
	VerifyUnmodifiedFlagsFromLDA(cpu, CPUCopy);
}

TEST_F(M6502Test1, LDAZeroPageXCanLoadAValueIntoTheARegister){
	// given:
	cpu.X = 5;
	// start - inline a little program
	memo[0xFFFC] = CPU::INSTR_LDA_ZPX;
	memo[0xFFFD] = 0x42;
	memo[0x0047] = 0x37;
	// end - inline a little program

	//when:
	CPU CPUCopy = cpu;
	s32 CyclesUsed = cpu.Execute(4, memo);

	//then:
	EXPECT_EQ(cpu.A, 0x37);
	EXPECT_EQ(CyclesUsed, 4);
	EXPECT_FALSE(cpu.Z);
	EXPECT_FALSE(cpu.N);
	VerifyUnmodifiedFlagsFromLDA(cpu, CPUCopy);
}

TEST_F(M6502Test1, LDAZeroPageXCanLoadAValueIntoTheARegisterWhenItWraps){
	// given:
	cpu.X = 0xFF;
	// start - inline a little program
	memo[0xFFFC] = CPU::INSTR_LDA_ZPX;
	memo[0xFFFD] = 0x80;
	memo[0x007F] = 0x37;
	// end - inline a little program

	//when:
	CPU CPUCopy = cpu;
	s32 CyclesUsed = cpu.Execute(4, memo);

	//then:
	EXPECT_EQ(cpu.A, 0x37);
	EXPECT_EQ(CyclesUsed, 4);
	EXPECT_FALSE(cpu.Z);
	EXPECT_FALSE(cpu.N);
	VerifyUnmodifiedFlagsFromLDA(cpu, CPUCopy);
}

TEST_F(M6502Test1, LDAAbsoluteCanLoadAValueIntoTheARegister){
	// given:
	memo[0xFFFC] = CPU::INSTR_LDA_ABS;
	memo[0xFFFD] = 0x80;
	memo[0xFFFE] = 0x44;	//0x4480
	memo[0x4480] = 0x37;
	constexpr s32 EXPECTED_CYCLES = 4;
	CPU CPUCopy = cpu;

	//when:
	s32 CyclesUsed = cpu.Execute(EXPECTED_CYCLES, memo);

	//then:
	EXPECT_EQ(cpu.A, 0x37);
	EXPECT_EQ(CyclesUsed, EXPECTED_CYCLES);
	EXPECT_FALSE(cpu.Z);
	EXPECT_FALSE(cpu.N);
	VerifyUnmodifiedFlagsFromLDA(cpu, CPUCopy);
}

TEST_F(M6502Test1, LDAAbsoluteXCanLoadAValueIntoTheARegister){
	// given:
	cpu.X = 1;
	memo[0xFFFC] = CPU::INSTR_LDA_ABSX;
	memo[0xFFFD] = 0x80;
	memo[0xFFFE] = 0x44;	//0x4480
	memo[0x4481] = 0x37;	
	constexpr s32 EXPECTED_CYCLES = 4;
	CPU CPUCopy = cpu;

	//when:
	s32 CyclesUsed = cpu.Execute(EXPECTED_CYCLES, memo);

	//then:
	EXPECT_EQ(cpu.A, 0x37);
	EXPECT_EQ(CyclesUsed, EXPECTED_CYCLES);
	EXPECT_FALSE(cpu.Z);
	EXPECT_FALSE(cpu.N);
	VerifyUnmodifiedFlagsFromLDA(cpu, CPUCopy);
}

TEST_F(M6502Test1, LDAAbsoluteXCanLoadAValueIntoTheARegisterWhenItCrossesAPageBoundary){
	// given:
	cpu.X = 0xFF;
	memo[0xFFFC] = CPU::INSTR_LDA_ABSX;
	memo[0xFFFD] = 0x02;
	memo[0xFFFE] = 0x44;	//0x4402
	memo[0x4501] = 0x37;	//0x4402+0xFF crosses page boundary!
	constexpr s32 EXPECTED_CYCLES = 5;
	CPU CPUCopy = cpu;

	//when:
	s32 CyclesUsed = cpu.Execute(EXPECTED_CYCLES, memo);

	//then:
	EXPECT_EQ(cpu.A, 0x37);
	EXPECT_EQ(CyclesUsed, EXPECTED_CYCLES);
	EXPECT_FALSE(cpu.Z);
	EXPECT_FALSE(cpu.N);
	VerifyUnmodifiedFlagsFromLDA(cpu, CPUCopy);
}

TEST_F(M6502Test1, LDAAbsoluteYCanLoadAValueIntoTheARegister){
	// given:
	cpu.Y = 1;
	memo[0xFFFC] = CPU::INSTR_LDA_ABSY;
	memo[0xFFFD] = 0x80;
	memo[0xFFFE] = 0x44;	//0x4480
	memo[0x4481] = 0x37;
	constexpr s32 EXPECTED_CYCLES = 4;
	CPU CPUCopy = cpu;

	//when:
	s32 CyclesUsed = cpu.Execute(EXPECTED_CYCLES, memo);

	//then:
	EXPECT_EQ(cpu.A, 0x37);
	EXPECT_EQ(CyclesUsed, EXPECTED_CYCLES);
	EXPECT_FALSE(cpu.Z);
	EXPECT_FALSE(cpu.N);
	VerifyUnmodifiedFlagsFromLDA(cpu, CPUCopy);
}

TEST_F(M6502Test1, LDAAbsoluteYCanLoadAValueIntoTheARegisterWhenItCrossesAPageBoundary){
	// given:
	cpu.Y = 0xFF;
	memo[0xFFFC] = CPU::INSTR_LDA_ABSY;
	memo[0xFFFD] = 0x02;
	memo[0xFFFE] = 0x44;	//0x4402
	memo[0x4501] = 0x37;	//0x4402+0xFF crosses page boundary!
	constexpr s32 EXPECTED_CYCLES = 5;
	CPU CPUCopy = cpu;

	//when:
	s32 CyclesUsed = cpu.Execute(EXPECTED_CYCLES, memo);

	//then:
	EXPECT_EQ(cpu.A, 0x37);
	EXPECT_EQ(CyclesUsed, EXPECTED_CYCLES);
	EXPECT_FALSE(cpu.Z);
	EXPECT_FALSE(cpu.N);
	VerifyUnmodifiedFlagsFromLDA(cpu, CPUCopy);
}

TEST_F(M6502Test1, LDAIndirectXCanLoadAValueIntoTheARegister){
	// given:
	cpu.X = 0x04;
	memo[0xFFFC] = CPU::INSTR_LDA_INDX;
	memo[0xFFFD] = 0x02;
	memo[0x0006] = 0x00;	//0x2 + 0x4
	memo[0x0007] = 0x80;	
	memo[0x8000] = 0x37;
	constexpr s32 EXPECTED_CYCLES = 6;
	CPU CPUCopy = cpu;

	//when:
	s32 CyclesUsed = cpu.Execute(EXPECTED_CYCLES, memo);

	//then:
	EXPECT_EQ(cpu.A, 0x37);
	EXPECT_EQ(CyclesUsed, EXPECTED_CYCLES);
	EXPECT_FALSE(cpu.Z);
	EXPECT_FALSE(cpu.N);
	VerifyUnmodifiedFlagsFromLDA(cpu, CPUCopy);
}

TEST_F(M6502Test1, LDAIndirectYCanLoadAValueIntoTheARegister){
	// given:
	cpu.Y = 0x04;
	memo[0xFFFC] = CPU::INSTR_LDA_INDY;
	memo[0xFFFD] = 0x02;
	memo[0x0002] = 0x00;	
	memo[0x0003] = 0x80;
	memo[0x8004] = 0x37;	//0x8000 + 0x4
	constexpr s32 EXPECTED_CYCLES = 5;
	CPU CPUCopy = cpu;

	//when:
	s32 CyclesUsed = cpu.Execute(EXPECTED_CYCLES, memo);

	//then:
	EXPECT_EQ(cpu.A, 0x37);
	EXPECT_EQ(CyclesUsed, EXPECTED_CYCLES);
	EXPECT_FALSE(cpu.Z);
	EXPECT_FALSE(cpu.N);
	VerifyUnmodifiedFlagsFromLDA(cpu, CPUCopy);
}

TEST_F(M6502Test1, LDAIndirectYCanLoadAValueIntoTheARegisterWhenItCrossesAPage){
	// given:
	cpu.Y = 0xFF;
	memo[0xFFFC] = CPU::INSTR_LDA_INDY;
	memo[0xFFFD] = 0x02;
	memo[0x0002] = 0x02;
	memo[0x0003] = 0x80;
	memo[0x8101] = 0x37;	//0x8002 + 0xFF
	constexpr s32 EXPECTED_CYCLES = 6;
	CPU CPUCopy = cpu;

	//when:
	s32 CyclesUsed = cpu.Execute(EXPECTED_CYCLES, memo);

	//then:
	EXPECT_EQ(cpu.A, 0x37);
	EXPECT_EQ(CyclesUsed, EXPECTED_CYCLES);
	EXPECT_FALSE(cpu.Z);
	EXPECT_FALSE(cpu.N);
	VerifyUnmodifiedFlagsFromLDA(cpu, CPUCopy);
}