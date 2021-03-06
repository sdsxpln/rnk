/*
 * Copyright (C) 2015  Raphaël Poggi <poggi.raph@gmail.com>
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

#define EXC_RETURN_PROCESS_STACK (1 << 2)

#define SW_CTX_FRAME_SIZE	(4 * 10)
#define HW_CTX_FRAME_SIZE	(4 * 8)


.syntax unified
.thumb

.thumb_func
.global interrupt_handler
interrupt_handler:
	cpsid i

	tst lr, #EXC_RETURN_PROCESS_STACK
	ittee eq
	mrseq r1, msp
	moveq r2, #0
	mrsne r1, psp
	movne r2, #1

	stmdb r1!, {r4-r11,r14}

	cmp r2, #0
	beq 1f

	msr psp, r1
1:
	ldr r0, =current_ctx_frame
	ldr r3, [r0]
	push {r1, r3}
	bl irq_action
	pop {r1, r3}

	ldr r0, =current_ctx_frame
	ldr r2, [r0]
	cmp r2, r3
	beq 2f

	ldr r1, [r2]
	add r1, r1, #4

2:
	ldmia	r1!, {r4-r11,r14}

	mrs r2, control
	tst r14, #EXC_RETURN_PROCESS_STACK /* nonzero if context on process stack */
	beq 3f

	orr r2, r2, #1
	msr psp, r1
	b 4f
3:
	bic r2, r2, #1
	msr msp, r1
4:
	msr control, r2

	cpsie i

	bx lr

.thumb_func
.global __svc
__svc:
	mrs r0, psp
	add r0, r0, #SW_CTX_FRAME_SIZE
	sub r0, r0, #4

	push {lr}
	bl svc_handler
	pop {pc}


.thumb_func
.global __pendsv
__pendsv:
	push {lr}
	bl pendsv_handler
	pop {pc}
