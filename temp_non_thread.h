/*
 * Excelfore Universal IPC and Configuration code generator
 * Copyright (C) 2020 Excelfore Corporation (https://excelfore.com)
 *
 * This file is part of Excelfore-unipac.
 *
 * Excelfore-unipac is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * Excelfore-unipac is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Excelfore-unipac.  If not, see
 * <https://www.gnu.org/licenses/old-licenses/gpl-2.0.html>.
 */
#ifndef ___CONFPREFIX__NON_THREAD_H_
#define ___CONFPREFIX__NON_THREAD_H_

#define CB_THREAD_T void*
#define CB_THREAD_MUTEX_T void*
#define CB_THREAD_COND_T void*
#define CB_THREAD_MUTEX_INIT(x, y) true
#define CB_THREAD_CREATE(a, b, c, d) true
#define CB_THREAD_JOIN(x, y)
#define CB_THREAD_MUTEX_DESTROY(x)
#define CB_THREAD_MUTEX_LOCK(x) 0
#define CB_THREAD_MUTEX_TIMEDLOCK(x, y) 0
#define CB_THREAD_MUTEX_UNLOCK(x)

#endif
