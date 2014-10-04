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
 * File: Archivable.cpp
 *
 * --------------------------------------------------------------------------*/

#include "Archivable.h"


BArchivable::BArchivable()
{
}


BArchivable::BArchivable(const BMessage *from)
{
}


BArchivable::~BArchivable()
{
}


status_t
BArchivable::Archive(BMessage *into, bool deep) const
{
	if (!into) return B_ERROR;
	into->AddString("class", "BArchivable");
	return B_OK;
}


BArchivable*
BArchivable::Instantiate(const BMessage *from)
{
	ETK_ERROR("[SUPPORT]: %s --- Can't instantiate a \"BArchivable\"", __PRETTY_FUNCTION__);
	return NULL;
}


bool e_validate_instantiation(const BMessage *from, const char *class_name)
{
	if (from == NULL || class_name == NULL || *class_name == 0) return false;

	int32 items = from->CountItems("class", B_STRING_TYPE);
	for (int32 i = 0; i < items; i++) {
		const char *_class_name = NULL;
		if (from->FindString("class", i, &_class_name) == false) continue;
		if (_class_name == NULL) continue;

		if (strlen(_class_name) != strlen(class_name)) continue;
		if (strcmp(_class_name, class_name) == 0) return true;
	}

	return false;
}


e_instantiation_func e_find_instantiation_func(const char *class_name)
{
	// TODO
	return NULL;
}


e_instantiation_func e_find_instantiation_func(const BMessage *archive_data)
{
	const char *class_name = NULL;
	if (archive_data == NULL) return NULL;
	archive_data->FindString("class", archive_data->CountItems("class", B_STRING_TYPE) - 1, &class_name);
	return e_find_instantiation_func(class_name);
}

