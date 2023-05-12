#include "tests.h"
#include "x86_desc.h"
#include "lib.h"
#include "filesys.h"
#include "terminal.h"
#include "types.h"
#include "rtc.h"
#include "syscall.h"

#define PASS 1
#define FAIL 0
#define KERNEL 0x400000
#define KB 0x1000
#define MB 0x100000
#define FILE_NUM 14
#define FRAME0_SIZE 264


/* format these macros as you see fit */
#define TEST_HEADER 	\
	printf("[TEST %s] Running %s at %s:%d\n", __FUNCTION__, __FUNCTION__, __FILE__, __LINE__)
#define TEST_OUTPUT(name, result)	\
	printf("[TEST %s] Result = %s\n", name, (result) ? "PASS" : "FAIL");

static inline void assertion_failure(){
	/* Use exception #15 for assertions, otherwise
	   reserved by Intel */
	asm volatile("int $15");
}


/* Checkpoint 1 tests */

/* IDT Test - Example
 * 
 * Asserts that first 10 IDT entries are not NULL
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: Load IDT, IDT definition
 * Files: x86_desc.h/S
 */
int idt_test(){
	TEST_HEADER;

	int i;
	int result = PASS;
	for (i = 0; i < 10; ++i){
		if ((idt[i].offset_15_00 == NULL) && 
			(idt[i].offset_31_16 == NULL)){
			assertion_failure();
			result = FAIL;
		}
	}

	// uncomment to check specific interrupt

	//asm volatile("int $0x0");
	//asm volatile("int $0x1");
	//asm volatile("int $0x2");
	//asm volatile("int $0x3");
	//asm volatile("int $0x4");
	//asm volatile("int $0x5");
	//asm volatile("int $0x6");
	//asm volatile("int $0x7");
	//asm volatile("int $0x8");
	//asm volatile("int $0x9");
	//asm volatile("int $0xA");
	//asm volatile("int $0xB");
	//asm volatile("int $0xC");
	//asm volatile("int $0xD");
	//asm volatile("int $0xE");
	//asm volatile("int $0x10");
	//asm volatile("int $0x11");
	//asm volatile("int $0x12");
	//asm volatile("int $0x13");

	return result;
}

/* Video Memory Test
 * 
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: Page Fault
 * Coverage: Accesses one before video memory
 * Files: None
 */
int v_mem(){
	TEST_HEADER;
	int* ptr = (int*)(VIDEO-1);
	//int* ptr = (int*)(0xB9000);
	int test = *ptr;
	if(test != -1){
		return PASS;
	}
	return PASS;
}

/* Kernel Memory Test
 * 
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: Page Fault
 * Coverage: Accesses one before kernel memory
 * Files: None
 */
int kernel_mem(){
	TEST_HEADER;
	int* ptr = (int*)(KERNEL-1);
	int test = *ptr;
	if(test != -1){
		return PASS;
	}
	return PASS;
}


/* Null Dereference Test
 * 
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: Seg Fault
 * Coverage: Dereferences null ptr
 * Files: None
 */
int null_deref(){
	TEST_HEADER;
	int* ptr = NULL;
	int test = *ptr;
	if(test != -1){
		return PASS;
	}
	return PASS;
}

/* Page Test
 * 
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: Prints addresses being checked
 * Coverage: Dereferences kernel memory and video memory
 * Files: None
 */
int page_check(){
    TEST_HEADER;
    uint32_t i, j;
    for( i = VIDEO; i < VIDEO+KB; i+= 4){
		printf("%x \n", i);
        char* test = (char*)i;
        j = *test;
    }
    for( i = KERNEL ; i < KERNEL+4*MB; i++){
		printf("%x \n", i);
        char* test = (char*)i;
        j = *test;
    }
    return PASS;
}

/* System Call Test
 * 
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: Prints message and stops program
 * Coverage: Acesses interrupt 0x80
 * Files: syscall.c/h
 */
int syscall_check(){
    asm volatile("int $0x80");
    return PASS;
}

/* Checkpoint 2 tests */

/* Read Dentry By Index Test
 * 
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: Prints all directory entry names
 * Coverage: Acesses all directory names
 * Files: filesys.c/h
 */
int read_dentry_by_index_test(){
	TEST_HEADER;
	int i;
	for(i = 0; i < FILE_NUM; i++){
		dentry_t extr;
		int32_t test = read_dentry_by_index(i, &extr);
		if(test < 0 && i > 0) return FAIL;
		printf("%s \n", extr.filename);
	}
	return PASS;
}

/* Read Dentry By Name Test
 * 
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: Prints all directory entry names
 * Coverage: Acesses all directory names
 * Files: filesys.c/h
 */
int read_dentry_by_name_test(){
	TEST_HEADER;
	char* filenombers[FILE_NUM] = {
		"cat",
		"counter",
		"fish",
		"frame0.txt",
		"frame1.txt",
		"grep",
		"hello", 
		"ls",
		"pingpong",
		"shell",
		"sigtest",
		"syserr",
		"testprint",
		"verylargetextwithverylongname.txt"
	};
	int i;
	for(i = 0; i < FILE_NUM; i++){
		dentry_t extr;
		int32_t test = read_dentry_by_name((uint8_t*)filenombers[i], &extr);
		if(test < 0){
			return FAIL;
		}
		printf("%s \n", filenombers[i]);
	}
	return PASS;
}

/* Directory Read Test
 * 
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: Prints all directory entry names
 * Coverage: Acesses all directory names
 * Files: filesys.c/h
 */
int read_fs_test(){
	TEST_HEADER;
	int i;
	for(i = 0; i < FILE_NUM; i++){
		char buf[FILENAME_SIZE];
		dir_read(0, &buf, FILENAME_SIZE);
		printf("%s \n", buf);
	}
	return PASS;
}

/* File Read Test
 * 
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: Prints content of files
 * Coverage: Checks file read on multiple files
 * Files: filesys.c/h
 */
int read_file_test(){
	TEST_HEADER;
	int i; 
	/* char buf[FRAME0_SIZE] = "frame0.txt";
	if(file_read(0, &buf, FRAME0_SIZE) <= 0)
	{
		return FAIL;
	}
	printf("%s", buf); */

	/* char buf3[10000] = "verylargetextwithverylongname.txt";
	if(file_read(0, &buf3, 10000) <= 0)
	{
		return FAIL;
	}
	printf("%s", buf3); */

	char buf4[5605] = "shell";
	if(file_read(0, &buf4, 5605) <= 0)
	{
		return FAIL;
	}

	clear();

	printf("%s", buf4);
	
	for(i = 0; i < 5605; i++){
		putc(buf4[i]);
	}
	return PASS;
}

/* Terminal Read Test
 * 
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: Prints to screen and asks for terminal input
 * Coverage: Checks terminal read
 * Files: terminal.c/h
 */
int terminal_read_test(){
	TEST_HEADER;
	printf("TYPE: (asdf) \n");
	char asdf[4];
	while(terminal_read(0, &asdf ,4) < 0);
	char cmp[4] = "asdf";
	if(strncmp((asdf), (cmp), 4) != 0) return FAIL;
	return PASS;
}

/* Terminal Write Test
 * 
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: Prints to screen 
 * Coverage: Checks terminal write
 * Files: terminal.c/h
 */
int terminal_write_test(){
	TEST_HEADER;
	char buf[16] = "16 byte test aaa";
	if(terminal_write(0, buf, 16) != 0)
		return FAIL;
	printf("\n");

	int i;
	char buf2[160];
	for(i = 0; i < 160; i++){
		buf2[i] = 'a';
	}

	if(terminal_write(0, buf2, 160) != 0)
		return FAIL;
	printf("\n");

	char buf3[128];
	for(i = 0; i < 128; i++){
		buf3[i] = 'a';
	}

	if(terminal_write(0, buf2, 128) != 0)
		return FAIL;
	return PASS;
}

/* RTC Test
 * 
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: Prints to screen 
 * Coverage: Checks rtc and rtc read
 * Files: rtc.c/h
 */
int rtc_test(){
	TEST_HEADER;
	uint8_t buf[4];
	rtc_open(buf);
	// default freq 2Hz
	int i;
	int x = 0;
	for(i = 0; i < 20; i++)
	{
		while(rtc_read(0, buf, 0) != 0);
		printf("%d \n", i);
	}

	// not power of 2
	uint32_t fre[1] = {513};
	rtc_write(0, &fre, 4);
	for(i = 0; i < 20; i++)
	{
		while(rtc_read(0, buf, 0) != 0);
		printf("%d \n", i);
	}

	// set to 1024Hz
	x = 0;
	uint32_t freq[1] = {1024};
	rtc_write(0, &freq, 4);
	for(i = 0; i < 2000; i++)
	{
		while(rtc_read(0, buf, 0) != 0);
		printf("%d \n", i);
	}
	rtc_close(0);
	return PASS;
}

/* Checkpoint 3 tests */
/* int context_switch_spam(){
	TEST_HEADER;
	execute((uint8_t*)"shell");
	char buf[] = "ls\n";
	int i;
	for(i = 0; i < 10000; i++){
		//terminal_write(0, buf, 3);
		write(1, buf, 3);
	}
	return PASS;
} */
/* Checkpoint 4 tests */
/* Checkpoint 5 tests */


/* Test suite entry point */
void launch_tests(){
	// checkpoint 1

	//TEST_OUTPUT("idt_test", idt_test());
	//TEST_OUTPUT("v_mem", v_mem());
	//TEST_OUTPUT("kernel_mem", kernel_mem());
	//TEST_OUTPUT("null_deref", null_deref());
	//TEST_OUTPUT("page_check", page_check());

	// checkpoint 2

	//TEST_OUTPUT("read_dentry_by_index_test", read_dentry_by_index_test());
	//TEST_OUTPUT("read_dentry_by_name_test", read_dentry_by_name_test());
	//TEST_OUTPUT("read_file_test", read_file_test());
	//TEST_OUTPUT("read_fs_test", read_fs_test());
	//TEST_OUTPUT("terminal_read_test", terminal_read_test());
	//TEST_OUTPUT("terminal_write_test", terminal_write_test());
	//TEST_OUTPUT("rtc_test", rtc_test());

	// checkpoint 3

	//TEST_OUTPUT("context_switch_spam", context_switch_spam());

}
