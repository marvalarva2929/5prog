/*
 * Test harness for TinkerSim
 * Compiles assembly files with ./hw3 and runs with ./hw4
 * Validates stdout output
 * Compile with: gcc -o test_harness test_harness.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>

typedef struct {
    int num;
    const char *expected_output;
    const char *stdin_input;
} TestCase;

// Array of all test cases with expected outputs
TestCase tests[] = {
    {1, "7\n", NULL},
    {2, "12\n", NULL},
    {3, "10\n", NULL},
    {4, "18446744073709551615\n", NULL},
    {5, "8\n", NULL},
    {6, "8\n", NULL},
    {7, "6\n", NULL},
    {8, "7\n", NULL},
    {9, "20\n", NULL},
    {10, "5\n", NULL},
    {11, "12\n", NULL},
    {12, "3\n", NULL},
    {13, "16\n", NULL},
    {14, "2\n", NULL},
    {15, "7\n", NULL},
    {16, "0\n", NULL},
    {17, "0\n", NULL},
    {18, "0\n", NULL},
    {19, "15\n", NULL},
    {20, "0\n", NULL},
    {21, "8190\n", NULL},
    {22, "4\n", NULL},
    {23, "3\n", NULL},
    {24, "7\n", NULL},
    {25, "1\n2\n3\n", NULL},
};

int num_tests = sizeof(tests) / sizeof(TestCase);

typedef struct {
    int passed;
    int failed;
    int total;
} TestStats;

int compile_test(int test_num) {
    char asm_file[64];
    char intermediate[64];
    char binary[64];
    int status;
    
    snprintf(asm_file, sizeof(asm_file), "test%d.s", test_num);
    snprintf(intermediate, sizeof(intermediate), "test%d.int", test_num);
    snprintf(binary, sizeof(binary), "test%d.tko", test_num);
    
    pid_t pid = fork();
    if (pid == -1) {
        perror("fork");
        return 0;
    }
    
    if (pid == 0) {
        // Redirect stdout and stderr to /dev/null
        int devnull = open("/dev/null", O_WRONLY);
        dup2(devnull, STDOUT_FILENO);
        dup2(devnull, STDERR_FILENO);
        close(devnull);
        
        execlp("./hw3", "./hw3", asm_file, intermediate, binary, NULL);
        perror("execlp hw3");
        exit(1);
    } else {
        waitpid(pid, &status, 0);
        
        if (!WIFEXITED(status)) {
            return 0;
        }
        
        return WEXITSTATUS(status) == 0 ? 1 : 0;
    }
}

int run_test(const TestCase *test) {
    char binary_file[64];
    char output_buf[4096];
    int output_len = 0;
    FILE *fp;
    memset(output_buf, 0, sizeof(output_buf));
    
    snprintf(binary_file, sizeof(binary_file), "test%d.tko", test->num);
    
    int pipefd[2];
    if (pipe(pipefd) == -1) {
        perror("pipe");
        return 0;
    }
    
    pid_t pid = fork();
    if (pid == -1) {
        perror("fork");
        return 0;
    }
    
    if (pid == 0) {
        close(pipefd[0]);
        dup2(pipefd[1], STDOUT_FILENO);
        close(pipefd[1]);
        
        if (test->stdin_input != NULL) {
            int stdin_pipe[2];
            pipe(stdin_pipe);
            dup2(stdin_pipe[0], STDIN_FILENO);
            write(stdin_pipe[1], test->stdin_input, strlen(test->stdin_input));
            close(stdin_pipe[1]);
        }
        
        execlp("./hw4", "./hw4", binary_file, NULL);
        perror("execlp hw4");
        exit(1);
    } else {
        close(pipefd[1]);
        
        fp = fdopen(pipefd[0], "r");
        while (fgets(&output_buf[output_len], sizeof(output_buf) - output_len, fp)) {
            output_len = strlen(output_buf);
        }
        fclose(fp);
        
        int status;
        waitpid(pid, &status, 0);
        
        if (!WIFEXITED(status)) {
            printf("    FAILED: Simulator crashed\n");
            return 0;
        }
        
        if (WEXITSTATUS(status) != 0) {
            printf("    FAILED: Exit code %d\n", WEXITSTATUS(status));
            return 0;
        }
        
        if (strcmp(output_buf, test->expected_output) == 0) {
            printf("    PASSED\n");
            return 1;
        } else {
            printf("    FAILED\n");
            printf("    Expected: ");
            for (int i = 0; i < strlen(test->expected_output); i++) {
                if (test->expected_output[i] == '\n') {
                    printf("\\n");
                } else {
                    printf("%c", test->expected_output[i]);
                }
            }
            printf("\n");
            printf("    Got:      ");
            for (int i = 0; i < strlen(output_buf); i++) {
                if (output_buf[i] == '\n') {
                    printf("\\n");
                } else {
                    printf("%c", output_buf[i]);
                }
            }
            printf("\n");
            return 0;
        }
    }
}

int main(int argc, char *argv[]) {
    TestStats stats = {0, 0, 0};
    
    printf("========================================\n");
    printf("TinkerSim Test Harness\n");
    printf("========================================\n\n");
    
    if (access("./hw3", X_OK) != 0) {
        printf("ERROR: ./hw3 binary not found or not executable\n");
        return 1;
    }
    
    if (access("./hw4", X_OK) != 0) {
        printf("ERROR: ./hw4 binary not found or not executable\n");
        return 1;
    }
    
    printf("Running %d test cases...\n\n", num_tests);
    
    for (int i = 0; i < num_tests; i++) {
        int test_num = tests[i].num;
        char asm_file[64];
        snprintf(asm_file, sizeof(asm_file), "test%d.s", test_num);
        
        if (access(asm_file, F_OK) != 0) {
            continue;
        }
        
        printf("[%2d/%2d] Test %d - Compiling...\n", i + 1, num_tests, test_num);
        
        if (!compile_test(test_num)) {
            printf("    FAILED: Compilation failed\n");
            stats.failed++;
            stats.total++;
            continue;
        }
        
        printf("         Running...\n");
        
        if (run_test(&tests[i])) {
            stats.passed++;
        } else {
            stats.failed++;
        }
        stats.total++;
    }
    
    printf("\n========================================\n");
    printf("Test Results\n");
    printf("========================================\n");
    printf("Passed: %d/%d\n", stats.passed, stats.total);
    printf("Failed: %d/%d\n", stats.failed, stats.total);
    
    if (stats.failed == 0 && stats.total > 0) {
        printf("\nAll tests passed!\n");
        return 0;
    } else {
        printf("\nSome tests failed\n");
        return 1;
    }
}
