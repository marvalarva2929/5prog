#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PASS "PASS"
#define FAIL "FAIL"

static int tests_run  = 0;
static int tests_pass = 0;

static char *run_with_input(const char *cmd, const char *input)
{
    FILE *tmp = fopen("/tmp/tk_test_input.txt", "w");
    if (!tmp) return NULL;
    fputs(input, tmp);
    fclose(tmp);

    char full_cmd[512];
    snprintf(full_cmd, sizeof(full_cmd), "%s < /tmp/tk_test_input.txt 2>/dev/null", cmd);

    FILE *fp = popen(full_cmd, "r");
    if (!fp) return NULL;

    char *buf = malloc(4096);
    if (!buf) { pclose(fp); return NULL; }
    buf[0] = '\0';

    size_t total = 0;
    char line[256];
    while (fgets(line, sizeof(line), fp)) {
        size_t len = strlen(line);
        memcpy(buf + total, line, len + 1);
        total += len;
    }
    pclose(fp);

    if (total > 0 && buf[total - 1] == '\n')
        buf[total - 1] = '\0';

    return buf;
}

static void check(const char *name, const char *got, const char *expected)
{
    tests_run++;
    int ok = (got != NULL) && (strcmp(got, expected) == 0);
    if (ok) tests_pass++;
    printf("[%s] %s\n", ok ? PASS : FAIL, name);
    if (!ok) {
        printf("expected: \"%s\"\n", expected);
        printf("got:      \"%s\"\n", got ? got : "(null)");
    }
    free(got);
}

static int assemble(const char *src, const char *out)
{
    char cmd[256];
    snprintf(cmd, sizeof(cmd), "./hw5-asm %s %s 2>/dev/null", src, out);
    int ret = system(cmd);
    if (ret != 0)
        fprintf(stderr, "Assembly of %s failed (exit %d)\n", src, ret);
    return ret;
}

static void test_fibonacci(void)
{
    puts("\n--- fibonacci tests ---");

    if (assemble("fibonacci.tk", "/tmp/fib.tko") != 0) {
        puts("Skipping fibonacci tests: assembly failed.");
        return;
    }

    const char *sim = "./hw5-sim /tmp/fib.tko";

    check("fib(1) == 0",  run_with_input(sim, "1"),  "0");
    check("fib(2) == 1",  run_with_input(sim, "2"),  "1");
    check("fib(3) == 1",  run_with_input(sim, "3"),  "1");
    check("fib(4) == 2",  run_with_input(sim, "4"),  "2");
    check("fib(5) == 3",  run_with_input(sim, "5"),  "3");
    check("fib(6) == 5",  run_with_input(sim, "6"),  "5");
    check("fib(7) == 8", run_with_input(sim, "7"),  "8");
    check("fib(8) == 13", run_with_input(sim, "8"),  "13");
    check("fib(9) == 21", run_with_input(sim, "9"),  "21");
    check("fib(10) == 34",run_with_input(sim, "10"), "34");
}

static char *make_bs_input(int *arr, int n, int target)
{
    char *buf = malloc(4096);
    if (!buf) return NULL;
    int off = 0;
    off += sprintf(buf + off, "%d\n", n);
    for (int i = 0; i < n; i++)
        off += sprintf(buf + off, "%d\n", arr[i]);
    off += sprintf(buf + off, "%d\n", target);
    return buf;
}

static void bs_check(const char *name, int *arr, int n, int target, int expect_found)
{
    char *input = make_bs_input(arr, n, target);
    char *got   = run_with_input("./hw5-sim /tmp/bsearch.tko", input);
    free(input);
    check(name, got, expect_found ? "found" : "not found");
}

static void test_binary_search(void)
{
    puts("\n--- binary search tests ---");

    if (assemble("binary_search.tk", "/tmp/bsearch.tko") != 0) {
        puts("Skipping binary search tests: assembly failed.");
        return;
    }

    int a1[] = {42};
    bs_check("single element, found",      a1, 1, 42, 1);

    int a2[] = {42};
    bs_check("single element, not found",  a2, 1, 99, 0);

    int a3[] = {1, 3, 5, 7, 9};
    bs_check("target at beginning",        a3, 5, 1, 1);

    bs_check("target at end",              a3, 5, 9, 1);

    bs_check("target in middle",           a3, 5, 5, 1);

    bs_check("target less than all",       a3, 5, 0, 0);

    bs_check("target greater than all",    a3, 5, 10, 0);

    bs_check("target between elements",    a3, 5, 4, 0);

    int a4[] = {2, 4, 6, 8};
    bs_check("even array, found (left)",   a4, 4, 2, 1);
    bs_check("even array, found (right)",  a4, 4, 8, 1);
    bs_check("even array, not found",      a4, 4, 5, 0);

    int a5[] = {10, 20, 30, 40, 50, 60, 70, 80, 90, 100};
    bs_check("large array, found (mid)",   a5, 10, 50, 1);
    bs_check("large array, found (first)", a5, 10, 10, 1);
    bs_check("large array, found (last)",  a5, 10, 100, 1);
    bs_check("large array, not found",     a5, 10, 55, 0);

    int a6[] = {7, 7, 7, 7, 7};
    bs_check("all same, search present",   a6, 5, 7, 1);
    bs_check("all same, search absent",    a6, 5, 3, 0);
}

int main(void)
{
    puts("hw5 test");

    test_fibonacci();
    test_binary_search();

    printf("\nResults: %d / %d passed\n", tests_pass, tests_run);
    return (tests_pass == tests_run) ? 0 : 1;
}
