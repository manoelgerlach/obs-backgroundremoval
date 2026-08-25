/*
 * SPDX-FileCopyrightText: 2021-2026 Roy Shilkrot <roy.shil@gmail.com>
 * SPDX-FileCopyrightText: 2023-2026 Kaito Udagawa <umireon@kaito.tokyo>
 * SPDX-FileCopyrightText: 2026 Manoel Gerlach <mail@manoel.us>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "plugin-support.h"

#ifndef PLUGIN_VERSION_STR
#error PLUGIN_VERSION_STR must be defined by the build system
#endif

extern void blogva(int log_level, const char *format, va_list args);

const char *PLUGIN_NAME = "obs-backgroundremoval";
const char *PLUGIN_VERSION = PLUGIN_VERSION_STR;

void obs_log(int log_level, const char *format, ...)
{
	size_t length = 4 + strlen(PLUGIN_NAME) + strlen(format);

	char *template = malloc(length + 1);

	snprintf(template, length, "[%s] %s", PLUGIN_NAME, format);

	va_list(args);

	va_start(args, format);
	blogva(log_level, template, args);
	va_end(args);

	free(template);
}
