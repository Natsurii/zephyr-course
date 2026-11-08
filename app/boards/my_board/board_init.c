/*
 * my_board board initialisation.
 *
 * Prints a banner so it is visible that the board brought itself up before the
 * application's main() runs.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdio.h>

#include <zephyr/init.h>
#include <zephyr/kernel.h>

static int my_board_init(void)
{
	printf("Board Initialized\n");

	return 0;
}

/*
 * PRE_KERNEL_1 runs long before main(); the banner is therefore printed before
 * the application entry point and before kernel init finishes.
 */
SYS_INIT(my_board_init, PRE_KERNEL_1, 0);