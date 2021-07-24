//
// Created by Joseph Hurdle on 7/24/20.
//

#ifndef SEARCHFILEC_SIMPLETEST_H
#define SEARCHFILEC_SIMPLETEST_H

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

typedef enum { NOTRUN, PASSED, FAILED } __simple_test_status_t;

typedef struct {
  bool *(function)();
  char *name;
  void *next;
} __simple_test_entry;

typedef struct {
  char *name;
  SimpleTestEntry *entries;
  void *(setupFunction)();
  void *(tearDownFunction)();
} __simple_test_context;

__simple_test_context *__simple_test_contexts = NULL;
int __simple_test_tests_run = 0;
int __simple_test_asserts_run = 0;
int __simple_test_tests_passed = 0;

void __simple_test_assign_string(char **dest, const char *src) {
   *dest = malloc(strlen(src) + 1);
   assert(NULL != *dest);

   strcpy(*dest, src);
}

__simple_test_context *__simple_test_context_alloc(const char *contextName) {
   __simple_test_context *context = malloc(sizeof(__simple_test_context));
   assert(NULL != context);
   
   if(NULL == contextName) { 
      __simple_test_assign_string(&context->name, "default");
   } else {
      __simple_test_assign_string(&context->name, contextName);
   }
   
   context->testEntries = NULL;
   context->setupFunction = NULL;
   context->tearDownFunction = NULL;
   return context;
}

__simple_test_context * simple_test_get_context(const char *contextName) {
  __simple_test_context * context;

  if(NULL == simpleTestContexts) {
     simpleTestContexts = simple_test_context_alloc(NULL);
  }
}

SimpleTestEntry * simple_test_entry_alloc(const char *contextName, 
                                          const char *testName, 
                                          void *testFunction()) {

  __simple_test_context* context = simple_test_get_context(contextName);
 
  SimpleTestEntry * ret = malloc(sizeof(SimpleTestEntry));
  assert(NULL != ret);

  ret->testName = malloc(strlen(testName) + 1);
  assert(NULL != ret->testName);

  strcpy(ret->testName, testName);
  ret->functionPointer = testFunction;
  ret->next = NULL;
  return ret;
}

void simple_test_add(const char *testName, void *testFunction()) {
 
  SimpleTestEntry *entry = simple_test_entry_alloc(testName, testFunction);
 
  if(NULL == simpleTestRegistry) simpleTestRegistry = entry; 
  else {
      SimpleTestEntry *end;
      for(end = simpleTestRegistry; NULL != end->next; end = end->next);
      end->next = entry;
  }
}

void simple_test_run_entry(SimpleTestEntry *entry) {
   fprintf(stderr, "Executing Test: [%s]\n", entry->testName);
   (*entry->functionPointer)();
} 

void simple_test_run_tests()  {
  simple_test_begin();
  for(SimpleTestEntry* entry=simpleTestRegistry; NULL!=entry; entry=entry->next) simple_test_run_entry(entry);  
}

void simple_test_begin() {
    testsRun = 0;
    testsPassed = 0;
}

void simple_test_end() {
    fprintf(stderr, "Tests Run: %d\n", testsRun);
    fprintf(stderr, "Tests Passed: %d\n", testsPassed);
}

int simple_test_all_tests_passed() {
    return testsRun != testsPassed;
}

void simple_test_assert(const char *sz, bool test) {
    testsRun++;
    if(test) {
        ++testsPassed;
        return;
    }

    fprintf(stderr, "Test Failed: [%s]\n", sz);
}

#endif //SEARCHFILEC_SIMPLETEST_H
