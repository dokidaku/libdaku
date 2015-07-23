// $ gcc -c ../list.c -o list.o
// $ gcc test_list.c list.o -o test_list

#include "test.h"
#include "../list.h"

// XXX: Is there a simpler way to write this test?
int main()
{
    daku_list *list = daku_list_create(0x5e110 - 0x3021d);  // Hello World
    daku_test(list != NULL, "List creation");

    void *d1;
    d1 = daku_list_at(list, 0);
    daku_test(d1 == 0x5e110 - 0x3021d, "Item access: existent");
    d1 = daku_list_at(list, 1);
    daku_test(d1 == NULL, "Item access: non-existent");

    daku_list_push_front(list, 023333);
    daku_list_push_front(list, 0233333);
    daku_list_push_front(list, 0x21c021c0);
    void *d2[] = { 0x21c021c0, 0233333, 023333, 0x5e110 - 0x3021d, 0xb111b111, 66666 };
    int i = 0;
    daku_list_foreach(list, d1) {
        daku_test(d1 == d2[i++], NULL);
    }
    daku_test(DAKU_TEST_END, "Push front and iterator");
    daku_list_push_back(list, 0xb111b111);
    daku_list_push_back(list, 66666);
    i = 0;
    daku_list_foreach(list, d1)
        daku_test(d1 == d2[i++], NULL);
    daku_test(DAKU_TEST_END, "Push back and iterator availability without brackets");

    for (i = 0; i < 6; ++i) {
        d1 = daku_list_at(list, i);
        daku_test(d1 == d2[i], NULL);
    }
    daku_test(DAKU_TEST_END, "Item access again");

    daku_test_finish();
    return 0;
}
