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

/* __simple_test_entry
 * A node in a linked list of tests to be executed within a context.
 * [test] - the function to call to execute the test
 * [name] - the name of the test
 * [next] - the next test to run after this one, null if there is none
 */

typedef struct __simple_test_entry {
  bool (*test)();
  char *name;
  struct __simple_test_entry *next;
} SimpleTest;

/* __simple_test_context
 * A context is the context within which to run a set of tests.  it is also
 * an entry in the linked list of contexts
 * [name] - the name of the contect
 * [entries] - a linked list defining those entries to be tested
 * [begin] - a function to run before running the tests
 * [end] - a function to run after running the tests
 * [next] - next context in the list
 */

typedef struct __simple_test_context {
  char *name;
  struct __simple_test_entry *entries;
  void (*begin)();
  void (*end)();
  struct __simple_test_context * next;
} SimpleTestContext;

struct __simple_test_context *__simple_test_contexts = NULL;
unsigned int __simple_test_tests_run = 0;
unsigned int __simple_test_asserts_run = 0;
unsigned int __simple_test_asserts_passed = 0;
unsigned int __simple_test_tests_passed = 0;

/* __simple_test_assign_string
 * a stupid simple function for copying a string.  
 * [dest] - location to assign to, *dest will be freed if it is not null
 * [src] - location to copy from, this should be a null terminated string
 */

void __simple_test_assign_string(char **dest, const char *src) {
   if(NULL != *dest) free(*dest);
   *dest = malloc(strlen(src) + 1);
   assert(NULL != *dest);

   strcpy(*dest, src);
}

/* __simple_test_context_init
 * allocate a context, initializing it appropriately and add it to the global
 * contexts  
 * [name] - the name of the context
 * [begin] - the begin function for the context
 * [end] - the teardown function for the context
*/
SimpleTestContext *simple_test_context_init(const char *name,
                                                  void (*begin)(),
                                                  void (*end)()) {

   SimpleTestContext *context = malloc(sizeof(SimpleTestContext));
   assert(NULL != context);
   
   context->name = NULL;
   if(NULL != name) __simple_test_assign_string(&context->name, name);
      
   context->entries = NULL;
   context->begin = begin;
   context->end = end;
   context->next = NULL;
   if (NULL == __simple_test_contexts) __simple_test_contexts = context;
   else {
     SimpleTestContext * dest = __simple_test_contexts;
     while(NULL != dest->next) dest = dest->next;
     dest->next = context;     
   }
   return context;
}

/* simple_test_entry_init
 * initialize a simple test entry, assigning values if they are provided.
 * [name] - name of simple test
 * [test] - function which will execute the test
*/
SimpleTest * simple_test_entry_init(const char *name, 
                                             bool (*test)()) {
                                              
  SimpleTest * entry = malloc(sizeof(SimpleTest));
  assert(NULL != entry);

  entry->next = NULL;
  entry->test = test;
  entry->name = NULL;
  if(NULL != name) __simple_test_assign_string(&entry->name, name);
  
  return entry;
}

/* simple_test_find_context
  find a context and return it if it exists, return null if it does not
  [name] - name of context to find (may be null)
*/
SimpleTestContext * simple_test_find_context(const char *name) {  
  if(NULL == __simple_test_contexts) return NULL;  

  SimpleTestContext * context = __simple_test_contexts; 
  while(NULL != context) {    
    if(NULL == name) {
      if(NULL == context->name) break;
    } 
    else if(strcmp(name, context->name) == 0) break;
    context = context->next;
  }      
  return context;  
}

/* simple_test_add_entry
  [entries] - entries to be added to
  [entry] - entry to add to entries
  */
void simple_test_add_entry(SimpleTest *entries, SimpleTest *entry) {
  assert(NULL != entries);
  assert(NULL != entry);
  SimpleTest *next = entries;
  while(NULL != next->next) next = next->next;
  next->next = entry;
}

/* simple_test_add_to_context
   add a test to a context
   [test] - test to add
   [context] - context to add test to
*/
void simple_test_add_to_context(SimpleTestContext *context, SimpleTest *entry) {
  assert(NULL != entry);
  assert(NULL != context);
  if(NULL == context->entries) context-> entries = entry;
  else simple_test_add_entry(context->entries, entry);
}

/* simple_test_run_test
  Run a simple tests in a linked list
  [context] - name of context
  [test] - test entry (and child entries) to run
*/
void simple_test_run_entry(const char *context, SimpleTest *entry) {   
  if(NULL == entry) return;

  fprintf(stderr, "\tExecuting Test [%s::%s]\n", 
           (NULL==context) ? "" : context,
           (NULL==entry->name) ? "" : entry->name);
   
   bool result = entry->test();
   ++__simple_test_tests_run;
   if(result) {
     ++__simple_test_tests_passed;
     fprintf(stderr, "\t\tPASSED\n");
   }
   else fprintf(stderr, "\t\tFAILED\n");
   simple_test_run_entry(context, entry->next);
} 

/* simple_test_run_context
  Run a simple test context in a linked list
  [context] - context to run (will run its children also)  
*/
void simple_test_run_context(SimpleTestContext *context) {   
  if(NULL == context) return;
   fprintf(stderr, "Executing Test Context [%s]\n", 
           (NULL==context->name) ? "" : context->name);
  if(NULL != context->begin) (*context->begin)();
  simple_test_run_entry(context->name, context->entries);
  if(NULL != context->end) (*context->end)();
  simple_test_run_context(context->next);
} 

/* simple_test_end
  summarize simple test results and reset  
*/
void simple_test_end() {
    fprintf(stderr, "Tests Run: %u\n", __simple_test_tests_run);
    fprintf(stderr, "Tests Passed: %u\n", __simple_test_tests_passed);
    fprintf(stderr, "Asserts Run: %u\n", __simple_test_asserts_run);
    fprintf(stderr, "Aserts Passed: %u\n", __simple_test_asserts_passed);
    __simple_test_tests_run=0;
    __simple_test_asserts_run=0;
    __simple_test_asserts_run=0;
    __simple_test_asserts_passed=0;
}

void simple_test_begin() {
  __simple_test_asserts_run = 0;
  __simple_test_asserts_passed = 0;
  __simple_test_tests_run = 0;
  __simple_test_tests_passed = 0;
}


bool simple_test_assert(const char *sz, bool test) {
    __simple_test_asserts_run++;
    if(test) {
        __simple_test_asserts_passed++;        
    }

    fprintf(stderr, "Assert Failed: [%s]\n", sz);
    return test;
}

#endif //SEARCHFILEC_SIMPLETEST_H
