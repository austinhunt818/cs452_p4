#include "harness/unity.h"
#include "../src/lab.h"
#include <pthread.h>
#include <unistd.h>

// NOTE: Due to the multi-threaded nature of this project. Unit testing for this
// project is limited. I have provided you with a command line tester in
// the file app/main.cp. Be aware that the examples below do not test the
// multi-threaded nature of the queue. You will need to use the command line
// tester to test the multi-threaded nature of your queue. Passing these tests
// does not mean your queue is correct. It just means that it can add and remove
// elements from the queue below the blocking threshold.


void setUp(void) {
  // set stuff up here
}

void tearDown(void) {
  // clean stuff up here
}




void test_create_destroy(void)
{
    queue_t q = queue_init(10);
    TEST_ASSERT_TRUE(q != NULL);
    queue_destroy(q);
}

void test_queue_dequeue(void)
{
    queue_t q = queue_init(10);
    TEST_ASSERT_TRUE(q != NULL);
    int data = 1;
    enqueue(q, &data);
    TEST_ASSERT_TRUE(dequeue(q) == &data);
    queue_destroy(q);
}

void test_queue_dequeue_multiple(void)
{
    queue_t q = queue_init(10);
    TEST_ASSERT_TRUE(q != NULL);
    int data = 1;
    int data2 = 2;
    int data3 = 3;
    enqueue(q, &data);
    enqueue(q, &data2);
    enqueue(q, &data3);
    TEST_ASSERT_TRUE(dequeue(q) == &data);
    TEST_ASSERT_TRUE(dequeue(q) == &data2);
    TEST_ASSERT_TRUE(dequeue(q) == &data3);
    queue_destroy(q);
}

void test_queue_dequeue_shutdown(void)
{
    queue_t q = queue_init(10);
    TEST_ASSERT_TRUE(q != NULL);
    int data = 1;
    int data2 = 2;
    int data3 = 3;
    enqueue(q, &data);
    enqueue(q, &data2);
    enqueue(q, &data3);
    TEST_ASSERT_TRUE(dequeue(q) == &data);
    TEST_ASSERT_TRUE(dequeue(q) == &data2);
    queue_shutdown(q);
    TEST_ASSERT_TRUE(dequeue(q) == &data3);
    TEST_ASSERT_TRUE(is_shutdown(q));
    TEST_ASSERT_TRUE(is_empty(q));
    queue_destroy(q);
}

void test_queue_enqueue_max_capacity(void)
{
  int capacity = 5;
  queue_t q = queue_init(capacity);
  TEST_ASSERT_TRUE(q != NULL);
  
  // Fill the queue to capacity
  int data[capacity + 1]; // +1 for the attempted overflow
  for (int i = 0; i < capacity; i++) {
    data[i] = i + 1;
    enqueue(q, &data[i]);
  }
  
  // Attempt to enqueue one more item in a separate thread (should block or fail)
  data[capacity] = capacity + 1;
  enqueue(q, &data[capacity]); // This should block or fail

  // NONE OF THE FOLLOWING CODE SHOULD EXECUTE

  queue_shutdown(q); // Signal shutdown to unblock any waiting threads
  
  // This is a bit tricky to test in a single-threaded context, but we can
  // check if the queue is full and that the last item was not added
  TEST_ASSERT_TRUE(is_empty(q) == false);
  TEST_ASSERT_TRUE(is_shutdown(q) == true);

  
  // Verify we can still dequeue all items that were successfully added
  for (int i = 0; i < capacity; i++) {
    void* dequeued = dequeue(q);
    TEST_ASSERT_TRUE(dequeued == &data[i]);
  }
  
  // Queue should now be empty
  TEST_ASSERT_TRUE(is_empty(q));
  
  queue_destroy(q);
}

void test_dequeue_empty_queue(void) {
  queue_t q = queue_init(10);
  TEST_ASSERT_TRUE(q != NULL);
  
  // Queue is initially empty, so dequeue should block or return NULL
  // In most implementations, this would block indefinitely without shutdown
  
  // First ensure the queue is actually empty
  TEST_ASSERT_TRUE(is_empty(q));
  
  // Shutdown the queue to allow dequeue to return rather than block
  queue_shutdown(q);
  
  // After shutdown, dequeue on an empty queue should return NULL
  TEST_ASSERT_NULL(dequeue(q));
  
  // Queue should still be empty and in shutdown state
  TEST_ASSERT_TRUE(is_empty(q));
  TEST_ASSERT_TRUE(is_shutdown(q));
  
  queue_destroy(q);
}

void test_enqueue_after_shutdown(void) {
  queue_t q = queue_init(10);
  TEST_ASSERT_TRUE(q != NULL);
  
  int data = 42;
  
  // Shutdown the queue
  queue_shutdown(q);
  TEST_ASSERT_TRUE(is_shutdown(q));
  
  // Attempt to enqueue after shutdown - this should fail
  // (typically by returning false or a non-zero value)
  // For this test, we'll assume enqueue returns 0 on failure
  enqueue(q, &data);
  
  // Queue should still be empty
  TEST_ASSERT_TRUE(is_empty(q));
  
  queue_destroy(q);
}

void test_queue_size(void) {
  queue_t q = queue_init(10);
  TEST_ASSERT_TRUE(q != NULL);
  
  // Test initial size
  TEST_ASSERT_TRUE(is_empty(q));
  
  // Add some items and check size
  int data1 = 1, data2 = 2, data3 = 3;
  enqueue(q, &data1);
  enqueue(q, &data2);
  enqueue(q, &data3);
  
  TEST_ASSERT_TRUE(!is_empty(q));
  
  // Remove items and check size
  dequeue(q);
  dequeue(q);
  dequeue(q);
  
  // Should be empty again
  TEST_ASSERT_TRUE(is_empty(q));
  
  queue_destroy(q);
}



int main(void) {
  UNITY_BEGIN();
  RUN_TEST(test_create_destroy);
  RUN_TEST(test_queue_dequeue);
  RUN_TEST(test_queue_dequeue_multiple);
  RUN_TEST(test_queue_dequeue_shutdown);

  // LLM generated tests
  // Not thread safe
  RUN_TEST(test_dequeue_empty_queue);
  RUN_TEST(test_enqueue_after_shutdown);
  RUN_TEST(test_queue_size);

  // The test should block the process if successful
  // RUN_TEST(test_queue_enqueue_max_capacity);

  return UNITY_END();
}
