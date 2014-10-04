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
 * File: NetEndpoint.cpp
 *
 * --------------------------------------------------------------------------*/

// WARNING: MT-SAFE uncompleted yet !!!

#include <errno.h>
#include <string.h>

#include <sys/types.h>
#include <signal.h>

#ifndef _WIN32
#	include <sys/time.h>
#	include <netdb.h>
#	include <unistd.h>
#	include <fcntl.h>

#	ifdef __BEOS__
#		include <sys/socket.h>
#		ifdef BONE_VERSION
#			include <arpa/inet.h>
#		endif
#		define socklen_t int
#	else
#		include <arpa/inet.h>
#	endif
#else
#	include <winsock2.h>
#	define socklen_t int
#	define EWOULDBLOCK WSAEWOULDBLOCK
#	define ECONNABORTED WSAECONNABORTED
#	undef EINTR
#	define EINTR WSAEINTR
#endif

#include "NetEndpoint.h"


#ifdef SIGPIPE
class _LOCAL BNetEndpointSignalIgnore
{
	public:
		BNetEndpointSignalIgnore() {
			ETK_DEBUG("[NET]: Ignore SIGPIPE.");
			signal(SIGPIPE, SIG_IGN);
		}
};
static BNetEndpointSignalIgnore _ignore;
#endif


BNetEndpoint::BNetEndpoint(int proto)
		: BArchivable(), fProtocol(proto), fBind(false), fNonBlocking(false), fTimeout(0)
{
	fSocket = socket(AF_INET, fProtocol, 0);
}


BNetEndpoint::BNetEndpoint(const BNetEndpoint &from)
		: BArchivable(), fSocket(-1), fBind(false), fNonBlocking(false)
{
	BNetEndpoint::operator=(from);
}


BNetEndpoint::~BNetEndpoint()
{
	_Close();
}


BNetEndpoint::BNetEndpoint(const BMessage *from)
		: BArchivable(from), fSocket(-1), fBind(false), fNonBlocking(false)
{
	// TODO
}


status_t
BNetEndpoint::Archive(BMessage *into, bool deep) const
{
	if (!into) return B_ERROR;

	BArchivable::Archive(into, deep);
	into->AddString("class", "BNetEndpoint");

	// TODO

	return B_OK;
}


BArchivable*
BNetEndpoint::Instantiate(const BMessage *from)
{
	if (e_validate_instantiation(from, "BNetEndpoint"))
		return new BNetEndpoint(from);
	return NULL;
}


status_t
BNetEndpoint::InitCheck() const
{
	return(fSocket == -1 ?B_ERROR :B_OK);
}


BNetEndpoint&
BNetEndpoint::operator=(const BNetEndpoint &endpoint)
{
	BNetEndpoint::Close();

	if (endpoint.fSocket != -1) {
		if (endpoint.fBind) BNetEndpoint::Bind(endpoint.fLocalAddr);
		BNetEndpoint::Connect(endpoint.fRemoteAddr);
		if (endpoint.fNonBlocking) SetNonBlocking(true);
	}

	return *this;
}


status_t
BNetEndpoint::SetProtocol(int proto)
{
	if (fProtocol != proto) {
		int s = socket(AF_INET, proto, 0);
		if (s == -1) return B_ERROR;

		_Close();

		fSocket = s;
		fProtocol = proto;
	}

	return B_OK;
}


int
BNetEndpoint::SetSocketOption(int32 level, int32 option, const void *data, size_t data_len)
{
	if (fSocket == -1) return -1;

	int retVal = setsockopt(fSocket, level, option, (const char*)data, data_len) < 0 ? -1 : 0;

#ifdef __BEOS__
	if (retVal == 0 && option == SO_NONBLOCK) {
		fNonBlocking = false;
		for (const uint8 *tmp = (const uint8*)data; data_len > 0; data_len--, tmp--) {
			if (*tmp != 0) {
				fNonBlocking = true;
				break;
			}
		}
	}
#endif

	return retVal;
}


int
BNetEndpoint::GetSocketOption(int32 level, int32 option, void *data, size_t *data_len) const
{
	if (fSocket == -1) return -1;

#if (defined(__BEOS__) && !defined(BONE_VERSION))
	if (level != SOL_SOCKET || option != SO_NONBLOCK ||
	        data == NULL || data_len == NULL || *data_len == 0) return -1;
	bzero(data, *data_len);
	if (fNonBlocking) *((uint8*)data) = 1;
	return 0;
#else
	socklen_t len = (data_len ? (socklen_t)*data_len : 0);
	if (getsockopt(fSocket, level, option, (char*)data, &len) < 0) return -1;
	if (data_len) *data_len = (size_t)len;
	return 0;
#endif
}


int
BNetEndpoint::SetNonBlocking(bool state)
{
	if (fSocket == -1) return -1;

	if (fNonBlocking == state) return 0;

#ifdef __BEOS__
	return SetSocketOption(SOL_SOCKET, SO_NONBLOCK, &state, sizeof(state));
#elif defined(_WIN32)
	u_long value = (u_long)state;
	if (ioctlsocket(fSocket, FIONBIO, &value) != 0) return -1;
	fNonBlocking = state;
	return 0;
#else
	int flags = fcntl(fSocket, F_GETFL, 0);
	if (state) flags |= O_NONBLOCK;
	else flags &= ~O_NONBLOCK;
	if (fcntl(fSocket, F_SETFL, flags) == -1) return -1;
	fNonBlocking = state;
	return 0;
#endif
}


bool
BNetEndpoint::IsNonBlocking() const
{
	return fNonBlocking;
}


const BNetAddress&
BNetEndpoint::LocalAddr() const
{
	return fLocalAddr;
}


const BNetAddress&
BNetEndpoint::RemoteAddr() const
{
	return fRemoteAddr;
}


void
BNetEndpoint::_Close()
{
	if (fSocket != -1) {
#if defined(_WIN32) || (defined(__BEOS__) && !defined(BONE_VERSION))
		closesocket(fSocket);
#else
		close(fSocket);
#endif
		fSocket = -1;
	}

	fLocalAddr = BNetAddress();
	fRemoteAddr = BNetAddress();

	fBind = false;
	fNonBlocking = false;
}


void
BNetEndpoint::Close()
{
	_Close();
	fSocket = socket(AF_INET, fProtocol, 0);
}


status_t
BNetEndpoint::Bind(const BNetAddress &addr)
{
	if (fSocket == -1) return B_ERROR;

	struct sockaddr_in sa;
	if (addr.GetAddr(sa) != B_OK) return B_ERROR;

	if (bind(fSocket, (struct sockaddr*)&sa, sizeof(sa)) != 0) {
		ETK_DEBUG("[NET]: %s --- bind() failed (errno:%d).", __PRETTY_FUNCTION__, errno);
		return B_ERROR;
	}

	socklen_t len = sizeof(sa);
	getsockname(fSocket, (struct sockaddr*)&sa, &len);
	fLocalAddr.SetTo(sa);
	fBind = true;

	return B_OK;
}


status_t
BNetEndpoint::Bind(uint16 port)
{
	BNetAddress addr(INADDR_LOOPBACK, port);
	return Bind(addr);
}


status_t
BNetEndpoint::Connect(const BNetAddress &addr)
{
	if (fSocket == -1) return B_ERROR;

	struct sockaddr_in sa;
	socklen_t len;

	if (addr.GetAddr(sa) != B_OK) return B_ERROR;

	if (connect(fSocket, (struct sockaddr*)&sa, sizeof(sa)) != 0) {
		ETK_DEBUG("[NET]: %s --- connect() failed (errno:%d).", __PRETTY_FUNCTION__, errno);
		return B_ERROR;
	}

	if (fBind == false) {
		len = sizeof(sa);
		if (getsockname(fSocket, (struct sockaddr*)&sa, &len) == 0) fLocalAddr.SetTo(sa);
	}

	len = sizeof(sa);
	if (getpeername(fSocket, (struct sockaddr*)&sa, &len) == 0) fRemoteAddr.SetTo(sa);

	return B_OK;
}


status_t
BNetEndpoint::Connect(const char *address, uint16 port)
{
	BNetAddress addr(address, port);
	return BNetEndpoint::Connect(addr);
}


status_t
BNetEndpoint::Listen(int backlog)
{
	if (fSocket == -1) return B_ERROR;
	return(listen(fSocket, backlog) == 0 ?B_OK :B_ERROR);
}


BNetEndpoint*
BNetEndpoint::Accept(int32 timeout_msec)
{
	if (fSocket == -1) return NULL;

	struct sockaddr_in sa;
	socklen_t len = sizeof(sa);
	int s = -1;

	if (timeout_msec < 0) {
		s = accept(fSocket, (struct sockaddr*)&sa, &len);
	} else {
		bool saveState = fNonBlocking;
		bigtime_t saveTime = e_real_time_clock_usecs();
		SetNonBlocking(true);
		do {
			if ((s = accept(fSocket, (struct sockaddr*)&sa, &len)) != -1) break;

			int err = Error();
			if (!(err == EWOULDBLOCK ||
			        err == ECONNABORTED ||
			        err == EINTR)) break;
			if (timeout_msec > 0) e_snooze(1000);
		} while (e_real_time_clock_usecs() - saveTime <= timeout_msec *B_INT64_CONSTANT(1000));
		SetNonBlocking(saveState);
	}

	if (s == -1) return NULL;

	BNetEndpoint *endpoint = new BNetEndpoint(fProtocol);
	endpoint->_Close();
	endpoint->fSocket = s;
	endpoint->fLocalAddr = fLocalAddr;
	endpoint->fRemoteAddr.SetTo(sa);

	return endpoint;
}


int
BNetEndpoint::Error() const
{
#ifdef _WIN32
	return WSAGetLastError();
#else
	return errno;
#endif
}


const char*
BNetEndpoint::ErrorStr() const
{
	// FIXME: wrong on Win32
	return strerror(Error());
}


int32
BNetEndpoint::Send(const void *buf, size_t len, int flags)
{
	if (fSocket == -1 || fLocalAddr.InitCheck() != B_OK) return -1;

	if (fProtocol == SOCK_DGRAM) {
		struct sockaddr_in sa;
		if (fRemoteAddr.GetAddr(sa) != B_OK) return -1;
		return sendto(fSocket, (const char*)buf, len, flags, (struct sockaddr*)&sa, sizeof(sa));
	} else {
		return send(fSocket, (const char*)buf, len, flags);
	}
}


int32
BNetEndpoint::Send(const BNetBuffer &buf, int flags)
{
	return BNetEndpoint::Send(buf.Data(), buf.Size(), flags);
}


int32
BNetEndpoint::SendTo(const void *buf, size_t len, const BNetAddress &to, int flags)
{
	if (fSocket == -1 || fLocalAddr.InitCheck() != B_OK) return -1;
	if (fProtocol != SOCK_DGRAM) return -1;

	struct sockaddr_in sa;
	if (fRemoteAddr.GetAddr(sa) != B_OK) return -1;
	return sendto(fSocket, (const char*)buf, len, flags, (struct sockaddr*)&sa, sizeof(sa));
}


int32
BNetEndpoint::SendTo(const BNetBuffer &buf, const BNetAddress &to, int flags)
{
	return BNetEndpoint::SendTo(buf.Data(), buf.Size(), to, flags);
}


void
BNetEndpoint::SetTimeout(bigtime_t timeout)
{
	if (timeout < 0) timeout = 0;
	fTimeout = timeout;
}


int32
BNetEndpoint::Receive(void *buf, size_t len, int flags)
{
	if (fSocket == -1 || fLocalAddr.InitCheck() != B_OK) return -1;
	if (!BNetEndpoint::IsDataPending(fTimeout)) return -1;
	return recv(fSocket, (char*)buf, len, flags);
}


int32
BNetEndpoint::Receive(BNetBuffer &buf, size_t len, int flags)
{
	void *data = (len != 0 ? malloc(len) : NULL);
	if (data == NULL) return -1;

	int32 bytes = BNetEndpoint::Receive(data, len, flags);
	if (bytes < 0) {
		free(data);
		return -1;
	}

	buf = BNetBuffer(bytes);
	buf.AppendData(data, (size_t)bytes);
	free(data);

	return bytes;
}


int32
BNetEndpoint::ReceiveFrom(void *buf, size_t len, const BNetAddress &from, int flags)
{
	if (fSocket == -1 || fLocalAddr.InitCheck() != B_OK) return -1;
	if (fProtocol != SOCK_DGRAM) return -1;

	struct sockaddr_in sa;
	if (from.GetAddr(sa) != B_OK) return -1;

	if (!BNetEndpoint::IsDataPending(fTimeout)) return -1;

	socklen_t _len = sizeof(sa);
	return recvfrom(fSocket, (char*)buf, len, flags, (struct sockaddr*)&sa, &_len);
}


int32
BNetEndpoint::ReceiveFrom(BNetBuffer &buf, size_t len, const BNetAddress &from, int flags)
{
	void *data = (len != 0 ? malloc(len) : NULL);
	if (data == NULL) return -1;

	int32 bytes = BNetEndpoint::ReceiveFrom(data, len, from, flags);
	if (bytes < 0) {
		free(data);
		return -1;
	}

	buf = BNetBuffer(bytes);
	buf.AppendData(data, (size_t)bytes);
	free(data);

	return bytes;
}


bool
BNetEndpoint::IsDataPending(bigtime_t _timeout)
{
	if (fSocket == -1) return false;

	struct timeval timeout;
	if (fNonBlocking) {
		timeout.tv_sec = 0;
		timeout.tv_usec = 0;
	} else if (_timeout != B_INFINITE_TIMEOUT) {
		timeout.tv_sec = (long)(_timeout /B_INT64_CONSTANT(1000000));
		timeout.tv_usec = (long)(_timeout %B_INT64_CONSTANT(1000000));
	}

	fd_set rset;
	FD_ZERO(&rset);
	FD_SET(fSocket, &rset);

	int status = select(fSocket + 1, &rset, NULL, NULL,
	                    (fNonBlocking || _timeout != B_INFINITE_TIMEOUT) ? &timeout : NULL);
	return(status > 0 && FD_ISSET(fSocket, &rset));
}


int
BNetEndpoint::Socket() const
{
	return fSocket;
}

