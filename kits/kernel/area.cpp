/*
 *  area.cpp
 *
 *  Copyright (C) 2007 Pier Luigi Fiorini
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public
 *  License along with this library; if not, write to the Free
 *  Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 * 
 *  Author:  Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
 *
 */

/* --------------------------------------------------------------------------
 *
 * Copyright (C) 2004-2006, Anthony Lee, All Rights Reserved
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR
 * IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * --------------------------------------------------------------------------*/

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>

#include <kernel/Kernel.h>
#include <support/StringArray.h>
#include <support/SimpleLocker.h>

typedef struct posix_area_t {
	posix_area_t()
			: name(NULL), domain(NULL), ipc_name(NULL), prot(0), length(0), addr(NULL), openedIPC(true), created(false) {
	}

	~posix_area_t() {
		if (created) {
			created = false;
			delete_area((void*)this);
		}
	}

	char		*name;
	char		*domain;
	char		*ipc_name;
	uint32		prot;
	size_t		length;
	void		*addr;
	bool		openedIPC;
	bool		created;
} posix_area_t;

// return value must be free by "free()"
static char* area_ipc_name(const char *name, const char *domain)
{
	if (name == NULL || *name == 0 || strlen(name) >B_OS_NAME_LENGTH ||
		!domain || strlen(domain) != 4)
		return NULL;

	return b_strdup_printf("/%s%s%s", domain, "_area_", name);
}


void*
create_area(const char *name, void **start_addr, size_t size, uint32 protection,
            const char *domain, area_access area_access)
{
	if (size <= 0)
		return NULL;

	char *ipc_name = area_ipc_name(name, domain);
	if (!ipc_name)
		return NULL;

	posix_area_t *area = new posix_area_t();
	if (!area) {
		free(ipc_name);
		return NULL;
	}

	area->prot = protection;

	mode_t openMode = S_IRUSR | S_IWUSR;
	if (area_access & ETK_AREA_ACCESS_GROUP_READ) openMode |= S_IRGRP;
	if (area_access & ETK_AREA_ACCESS_GROUP_WRITE) openMode |= S_IWGRP;
	if (area_access & ETK_AREA_ACCESS_OTHERS_READ) openMode |= S_IROTH;
	if (area_access & ETK_AREA_ACCESS_OTHERS_WRITE) openMode |= S_IWOTH;

	int handler;

	if ((handler = shm_open(ipc_name, O_CREAT | O_EXCL | O_RDWR, openMode)) == -1) {
		bool doFailed = true;

		ETK_DEBUG("[KERNEL]: %s --- Map \"%s\" existed, try again after unlink it.", __PRETTY_FUNCTION__, ipc_name);
		if (!(shm_unlink(ipc_name) != 0 ||
		        (handler = shm_open(ipc_name, O_CREAT | O_EXCL | O_RDWR, openMode)) == -1)) doFailed = false;

		if (doFailed) {
			ETK_DEBUG("[KERNEL]: %s --- CANNOT create map \"%s\": error_no: %d", __PRETTY_FUNCTION__, ipc_name, errno);
			free(ipc_name);
			delete area;
			return NULL;
		}
	}

	if (ftruncate(handler, size) != 0) {
		close(handler);
		shm_unlink(ipc_name);
		free(ipc_name);
		delete area;
		return NULL;
	}

	int prot = PROT_READ;
	if (protection &B_WRITE_AREA)
		prot |= PROT_WRITE;

	if ((area->addr = mmap(NULL, size, prot, MAP_SHARED, handler, 0)) == MAP_FAILED) {
		close(handler);
		shm_unlink(ipc_name);
		free(ipc_name);
		delete area;
		return NULL;
	}

	close(handler);

	area->length = size;
	area->openedIPC = false;
	area->name = b_strdup(name);
	area->domain = b_strdup(domain);
	area->ipc_name = ipc_name;
	area->created = true;

	if (start_addr)
		*start_addr = area->addr;

	return area;
}


void*
clone_area(const char *name, void **dest_addr, uint32 protection, const char *domain)
{
	char *ipc_name = area_ipc_name(name, domain);
	if (!ipc_name) return NULL;

	posix_area_t *area = new posix_area_t();
	if (!area) {
		free(ipc_name);
		return NULL;
	}

	area->prot = protection;

	int oflag;
	if (protection &B_WRITE_AREA)
		oflag = O_RDWR;
	else
		oflag = O_RDONLY;

	int handler;

	if ((handler = shm_open(ipc_name, oflag, 0)) == -1) {
		free(ipc_name);
		delete area;
		return NULL;
	}

	struct stat stat;
	bzero(&stat, sizeof(stat));
	fstat(handler, &stat);
	size_t size = stat.st_size;
	if (size <= 0) {
		close(handler);
		free(ipc_name);
		delete area;
		return NULL;
	}

	int prot = PROT_READ;
	if (protection &B_WRITE_AREA) prot |= PROT_WRITE;

	if ((area->addr = mmap(NULL, size, prot, MAP_SHARED, handler, 0)) == MAP_FAILED) {
		close(handler);
		free(ipc_name);
		delete area;
		return NULL;
	}

	close(handler);

	area->length = size;
	area->openedIPC = true;
	area->name = b_strdup(name);
	area->domain = b_strdup(domain);
	area->ipc_name = ipc_name;
	area->created = true;

	if (dest_addr)
		*dest_addr = area->addr;

	return area;
}


void*
clone_area_by_source(void *source_data, void **dest_addr, uint32 protection)
{
	posix_area_t *source_area = (posix_area_t*)source_data;
	if (!source_area) return NULL;

	return clone_area(source_area->name, dest_addr, protection, source_area->domain);
}


status_t
get_area_info(void *data, area_info *info)
{
	posix_area_t *area = (posix_area_t*)data;
	if (!area || !info) return B_BAD_VALUE;
	if (!area->name || *(area->name) == 0 || strlen(area->name) >B_OS_NAME_LENGTH ||
	        !area->domain || strlen(area->domain) != 4 ||
	        area->addr == NULL || area->addr == MAP_FAILED) return B_ERROR;

	bzero(info->name, B_OS_NAME_LENGTH + 1);
	bzero(info->domain, 5);

	strcpy(info->name, area->name);
	strcpy(info->domain, area->domain);
	info->protection = area->prot;
	info->address = area->addr;
	info->size = area->length;

	return B_OK;
}


status_t
delete_area(void *data)
{
	posix_area_t *area = (posix_area_t*)data;
	if (!area) return B_BAD_VALUE;

	if (!(area->addr == NULL || area->addr == MAP_FAILED)) munmap(area->addr, area->length);

	if (area->openedIPC == false) shm_unlink(area->ipc_name);
	free(area->ipc_name);

	free(area->name);
	free(area->domain);

	if (area->created) {
		area->created = false;
		delete area;
	}

	return B_OK;
}


status_t
delete_area_etc(void *data, bool no_clone)
{
	posix_area_t *area = (posix_area_t*)data;
	if (!area) return B_BAD_VALUE;

	if (!(area->addr == NULL || area->addr == MAP_FAILED))
		munmap(area->addr, area->length);

	if (no_clone && (area->openedIPC ? (area->prot &B_WRITE_AREA) : true)) shm_unlink(area->ipc_name);

	free(area->ipc_name);

	free(area->name);
	free(area->domain);

	if (area->created) {
		area->created = false;
		delete area;
	}

	return B_OK;
}


status_t
resize_area(void *data, void **start_addr, size_t new_size)
{
	posix_area_t *area = (posix_area_t*)data;
	if (!area || area->openedIPC || new_size <= 0) return B_BAD_VALUE;

	int handler;

	if ((handler = shm_open(area->ipc_name, O_RDWR, 0)) == -1) return B_ERROR;

	if (ftruncate(handler, new_size) == 0) {
		void *addr;
		if ((addr = mremap(area->addr, area->length, new_size, MREMAP_MAYMOVE)) == MAP_FAILED) {
			ftruncate(handler, area->length);
			close(handler);
			return(errno == ENOMEM ?B_NO_MEMORY :B_ERROR);
		}
		close(handler);

		area->length = new_size;
		area->addr = addr;

		if (start_addr) *start_addr = area->addr;
		return B_OK;
	}

	close(handler);

	return B_ERROR;
}


status_t
set_area_protection(void *data, uint32 new_protection)
{
	posix_area_t *area = (posix_area_t*)data;
	if (!area) return B_BAD_VALUE;

	int prot = PROT_READ;
	if (new_protection &B_WRITE_AREA) prot |= PROT_WRITE;

	if (mprotect(area->addr, area->length, prot) != 0) return B_ERROR;

	area->prot = new_protection;

	return B_OK;
}
