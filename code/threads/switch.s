/* switch.s 
 *   	Machine dependent context switch routines.  DO NOT MODIFY THESE! 
 *
 *	Context switching is inherently machine dependent, since
 *	the registers to be saved, how to set up an initial
 *	call frame, etc, are all specific to a processor architecture.
 *
 * 	This file currently supports the following architectures:
 *	    Intel 386
 * 	    Intel 64-bit
 *
 * We define two routines for each architecture:
 *
 * ThreadRoot(InitialPC, InitialArg, WhenDonePC, StartupPC)
 *	InitialPC  - The program counter of the procedure to run
 *			in this thread.
 *      InitialArg - The single argument to the thread.
 *	WhenDonePC - The routine to call when the thread returns.
 *	StartupPC  - Routine to call when the thread is started.
 *
 *	ThreadRoot is called from the SWITCH() routine to start
 *	a thread for the first time. 
 *
 * SWITCH(oldThread, newThread)
 * 	oldThread  - The current thread that was running, where the
 *		CPU register state is to be saved.
 * 	newThread  - The new thread to be run, where the CPU register
 *		state is to be loaded from.
 */

/*
 Copyright (c) 1992-1993 The Regents of the University of California.
 All rights reserved.  See copyright.h for copyright notice and limitation 
 of liability and disclaimer of warranty provisions.
 */

#include "copyright.h"
#include "switch.h"

#ifdef HOST_i386
        .text
        .align  2

        .globl  ThreadRoot

/* void ThreadRoot( void )
**
** expects the following registers to be initialized:
**      eax     points to startup function (interrupt enable)
**      edx     contains inital argument to thread function
**      esi     points to thread function
**      edi     point to Thread::Finish()
*/
ThreadRoot:
        pushl   %ebp
        movl    %esp,%ebp
        pushl   %edx /*InitialArg*/
        call    *%ecx /*StartupPC*/
        call    *%esi /*InitialPC*/
        call    *%edi /*WhenDonePC*/
        /*pushl   InitialArg
        call    StartupPC
        call    InitialPC
        call    WhenDonePC*/

        # NOT REACHED
        movl    %ebp,%esp
        popl    %ebp
        ret



/* void SWITCH( thread *t1, thread *t2 )
**
** on entry, stack looks like this:
**      8(esp)  ->              thread *t2
**      4(esp)  ->              thread *t1
**       (esp)  ->              return address
**
** we push the current eax on the stack so that we can use it as
** a pointer to t1, this decrements esp by 4, so when we use it
** to reference stuff on the stack, we add 4 to the offset.
*/
        .comm   _eax_save,4

        .globl  SWITCH
SWITCH:
        movl    %eax,_eax_save          # save the value of eax
        movl    4(%esp),%eax            # move pointer to t1 into eax
        movl    %ebx,_EBX(%eax)         # save registers
        movl    %ecx,_ECX(%eax)
        movl    %edx,_EDX(%eax)
        movl    %esi,_ESI(%eax)
        movl    %edi,_EDI(%eax)
        movl    %ebp,_EBP(%eax)
        movl    %esp,_ESP(%eax)         # save stack pointer
        movl    _eax_save,%ebx          # get the saved value of eax
        movl    %ebx,_EAX(%eax)         # store it
        movl    0(%esp),%ebx            # get return address from stack into ebx
        movl    %ebx,_PC(%eax)          # save it into the pc storage

        movl    8(%esp),%eax            # move pointer to t2 into eax

        movl    _EAX(%eax),%ebx         # get new value for eax into ebx
        movl    %ebx,_eax_save          # save it
        movl    _EBX(%eax),%ebx         # retore old registers
        movl    _ECX(%eax),%ecx
        movl    _EDX(%eax),%edx
        movl    _ESI(%eax),%esi
        movl    _EDI(%eax),%edi
        movl    _EBP(%eax),%ebp
        movl    _ESP(%eax),%esp         # restore stack pointer
        movl    _PC(%eax),%eax          # restore return address into eax
        movl    %eax,4(%esp)            # copy over the ret address on the stack
        movl    _eax_save,%eax

        ret

#elif defined(HOST_x86_64)
// x86 64-bit architecture
        .text
        .align  8

        .globl  ThreadRoot

/* void ThreadRoot( void )
**
** expects the following registers to be initialized:
**      rax     points to startup function (interrupt enable)
**      rbx     contains initial argument to thread function [InitialArg]
**      rsi     points to thread function [InitialPC]
**      rdi     point to Thread::Finish() [WhenDonePCState]
*/
ThreadRoot:
        push   %rbp
        mov    %rsp,%rbp
        push   %rdi
        push   %rsi
        callq  *%rax		# StartupPC ()
        mov    %rbx,%rdi
        pop    %rsi
        callq  *%rsi		# InitialPC (InitialArg)
        pop    %rsi
        callq  *%rsi		# WhenDonePC ()

        # NOT REACHED
        mov    %rbp,%rsp
        pop    %rbp
        ret

/* void SWITCH( thread *t1, thread *t2 )
**
** on entry, this is the state of stack and registers
** (see abi.pdf in the http://www.x86-64.org site):
**	rdi	 ->	thread *t1
**	rsi	 ->	thread *t2
**	(rsp)	 ->	return address
**
*/
        .globl  SWITCH
SWITCH:
        mov    %rax,_RAX(%rdi)         # save registers 
        mov    %rbx,_RBX(%rdi)         
        mov    %rcx,_RCX(%rdi)
        mov    %rdx,_RDX(%rdi)
        mov    %rsi,_RSI(%rdi)
        mov    %rdi,_RDI(%rdi)
        mov    %rbp,_RBP(%rdi)
        mov    %rsp,_RSP(%rdi)         # save stack pointer
        movq   0(%rsp),%rax            # get return address from stack into rax
        mov    %rax,_PC(%rdi)          # save it into the pc storage
        mov    %r8,_R8(%rdi)
        mov    %r9,_R9(%rdi)
        mov    %r10,_R10(%rdi)
        mov    %r11,_R11(%rdi)
        mov    %r12,_R12(%rdi)
        mov    %r13,_R13(%rdi)
        mov    %r14,_R14(%rdi)
        mov    %r15,_R15(%rdi)

        mov    _RBX(%rsi),%rbx         # restore old registers
        mov    _RCX(%rsi),%rcx
        mov    _RDX(%rsi),%rdx
        mov    _RDI(%rsi),%rdi
        mov    _RBP(%rsi),%rbp
        mov    _RSP(%rsi),%rsp         # restore stack pointer
        mov    _R8(%rsi),%r8
        mov    _R9(%rsi),%r9
        mov    _R10(%rsi),%r10
        mov    _R11(%rsi),%r11
        mov    _R12(%rsi),%r12
        mov    _R13(%rsi),%r13
        mov    _R14(%rsi),%r14
        mov    _R15(%rsi),%r15
        mov    _PC(%rsi),%rax          # restore return address into eax
        mov    %rax,0(%rsp)            # copy over the ret address on the stack
        mov    %rsi,%rax
        mov    _RSI(%rsi),%rsi         # replace rsi with stored value
        mov    _RAX(%rax),%rax         # use t2 pointer previously saved in rax

        ret

#endif
