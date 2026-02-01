#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "shared.h"

int tests_passed = 0;
int tests_failed = 0;

#define TEST(name) void name()
#define RUN_TEST(name) do{ \
    printf("Running %s... ", #name); \
    name(); \
    printf("PASSED\n"); \
    tests_passed++;\
} while(0)

#define ASSERT_EQ(a, b) do{ \
    if((a) != (b)){\
        printf("FAILED: %s != %s (%d != %d)", #a, #b, (int)(a), (int)(b)); \
        tests_failed++;\
        return;\
    }\
} while(0)

#define ASSERT_STR_EQ(a, b) do { \
    if (strcmp(a, b) != 0) { \
        printf("FAILED: '%s' != '%s'\n", a, b); \
        tests_failed++; \
        return; \
    } \
} while(0)

TEST(test_opcode_lookup) {
    uint8_t opcode = opcode_map[OP_LOAD][ADDR_MEM];
    ASSERT_EQ(opcode, 0x01);
    
    opcode = opcode_map[OP_STORE][ADDR_DIR];
    ASSERT_EQ(opcode, 0x05);
}

TEST(test_addressing_mode_detection){
    Addr_Mode mode = get_addr_mode("@foo");
    ASSERT_EQ(mode, ADDR_MEM);

    Addr_Mode mode = get_addr_mode("15");
    ASSERT_EQ(mode, ADDR_DIR);

    Addr_Mode mode = get_addr_mode("R0");
    ASSERT_EQ(mode, ADDR_REG);
}

int main(){
    printf("=== Running Assembler Unit Tests ===\n\n");
    RUN_TEST(test_opcode_lookup);
    printf("\n=== Test Results ===\n");

    printf("Passed: %d\n", tests_passed);
    printf("Failed: %d\n", tests_failed);
    
    return tests_failed > 0 ? 1 : 0;
}