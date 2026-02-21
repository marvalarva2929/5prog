#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>

// Include all headers
#include "argparse.h"
#include "labletable.h"
#include "macro.h"
#include "parse.h"
#include "encode.h"

// Forward declarations for functions not in headers
int isLiteralArg(char * arg);
int isParArg(char * arg);
int isRegArg(char * arg);
char * trimWhitespace(char * str);
char * trimWhitespaceAlloc(char * str);

// Color codes for output
#define GREEN "\x1b[32m"
#define RED "\x1b[31m"
#define RESET "\x1b[0m"
#define YELLOW "\x1b[33m"

int tests_passed = 0;
int tests_failed = 0;

#define TEST(name) void test_##name()
#define RUN_TEST(name) run_test(#name, test_##name)

void run_test(const char *name, void (*test_func)()) {
    printf("Testing %s... ", name);
    fflush(stdout);
    
    test_func();
    
    printf(GREEN "PASS" RESET "\n");
    tests_passed++;
}

void assert_equal_int(int actual, int expected, const char *msg) {
    if (actual != expected) {
        printf(RED "FAIL" RESET " - %s (expected %d, got %d)\n", msg, expected, actual);
        tests_failed++;
        exit(1);
    }
}

void assert_equal_ull(unsigned long long actual, unsigned long long expected, const char *msg) {
    if (actual != expected) {
        printf(RED "FAIL" RESET " - %s (expected %llu, got %llu)\n", msg, expected, actual);
        tests_failed++;
        exit(1);
    }
}

void assert_equal_str(const char *actual, const char *expected, const char *msg) {
    if (strcmp(actual, expected) != 0) {
        printf(RED "FAIL" RESET " - %s (expected '%s', got '%s')\n", msg, expected, actual);
        tests_failed++;
        exit(1);
    }
}

void assert_true(int condition, const char *msg) {
    if (!condition) {
        printf(RED "FAIL" RESET " - %s\n", msg);
        tests_failed++;
        exit(1);
    }
}

void assert_false(int condition, const char *msg) {
    if (condition) {
        printf(RED "FAIL" RESET " - %s\n", msg);
        tests_failed++;
        exit(1);
    }
}

// ============ ARGPARSE TESTS ============

TEST(trimWhitespace_leading) {
    char *str = malloc(20);
    strcpy(str, "   hello");
    char *result = trimWhitespace(str);
    assert_equal_str(result, "hello", "trim leading whitespace");
    free(str);
}

TEST(trimWhitespace_trailing) {
    char *str = malloc(20);
    strcpy(str, "hello   ");
    char *result = trimWhitespace(str);
    assert_equal_str(result, "hello", "trim trailing whitespace");
    free(str);
}

TEST(trimWhitespace_both) {
    char *str = malloc(30);
    strcpy(str, "   hello world   ");
    char *result = trimWhitespace(str);
    assert_equal_str(result, "hello world", "trim both sides");
    free(str);
}

TEST(trimWhitespace_empty) {
    char *str = malloc(10);
    strcpy(str, "     ");
    char *result = trimWhitespace(str);
    assert_equal_str(result, "", "trim all whitespace");
    free(str);
}

TEST(trimWhitespaceAlloc_basic) {
    char *str = malloc(20);
    strcpy(str, "  test  ");
    char *result = trimWhitespaceAlloc(str);
    assert_equal_str(result, "test", "allocate trimmed string");
    free(str);
    free(result);
}

TEST(parseRegister_basic) {
    // parseRegister modifies the pointer, so we need to be careful
    char *str = malloc(10);
    strcpy(str, "r5");
    int reg = parseRegister(str);
    assert_equal_int(reg, 5, "parse register r5");
    free(str);
}

TEST(parseRegister_zero) {
    char *str = malloc(10);
    strcpy(str, "r0");
    int reg = parseRegister(str);
    assert_equal_int(reg, 0, "parse register r0");
    free(str);
}

TEST(parseRegister_high) {
    char *str = malloc(10);
    strcpy(str, "r31");
    int reg = parseRegister(str);
    assert_equal_int(reg, 31, "parse register r31");
    free(str);
}

TEST(parseLiteral_decimal) {
    char *str = malloc(10);
    strcpy(str, "42");
    unsigned long long val = parseLiteral(str);
    assert_equal_ull(val, 42, "parse decimal literal");
    free(str);
}

TEST(parseLiteral_hex) {
    char *str = malloc(10);
    strcpy(str, "0xFF");
    unsigned long long val = parseLiteral(str);
    assert_equal_ull(val, 255, "parse hex literal");
    free(str);
}

TEST(parseLiteral_hex_lowercase) {
    char *str = malloc(10);
    strcpy(str, "0xabc");
    unsigned long long val = parseLiteral(str);
    assert_equal_ull(val, 2748, "parse lowercase hex");
    free(str);
}

TEST(parseLiteral_large) {
    char *str = malloc(20);
    strcpy(str, "1000000");
    unsigned long long val = parseLiteral(str);
    assert_equal_ull(val, 1000000, "parse large decimal");
    free(str);
}

TEST(isLabel_true) {
    char *str = malloc(20);
    strcpy(str, ":myLabel");
    int result = isLabel(str);
    assert_true(result, "identify label reference");
    free(str);
}

TEST(isLabel_false) {
    char *str = malloc(10);
    strcpy(str, "r5");
    int result = isLabel(str);
    assert_false(result, "reject non-label");
    free(str);
}

TEST(isLabel_empty) {
    char *str = malloc(5);
    strcpy(str, "");
    int result = isLabel(str);
    assert_false(result, "reject empty string");
    free(str);
}

TEST(parseThreeReg_basic) {
    int rd, rs, rt;
    char *str = malloc(20);
    strcpy(str, "r1, r2, r3");
    parseThreeReg(str, &rd, &rs, &rt);
    assert_equal_int(rd, 1, "parse three regs - rd");
    assert_equal_int(rs, 2, "parse three regs - rs");
    assert_equal_int(rt, 3, "parse three regs - rt");
    free(str);
}

TEST(parseThreeReg_with_spaces) {
    int rd, rs, rt;
    char *str = malloc(30);
    strcpy(str, "  r10 ,  r15 ,  r20  ");
    parseThreeReg(str, &rd, &rs, &rt);
    assert_equal_int(rd, 10, "parse three regs with spaces - rd");
    assert_equal_int(rs, 15, "parse three regs with spaces - rs");
    assert_equal_int(rt, 20, "parse three regs with spaces - rt");
    free(str);
}

TEST(parseTwoReg_basic) {
    int rd, rs;
    char *str = malloc(10);
    strcpy(str, "r7, r8");
    parseTwoReg(str, &rd, &rs);
    assert_equal_int(rd, 7, "parse two regs - rd");
    assert_equal_int(rs, 8, "parse two regs - rs");
    free(str);
}

TEST(parseTwoReg_high_registers) {
    int rd, rs;
    char *str = malloc(10);
    strcpy(str, "r31, r0");
    parseTwoReg(str, &rd, &rs);
    assert_equal_int(rd, 31, "parse two regs - high rd");
    assert_equal_int(rs, 0, "parse two regs - zero rs");
    free(str);
}

TEST(parseRegLit_basic) {
    int rd, L;
    char *str = malloc(20);
    strcpy(str, "r5, 100");
    parseRegLit(str, &rd, &L);
    assert_equal_int(rd, 5, "parse reg+lit - rd");
    assert_equal_int(L, 100, "parse reg+lit - literal");
    free(str);
}

TEST(parseRegLit_hex) {
    int rd, L;
    char *str = malloc(20);
    strcpy(str, "r10, 0x200");
    parseRegLit(str, &rd, &L);
    assert_equal_int(rd, 10, "parse reg+lit hex - rd");
    assert_equal_int(L, 512, "parse reg+lit hex - literal");
    free(str);
}

TEST(parseSingleReg_basic) {
    char *str = malloc(10);
    strcpy(str, "r15");
    int reg = parseSingleReg(str);
    assert_equal_int(reg, 15, "parse single register");
    free(str);
}

TEST(parseSingleReg_with_spaces) {
    char *str = malloc(10);
    strcpy(str, "  r3  ");
    int reg = parseSingleReg(str);
    assert_equal_int(reg, 3, "parse single register with spaces");
    free(str);
}

TEST(extractCommandName_basic) {
    char *str = malloc(20);
    strcpy(str, "add r1, r2, r3");
    char *cmd = extractCommandName(str);
    assert_equal_str(cmd, "add", "extract command name");
    free(str);
    free(cmd);
}

TEST(extractCommandName_no_args) {
    char *str = malloc(20);
    strcpy(str, "halt");
    char *cmd = extractCommandName(str);
    assert_equal_str(cmd, "halt", "extract command name without args");
    free(str);
    free(cmd);
}

TEST(extractArguments_basic) {
    char *str = malloc(20);
    strcpy(str, "add r1, r2, r3");
    char *args = extractArguments(str);
    assert_equal_str(args, "r1, r2, r3", "extract arguments");
    free(str);
    free(args);
}

TEST(extractArguments_no_args) {
    char *str = malloc(10);
    strcpy(str, "halt");
    char *args = extractArguments(str);
    assert_equal_str(args, "", "extract arguments when none");
    free(str);
    free(args);
}

// ============ LABLETABLE TESTS ============

TEST(insertLabel_single) {
    ltable table;
    table.count = 0;
    insertLabel((char *)"start", 0x1000, &table);
    assert_equal_int(table.count, 1, "insert single label");
}

TEST(insertLabel_multiple) {
    ltable table;
    table.count = 0;
    insertLabel((char *)"start", 0x1000, &table);
    insertLabel((char *)"loop", 0x2000, &table);
    insertLabel((char *)"end", 0x3000, &table);
    assert_equal_int(table.count, 3, "insert multiple labels");
}

TEST(getintAddress_exists) {
    ltable table;
    table.count = 0;
    insertLabel((char *)"target", 0x5000, &table);
    uint64_t addr = getintAddress((char *)"target", &table);
    assert_equal_ull(addr, 0x5000, "retrieve existing label address");
}

TEST(insertLabel_name_preserved) {
    ltable table;
    table.count = 0;
    insertLabel((char *)"myLabel", 0x1234, &table);
    assert_equal_str(table.labels[0], "myLabel", "label name preserved");
}

TEST(insertLabel_address_preserved) {
    ltable table;
    table.count = 0;
    insertLabel((char *)"test", 0x1234, &table);
    assert_equal_ull(table.addresses[0], 0x1234, "label address preserved");
}

// ============ ENCODE TESTS ============

TEST(isLiteralArg_number) {
    char *str = malloc(10);
    strcpy(str, "42");
    int result = isLiteralArg(str);
    assert_true(result, "recognize literal argument");
    free(str);
}

TEST(isLiteralArg_register) {
    char *str = malloc(10);
    strcpy(str, "r5");
    int result = isLiteralArg(str);
    assert_false(result, "reject register as literal");
    free(str);
}

TEST(isLiteralArg_memory) {
    char *str = malloc(10);
    strcpy(str, "(r5)");
    int result = isLiteralArg(str);
    assert_false(result, "reject memory as literal");
    free(str);
}

TEST(isRegArg_register) {
    char *str = malloc(10);
    strcpy(str, "r10");
    int result = isRegArg(str);
    assert_true(result, "recognize register argument");
    free(str);
}

TEST(isRegArg_literal) {
    char *str = malloc(10);
    strcpy(str, "100");
    int result = isRegArg(str);
    assert_false(result, "reject literal as register");
    free(str);
}

TEST(isParArg_memory) {
    char *str = malloc(10);
    strcpy(str, "(r3)");
    int result = isParArg(str);
    assert_true(result, "recognize memory argument");
    free(str);
}

TEST(isParArg_register) {
    char *str = malloc(10);
    strcpy(str, "r3");
    int result = isParArg(str);
    assert_false(result, "reject register as memory");
    free(str);
}

TEST(build_instruction_basic) {
    uint32_t instr = build_instruction(0x18, 1, 2, 3, 0);
    // Verify structure: opcode(5) + rd(5) + rs(5) + rt(12) + imm(12)
    assert_true(instr != 0, "build instruction produces non-zero value");
}

TEST(build_instruction_opcode) {
    uint32_t instr = build_instruction(0x1F, 0, 0, 0, 0);
    // Top 5 bits should contain opcode
    uint32_t opcode = (instr >> 27) & 0x1F;
    assert_equal_int(opcode, 0x1F, "build instruction preserves opcode");
}

// ============ MACRO TESTS ============

TEST(isMacro_clr) {
    int result = isMacro(CLR);
    assert_true(result, "identify CLR as macro");
}

TEST(isMacro_ld) {
    int result = isMacro(LD);
    assert_true(result, "identify LD as macro");
}

TEST(isMacro_push) {
    int result = isMacro(PUSH);
    assert_true(result, "identify PUSH as macro");
}

TEST(isMacro_pop) {
    int result = isMacro(POP);
    assert_true(result, "identify POP as macro");
}

TEST(isMacro_non_macro) {
    int result = isMacro(ADD);
    assert_false(result, "reject ADD as non-macro");
}

TEST(isLabelReference_true) {
    char *str = malloc(20);
    strcpy(str, ":myLabel");
    int result = isLabelReference(str);
    assert_true(result, "identify label reference");
    free(str);
}

TEST(isLabelReference_false) {
    char *str = malloc(20);
    strcpy(str, "0x1000");
    int result = isLabelReference(str);
    assert_false(result, "reject non-label");
    free(str);
}

TEST(isLabelReference_empty) {
    char *str = malloc(5);
    strcpy(str, "");
    int result = isLabelReference(str);
    assert_false(result, "reject empty as label");
    free(str);
}

TEST(extractLabelName_basic) {
    char *str = malloc(20);
    strcpy(str, ":myLabel");
    char *name = extractLabelName(str);
    assert_equal_str(name, "myLabel", "extract label name");
    free(str);
    free(name);
}

// ============ PARSE TESTS ============

TEST(trim_basic) {
    char *str = malloc(20);
    strcpy(str, "  hello  ");
    char *result = trim(str);
    assert_equal_str(result, "hello", "trim basic");
    free(str);
    free(result);
}

TEST(trim_all_whitespace) {
    char *str = malloc(10);
    strcpy(str, "     ");
    char *result = trim(str);
    assert_equal_str(result, "", "trim all whitespace");
    free(str);
    free(result);
}

TEST(trim_tabs_and_spaces) {
    char *str = malloc(20);
    strcpy(str, "\t  hello  \t");
    char *result = trim(str);
    assert_equal_str(result, "hello", "trim tabs and spaces");
    free(str);
    free(result);
}

// ============ MAIN TEST RUNNER ============

int main() {
    printf("\n" YELLOW "=== Assembler Unit Tests ===" RESET "\n\n");
    
    // Run argparse tests
    printf(YELLOW "ArgParse Tests:\n" RESET);
    RUN_TEST(trimWhitespace_leading);
    RUN_TEST(trimWhitespace_trailing);
    RUN_TEST(trimWhitespace_both);
    RUN_TEST(trimWhitespace_empty);
    RUN_TEST(trimWhitespaceAlloc_basic);
    RUN_TEST(parseRegister_basic);
    RUN_TEST(parseRegister_zero);
    RUN_TEST(parseRegister_high);
    RUN_TEST(parseLiteral_decimal);
    RUN_TEST(parseLiteral_hex);
    RUN_TEST(parseLiteral_hex_lowercase);
    RUN_TEST(parseLiteral_large);
    RUN_TEST(isLabel_true);
    RUN_TEST(isLabel_false);
    RUN_TEST(isLabel_empty);
    RUN_TEST(parseThreeReg_basic);
    RUN_TEST(parseThreeReg_with_spaces);
    RUN_TEST(parseTwoReg_basic);
    RUN_TEST(parseTwoReg_high_registers);
    RUN_TEST(parseRegLit_basic);
    RUN_TEST(parseRegLit_hex);
    RUN_TEST(parseSingleReg_basic);
    RUN_TEST(parseSingleReg_with_spaces);
    RUN_TEST(extractCommandName_basic);
    RUN_TEST(extractCommandName_no_args);
    RUN_TEST(extractArguments_basic);
    RUN_TEST(extractArguments_no_args);
    
    // Run labletable tests
    printf(YELLOW "\nLableTable Tests:\n" RESET);
    RUN_TEST(insertLabel_single);
    RUN_TEST(insertLabel_multiple);
    RUN_TEST(getintAddress_exists);
    RUN_TEST(insertLabel_name_preserved);
    RUN_TEST(insertLabel_address_preserved);
    
    // Run encode tests
    printf(YELLOW "\nEncode Tests:\n" RESET);
    RUN_TEST(isLiteralArg_number);
    RUN_TEST(isLiteralArg_register);
    RUN_TEST(isLiteralArg_memory);
    RUN_TEST(isRegArg_register);
    RUN_TEST(isRegArg_literal);
    RUN_TEST(isParArg_memory);
    RUN_TEST(isParArg_register);
    RUN_TEST(build_instruction_basic);
    RUN_TEST(build_instruction_opcode);
    
    // Run macro tests
    printf(YELLOW "\nMacro Tests:\n" RESET);
    RUN_TEST(isMacro_clr);
    RUN_TEST(isMacro_ld);
    RUN_TEST(isMacro_push);
    RUN_TEST(isMacro_pop);
    RUN_TEST(isMacro_non_macro);
    RUN_TEST(isLabelReference_true);
    RUN_TEST(isLabelReference_false);
    RUN_TEST(isLabelReference_empty);
    RUN_TEST(extractLabelName_basic);
    
    // Run parse tests
    printf(YELLOW "\nParse Tests:\n" RESET);
    RUN_TEST(trim_basic);
    RUN_TEST(trim_all_whitespace);
    RUN_TEST(trim_tabs_and_spaces);
    
    // Summary
    printf("\n" YELLOW "=== Test Summary ===" RESET "\n");
    printf(GREEN "Passed: %d\n" RESET, tests_passed);
    if (tests_failed > 0) {
        printf(RED "Failed: %d\n" RESET, tests_failed);
        return 1;
    } else {
        printf(GREEN "All tests passed!\n" RESET);
        return 0;
    }
}
