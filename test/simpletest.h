//
// Created by Joseph Hurdle on 7/24/20.
//

#ifndef SEARCHFILEC_SIMPLETEST_H
#define SEARCHFILEC_SIMPLETEST_H

#include <stdbool.h>
#include <stdio.h>

/* file: minunit.h */

extern int tests_run;
extern int tests_passed;

void simple_test_begin() {
    tests_run = 0;
    tests_passed = 0;
}

void simple_test_end() {
    fprintf(stderr, "Tests Run: %d\n", tests_run);
    fprintf(stderr, "Tests Passed: %d\n", tests_passed);
}

int all_tests_passed() {
    return tests_run != tests_passed;
}

void simple_test_assert(const char *sz, bool test) {
    tests_run++;
    if(test) {
        ++tests_passed;
        return;
    }

    fprintf(stderr, "Test Failed: [%s]\n", sz);
}

#endif //SEARCHFILEC_SIMPLETEST_H
