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

#pragma once
#include <modm/architecture/interface/register.hpp>
#include <modm/platform/clock/rcc.hpp>

#include "../device.hpp"

namespace modm::platform
{

/// @ingroup modm_platform_flash
class Flash
{
public:
	enum class Status : uint8_t
	{
		idle = 0,
		operationOngoing = 1,
		operationSuccess = 2,
		operationFailed = 3
	};

	static constexpr uint32_t FLASH_SR_CCR_ERR = 0x1FEE0000;

	static constexpr uintptr_t OriginAddr{0x8000000};
	static constexpr size_t Size{0x200000};
	static constexpr uint8_t SectorShift = 17U;
	static constexpr size_t SectorSize = (1ul << 17);
	static constexpr size_t FlashWord = 8U;
	static constexpr size_t NumberOfBanks = 2U;

	static inline uint8_t *const Origin{(uint8_t *)(OriginAddr)};

	enum class WordSize : uint32_t
	{
		B8 = 0,
		B16 = FLASH_CR_PSIZE_0,
		B32 = FLASH_CR_PSIZE_1,
	};

private:

	modm_always_inline static void
	checkAndClear(uint32_t volatile& SR, uint32_t volatile& CCR, uint8_t bank)
	{
		if((SR & (FLASH_SR_EOP | FLASH_SR_CCR_ERR)))
		{
			if(SR & (FLASH_SR_QW | FLASH_SR_BSY))
			{
				Flash::bankStatus[bank] = Flash::Status::operationOngoing;
			}
			else if(SR & FLASH_SR_CCR_ERR) 
			{
				Flash::bankStatus[bank] = Flash::Status::operationFailed;
			}
			else
			{
				Flash::bankStatus[bank] = Flash::Status::operationSuccess;
			}
			CCR |= FLASH_CCR_CLR_EOP | FLASH_SR_CCR_ERR;
		}
	}

public:
	modm_always_inline static void
	interruptHandler()
	{
		checkAndClear(FLASH->SR1, FLASH->CCR1, 0);
		checkAndClear(FLASH->SR2, FLASH->CCR2, 1);
	}

	inline static void
	enable()
	{
		FLASH->CCR1 |= FLASH_CCR_CLR_EOP | FLASH_SR_CCR_ERR;
		FLASH->CCR2 |= FLASH_CCR_CLR_EOP | FLASH_SR_CCR_ERR;
		NVIC_EnableIRQ(FLASH_IRQn);
		// we are just guessing a priority here, adapt if needed
		NVIC_SetPriority(FLASH_IRQn, 5);
	}

	inline static void
	disable()
	{
		NVIC_DisableIRQ(FLASH_IRQn);
	}

	static bool
	isLocked(uint8_t bank)
	{
		if (bank == 1) { return FLASH->CR1 & FLASH_CR_LOCK; }
		if (bank == 2) { return FLASH->CR2 & FLASH_CR_LOCK; }
		return true;
	}

	static inline Status
	getStatus(uint8_t bank)
	{
		return Flash::bankStatus[bank-1];
	}

	static bool
	unlock(uint8_t bank);

	static bool
	lock(uint8_t bank);

	static constexpr uint8_t
	getBank(uint8_t sector)
	{
		return sector > 7 ? 2 : 1;
	}

	static constexpr uint8_t
	getBank(uintptr_t addr)
	{
		return getBank(getSector(addr));
	}

	static uint8_t
	getSector(uint8_t *ptr)
	{
		return getPage(ptr - Flash::Origin);
	}

	static constexpr uint8_t
	getSector(uintptr_t offset)
	{
		return getPage(offset);
	}

	static uint8_t
	getPage(uint8_t *ptr)
	{
		return getPage(ptr - Flash::Origin);
	}

	static constexpr uint8_t
	getPage(uintptr_t offset)
	{
		return (offset >> SectorShift);
	}

	static uint8_t *
	getAddr(uint8_t sector)
	{
		return Origin + getOffset(sector);
	}

	static constexpr uint32_t
	getOffset(uint8_t index)
	{
		return SectorSize * index;
	}

	static constexpr size_t
	getSize(uint8_t /* ignored */)
	{
		return SectorSize;
	}

	static void
	initiateErase(uint8_t sector);

	/**
	 * @brief Cleans up after an erase by setting the control register, bank status and doing final error checks
	 * @param bankId Either 1 or 2 to control the corresponding bank
	 * @return 0 if the error register is cleared, error bits otherwise
	 */
	static uint32_t
	finalizeErase(uint8_t bankId);

	static void
	initiateProgram(uintptr_t addr, uintptr_t data);

	/**
	 * @brief Cleans up after a program operation by setting the control register, bank status and doing final error checks
	 * @param bankId Either 1 or 2 to control the corresponding bank
	 * @return 0 if the error register is cleared, error bits otherwise
	 */
	static uint32_t
	finalizeProgram(uint8_t bankId);

private:
	static inline void
	initErase(uint32_t volatile& CR, uint8_t index)
	{
		CR = FLASH_CR_START | FLASH_CR_EOPIE | FLASH_CR_SER | 
		        (uint32_t)FLASH_CR_PSIZE_1 | FLASH_SR_CCR_ERR |
				((index << FLASH_CR_SNB_Pos) & FLASH_CR_SNB_Msk);
	}

	static inline uint8_t
	getOtherBank(uint8_t bankId)
	{
		static_assert(NumberOfBanks == 2, "Other-Bank-check is only correct if flash has exactly two banks");
		return bankId == 1 ? 2U : 1U;
	}

	static volatile Status bankStatus[NumberOfBanks];
};

}  // namespace modm::platform