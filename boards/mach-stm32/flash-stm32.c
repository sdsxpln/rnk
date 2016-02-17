/*
 * Copyright (C) 2016  Raphaël Poggi <poggi.raph@gmail.com>
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
 * along with this program; if not, write to the Frrestore * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include <board.h>
#include <errno.h>

#define FLASH_PSIZE_BYTE	0
#define CR_PSIZE_MASK		0xFFFFFCFF

#define FLASH_KEY1	0x45670123
#define FLASH_KEY2      0xCDEF89AB

#define SECTOR_MASK	0xFFFFFF07

static void stm32_flash_lock(void)
{
	FLASH->CR |= FLASH_CR_LOCK;
}

static void stm32_flash_unlock(void)
{
	if (!(FLASH->CR & FLASH_CR_LOCK)) {
		FLASH->KEYR = FLASH_KEY1;
		FLASH->KEYR = FLASH_KEY2;
	}
}

static int stm32_flash_wait_operation(void)
{
	int ret = 0;

	if (FLASH->SR & FLASH_SR_BSY)
		while (FLASH->SR & FLASH_SR_BSY)
			;
	else if (FLASH->SR & (FLASH_SR_WRPERR | FLASH_SR_PGAERR | FLASH_SR_PGPERR))
		ret = -EIO;

	return ret;
}

static int stm32_flash_erase(unsigned int sector)
{
	int ret = 0;

	stm32_flash_unlock();

	ret = stm32_flash_wait_operation();
	if (ret < 0) {
		error_printk("last operation did not success\n");
		return ret;
	}

	FLASH->CR &= CR_PSIZE_MASK;
	FLASH->CR |= FLASH_PSIZE_BYTE;
	FLASH->CR &= SECTOR_MASK;
	FLASH->CR |= FLASH_CR_SER | sector;
	FLASH->CR |= FLASH_CR_STRT;

	ret = stm32_flash_wait_operation();
	if (ret < 0) {
		error_printk("failed to erase sector\n");
		return ret;
	}

	FLASH->CR &= (~FLASH_CR_SER);
	FLASH->CR &= SECTOR_MASK;

	stm32_flash_lock();

	return ret;
}

static int stm32_flash_write_byte(unsigned int address, unsigned char data)
{
	int ret = 0;

	stm32_flash_unlock();

	ret = stm32_flash_wait_operation();
	if (ret < 0) {
		error_printk("last operation did not success\n");
		return ret;
	}

	FLASH->CR &= CR_PSIZE_MASK;
	FLASH->CR |= FLASH_PSIZE_BYTE;
	FLASH->CR |= FLASH_CR_PG;

	*(unsigned char *)address = data;

	ret = stm32_flash_wait_operation();
	if (ret < 0) {
		error_printk("failed to write 0x%x at 0x%x\n", data, address);
		return ret;
	}

	FLASH->CR &= (~FLASH_CR_PG);

	stm32_flash_lock();

	return ret;
}

struct mtd_operations mtd_ops = {
	.erase = stm32_flash_erase,
	.write_byte = stm32_flash_write_byte,
};