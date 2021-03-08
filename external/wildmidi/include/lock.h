/*
 * lock.h - data locking code for lib
 *
 * Copyright (C) Chris Ison 2001-2011
 * Copyright (C) Bret Curtis 2013-2016
 *
 * This file is part of WildMIDI.
 *
 * WildMIDI is free software: you can redistribute and/or modify the player
 * under the terms of the GNU General Public License and you can redistribute
 * and/or modify the library under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation, either version 3 of
 * the licenses, or(at your option) any later version.
 *
 * WildMIDI is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License and
 * the GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License and the
 * GNU Lesser General Public License along with WildMIDI.  If not,  see
 * <http://www.gnu.org/licenses/>.
 */

#ifndef __LOCK_H
#define __LOCK_H

extern void _WM_Lock (int * wmlock);
extern void _WM_Unlock (int *wmlock);

#if defined WM_NO_LOCK
#define _WM_Lock(p) do {} while (0)
#define _WM_Unlock(p) do {} while (0)
#endif

#endif /* __LOCK_H */
