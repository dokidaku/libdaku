#ifndef __DAKU_TEST_H__
#define __DAKU_TEST_H__

#include <stdlib.h>

void daku_test(unsigned char bool_value, const char *description)
{
    static int test_case_num = 1;
    if (description)
        printf("Test #%d: %s\n", test_case_num++, description);
    if (!bool_value) {
        printf("FAILED ON TEST #%d (/_<)\n", test_case_num);
        exit(test_case_num);
    }
}

void daku_test_finish()
{
    puts("ACCEPTED (^o^)/*");
}

#define DAKU_TEST_END 1

#endif
