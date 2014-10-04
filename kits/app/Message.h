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
 * File: Message.h
 * Description: Message for communication between loopers/teams
 *
 * --------------------------------------------------------------------------*/

#ifndef __ETK_MESSAGE_H__
#define __ETK_MESSAGE_H__

#include <support/SupportDefs.h>
#include <support/String.h>
#include <support/List.h>
#include <interface/Point.h>
#include <interface/Rect.h>
#include <kernel/OS.h>

#ifdef __cplusplus /* Just for C++ */

class BMessenger;
class BHandler;

class BMessage
{
	public:
		BMessage();
		BMessage(uint32 what);
		BMessage(const BMessage &msg);
		virtual ~BMessage();

		uint32		what;

		BMessage	&operator=(const BMessage &msg);

		int32		CountTypesByName(const char *name) const;
		int32		CountTypesByName(int32 nameIndex) const;
		bool		TypeAt(const char *name, int32 typeIndex, type_code *type) const;
		bool		TypeAt(int32 nameIndex, int32 typeIndex, type_code *type) const;

		// CountItems():
		// 	It don't count all items when you pass "B_ANY_TYPE" to "type",
		// 	In BMessage, "B_ANY_TYPE" IS AND JUST a type code!
		int32		CountItems(const char *name, type_code type) const;
		int32		CountItems(int32 nameIndex, int32 typeIndex, type_code *type = NULL) const;

		// CountNames():
		// 	Counts all named fields when you pass "B_ANY_TYPE" to first argument and "true" to second argument.
		// 	If you wanna iterate through all it's data, for example:
		// 	...
		// 	for(int32 i = 0; i < msg->CountNames(B_ANY_TYPE, true); i++)
		// 	{
		// 		const char *name = msg->NameAt(i);
		// 		int32 typesCount = msg->CountTypesByName(i);
		// 		for(int32 k = 0; k < typesCount; k++)
		// 		{
		// 			type_code type;
		// 			int32 count = msg->CountItems(i, k, &type);
		// 			for(int32 m = 0; m < count; m++)
		// 			{
		// 				ssize_t numBytes = 0;
		// 				const void *data = NULL;
		// 				msg->FindData(i, k, m, &data, &numBytes);
		// 				...
		// 			}
		// 		}
		// 	}
		int32		CountNames(type_code type, bool count_all_names_when_any_type = true) const;
		int32		FindName(const char *name) const;
		const char	*NameAt(int32 nameIndex) const;

		void		MakeEmpty();
		bool		IsEmpty() const;

		bool		Rename(const char *old_entry, const char *new_entry);

		bool		AddString(const char *name, const char *aString);
		bool		AddString(const char *name, const BString &aString);
		bool		AddInt8(const char *name, int8 val);
		bool		AddInt16(const char *name, int16 val);
		bool		AddInt32(const char *name, int32 val);
		bool		AddInt64(const char *name, int64 val);
		bool		AddBool(const char *name, bool aBoolean);
		bool		AddFloat(const char *name, float aFloat);
		bool		AddDouble(const char *name, double aDouble);
		bool		AddPoint(const char *name, BPoint pt);
		bool		AddRect(const char *name, BRect r);
		bool		AddPointer(const char *name, const void *ptr);
		bool		AddMessage(const char *name, const BMessage *msg);
		bool		AddMessenger(const char *name, const BMessenger *msgr);
		bool		AddMessenger(const char *name, const BMessenger &msgr);
		bool		AddData(const char *name, type_code type, const void *data, size_t numBytes, bool is_fixed_size = true);

		bool		FindString(const char *name, int32 index, const char **str) const;
		bool		FindString(const char *name, const char **str) const;
		bool		FindString(const char *name, int32 index, BString *str) const;
		bool		FindString(const char *name, BString *str) const;
		bool		FindInt8(const char *name, int8 *val) const;
		bool		FindInt8(const char *name, int32 index, int8 *val) const;
		bool		FindInt16(const char *name, int16 *val) const;
		bool		FindInt16(const char *name, int32 index, int16 *val) const;
		bool		FindInt32(const char *name, int32 *val) const;
		bool		FindInt32(const char *name, int32 index, int32 *val) const;
		bool		FindInt64(const char *name, int64 *val) const;
		bool		FindInt64(const char *name, int32 index, int64 *val) const;
		bool		FindBool(const char *name, bool *aBoolean) const;
		bool		FindBool(const char *name, int32 index, bool *aBoolean) const;
		bool		FindFloat(const char *name, float *f) const;
		bool		FindFloat(const char *name, int32 index, float *f) const;
		bool		FindDouble(const char *name, double *d) const;
		bool		FindDouble(const char *name, int32 index, double *d) const;
		bool		FindPoint(const char *name, BPoint *pt) const;
		bool		FindPoint(const char *name, int32 index, BPoint *pt) const;
		bool		FindRect(const char *name, BRect *r) const;
		bool		FindRect(const char *name, int32 index, BRect *r) const;
		bool		FindPointer(const char *name, void **ptr) const;
		bool		FindPointer(const char *name, int32 index, void **ptr) const;
		bool		FindMessage(const char *name, BMessage *msg) const;
		bool		FindMessage(const char *name, int32 index, BMessage *msg) const;
		bool		FindMessenger(const char *name, BMessenger *msgr) const;
		bool		FindMessenger(const char *name, int32 index, BMessenger *msgr) const;
		bool		FindData(const char *name, type_code type, const void **data, ssize_t *numBytes) const;
		bool		FindData(const char *name, type_code type, int32 index, const void **data, ssize_t *numBytes) const;
		bool		FindData(int32 nameIndex, int32 typeIndex, int32 index, const void **data, ssize_t *numBytes) const;

		bool		HasString(const char *name, int32 index = 0) const;
		bool		HasInt8(const char *name, int32 index = 0) const;
		bool		HasInt16(const char *name, int32 index = 0) const;
		bool		HasInt32(const char *name, int32 index = 0) const;
		bool		HasInt64(const char *name, int32 index = 0) const;
		bool		HasBool(const char *name, int32 index = 0) const;
		bool		HasFloat(const char *name, int32 index = 0) const;
		bool		HasDouble(const char *name, int32 index = 0) const;
		bool		HasPoint(const char *name, int32 index = 0) const;
		bool		HasRect(const char *name, int32 index = 0) const;
		bool		HasPointer(const char *name, int32 index = 0) const;
		bool		HasMessage(const char *name, int32 index = 0) const;
		bool		HasMessenger(const char *name, int32 index = 0) const;
		bool		HasData(const char *name, type_code type, int32 index = 0) const;

		bool		RemoveString(const char *name, int32 index = 0);
		bool		RemoveInt8(const char *name, int32 index = 0);
		bool		RemoveInt16(const char *name, int32 index = 0);
		bool		RemoveInt32(const char *name, int32 index = 0);
		bool		RemoveInt64(const char *name, int32 index = 0);
		bool		RemoveBool(const char *name, int32 index = 0);
		bool		RemoveFloat(const char *name, int32 index = 0);
		bool		RemoveDouble(const char *name, int32 index = 0);
		bool		RemovePoint(const char *name, int32 index = 0);
		bool		RemoveRect(const char *name, int32 index = 0);
		bool		RemovePointer(const char *name, int32 index = 0);
		bool		RemoveMessage(const char *name, int32 index = 0);
		bool		RemoveMessenger(const char *name, int32 index = 0);
		bool		RemoveData(const char *name, type_code type, int32 index = 0);
		bool		RemoveData(const char *name, type_code type);
		bool		RemoveData(const char *name);

		bool		ReplaceString(const char *name, int32 index, const char *aString);
		bool		ReplaceString(const char *name, const char *aString);
		bool		ReplaceString(const char *name, int32 index, const BString &aString);
		bool		ReplaceString(const char *name, const BString &aString);
		bool		ReplaceInt8(const char *name, int8 val);
		bool		ReplaceInt8(const char *name, int32 index, int8 val);
		bool		ReplaceInt16(const char *name, int16 val);
		bool		ReplaceInt16(const char *name, int32 index, int16 val);
		bool		ReplaceInt32(const char *name, int32 val);
		bool		ReplaceInt32(const char *name, int32 index, int32 val);
		bool		ReplaceInt64(const char *name, int64 val);
		bool		ReplaceInt64(const char *name, int32 index, int64 val);
		bool		ReplaceBool(const char *name, bool aBoolean);
		bool		ReplaceBool(const char *name, int32 index, bool aBoolean);
		bool		ReplaceFloat(const char *name, float f);
		bool		ReplaceFloat(const char *name, int32 index, float f);
		bool		ReplaceDouble(const char *name, double d);
		bool		ReplaceDouble(const char *name, int32 index, double d);
		bool		ReplacePoint(const char *name, BPoint pt);
		bool		ReplacePoint(const char *name, int32 index, BPoint pt);
		bool		ReplaceRect(const char *name, BRect r);
		bool		ReplaceRect(const char *name, int32 index, BRect r);
		bool		ReplacePointer(const char *name, const void *ptr);
		bool		ReplacePointer(const char *name, int32 index, const void *ptr);
		bool		ReplaceMessage(const char *name, const BMessage *msg);
		bool		ReplaceMessage(const char *name, int32 index, const BMessage *msg);
		bool		ReplaceMessenger(const char *name, const BMessenger *msgr);
		bool		ReplaceMessenger(const char *name, int32 index, const BMessenger *msgr);
		bool		ReplaceMessenger(const char *name, const BMessenger &msgr);
		bool		ReplaceMessenger(const char *name, int32 index, const BMessenger &msgr);
		bool		ReplaceData(const char *name, type_code type, const void *data, size_t numBytes, bool is_fixed_size);
		bool		ReplaceData(const char *name, type_code type, int32 index, const void *data, size_t numBytes, bool is_fixed_size);

		void		PrintToStream() const;

		size_t		FlattenedSize() const;
		bool		Flatten(char *buffer, size_t bufferSize) const;
		bool		Unflatten(const char *buffer, size_t bufferSize);

		bool		WasDelivered() const;
		bool		IsReply() const;
		bool		IsSourceWaiting() const;

		status_t	SendReply(uint32 command, BHandler *replyHandler = NULL) const;
		status_t	SendReply(const BMessage *message,
		                   BHandler *replyHandler = NULL,
		                   bigtime_t sendTimeout = B_INFINITE_TIMEOUT) const;

		/* GetInfo()/__FindData(): likes BMessage */
		status_t	GetInfo(type_code type, int32 index,
		                 char **nameFound, type_code *typeFound, int32 *countFound = NULL) const;
		status_t	__FindData(const char *name, type_code type, int32 index,
		                    const void **data, ssize_t *numBytes) const;
		status_t	__FindData(const char *name, type_code type,
		                    const void **data, ssize_t *numBytes) const;

	private:
		friend class BLooper;
		friend class BMessenger;

		typedef struct list_data {
			char 		*name;
			BList		list;
		} list_data;

		typedef struct type_list_data {
			type_code	type;
			BList		list;
		} type_list_data;

		typedef struct _object_t {
			size_t		bytes;
			bool		fixed_size;
			void		*data;
		} _object_t;

		BList fObjectsList;

		static list_data *_find_list(const BList *list, const char *name, int32 *index = NULL);
		static type_list_data *_find_type_list(const BList *list, type_code type);

		int64 fTeam;

		uint64 fTargetToken;
		bigtime_t fTargetTokenTimestamp;
		uint64 fReplyToken;
		bigtime_t fReplyTokenTimestamp;

		bool fNoticeSource;
		void *fSource;

		bool fIsReply;
};


inline status_t
BMessage::GetInfo(type_code type, int32 index,
                  char **nameFound, type_code *typeFound, int32 *countFound) const
{
	if (index < 0) return B_BAD_INDEX;
	int32 aIndex = index;

	for (int32 i = 0; i < CountNames(B_ANY_TYPE, true); i++) {
		int32 typesCount = CountTypesByName(i);
		for (int32 k = 0; k < typesCount; k++) {
			type_code aType;
			int32 count = CountItems(i, k, &aType);
			if (!(type == B_ANY_TYPE || aType == type) || (aIndex--) > 0) continue;
			if (nameFound) *nameFound = (char*)NameAt(i);
			if (typeFound) *typeFound = aType;
			if (countFound) *countFound = count;
			return B_OK;
		}
	}

	return(aIndex == index ?B_BAD_TYPE :B_BAD_INDEX);
}


inline status_t
BMessage::__FindData(const char *name, type_code type, int32 index,
                     const void **data, ssize_t *numBytes) const
{
	if (index < 0) return B_BAD_INDEX;

	int32 nameIndex = FindName(name);
	if (nameIndex < 0) return B_NAMB_NOT_FOUND;

	int32 typesCount = CountTypesByName(nameIndex);
	int32 aIndex = index;

	for (int32 k = 0; k < typesCount; k++) {
		type_code aType;
		int32 count = CountItems(nameIndex, k, &aType);
		if (!(type == B_ANY_TYPE || aType == type)) continue;

		if (aIndex < count)
			return(FindData(nameIndex, k, aIndex, data, numBytes) ?B_OK :B_ERROR);

		aIndex -= count;
	}

	return(aIndex == index ?B_BAD_TYPE :B_BAD_INDEX);
}


inline status_t
BMessage::__FindData(const char *name, type_code type,
                     const void **data, ssize_t *numBytes) const
{
	return __FindData(name, type, 0, data, numBytes);
}

#endif /* __cplusplus */

#endif /* __ETK_MESSAGE_H__ */

