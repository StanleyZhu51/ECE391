#ifndef _SYSCALL_LINKAGE_H
#define _SYSCALL_LINKAGE_H

extern void syscall_linker(void);
extern void halt_ret(uint32_t s_esp, uint32_t s_ebp, int32_t status);
extern void term_switch(uint32_t s_esp, uint32_t s_ebp, int32_t status);


#endif /* SYSCALL_LINKAGE.H */
