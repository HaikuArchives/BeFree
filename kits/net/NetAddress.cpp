/* --------------------------------------------------------------------------
 *
 * ETK++ --- The Easy Toolkit for C++ programing
 * Copyright (C) 2004-2006, Anthony Lee, All Rights Reserved
 *
 * ETK++ library is a freeware; it may be used and distributed according to
 * the terms of The MIT License.
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
 * File: NetAddress.cpp
 *
 * --------------------------------------------------------------------------*/

#ifndef _WIN32
#	include <netdb.h>
#	include <netinet/in.h>

#	ifdef __BEOS__
#		include <sys/socket.h>
#		ifdef BONE_VERSION
#			include <arpa/inet.h>
#		endif
#	else
#		include <arpa/inet.h>
#	endif
#else
#	include <winsock2.h>
#endif

#include <support/ByteOrder.h>

#include "NetAddress.h"


BNetAddress::BNetAddress(const char *hostname, uint16 port)
		: BArchivable(), fStatus(B_NO_INIT)
{
	bzero(&fAddr, sizeof(struct sockaddr_in));
	SetTo(hostname, port);
}


BNetAddress::BNetAddress(const char *hostname, const char *protocol, const char *service)
		: BArchivable(), fStatus(B_NO_INIT)
{
	bzero(&fAddr, sizeof(struct sockaddr_in));
	SetTo(hostname, protocol, service);
}


BNetAddress::BNetAddress(const struct sockaddr_in &sa)
		: BArchivable(), fStatus(B_NO_INIT)
{
	bzero(&fAddr, sizeof(struct sockaddr_in));
	SetTo(sa);
}


BNetAddress::BNetAddress(const struct in_addr addr, uint16 port)
		: BArchivable(), fStatus(B_NO_INIT)
{
	bzero(&fAddr, sizeof(struct sockaddr_in));
	SetTo(addr, port);
}


BNetAddress::BNetAddress(uint32 addr, uint16 port)
		: BArchivable(), fStatus(B_NO_INIT)
{
	bzero(&fAddr, sizeof(struct sockaddr_in));
	SetTo(addr, port);
}


BNetAddress::BNetAddress(const BNetAddress &from)
		: BArchivable(), fStatus(B_NO_INIT)
{
	bzero(&fAddr, sizeof(struct sockaddr_in));
	BNetAddress::operator=(from);
}


BNetAddress::~BNetAddress()
{
}


BNetAddress::BNetAddress(const BMessage *from)
		: BArchivable(from), fStatus(B_NO_INIT)
{
	bzero(&fAddr, sizeof(struct sockaddr_in));
	// TODO
}


status_t
BNetAddress::Archive(BMessage *into, bool deep) const
{
	if (!into) return B_ERROR;

	BArchivable::Archive(into, deep);
	into->AddString("class", "BNetAddress");

	// TODO

	return B_OK;
}


BArchivable*
BNetAddress::Instantiate(const BMessage *from)
{
	if (e_validate_instantiation(from, "BNetAddress"))
		return new BNetAddress(from);
	return NULL;
}


status_t
BNetAddress::InitCheck() const
{
	return fStatus;
}


BNetAddress&
BNetAddress::operator=(const BNetAddress &addr)
{
	fStatus = addr.fStatus;
	fAddr = addr.fAddr;
	return *this;
}


status_t
BNetAddress::SetTo(const char *hostname, uint16 port)
{
	if (hostname == NULL) return B_ERROR;

	struct hostent *ent = NULL;

	ent = gethostbyname(hostname);

	struct hostent _ent;
	char buf[8192];
	int err;
	gethostbyname_r(hostname, &_ent, buf, sizeof(buf), &ent, &err);

	if (ent == NULL) return B_ERROR;

	status_t retVal = B_ERROR;

	switch (ent->h_addrtype) {
		case AF_INET:
			fAddr.sin_addr.s_addr = *((uint32*)ent->h_addr);
			fAddr.sin_family = AF_INET;
			fAddr.sin_port = htons(port);
			retVal = fStatus = B_OK;
			break;

		default:
			ETK_DEBUG("[NET]: %s --- unknown address type.", __PRETTY_FUNCTION__);
			break;
	}

	return retVal;
}


status_t
BNetAddress::SetTo(const char *hostname, const char *protocol, const char *service)
{
	if (hostname == NULL) return B_ERROR;

	struct servent *ent = NULL;

	ent = getservbyname(service, protocol);
	struct servent _ent;
	char buf[8192];
	getservbyname_r(service, protocol, &_ent, buf, sizeof(buf), &ent);

	if (ent == NULL) return B_ERROR;

	return SetTo(hostname, ntohs(ent->s_port));
}


status_t
BNetAddress::SetTo(const struct sockaddr_in &sa)
{
	if (sa.sin_family != AF_INET) {
		// TODO
		return B_ERROR;
	}

	fAddr = sa;
	return(fStatus = B_OK);
}


status_t
BNetAddress::SetTo(const struct in_addr addr, uint16 port)
{
	fAddr.sin_family = AF_INET;
	fAddr.sin_port = htons(port);
	fAddr.sin_addr = addr;
	return(fStatus = B_OK);
}


status_t
BNetAddress::SetTo(uint32 addr, uint16 port)
{
	fAddr.sin_family = AF_INET;
	fAddr.sin_port = htons(port);
	fAddr.sin_addr.s_addr = htonl(addr);
	return(fStatus = B_OK);
}


status_t
BNetAddress::GetAddr(char *hostname, size_t hostname_len, uint16 *port) const
{
	if (fStatus != B_OK) return B_ERROR;
	if (!(hostname == NULL || hostname_len == 0)) {
		struct hostent *ent = NULL;

		ent = gethostbyaddr((const char*)&fAddr.sin_addr, sizeof(struct in_addr), AF_INET);
		struct hostent _ent;
		char buf[8192];
		int err;
		gethostbyaddr_r((const char*)&fAddr.sin_addr, sizeof(struct in_addr), AF_INET,
		                &_ent, buf, sizeof(buf), &ent, &err);

		if (ent == NULL) return B_ERROR;

		if (hostname_len > 1) {
			hostname_len = min_c(hostname_len, strlen(ent->h_name) + 1);
			memcpy(hostname, ent->h_name, hostname_len - 1);
		}

		*(hostname + hostname_len - 1) = 0;
	}
	if (port) *port = ntohs(fAddr.sin_port);
	return B_OK;
}


status_t
BNetAddress::GetAddr(struct sockaddr_in &sa) const
{
	if (fStatus != B_OK) return B_ERROR;
	sa = fAddr;
	return B_OK;
}


status_t
BNetAddress::GetAddr(struct in_addr &addr, uint16 *port) const
{
	if (fStatus != B_OK) return B_ERROR;
	addr = fAddr.sin_addr;
	if (port) *port = ntohs(fAddr.sin_port);
	return B_OK;
}

