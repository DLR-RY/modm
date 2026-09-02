/*
 * Copyright (c) 2020, Niklas Hauser
 *
 * This file is part of the modm project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
// ----------------------------------------------------------------------------

#include "flash.hpp"
#include <modm/architecture/interface/interrupt.hpp>

namespace modm::platform
{

MODM_ISR(FLASH)
{
	Flash::interruptHandler();
}

volatile Flash::Status Flash::bankStatus[Flash::NumberOfBanks] = {Flash::Status::idle, Flash::Status::idle};

bool
Flash::unlock(uint8_t bank)
{
	if (isLocked(bank))
	{
		// only enable and clear interrupts if not yet in use
		if(isLocked(getOtherBank(bank)))
		{
			enable();
		}
		if(1U == bank)
		{
			FLASH->KEYR1 = 0x45670123;
			FLASH->KEYR1 = 0xCDEF89AB;
		} else if(2U == bank)
		{
			FLASH->KEYR2 = 0x45670123;
			FLASH->KEYR2 = 0xCDEF89AB;
		}
	}
	return not isLocked(bank);
}

bool
Flash::lock(uint8_t bank)
{
	if(1U == bank)
	{
		FLASH->CR1 |= FLASH_CR_LOCK;
	} else if(2U == bank)
	{
		FLASH->CR2 |= FLASH_CR_LOCK;
	}

	if(isLocked(getOtherBank(bank)))
	{
		disable();
	}

	return isLocked(bank);
}

modm_ramcode void
Flash::initiateErase(uint8_t index)
{
	uint8_t bank = getBank(index);
	Flash::bankStatus[bank-1] = Flash::Status::operationOngoing;
	if(bank == 1)
	{
		initErase(FLASH->CR1, index);
	} else if(bank == 2)
	{
		initErase(FLASH->CR2, index);
	}
}

modm_ramcode uint32_t
Flash::finalizeErase(uint8_t bank)
{
	if(bank == 1)
	{
		FLASH->CR1 = 0;
		bankStatus[0] = Status::idle;
		return FLASH->SR1 & FLASH_SR_CCR_ERR;
	} else if(bank == 2)
	{
		FLASH->CR2 = 0;
		bankStatus[1] = Status::idle;
		return FLASH->SR2 & FLASH_SR_CCR_ERR;
	}
	return 0xffff'ffff;
}
		

modm_ramcode void
Flash::initiateProgram(uintptr_t addr, uintptr_t data)
{
	if((addr < OriginAddr) || ((addr + (FlashWord * sizeof(uint32_t))) > (OriginAddr + Size)))
	{
		return;
	}

	uint8_t bank = getBank(addr);
	if(1U == bank)
	{
		FLASH->CR1 |= (FLASH_CR_PG | FLASH_CR_EOPIE | FLASH_SR_CCR_ERR);
	} else if(2U == bank)
	{
		FLASH->CR2 = (FLASH_CR_PG | FLASH_CR_EOPIE | FLASH_SR_CCR_ERR);
	}
	Flash::bankStatus[bank-1] = Flash::Status::operationOngoing;
	for(size_t i = 0; i < FlashWord; i++)
	{
		*(uint32_t *) addr = *(uint32_t *)data;
		addr += 4;
		data += 4;
	}
}

modm_ramcode uint32_t
Flash::finalizeProgram(uint8_t bank)
{
	if(1U == bank)
	{
		FLASH->CR1 &= ~(FLASH_CR_PG | FLASH_CR_EOPIE);
		bankStatus[0] = Status::idle;
		return FLASH->SR1 & FLASH_SR_CCR_ERR;
	} else if(2U == bank)
	{
		FLASH->CR2 &= ~(FLASH_CR_PG | FLASH_CR_EOPIE);
		bankStatus[1] = Status::idle;
		return FLASH->SR2 & FLASH_SR_CCR_ERR;
	}
	return 0xFFFF'FFFF;
}

} // namespace modm::platform