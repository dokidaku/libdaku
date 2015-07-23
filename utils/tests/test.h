#ifndef __DAKU_TEST_H__
#define __DAKU_TEST_H__

#include <stdio.h>
#include <stdlib.h>

/**
 * Checks for one condition. Similar to assert() but modified to do tests.
 * One test can consist of multiple test cases. If description is NULL, the test continues.
 * If one test fails, the entire program will be terminated with the exit value of the number of that test.
 *
 * @param[in] bool_value  The test will be marked as failed if this is zero.
 * @param[in] description If not NULL, current test will be ended
 *                        and it'll be the description text for the test.
 */
void daku_test(unsigned char bool_value, const char *description)
{
    static int test_num = 1, test_case_num = 1;
    if (description) {
        printf("Test #%d: %s\n", test_num++, description);
        test_case_num = 1;
    } else {
        printf("Test #%d, case #%d\n", test_num, test_case_num++);
    }
    if (!bool_value) {
        printf("FAILED ON TEST #%d CASE #%d (/_<)\n", test_num, test_case_num);
        exit(test_num);
    }
}

void daku_test_finish()
{
    puts("ACCEPTED (^o^)/*");
}

/**
 * Used in test calls to mark the end of a test.
 * @see daku_test
 */
#define DAKU_TEST_END 1

#endif
