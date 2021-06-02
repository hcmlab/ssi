/***************************************************************************
                          german_stem.h  -  description
                             -------------------
    begin                : Sat May 18 2004
    copyright            : (C) 2004 by Blake Madden
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Lesser General Public License as        *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 ***************************************************************************/

#ifndef __GERMAN_STEM_H__
#define __GERMAN_STEM_H__

#include "Stemming.h"

namespace stemming
	{
	///todo add umlaut variant support
	/**German includes the following accented forms,
		-ä ö ü 
	and a special letter, ß, equivalent to double s. 

	The following letters are vowels:
		-a e i o u y ä ö ü*.*/
	//------------------------------------------------------
	template<typename Tchar_type = char,
			typename Tchar_traits = std::char_traits<Tchar_type> >
	class GermanStem : stem<Tchar_type, Tchar_traits>
		{
	public:
		//---------------------------------------------
		///@param text: string to stem
		///@param contract_transliterated_umlauts: apply variant algorithm
		///that contracts "ä" to "ae", ect...
		void operator()(std::basic_string<Tchar_type, Tchar_traits>& text,
						bool contract_transliterated_umlauts = false)
			{
			if (text.length() < 3)
				{
				remove_german_umlauts(text);
				return;
				}

			//reset internal data
			m_r1 = m_r2 = m_rv = 0;

			trim_western_punctuation(text);

			hash_german_yu(text, "aeiouüyäöAÄEIOÖUÜY");
			///change 'ß' to "ss"
			string_util::replace_all(text, "ß", "ss");
			///German variant addition
			if (contract_transliterated_umlauts)
				{
				string_util::replace_all(text, "ae", "ä");
				string_util::replace_all(text, "oe", "ö");
				///ue to ü, if not in front of 'q'
				size_t start = 1;
				while (start != std::basic_string<Tchar_type, Tchar_traits>::npos)
					{
					start = text.find("ue", start);
					if (start == std::basic_string<Tchar_type, Tchar_traits>::npos ||
						is_either(text[start-1], 'q', 'Q') )
						{
						break;
						}
					text.replace(start, 2, "ü");
					}
				}

			find_r1(text, "aeiouüyäöAÄEIOÖUÜY");
			if (m_r1 == text.length() )
				{
				remove_german_umlauts(text);
				unhash_german_yu(text);
				return;
				}
			find_r2(text, "aeiouüyäöAÄEIOÖUÜY");
			///R1 must have at least 3 characters in front of it
			if (m_r1 < 3)
				{
				m_r1 = 3;	
				}

			///step 1:
			step_1(text);
			///step 2:
			step_2(text);
			///step 3:
			step_3(text);

			///unhash special 'u' and 'y' back, and remove the umlaut accent from a, o and u. 
			remove_german_umlauts(text);
			unhash_german_yu(text);
			}
	private:
		/**Search for the longest among the following suffixes, 
			-#e em en ern er es
			-#s (preceded by a valid s-ending) 
		and delete if in R1. (Of course the letter of the valid s-ending is not necessarily in R1) 

		(For example, äckern -> äck, ackers -> acker, armes -> arm) */
		//---------------------------------------------
		void step_1(std::basic_string<Tchar_type, Tchar_traits>& text)
			{
			if (delete_if_is_in_r1(text,/*ern*/'e', 'E', 'r', 'R', 'n', 'N') )
				{
				return;
				}
			else if (delete_if_is_in_r1(text,/*er*/'e', 'E', 'r', 'R') )
				{
				return;
				}
			else if (delete_if_is_in_r1(text,/*es*/'e', 'E', 's', 'S') )
				{
				return;
				}
			else if (delete_if_is_in_r1(text,/*en*/'e', 'E', 'n', 'N') )
				{
				return;
				}
			else if (delete_if_is_in_r1(text,/*em*/'e', 'E', 'm', 'M') )
				{
				return;
				}
			else if (delete_if_is_in_r1(text, 'e', 'E') )
				{
				return;
				}
			///Define a valid s-ending as one of b, d, f, g, h, k, l, m, n, r or t.
			else if (is_suffix_in_r1(text, 's', 'S') )
				{
				if (string_util::is_one_of(text[text.length()-2], "bdfghklmnrtBDFGHKLMNRT") )
					{
					text.erase(text.end()-1, text.end() );
					update_r_sections(text);
					}
				return;
				}
			}
		/**Search for the longest among the following suffixes,
			-#en er est
			-#st (preceded by a valid st-ending, itself preceded by at least 3 letters) 
		and delete if in R1. 

		(For example, derbsten -> derbst by step 1, and derbst -> derb by step 2,
		because b is a valid st-ending, and is preceded by just 3 letters)*/
		//---------------------------------------------
		void step_2(std::basic_string<Tchar_type, Tchar_traits>& text)
			{
			if (delete_if_is_in_r1(text,/*est*/'e', 'E', 's', 'S', 't', 'T') )
				{
				return;
				}
			else if (delete_if_is_in_r1(text,/*er*/'e', 'E', 'r', 'R') )
				{
				return;
				}
			else if (delete_if_is_in_r1(text,/*en*/'e', 'E', 'n', 'N') )
				{
				return;
				}
			///Define a valid st-ending as the same list, excluding letter r.
			else if (text.length() >= 6 &&
					is_suffix_in_r1(text,/*st*/'s', 'S', 't', 'T') )
				{
				if (string_util::is_one_of(text[text.length()-3], "bdfghklmntBDFGHKLMNT") )
					{
					text.erase(text.end()-2, text.end() );
					update_r_sections(text);
					}
				return;
				}
			}
		/**Search for the longest among the following suffixes, and perform the action indicated. 
	
			-end ung 
				-delete if in R2 
				-if preceded by ig, delete if in R2 and not preceded by e 

			-ig ik isch 
				-delete if in R2 and not preceded by e 

			-lich heit 
				-delete if in R2 
				-if preceded by er or en, delete if in R1 

			-keit 
				-delete if in R2 
				-if preceded by lich or ig, delete if in R2*/
		//---------------------------------------------
		void step_3(std::basic_string<Tchar_type, Tchar_traits>& text) 
			{
			if (delete_if_is_in_r2(text,/*heit*/'h', 'H', 'e', 'E', 'i', 'I', 't', 'T') ||
				delete_if_is_in_r2(text,/*lich*/'l', 'L', 'i', 'I', 'c', 'C', 'h', 'H') )
				{
				if (delete_if_is_in_r1(text,/*er*/'e', 'E', 'r', 'R') ||
					delete_if_is_in_r1(text,/*en*/'e', 'E', 'n', 'N') )
					{
					return;
					}
				return;
				}
			else if (delete_if_is_in_r2(text,/*keit*/'k', 'K', 'e', 'E', 'i', 'I', 't', 'T') )
				{
				if (delete_if_is_in_r2(text,/*lich*/'l', 'L', 'i', 'I', 'c', 'C', 'h', 'H') ||
					delete_if_is_in_r2(text,/*ig*/'i', 'I', 'g', 'G') )
					{
					return;
					}
				return;
				}
			else if (is_suffix(text,/*isch*/'i', 'I', 's', 'S', 'c', 'C', 'h', 'H') )
				{
				if (m_r2 <= static_cast<int>(text.length()-4) &&
					is_neither(text[text.length()-5], 'e', 'E') )
					{
					text.erase(text.end()-4, text.end() );
					update_r_sections(text);
					}
				}
			else if (delete_if_is_in_r2(text,/*end*/'e', 'E', 'n', 'N', 'd', 'D') )
				{
				if (text.length() >= 3 &&
					is_suffix_in_r2(text,/*ig*/'i', 'I', 'g', 'G')  &&
					is_neither(text[text.length()-3], 'e', 'E') )
					{
					text.erase(text.end()-2, text.end() );
					update_r_sections(text);
					}
				return;
				}
			else if (delete_if_is_in_r2(text,/*ung*/'u', 'U', 'n', 'N', 'g', 'G') )
				{
				if (text.length() >= 3 &&
					is_suffix_in_r2(text,/*ig*/'i', 'I', 'g', 'G')  &&
					is_neither(text[text.length()-3], 'e', 'E') )
					{
					text.erase(text.end()-2, text.end() );
					update_r_sections(text);
					}
				return;
				}
			else if (is_suffix(text,/*ig*/'i', 'I', 'g', 'G') )
				{
				if (text.length() >= 3 &&
					m_r2 <= text.length()-2 &&
					is_neither(text[text.length()-3], 'e', 'E') )
					{
					text.erase(text.end()-2, text.end() );
					update_r_sections(text);
					}
				}
			else if (is_suffix(text,/*ik*/'i', 'I', 'k', 'K') )
				{
				if (text.length() >= 3 &&
					m_r2 <= text.length()-2 &&
					is_neither(text[text.length()-3], 'e', 'E') )
					{
					text.erase(text.end()-2, text.end() );
					update_r_sections(text);
					}
				}
			}
		};
	}

#endif //__GERMAN_STEM_H__
