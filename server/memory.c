//
// Created by frixs on 10/14/18.
//

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "memory.h"

// Global variables.
long m_current_allocation_count = 0;

 /// Custom malloc function for better debug memory allocation.
 /// \param size     Allocate memory of that size.
 /// \param c        Constant for debugging. Default = 0;
 /// \return         Pointer to the location the memory.
void *memory_malloc(size_t size, int c) {
    if (!size)
        return NULL;

    void *m = NULL;

    m = malloc(size);
    while (m == NULL) {
        sleep(1);
        m = malloc(size);
    }

    m_current_allocation_count++;

    if (c > 0)
        printf("::: %d\n", c);

    return m;
}

 /// Custom free function for better debug memory allocation.
 /// \param ptr      Free the pointer memory.
 /// \param c        Constant for debugging. Default = 0;
void memory_free(void *ptr, int c) {
    if (!ptr)
        return;

    m_current_allocation_count--;
    free(ptr);

    if (c > 0)
        printf("::: %d\n", c);
}

/// Print status of the memory.
void memory_print_status() {
    printf("==============================\nMemory allocation: %li times.\n==============================\n", m_current_allocation_count);
}