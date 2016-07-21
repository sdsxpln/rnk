/*
 * Copyright (C) 2014  Raphaël Poggi <poggi.raph@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef ARCH_SVC_H
#define ARCH_SVC_H

#ifdef CONFIG_CPU_ARMV7M
#include <armv7m/system.h>
#include <armv7m/svc.h>
#elif defined(CONFIG_CPU_ARMV8A)
#include <armv8a/system.h>
#include <armv8a/svc.h>
#endif

extern void arch_system_call(unsigned int call, void *arg1, void *arg2, void *arg3);

#endif /* ARCH_SVC_H */
