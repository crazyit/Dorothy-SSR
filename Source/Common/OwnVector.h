/* Copyright (c) 2016 Jin Li, http://www.luvfight.me

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#pragma once

#include "Common/Own.h"

NS_DOROTHY_BEGIN

/** @brief vector of pointers, but accessed as values
 pointers pushed into OwnVector are owned by the vector,
 pointers will be auto deleted when it`s erased/removed from the vector
 or the vector is destroyed.
 Used with Composition Relationship.
*/
template<class T>
class OwnVector: public vector<Own<T>>
{
	typedef vector<Own<T>> OwnV;
public:
	inline void push_back(T* item)
	{
		OwnV::push_back(oOwnMake(item));
	}
	inline bool insert(size_t where, T* item)
	{
		if (where >= 0 && where < OwnV::size())
		{
			auto it = OwnV::begin();
			for (int i = 0; i < where; ++i, ++it);
			OwnV::insert(it, oOwnMake(item));
			return true;
		}
		return false;
	}
	bool remove(T* item)
	{
		for (auto it = OwnV::begin(); it != OwnV::end(); ++it)
		{
			if ((*it) == item)
			{
				OwnV::erase(it);
				return true;
			}
		}
		return false;
	}
};

NS_DOROTHY_END
