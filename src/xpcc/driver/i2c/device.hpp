// coding: utf-8
// ----------------------------------------------------------------------------
/* Copyright (c) 2009, Roboterclub Aachen e.V.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the Roboterclub Aachen e.V. nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY ROBOTERCLUB AACHEN E.V. ''AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL ROBOTERCLUB AACHEN E.V. BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * $Id$
 */
// ----------------------------------------------------------------------------

#ifndef XPCC_I2C__DEVICE_HPP
#define XPCC_I2C__DEVICE_HPP

#include "interface.hpp"

namespace xpcc
{
	namespace i2c
	{
		/**
		 * \brief	Base class for all I2C Devices
		 * 
		 * \tparam	I2C		Must be a driver that implements the
		 * 					xpcc::i2c::SynchronousMaster interface, or
		 *					xpcc::i2c::AsynchronousMaster interface.
		 * 
		 * \ingroup	i2c
		 * \author	Fabian Greif
		 */
		template <typename I2C>
		class Device
		{
		public:
			Device(uint8_t address);
			
			/**
			 * \brief	Check if the device is accessable
			 * 
			 * \return	\c true if the device responds to its address,
			 * 			\c false otherwise.
			 */
			bool
			isAvailable() const;
			
		protected:
			const uint8_t deviceAddress;
			static I2C i2c;
		};
		
		template <>
		class Device< AsynchronousMaster >
		{
		public:
			Device(uint8_t address);
			
			bool
			isAvailable() const;
			
		protected:
			const uint8_t deviceAddress;
			static AsynchronousMaster i2c;
		};
	}
}

#include "device_impl.hpp"

#endif // XPCC_I2C__DEVICE_HPP
