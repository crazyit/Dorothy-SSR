/* Copyright (c) 2016 Jin Li, http://www.luvfight.me

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#pragma once

#include "Common/WRef.h"

NS_DOROTHY_BEGIN

/** @brief Used with Aggregation Relationship. */
template<class T = Object>
class WRefVector: public vector<WRef<T>>
{
	typedef vector<WRef<T>> WRefV;
public:
	inline void push_back(T* item)
	{
		WRefV::push_back(oWRefMake(item));
	}
	bool insert(size_t where, T* item)
	{
		if (where >= 0 && where < WRefV::size())
		{
			auto it = WRefV::begin();
			for (int i = 0; i < where; ++i, ++it);
			WRefV::insert(it, WRefMake(item));
			return true;
		}
		return false;
	}
	bool remove(T* item)
	{
		for (auto it = WRefV::begin(); it != WRefV::end(); it++)
		{
			if ((*it) == item)
			{
				WRefV::erase(it);
				return true;
			}
		}
		return false;
	}
	bool fast_remove(T* item)
	{
		int size = WRefV::size();
		WRef<T>* data = data();
		for (int i = 0; i < size; i++)
		{
			if (data[i] == item)
			{
				data[i] = data[size - 1];
				WRefV::pop_back();
				return true;
			}
		}
		return false;
	}
};

NS_DOROTHY_END
