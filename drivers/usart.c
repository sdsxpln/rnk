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

#include <board.h>
#include <utils.h>
#include <usart.h>
#include <mm.h>
#include <errno.h>

static struct usart *usart;

int usart_init(unsigned int num, unsigned int base_reg, unsigned int baud_rate)
{
	usart = (struct usart *)kmalloc(sizeof(*usart));
	if (usart < 0) {
		error_printk("cannot allocate usart\r\n");
		return -ENOMEM;
	}

	usart->num = num;
	usart->base_reg = base_reg;
	usart->baud_rate = baud_rate;

	return usart_ops.init(usart);
}

void usart_print(unsigned char byte)
{
	usart_ops.print(usart, byte);
}


int usart_printl(const char *string)
{
	return usart_ops.printl(usart, string);
}

struct io_operations io_op = {
	.write = usart_print,
	.write_string = usart_printl,
};
