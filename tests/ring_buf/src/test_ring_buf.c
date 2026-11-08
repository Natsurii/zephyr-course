/*
 * Ring Buffer Module - Homework Test Skeleton
 *
 * test_fresh_state is provided as a worked example. Fill in the remaining
 * 7 ZTEST bodies according to TEST_SPEC.md. Stubs call ztest_test_skip()
 * so the binary builds and runs cleanly before each test is implemented.
 *
 * Run:
 *   west twister -T tests/ring_buf -p native_sim
 */

#include <zephyr/ztest.h>
#include <errno.h>

#include "ring_buf.h"

/*
 * Shared before hook: every suite reinitialises the ring buffer with a
 * capacity of 4 so tests start from a clean, known state. Capacity 4 is
 * enough to exercise FIFO order (push 1, 2, 3) and overflow (full at 4).
 */
static void before(void *f)
{
	ARG_UNUSED(f);
	rb_init(4);
}

/*
 * ============================================================================
 * Test Suite: ring_buf_init
 *
 * Initial state and re-initialization behaviour.
 * ============================================================================
 */
ZTEST_SUITE(ring_buf_init, NULL, NULL, before, NULL, NULL);

/* PROVIDED — study this test before writing the rest. */
ZTEST(ring_buf_init, test_fresh_state)
{
	zassert_true(rb_is_empty(), "Fresh buffer must be empty");
	zassert_equal(rb_count(), 0, "Fresh buffer count must be 0");
}

ZTEST(ring_buf_init, test_reinit_clears_state)
{
	/* Push something so the buffer is provably not in its initial state. */
	zassert_equal(rb_push(99), 0, "push(99) must succeed");
	zassert_equal(rb_count(), 1, "count must be 1 after the push");

	/* A second init must discard the stored element entirely. */
	zassert_equal(rb_init(4), 0, "re-init with capacity 4 must succeed");

	zassert_true(rb_is_empty(), "Re-initialised buffer must be empty");
	zassert_equal(rb_count(), 0, "Re-initialised buffer count must be 0");
}

/*
 * ============================================================================
 * Test Suite: ring_buf_push_pop
 *
 * Single push/pop round-trip, FIFO order, full error path.
 * ============================================================================
 */
ZTEST_SUITE(ring_buf_push_pop, NULL, NULL, before, NULL, NULL);

ZTEST(ring_buf_push_pop, test_single_push_pop)
{
	int v = 0;

	zassert_equal(rb_push(42), 0, "push(42) must succeed");
	zassert_equal(rb_pop(&v), 0, "pop must succeed when one element is held");
	zassert_equal(v, 42, "popped value must equal the pushed value");

	zassert_true(rb_is_empty(), "buffer must be empty after the only pop");
}

ZTEST(ring_buf_push_pop, test_fifo_order)
{
	int v = 0;

	zassert_equal(rb_push(1), 0, "push(1) must succeed");
	zassert_equal(rb_push(2), 0, "push(2) must succeed");
	zassert_equal(rb_push(3), 0, "push(3) must succeed");

	/* Oldest first: the reads must come back in push order. */
	zassert_equal(rb_pop(&v), 0, "first pop must succeed");
	zassert_equal(v, 1, "first pop must return 1");

	zassert_equal(rb_pop(&v), 0, "second pop must succeed");
	zassert_equal(v, 2, "second pop must return 2");

	zassert_equal(rb_pop(&v), 0, "third pop must succeed");
	zassert_equal(v, 3, "third pop must return 3");

	zassert_true(rb_is_empty(), "buffer must be empty after popping all elements");
}

ZTEST(ring_buf_push_pop, test_push_full_returns_enospc)
{
	/* Fill the buffer to its capacity of 4. */
	zassert_equal(rb_push(1), 0, "push(1) must succeed");
	zassert_equal(rb_push(2), 0, "push(2) must succeed");
	zassert_equal(rb_push(3), 0, "push(3) must succeed");
	zassert_equal(rb_push(4), 0, "push(4) must succeed");
	zassert_true(rb_is_full(), "buffer must be full after capacity pushes");

	/* The overflow push must be rejected, not silently overwrite. */
	zassert_equal(rb_push(99), -ENOSPC, "push on a full buffer must return -ENOSPC");
	zassert_equal(rb_count(), 4, "rejected push must not consume a slot");
}

/*
 * ============================================================================
 * Test Suite: ring_buf_boundaries
 *
 * Peek semantics and NULL-pointer boundary conditions.
 * ============================================================================
 */
ZTEST_SUITE(ring_buf_boundaries, NULL, NULL, before, NULL, NULL);

ZTEST(ring_buf_boundaries, test_peek_does_not_consume)
{
	int v = 0;

	zassert_equal(rb_push(7), 0, "push(7) must succeed");

	/* Peeking twice must report the same value both times. */
	zassert_equal(rb_peek(&v), 0, "first peek must succeed");
	zassert_equal(v, 7, "first peek must report 7");

	zassert_equal(rb_peek(&v), 0, "second peek must succeed");
	zassert_equal(v, 7, "second peek must still report 7");

	/* The element survives both peeks. */
	zassert_equal(rb_count(), 1, "peek must not consume the element");
}

ZTEST(ring_buf_boundaries, test_pop_null_returns_einval)
{
	/* A NULL output pointer is rejected before the emptiness check. */
	zassert_equal(rb_pop(NULL), -EINVAL, "pop(NULL) must return -EINVAL");
}

ZTEST(ring_buf_boundaries, test_is_full_after_fill)
{
	zassert_equal(rb_push(1), 0, "push(1) must succeed");
	zassert_equal(rb_push(2), 0, "push(2) must succeed");
	zassert_equal(rb_push(3), 0, "push(3) must succeed");
	zassert_equal(rb_push(4), 0, "push(4) must succeed");

	zassert_true(rb_is_full(), "buffer must report full at capacity 4");
	zassert_equal(rb_count(), 4, "count must equal capacity when full");
}
