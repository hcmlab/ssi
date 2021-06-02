/***************************************************************************
                          string_util.h  -  description
                             -------------------
    begin                : Sat Apr 26 2003
    copyright            : (C) 2003 by Blake Madden
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef __STRING_UTIL_H__
#define __STRING_UTIL_H__

#include <cwctype>
#include <cstring>
#include <cwchar>
#include <string>
#include <cstdlib>
#include <functional>
#include <cassert>
#include "meta.h"

namespace string_util
	{
	template<typename Tchar_type>
	bool is_one_of(const Tchar_type character, const Tchar_type* char_string)
		{
		while(*char_string)
			{
			if (character == char_string[0])
				{
				return true;
				}
			++char_string;
			}
		return false;
		}

	template<typename T, typename Tchar_type>
	void replace_all(T& text, Tchar_type text_to_replace, Tchar_type replacement_text)
		{
		size_t start = 0;
		while (start != T::npos)
			{
			start = text.find(text_to_replace, start);
			if (start == T::npos)
				{
				return;
				}
			text[start++] = replacement_text;
			}
		}

	template<typename T, typename Tchar_type>
	void replace_all(T& text, const Tchar_type* text_to_replace, const Tchar_type* replacement_text)
		{
		size_t start = 0;
		while (start != T::npos)
			{
			start = text.find(text_to_replace, start);
			if (start == T::npos)
				{
				return;
				}
			text.replace(start, strlen(text_to_replace), replacement_text);
			}
		}

	template<typename T>
	void remove_control_characters(T& text)
		{
		for (size_t i = 0; i < text.length(); ++i)
			{
			if (text[i] >= 0 && text[i] <= 31)
				{
				text.erase(i, 1);
				--i;
				}
			}
		}

	//functions not available in ANSI C
	//search for substring in string (case-insensitive)
	template<typename T>
	T* stristr(const T* string, const T* strSearch)
		{
		assert(string); assert(strSearch);
		if (!string)
			{
			return NULL;
			}
		size_t substring_len = string_util::strlen(strSearch);
		size_t string_len = string_util::strlen(string);
		while (string)
			{
			if (string_len < substring_len)
				{
				return NULL;
				}
			//compare the characters one at a time
			size_t i = 0;
			for (i = 0; i < substring_len; ++i)
				{
				if (strSearch[i] != string[i] &&
					toupper(strSearch[i]) != toupper(string[i]) )
					{
					++string;
					--string_len;
					break;
					}
				}
			//if the substring loop completed then the substring was found
			if (i == substring_len)
				{
				return const_cast<T*>(string);
				}
			}
		return NULL;
		}

	//Case-insensitive comparison by character count
	template<typename T>
	int strnicmp(const T* first, const T* last, size_t count)
		{
		int f,l;
		int result = 0;

		if (count)
			{
			do
				{
				f = tolower(*(first++) );
				l = tolower(*(last++) );
				} while ( (--count) && f && (f == l) );
			result = static_cast<int>(f - l);
			}
		return result;
		}

	///Case-insensitve comparison
	template<typename T>
	int stricmp(const T* dst, const T* src)
		{
		int f,l;
		do
			{
			f = tolower(*(dst++) );
			l = tolower(*(src++) );
            } while (f && (f == l) );

		return static_cast<int>(f - l);
		}

	///lowercases any Western European alphabetic characters
	inline char tolower_western(const char c)
		{
		return ((c >= 'A') && (c <= 'Z')) ||
			((c >= sign_char(0xC0)) && (c <= sign_char(0xD6))) ||
			((c >= sign_char(0xD8)) && (c <= sign_char(0xDE))) 
				? (c + 32) : c;
		}
	inline wchar_t tolower_western(const wchar_t c)
		{
		return ((c >= L'A') && (c <= L'Z')) ||
			((c >= sign_char(0xC0)) && (c <= sign_char(0xD6))) ||
			((c >= sign_char(0xD8)) && (c <= sign_char(0xDE))) 
				? (c + 32) : c;
		}
	///uppercases any Western European alphabetic characters
	inline char toupper_western(const char c)
		{
		return ((c >= 'a') && (c <= 'z')) ||
			((c >= sign_char(0xE0)) && (c <= sign_char(0xF6))) ||
			((c >= sign_char(0xF8)) && (c <= sign_char(0xFE)))  
				? (c - 32) : c;
		}
	inline wchar_t toupper_western(const wchar_t c)
		{
		return ((c >= L'a') && (c <= L'z')) ||
			((c >= sign_char(0xE0)) && (c <= sign_char(0xF6))) ||
			((c >= sign_char(0xF8)) && (c <= sign_char(0xFE)))   
				? (c - 32) : c;
		}

	//ANSI C decorators
	//tolower
	inline char tolower(char c)
		{
		return (((c) >= 'A') && ((c) <= 'Z')) ? ((c) - 'A' + 'a') : (c);
		}
	inline wchar_t tolower(wchar_t c)
		{
		return (((c) >= L'A') && ((c) <= L'Z')) ? ((c) - L'A' + L'a') : (c);
		}

	//toupper
	inline char toupper(char c)
		{
		return (((c) >= 'a') && ((c) <= 'z')) ? ((c) - 'a' + 'A') : (c);
		}
	inline wchar_t toupper(wchar_t c)
		{
		return (((c) >= L'a') && ((c) <= L'z')) ? ((c) - L'a' + L'A') : (c);
		}

	//strchr
	inline const char* strchr(const char* s, int ch)
		{
		return std::strchr(s, ch);
		}
	inline const wchar_t* strchr(const wchar_t* s, wchar_t ch)
		{
        return std::wcschr(s, ch);
		}

	//strstr
	inline const char* strstr(const char* s1, const char* s2)
		{
		return std::strstr(s1, s2);
		}
	inline const wchar_t* strstr(const wchar_t* s1, const wchar_t* s2)
		{
        return std::wcsstr(s1, s2);
		}

	//strcspn
	inline size_t strcspn(const char* string1,const char* string2 )
		{
		return std::strcspn(string1, string2);
		}
	inline size_t strcspn(const wchar_t* string1,const wchar_t* string2 )
		{
		return std::wcscspn(string1, string2);
		}

	//strncat
	inline char* strncat(char* strDest, const char* strSource, size_t count)
		{
		return std::strncat(strDest, strSource, count);
		}
	inline wchar_t* strncat(wchar_t* strDest, const wchar_t* strSource, size_t count)
		{
        return std::wcsncat(strDest, strSource, count);
		}

	//wctomb
	inline int wctomb(wchar_t* s, wchar_t wc)
		{
		s[0] = wc;
		return -1;
		}
	inline int wctomb(char* s, wchar_t wc)
		{
		return std::wctomb(s, wc);
		}

	//strlen
	inline size_t strlen(const char* text) { return std::strlen(text); }
	inline size_t strlen(const wchar_t* text)
		{ return std::wcslen(text); }
	//strncmp
	inline int strncmp(const char* string1, const char* string2, size_t count)
		{
		return std::strncmp(string1, string2, count);
		}
	inline int strncmp(const wchar_t* string1, const wchar_t* string2, size_t count)
		{
		return std::wcsncmp(string1, string2, count);
		}

	//atoi
	inline int atoi(const char* ptr)
		{
		return std::atoi(ptr);
		}
	inline int atoi(const wchar_t* ptr)
		{
		char* temp_chars = new char[(2*(wcslen(ptr)+1))];
		wcstombs(temp_chars, (const wchar_t*)ptr, (2*(std::wcslen(ptr)+1)));
		int num = atoi(temp_chars);
		delete [] temp_chars;
		return num;
		}
	//strncpy
	inline char* strncpy(char* strDest, const char* strSource, size_t count)
		{
		return std::strncpy(strDest, strSource, count);
		}
	inline wchar_t* strncpy(wchar_t* strDest, const wchar_t* strSource, size_t count)
		{
		return std::wcsncpy(strDest, strSource, count);
		}

	//utility classes
	template<typename T>
	class less_string_n_compare
		: public std::binary_function<const T*, const T*, bool>
		{
	public:
		less_string_n_compare(size_t comparison_size) : m_comparison_size(comparison_size) {}
		inline bool operator()(const T* a_, const T* b_) const
			{
			return (string_util::strncmp(a_, b_, m_comparison_size) < 0);
			}
	private:
		size_t m_comparison_size;
		};

	template<typename T>
	class less_string_ni_compare
		: public std::binary_function<const T*, const T*, bool>
		{
	public:
		less_string_ni_compare(size_t comparison_size) : m_comparison_size(comparison_size) {}
		inline bool operator()(const T* a_, const T* b_) const
			{
			return (string_util::strnicmp(a_, b_, m_comparison_size) < 0);
			}
	private:
		size_t m_comparison_size;
		};

	template<typename T>
	class less_string_i_compare
		: public std::binary_function<const T*, const T*, bool>
		{
	public:
		inline bool operator()(const T* a_, const T* b_) const
			{
			return (string_util::stricmp(a_, b_) < 0);
			}
		};
	}

#endif //__STRING_UTIL_H__
