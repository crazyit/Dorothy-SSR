/* Copyright (c) 2016 Jin Li, http://www.luvfight.me

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#pragma once

NS_DOROTHY_BEGIN

/* Content */
void __Content_loadFile(lua_State* L, Content* self, const char* filename);
#define Content_loadFile(self,filename) {__Content_loadFile(tolua_S,self,filename);return 1;}
void __Content_getDirEntries(lua_State* L, Content* self, const char* path, bool isFolder);
#define Content_getDirEntries(self,path,isFolder) {__Content_getDirEntries(tolua_S,self,path,isFolder);return 1;}
void Content_setSearchPaths(Content* self, char* paths[], int length);
inline Content* Content_shared() { return &SharedContent; }

NS_DOROTHY_END
