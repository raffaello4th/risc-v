#include "../3rd/greatest/greatest.h"
#include "../rv_core.h"
rv32* _risc_v = NULL;
TEST testInit(rv32** _rv32) {
	*_rv32 = rv32_init(4096);
	ASSERT(*_rv32);
	ASSERT((*_rv32)->mem);
	ASSERT((*_rv32)->mem->head);
	ASSERT_EQ_FMT(4096, (*_rv32)->mem->size, "rv memory init %d!");
	PASS();
}
TEST testLoad(rv32** _rv32, const char* _filename) {
	ASSERT(*_rv32);
	ASSERT(_filename);
	rv32_load(*_rv32, _filename);
	ASSERT(*_rv32);
	PASS();
}
TEST testInstruc(rv32** _rv32) {
	rv32_run(*_rv32);
	PASS();
}
TEST testFree(rv32** _rv32) {
	rv32_free(_rv32);
	ASSERT_FALSE(*_rv32);
	PASS();
}
TEST testLoad0() {
	return testLoad(&_risc_v, "../data/example01.bin");
}
SUITE(suite) {
	RUN_TEST1(testInit, &_risc_v);
	RUN_TEST(testLoad0);
	RUN_TEST1(testInstruc, &_risc_v);
	RUN_TEST1(testFree, &_risc_v);
}
/* Add definitions that need to be in the test runner's main file. */
GREATEST_MAIN_DEFS();
int main(int argc, char **argv){
	GREATEST_MAIN_BEGIN();      /* command-line options, initialization. */
		RUN_SUITE(suite);
	GREATEST_MAIN_END();        /* display results */
}