// mipssim.cc -- simulate a MIPS R2/3000 processor
//
//   This code has been adapted from Ousterhout's MIPSSIM package.
//   Byte ordering is little-endian, so we can be compatible with
//   DEC RISC systems.
//
//   DO NOT CHANGE -- part of the machine emulation
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"

#include "machine.h"
#include "mipssim.h"
#include "system.h"

static void Mult(int a, int b, bool signedArith, int* hiPtr, int* loPtr);

//----------------------------------------------------------------------
// Machine::Run
// 	Simulate the execution of a user-level program on Nachos.
//	Called by the kernel when the program starts up; never returns.
//
//	This routine is re-entrant, in that it can be called multiple
//	times concurrently -- one for each thread executing user code.
//----------------------------------------------------------------------

void
Machine::Run()
{
    Instruction *instr = new Instruction;  // storage for decoded instruction

    if(DebugIsEnabled('m'))
        printf("Starting thread \"%s\" at time %d\n",
	       currentThread->getName(), stats->totalTicks);
    interrupt->setStatus(UserMode);
    for (;;) {
        OneInstruction(instr);
	interrupt->OneTick();
	if (singleStep && (runUntilTime <= stats->totalTicks))
	  Debugger();
    }
}


//----------------------------------------------------------------------
// TypeToReg
// 	Retrieve the register # referred to in an instruction. 
//----------------------------------------------------------------------

static int 
TypeToReg(RegType reg, Instruction *instr)
{
    switch (reg) {
      case RS:
	return instr->rs;
      case RT:
	return instr->rt;
      case RD:
	return instr->rd;
      case EXTRA:
	return instr->extra;
      default:
	return -1;
    }
}

//----------------------------------------------------------------------
// Machine::OneInstruction
// 	Execute one instruction from a user-level program
//
// 	If there is any kind of exception or interrupt, we invoke the 
//	exception handler, and when it returns, we return to Run(), which
//	will re-invoke us in a loop.  This allows us to
//	re-start the instruction execution from the beginning, in
//	case any of our state has changed.  On a syscall,
// 	the OS software must increment the PC so execution begins
// 	at the instruction immediately after the syscall. 
//
//	This routine is re-entrant, in that it can be called multiple
//	times concurrently -- one for each thread executing user code.
//	We get re-entrancy by never caching any data -- we always re-start the
//	simulation from scratch each time we are called (or after trapping
//	back to the Nachos kernel on an exception or interrupt), and we always
//	store all data back to the machine registers and memory before
//	leaving.  This allows the Nachos kernel to control our behavior
//	by controlling the contents of memory, the translation table,
//	and the register set.
//----------------------------------------------------------------------

void
Machine::OneInstruction(Instruction *instr)
{
    int raw;
    int nextLoadReg = 0; 	
    int nextLoadValue = 0; 	// record delayed load operation, to apply
				// in the future

    // Fetch instruction 
    if (!machine->ReadMem(registers[PCReg], 4, &raw))
	return;			// exception occurred
    instr->value = raw;
    instr->Decode();

    if (DebugIsEnabled('m')) {
       struct OpString *str = &opStrings[(int)instr->opCode];

       ASSERT(instr->opCode <= MaxOpcode);
       printf("At PC = 0x%x: ", registers[PCReg]);
       printf(str->string, TypeToReg(str->args[0], instr), 
		TypeToReg(str->args[1], instr), TypeToReg(str->args[2], instr));
       printf("\n");
       }
    
    // Compute next pc, but don't install in case there's an error or branch.
    int pcAfter = registers[NextPCReg] + 4;
    int sum, diff, tmp, value;
    unsigned int rs, rt, imm;

    // Execute the instruction (cf. Kane's book)
    switch (instr->opCode) {
	
      case OP_ADD:
	sum = registers[(int)instr->rs] + registers[(int)instr->rt];
	if (!((registers[(int)instr->rs] ^ registers[(int)instr->rt]) & SIGN_BIT) &&
	    ((registers[(int)instr->rs] ^ sum) & SIGN_BIT)) {
	    RaiseException(OverflowException, 0);
	    return;
	}
	registers[(int)instr->rd] = sum;
	break;
	
      case OP_ADDI:
	sum = registers[(int)instr->rs] + instr->extra;
	if (!((registers[(int)instr->rs] ^ instr->extra) & SIGN_BIT) &&
	    ((instr->extra ^ sum) & SIGN_BIT)) {
	    RaiseException(OverflowException, 0);
	    return;
	}
	registers[(int)instr->rt] = sum;
	break;
	
      case OP_ADDIU:
	registers[(int)instr->rt] = registers[(int)instr->rs] + instr->extra;
	break;
	
      case OP_ADDU:
	registers[(int)instr->rd] = registers[(int)instr->rs] + registers[(int)instr->rt];
	break;
	
      case OP_AND:
	registers[(int)instr->rd] = registers[(int)instr->rs] & registers[(int)instr->rt];
	break;
	
      case OP_ANDI:
	registers[(int)instr->rt] = registers[(int)instr->rs] & (instr->extra & 0xffff);
	break;
	
      case OP_BEQ:
	if (registers[(int)instr->rs] == registers[(int)instr->rt])
	    pcAfter = registers[NextPCReg] + IndexToAddr(instr->extra);
	break;
	
      case OP_BGEZAL:
	registers[R31] = registers[NextPCReg] + 4;
      case OP_BGEZ:
	if (!(registers[(int)instr->rs] & SIGN_BIT))
	    pcAfter = registers[NextPCReg] + IndexToAddr(instr->extra);
	break;
	
      case OP_BGTZ:
	if (registers[(int)instr->rs] > 0)
	    pcAfter = registers[NextPCReg] + IndexToAddr(instr->extra);
	break;
	
      case OP_BLEZ:
	if (registers[(int)instr->rs] <= 0)
	    pcAfter = registers[NextPCReg] + IndexToAddr(instr->extra);
	break;
	
      case OP_BLTZAL:
	registers[R31] = registers[NextPCReg] + 4;
      case OP_BLTZ:
	if (registers[(int)instr->rs] & SIGN_BIT)
	    pcAfter = registers[NextPCReg] + IndexToAddr(instr->extra);
	break;
	
      case OP_BNE:
	if (registers[(int)instr->rs] != registers[(int)instr->rt])
	    pcAfter = registers[NextPCReg] + IndexToAddr(instr->extra);
	break;
	
      case OP_DIV:
	if (registers[(int)instr->rt] == 0) {
	    registers[LoReg] = 0;
	    registers[HiReg] = 0;
	} else {
	    registers[LoReg] =  registers[(int)instr->rs] / registers[(int)instr->rt];
	    registers[HiReg] = registers[(int)instr->rs] % registers[(int)instr->rt];
	}
	break;
	
      case OP_DIVU:	  
	  rs = (unsigned int) registers[(int)instr->rs];
	  rt = (unsigned int) registers[(int)instr->rt];
	  if (rt == 0) {
	      registers[LoReg] = 0;
	      registers[HiReg] = 0;
	  } else {
	      tmp = rs / rt;
	      registers[LoReg] = (int) tmp;
	      tmp = rs % rt;
	      registers[HiReg] = (int) tmp;
	  }
	  break;
	
      case OP_JAL:
	registers[R31] = registers[NextPCReg] + 4;
      case OP_J:
	pcAfter = (pcAfter & 0xf0000000) | IndexToAddr(instr->extra);
	break;
	
      case OP_JALR:
	registers[(int)instr->rd] = registers[NextPCReg] + 4;
      case OP_JR:
	pcAfter = registers[(int)instr->rs];
	break;
	
      case OP_LB:
      case OP_LBU:
	tmp = registers[(int)instr->rs] + instr->extra;
	if (!machine->ReadMem(tmp, 1, &value))
	    return;

	if ((value & 0x80) && (instr->opCode == OP_LB))
	    value |= 0xffffff00;
	else
	    value &= 0xff;
	nextLoadReg = instr->rt;
	nextLoadValue = value;
	break;
	
      case OP_LH:
      case OP_LHU:	  
	tmp = registers[(int)instr->rs] + instr->extra;
	if (tmp & 0x1) {
	    RaiseException(AddressErrorException, tmp);
	    return;
	}
	if (!machine->ReadMem(tmp, 2, &value))
	    return;

	if ((value & 0x8000) && (instr->opCode == OP_LH))
	    value |= 0xffff0000;
	else
	    value &= 0xffff;
	nextLoadReg = instr->rt;
	nextLoadValue = value;
	break;
      	
      case OP_LUI:
	DEBUG('m', "Executing: LUI r%d,%d\n", instr->rt, instr->extra);
	registers[(int)instr->rt] = instr->extra << 16;
	break;
	
      case OP_LW:
	tmp = registers[(int)instr->rs] + instr->extra;
	if (tmp & 0x3) {
	    RaiseException(AddressErrorException, tmp);
	    return;
	}
	if (!machine->ReadMem(tmp, 4, &value))
	    return;
	nextLoadReg = instr->rt;
	nextLoadValue = value;
	break;
    	
      case OP_LWL:	  
	tmp = registers[(int)instr->rs] + instr->extra;

	// ReadMem assumes all 4 byte requests are aligned on an even 
	// word boundary.  Also, the little endian/big endian swap code would
        // fail (I think) if the other cases are ever exercised.
	ASSERT((tmp & 0x3) == 0);  

	if (!machine->ReadMem(tmp, 4, &value))
	    return;
	if (registers[LoadReg] == instr->rt)
	    nextLoadValue = registers[LoadValueReg];
	else
	    nextLoadValue = registers[(int)instr->rt];
	switch (tmp & 0x3) {
	  case 0:
	    nextLoadValue = value;
	    break;
	  case 1:
	    nextLoadValue = (nextLoadValue & 0xff) | (value << 8);
	    break;
	  case 2:
	    nextLoadValue = (nextLoadValue & 0xffff) | (value << 16);
	    break;
	  case 3:
	    nextLoadValue = (nextLoadValue & 0xffffff) | (value << 24);
	    break;
	}
	nextLoadReg = instr->rt;
	break;
      	
      case OP_LWR:
	tmp = registers[(int)instr->rs] + instr->extra;

	// ReadMem assumes all 4 byte requests are aligned on an even 
	// word boundary.  Also, the little endian/big endian swap code would
        // fail (I think) if the other cases are ever exercised.
	ASSERT((tmp & 0x3) == 0);  

	if (!machine->ReadMem(tmp, 4, &value))
	    return;
	if (registers[LoadReg] == instr->rt)
	    nextLoadValue = registers[LoadValueReg];
	else
	    nextLoadValue = registers[(int)instr->rt];
	switch (tmp & 0x3) {
	  case 0:
	    nextLoadValue = (nextLoadValue & 0xffffff00) |
		((value >> 24) & 0xff);
	    break;
	  case 1:
	    nextLoadValue = (nextLoadValue & 0xffff0000) |
		((value >> 16) & 0xffff);
	    break;
	  case 2:
	    nextLoadValue = (nextLoadValue & 0xff000000)
		| ((value >> 8) & 0xffffff);
	    break;
	  case 3:
	    nextLoadValue = value;
	    break;
	}
	nextLoadReg = instr->rt;
	break;
    	
      case OP_MFHI:
	registers[(int)instr->rd] = registers[HiReg];
	break;
	
      case OP_MFLO:
	registers[(int)instr->rd] = registers[LoReg];
	break;
	
      case OP_MTHI:
	registers[HiReg] = registers[(int)instr->rs];
	break;
	
      case OP_MTLO:
	registers[LoReg] = registers[(int)instr->rs];
	break;
	
      case OP_MULT:
	Mult(registers[(int)instr->rs], registers[(int)instr->rt], true,
	     &registers[HiReg], &registers[LoReg]);
	break;
	
      case OP_MULTU:
	Mult(registers[(int)instr->rs], registers[(int)instr->rt], false,
	     &registers[HiReg], &registers[LoReg]);
	break;
	
      case OP_NOR:
	registers[(int)instr->rd] = ~(registers[(int)instr->rs] | registers[(int)instr->rt]);
	break;
	
      case OP_OR:
	registers[(int)instr->rd] = registers[(int)instr->rs] | registers[(int)instr->rs];
	break;
	
      case OP_ORI:
	registers[(int)instr->rt] = registers[(int)instr->rs] | (instr->extra & 0xffff);
	break;
	
      case OP_SB:
	if (!machine->WriteMem((unsigned) 
		(registers[(int)instr->rs] + instr->extra), 1, registers[(int)instr->rt]))
	    return;
	break;
	
      case OP_SH:
	if (!machine->WriteMem((unsigned) 
		(registers[(int)instr->rs] + instr->extra), 2, registers[(int)instr->rt]))
	    return;
	break;
	
      case OP_SLL:
	registers[(int)instr->rd] = registers[(int)instr->rt] << instr->extra;
	break;
	
      case OP_SLLV:
	registers[(int)instr->rd] = registers[(int)instr->rt] <<
	    (registers[(int)instr->rs] & 0x1f);
	break;
	
      case OP_SLT:
	if (registers[(int)instr->rs] < registers[(int)instr->rt])
	    registers[(int)instr->rd] = 1;
	else
	    registers[(int)instr->rd] = 0;
	break;
	
      case OP_SLTI:
	if (registers[(int)instr->rs] < instr->extra)
	    registers[(int)instr->rt] = 1;
	else
	    registers[(int)instr->rt] = 0;
	break;
	
      case OP_SLTIU:	  
	rs = registers[(int)instr->rs];
	imm = instr->extra;
	if (rs < imm)
	    registers[(int)instr->rt] = 1;
	else
	    registers[(int)instr->rt] = 0;
	break;
      	
      case OP_SLTU:	  
	rs = registers[(int)instr->rs];
	rt = registers[(int)instr->rt];
	if (rs < rt)
	    registers[(int)instr->rd] = 1;
	else
	    registers[(int)instr->rd] = 0;
	break;
      	
      case OP_SRA:
	registers[(int)instr->rd] = registers[(int)instr->rt] >> instr->extra;
	break;
	
      case OP_SRAV:
	registers[(int)instr->rd] = registers[(int)instr->rt] >>
	    (registers[(int)instr->rs] & 0x1f);
	break;
	
      case OP_SRL:
	tmp = registers[(int)instr->rt];
	tmp >>= instr->extra;
	registers[(int)instr->rd] = tmp;
	break;
	
      case OP_SRLV:
	tmp = registers[(int)instr->rt];
	tmp >>= (registers[(int)instr->rs] & 0x1f);
	registers[(int)instr->rd] = tmp;
	break;
	
      case OP_SUB:	  
	diff = registers[(int)instr->rs] - registers[(int)instr->rt];
	if (((registers[(int)instr->rs] ^ registers[(int)instr->rt]) & SIGN_BIT) &&
	    ((registers[(int)instr->rs] ^ diff) & SIGN_BIT)) {
	    RaiseException(OverflowException, 0);
	    return;
	}
	registers[(int)instr->rd] = diff;
	break;
      	
      case OP_SUBU:
	registers[(int)instr->rd] = registers[(int)instr->rs] - registers[(int)instr->rt];
	break;
	
      case OP_SW:
	if (!machine->WriteMem((unsigned) 
		(registers[(int)instr->rs] + instr->extra), 4, registers[(int)instr->rt]))
	    return;
	break;
	
      case OP_SWL:	  
	tmp = registers[(int)instr->rs] + instr->extra;

	// The little endian/big endian swap code would
        // fail (I think) if the other cases are ever exercised.
	ASSERT((tmp & 0x3) == 0);  

	if (!machine->ReadMem((tmp & ~0x3), 4, &value))
	    return;
	switch (tmp & 0x3) {
	  case 0:
	    value = registers[(int)instr->rt];
	    break;
	  case 1:
	    value = (value & 0xff000000) | ((registers[(int)instr->rt] >> 8) &
					    0xffffff);
	    break;
	  case 2:
	    value = (value & 0xffff0000) | ((registers[(int)instr->rt] >> 16) &
					    0xffff);
	    break;
	  case 3:
	    value = (value & 0xffffff00) | ((registers[(int)instr->rt] >> 24) &
					    0xff);
	    break;
	}
	if (!machine->WriteMem((tmp & ~0x3), 4, value))
	    return;
	break;
    	
      case OP_SWR:	  
	tmp = registers[(int)instr->rs] + instr->extra;

	// The little endian/big endian swap code would
        // fail (I think) if the other cases are ever exercised.
	ASSERT((tmp & 0x3) == 0);  

	if (!machine->ReadMem((tmp & ~0x3), 4, &value))
	    return;
	switch (tmp & 0x3) {
	  case 0:
	    value = (value & 0xffffff) | (registers[(int)instr->rt] << 24);
	    break;
	  case 1:
	    value = (value & 0xffff) | (registers[(int)instr->rt] << 16);
	    break;
	  case 2:
	    value = (value & 0xff) | (registers[(int)instr->rt] << 8);
	    break;
	  case 3:
	    value = registers[(int)instr->rt];
	    break;
	}
	if (!machine->WriteMem((tmp & ~0x3), 4, value))
	    return;
	break;
    	
      case OP_SYSCALL:
	RaiseException(SyscallException, 0);
	return; 
	
      case OP_XOR:
	registers[(int)instr->rd] = registers[(int)instr->rs] ^ registers[(int)instr->rt];
	break;
	
      case OP_XORI:
	registers[(int)instr->rt] = registers[(int)instr->rs] ^ (instr->extra & 0xffff);
	break;
	
      case OP_RES:
      case OP_UNIMP:
	RaiseException(IllegalInstrException, 0);
	return;
	
      default:
	ASSERT(false);
    }
    
    // Now we have successfully executed the instruction.
    
    // Do any delayed load operation
    DelayedLoad(nextLoadReg, nextLoadValue);
    
    // Advance program counters.
    registers[PrevPCReg] = registers[PCReg];	// for debugging, in case we
						// are jumping into lala-land
    registers[PCReg] = registers[NextPCReg];
    registers[NextPCReg] = pcAfter;
}

//----------------------------------------------------------------------
// Machine::DelayedLoad
// 	Simulate effects of a delayed load.
//
// 	NOTE -- RaiseException/CheckInterrupts must also call DelayedLoad,
//	since any delayed load must get applied before we trap to the kernel.
//----------------------------------------------------------------------

void
Machine::DelayedLoad(int nextReg, int nextValue)
{
    registers[registers[LoadReg]] = registers[LoadValueReg];
    registers[LoadReg] = nextReg;
    registers[LoadValueReg] = nextValue;
    registers[0] = 0; 	// and always make sure R0 stays zero.
}

//----------------------------------------------------------------------
// Instruction::Decode
// 	Decode a MIPS instruction 
//----------------------------------------------------------------------

void
Instruction::Decode()
{
    OpInfo *opPtr;
    
    rs = (value >> 21) & 0x1f;
    rt = (value >> 16) & 0x1f;
    rd = (value >> 11) & 0x1f;
    opPtr = &opTable[(value >> 26) & 0x3f];
    opCode = opPtr->opCode;
    if (opPtr->format == IFMT) {
	extra = value & 0xffff;
	if (extra & 0x8000) {
    	   extra |= 0xffff0000;
	}
    } else if (opPtr->format == RFMT) {
	extra = (value >> 6) & 0x1f;
    } else {
	extra = value & 0x3ffffff;
    }
    if (opCode == SPECIAL) {
	opCode = specialTable[value & 0x3f];
    } else if (opCode == BCOND) {
	int i = value & 0x1f0000;

	if (i == 0) {
    	    opCode = OP_BLTZ;
	} else if (i == 0x10000) {
    	    opCode = OP_BGEZ;
	} else if (i == 0x100000) {
    	    opCode = OP_BLTZAL;
	} else if (i == 0x110000) {
    	    opCode = OP_BGEZAL;
	} else {
    	    opCode = OP_UNIMP;
	}
    }
}

//----------------------------------------------------------------------
// Mult
// 	Simulate R2000 multiplication.
// 	The words at *hiPtr and *loPtr are overwritten with the
// 	double-length result of the multiplication.
//----------------------------------------------------------------------

static void
Mult(int a, int b, bool signedArith, int* hiPtr, int* loPtr)
{
    if ((a == 0) || (b == 0)) {
	*hiPtr = *loPtr = 0;
	return;
    }

    // Compute the sign of the result, then make everything positive
    // so unsigned computation can be done in the main loop.
    bool negative = false;
    if (signedArith) {
	if (a < 0) {
	    negative = !negative;
	    a = -a;
	}
	if (b < 0) {
	    negative = !negative;
	    b = -b;
	}
    }

    // Compute the result in unsigned arithmetic (check a's bits one at
    // a time, and add in a shifted value of b).
    unsigned int bLo = b;
    unsigned int bHi = 0;
    unsigned int lo = 0;
    unsigned int hi = 0;
    for (int i = 0; i < 32; i++) {
	if (a & 1) {
	    lo += bLo;
	    if (lo < bLo)  // Carry out of the low bits?
		hi += 1;
	    hi += bHi;
	    if ((a & 0xfffffffe) == 0)
		break;
	}
	bHi <<= 1;
	if (bLo & 0x80000000)
	    bHi |= 1;
	
	bLo <<= 1;
	a >>= 1;
    }

    // If the result is supposed to be negative, compute the two's
    // complement of the double-word result.
    if (negative) {
	hi = ~hi;
	lo = ~lo;
	lo++;
	if (lo == 0)
	    hi++;
    }
    
    *hiPtr = (int) hi;
    *loPtr = (int) lo;
}
