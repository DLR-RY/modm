/*
 * Copyright (c) 2016-2017, Niklas Hauser
 *
 * This file is part of the modm project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
// ----------------------------------------------------------------------------

#include <modm/board.hpp>
#include <modm/debug/logger.hpp>

// ----------------------------------------------------------------------------
// Set the log level
#undef	MODM_LOG_LEVEL
#define	MODM_LOG_LEVEL modm::log::INFO

// Create an IODeviceWrapper around the Uart Peripheral we want to use
using Usart1 = BufferedUart<UsartHal1>;
modm::IODeviceWrapper< Usart1, modm::IOBuffer::BlockIfFull > loggerDevice;

// Set all four logger streams to use the UART
modm::log::Logger modm::log::debug(loggerDevice);
modm::log::Logger modm::log::info(loggerDevice);
modm::log::Logger modm::log::warning(loggerDevice);
modm::log::Logger modm::log::error(loggerDevice);


int
main()
{
	Board::initialize();

	// initialize Uart for MODM_LOG
	Usart1::connect<GpioOutputA9::Tx>();
	Usart1::initialize<Board::SystemClock, 115200_Bd>();

	if (FaultReporter::hasReport())
	{
		MODM_LOG_ERROR << "\n\nHardFault! Copy the data into a 'coredump.txt' file, ";
		MODM_LOG_ERROR << "then execute\n\n\tscons postmortem firmware=" << modm::hex;
		for (const auto data : FaultReporter::buildId()) MODM_LOG_ERROR << data;
		MODM_LOG_ERROR << "\n\n";
		for (const auto data : FaultReporter())
			MODM_LOG_ERROR << modm::hex << data << modm::flush;
		MODM_LOG_ERROR << "\n\n\n" << modm::flush;
		FaultReporter::clearAndReboot();
	}

	MODM_LOG_INFO << "Causing a Hardfault now!" << modm::endl;

	// simulate some stack usage
	asm volatile ("push {r0-r7}");
	asm volatile ("push {r0-r7}");
	asm volatile ("push {r0-r7}");
	asm volatile ("pop {r0-r7}");
	asm volatile ("pop {r0-r7}");
	asm volatile ("pop {r0-r7}");

	// load some patterns into r0-r7
	asm volatile ("ldr r0, =0xA000000A");
	asm volatile ("ldr r1, =0x0AAAAAA0");
	asm volatile ("ldr r2, =0x00A00A00");
	asm volatile ("ldr r3, =0x000AA000");
	asm volatile ("ldr r4, =0x04040404");
	asm volatile ("ldr r5, =0x05050505");
	asm volatile ("ldr r6, =0x06060606");
	asm volatile ("ldr r7, =0x07070707");

	// execute unused stack
	asm volatile ("ldr r4, =0x20000247");
	asm volatile ("bx r4");

	// undefined instruction
	asm volatile (".short 0xde00");

	// stack overflow
	while(true) {
		asm volatile ("push {r0-r7}");
	}

	while (true)
	{
		modm::delay(1000ms);
		Board::LedUp::toggle();
	}

	return 0;
}
