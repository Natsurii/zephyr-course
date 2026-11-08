# Lesson 06-05 — Extended Sensor Driver

**Module:** 06 — Zephyr Driver Development
**Type:** Assignment · **Tag:** `l6-task2`

## Before you review this

You are answering one question: **does the extension API reach the driver through
`dev->api` and change mutable state, without disturbing the standard sensor
slots?**

The Sensor API models reads. There is no way to *write* a sensor, so a driver that
needs a verb must publish its own. The principle that makes this safe is the
subtyping trick: the driver's API struct starts with the standard sensor API
struct as its first member. In C a pointer to a struct is a pointer to its first
member, so a `struct our_driver_api *` is simultaneously a valid
`struct sensor_driver_api *`. Both APIs share one object, and no standard slot
moves.

The other principle is state discipline: touch the pin first, mutate the data
second, so a failed write cannot leave the struct claiming something untrue.

## The rules, and why each one exists

**Rule 1 — The standard struct is the first member of the extended struct.**
*Failure it prevents:* the two views disagreeing, which breaks the standard API
the moment an extension field is added.

**Rule 2 — Exactly one new hook is added, with a matching public wrapper.**
*Failure it prevents:* callers touching `dev->api` themselves, which leaks the
layout into applications.

**Rule 3 — The hook mutates the dynamic data struct.** *Failure it prevents:* an
extension that only logs, proving nothing about data ownership.

**Rule 4 — The application calls it from `main.c`.** *Failure it prevents:* a
driver capability nothing exercises.

**Rule 5 — Hardware is driven before state is updated.** *Failure it prevents:*
`data` recording a level the pin never reached after a failed write.

## Reading the tag

Rule 1 holds by the struct's own shape:

```c
struct our_driver_api {
        struct sensor_driver_api sensor;   /* must be first */
        int (*toggle)(const struct device *dev);
};
```

`toggle` is appended after `sensor`, never before it, so the embedded view stays
at offset zero (Rule 2 holds too).

Rule 2's wrapper keeps callers out of the layout:

```c
int our_driver_toggle(const struct device *dev)
{
        const struct our_driver_api *api = (const struct our_driver_api *)dev->api;

        return api->toggle(dev);
}
```

The cast is legitimate because `api` points at a `our_driver_api`; a sensor driver
only holds `const struct device *`, and `api` never changes after init, so
discarding the const for the call is safe.

Rule 3 holds in the implementation, which changes `data->sampled` and increments
`data->toggle_count` — both mutable, neither in the `const` config. `main.c`
calls the wrapper in its loop (Rule 4), pairing it with the toggle-count channel.

Rule 5 is the ordering, and it is the point of the function:

```c
ret = our_driver_internal_write(dev, next != 0U);
if (ret < 0) {
        LOG_ERR("write failed (%d), state unchanged", ret);
        return ret;
}
data->sampled = next;
data->toggle_count++;
```

The early return sits between write and mutation, so the struct cannot diverge
from the pin.

## Running the check yourself

```bash
git worktree add /tmp/wt l6-task2 && cd /tmp/wt
west build -p always -b blackpill_f401ce app -DBOARD_ROOT=/tmp/wt/app
```

Then confirm the extension resolves through the table rather than straight from C:

```bash
arm-none-eabi-nm /tmp/b/zephyr/zephyr.elf | grep -i toggle
```

A pass shows `our_driver_toggle` and `our_driver_toggle_impl` both linked. To check
Rule 1 concretely, build with a deliberately reordered struct (move `toggle` above
`sensor`) and watch the standard sensor calls misfire.

## Verdict

All five rules hold. The extension rides `dev->api`, mutates data, is exercised
from `main.c`, and keeps state consistent on failure.

## If you want to go further

`dev->api` is a `const void *`. Drivers that need to write through it often keep a
non-const copy obtained at init instead of casting at every call; the cast here is
the smaller version for a read-only table. Also note that `toggle` is not a
replacement for `sample_fetch` — the app still fetches, because the standard API
remains the read path.