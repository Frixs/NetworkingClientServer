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
 /// \return         Pointer to the location the memory.
void *memory_malloc(size_t size) {
    if (!size)
        return NULL;

    void *m = NULL;

    m_current_allocation_count++;

    m = malloc(size);
    while (m == NULL) {
        sleep(1);
        m = malloc(size);
    }

    return m;
}

 /// Custom free function for better debug memory allocation.
 /// \param ptr      Free the pointer memory.
void memory_free(void *ptr) {
    if (!ptr)
        return;

    m_current_allocation_count--;
    free(ptr);
}

/// Print status of the memory.
void memory_print_status() {
    printf("==============================\nMemory allocation: %li times.\n==============================\n", m_current_allocation_count);
}