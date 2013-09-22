/*
 * skeldal_pch.h
 *
 *  Created on: 22.9.2013
 *      Author: ondra
 */

#ifndef SKELDAL_PCH_H_
#define SKELDAL_PCH_H_

#ifdef _WIN32
#include "Windows/skeldal_win.h"
#else
#include "linux/skeldal_lin.h"
#endif

#endif /* SKELDAL_PCH_H_ */
