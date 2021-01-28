/***************************************************************************
                          english_stem.h  -  description
                             -------------------
    begin                : Sat May 25 2004
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

#ifndef __ENGLISH_STEM_H__
#define __ENGLISH_STEM_H__

#include "stemming.h"

namespace stemming
	{
	/**Overview

	I have made more than one attempt to improve the structure of the Porter algorithm 
	by making it follow the pattern of ending removal of the Romance language stemmers.
	It is not hard to see why one should want to do this: step 1b of the Porter stemmer 
	removes ed and ing, which are i-suffixes (*) attached to verbs. If these suffixes are
	removed, there should be no need to remove d-suffixes which are not verbal, although it
	will try to do so. This seems to be a deficiency in the Porter stemmer, not shared by
	the Romance stemmers. Again, the divisions between steps 2, 3 and 4 seem rather arbitrary,
	and are not found in the Romance stemmers. 

	Nevertheless, these attempts at improvement have been abandoned. They seem to lead to a
	more complicated algorithm with no very obvious improvements. A reason for not taking
	note of the outcome of step 1b may be that English endings do not determine word categories
	quite as strongly as endings in the Romance languages. For example, condition and position
	in French have to be nouns, but in English they can be verbs as well as nouns,

	We are all conditioned by advertising
	They are positioning themselves differently today

	A possible reason for having separate steps 2, 3 and 4 is that d-suffix combinations in
	English are quite complex, a point which has been made elsewhere. 

	But it is hardly surprising that after twenty years of use of the Porter stemmer, certain
	improvements do suggest themselves, and a new algorithm for English is therefore offered
	here. (It could be called the ‘Porter2’ stemmer to distinguish it from the Porter stemmer,
	from which it derives.) The changes are not so very extensive: (1) terminating y is changed
	to i rather less often, (2) suffix us does not lose its s, (3) a few additional suffixes
	are included for removal, including (4) suffix ly. In addition, a small list of exceptional
	forms is included. In December 2001 there were two further adjustments: (5) Steps 5a and 5b
	of the old Porter stemmer were combined into a single step. This means that undoubling final
	ll is not done with removal of final e. (6) In Step 3 ative is removed only when in region R2. 

	To begin with, here is the basic algorithm without reference to the exceptional forms.
	An exact comparison with the Porter algorithm needs to be done quite carefully if done at
	all. Here we indicate by * points of departure, and by + additional features.
	In the sample vocabulary, Porter and Porter2 stem slightly under 5% of words to different forms.
	
	Dr. Martin Porter*/

	/**Define a vowel as one of 
		-a e i o u y 
	
	Define a double as one of 
		-bb dd ff gg mm nn pp rr tt 
		
	Define a valid li-ending as one of 
		-c d e g h k m n r t 
		
	Define a short syllable in a word as either (a) a vowel followed by a non-vowel
	other than w, x or Y and preceded by a non-vowel, or * (b) a vowel at the beginning
	of the word followed by a non-vowel. 

	So rap, trap, entrap end with a short syllable, and ow, on, at are classed as short syllables.
	But uproot, bestow, disturb do not end with a short syllable. 

	A word is called short if it consists of a short syllable preceded by zero or more consonants. 

	R1 is the region after the first non-vowel following a vowel, or the end of the word if there is no such non-vowel. 

	R2 is the region after the first non-vowel following a vowel in R1, or the end of the word if there is no such non-vowel.

	If the word has two letters or less, leave it as it is. 

	Otherwise, do each of the following operations, 

	Set initial y, or y after a vowel, to Y, and then establish the regions R1 and R2.*/
	//------------------------------------------------------
	template<typename Tchar_type = char,
			typename Tchar_traits = std::char_traits<Tchar_type> >
	class EnglishStem : stem<Tchar_type,Tchar_traits>
		{	
	public:
		EnglishStem() : m_first_vowel(0)
			{}
		//---------------------------------------------
		void operator()(std::basic_string<Tchar_type, Tchar_traits>& text)
			{
			if (text.length() < 3)
				{
				return;
				}

			//reset internal data
			m_first_vowel = 0;
			m_r1 = m_r2 = m_rv =0;

			trim_western_punctuation(text);

			//handle exceptions first			
			if (is_exception(text) )
				{
				return;
				}

			hash_y(text, "aeiouyAEIOUY");
			m_first_vowel = text.find_first_of("aeiouyAEIOUY");

			if (text.length() >= 5 &&
				/*gener*/
				(is_either(text[0], 'g', 'G') &&
					is_either(text[1], 'e', 'E') &&
					is_either(text[2], 'n', 'N') &&
					is_either(text[3], 'e', 'E') &&
					is_either(text[4], 'r', 'R') ) )
				{
				m_r1 = 5;
				}
			else
				{
				find_r1(text, "aeiouyAEIOUY");
				}

			find_r2(text, "aeiouyAEIOUY");

			//step 1a:
			step_1a(text);
			//exception #2
			if (is_exception_post_step1a(text) )
				{
				return;
				}
			//step 1b:
			step_1b(text);
			//step 1c:
			step_1c(text);
			//step 2:
			step_2(text);
			//step 3:
			step_3(text);
			//step 4:
			step_4(text);
			//step 5:
			step_5(text);

			unhash_y(text);
			}
	private:
		//---------------------------------------------
		bool is_exception(std::basic_string<Tchar_type, Tchar_traits>& text)
			{
			//exception #0
			/*skis*/
			if (text.length() == 4 &&
				is_either(text[0], 's', 'S') &&
				is_either(text[1], 'k', 'K') &&
				is_either(text[2], 'i', 'I') &&
				is_either(text[3], 's', 'S') )
				{
				text = "ski";
				return true;
				}
			/*skies*/
			else if (text.length() == 5 &&
					is_either(text[0], 's', 'S') &&
					is_either(text[1], 'k', 'K') &&
					is_either(text[2], 'i', 'I') &&
					is_either(text[3], 'e', 'E') &&
					is_either(text[4], 's', 'S') )
				{
				text = "sky";
				return true;
				}
			/*dying*/
			else if (text.length() == 5 &&
					is_either(text[0], 'd', 'D') &&
					is_either(text[1], 'y', 'Y') &&
					is_either(text[2], 'i', 'I') &&
					is_either(text[3], 'n', 'N') &&
					is_either(text[4], 'g', 'G') )
				{
				text = "die";
				return true;
				}
			/*lying*/
			else if (text.length() == 5 &&
					is_either(text[0], 'l', 'L') &&
					is_either(text[1], 'y', 'Y') &&
					is_either(text[2], 'i', 'I') &&
					is_either(text[3], 'n', 'N') &&
					is_either(text[4], 'g', 'G') )
				{
				text = "lie";
				return true;
				}
			/*tying*/
			else if (text.length() == 5 &&
					is_either(text[0], 't', 'T') &&
					is_either(text[1], 'y', 'Y') &&
					is_either(text[2], 'i', 'I') &&
					is_either(text[3], 'n', 'N') &&
					is_either(text[4], 'g', 'G') )
				{
				text = "tie";
				return true;
				}
			/*idly*/
			else if (text.length() == 4 &&
					is_either(text[0], 'i', 'I') &&
					is_either(text[1], 'd', 'D') &&
					is_either(text[2], 'l', 'L') &&
					is_either(text[3], 'y', 'Y') )
				{
				text = "idl";
				return true;
				}
			/*gently*/
			else if (text.length() == 6 &&
					is_either(text[0], 'g', 'G') &&
					is_either(text[1], 'e', 'E') &&
					is_either(text[2], 'n', 'N') &&
					is_either(text[3], 't', 'T') &&
					is_either(text[4], 'l', 'L') &&
					is_either(text[5], 'y', 'Y') )
				{
				text = "gentl";
				return true;
				}
			/*ugly*/
			else if (text.length() == 4 &&
					is_either(text[0], 'u', 'U') &&
					is_either(text[1], 'g', 'G') &&
					is_either(text[2], 'l', 'L') &&
					is_either(text[3], 'y', 'Y') )
				{
				text = "ugli";
				return true;
				}
			/*early*/
			else if (text.length() == 5 &&
					is_either(text[0], 'e', 'E') &&
					is_either(text[1], 'a', 'A') &&
					is_either(text[2], 'r', 'R') &&
					is_either(text[3], 'l', 'L') &&
					is_either(text[4], 'y', 'Y') )
				{
				text = "earli";
				return true;
				}
			/*only*/
			else if (text.length() == 4 &&
					is_either(text[0], 'o', 'O') &&
					is_either(text[1], 'n', 'N') &&
					is_either(text[2], 'l', 'L') &&
					is_either(text[3], 'y', 'Y') )
				{
				text = "onli";
				return true;
				}
			/*singly*/
			else if (text.length() == 6 &&
					is_either(text[0], 's', 'S') &&
					is_either(text[1], 'i', 'I') &&
					is_either(text[2], 'n', 'N') &&
					is_either(text[3], 'g', 'G') &&
					is_either(text[4], 'l', 'L') &&
					is_either(text[5], 'y', 'Y') )
				{
				text = "singl";
				return true;
				}
			//exception #1
			else if (
				/*sky*/
				(text.length() == 3 &&
					is_either(text[0], 's', 'S') &&
					is_either(text[1], 'k', 'K') &&
					is_either(text[2], 'y', 'Y') ) ||
				/*news*/
				(text.length() == 4 &&
					is_either(text[0], 'n', 'N') &&
					is_either(text[1], 'e', 'E') &&
					is_either(text[2], 'w', 'W') &&
					is_either(text[3], 's', 'S') ) ||
				/*howe*/
				(text.length() == 4 &&
					is_either(text[0], 'h', 'H') &&
					is_either(text[1], 'o', 'O') &&
					is_either(text[2], 'w', 'W') &&
					is_either(text[3], 'e', 'E') ) ||
				/*atlas*/
				(text.length() == 5 &&
					is_either(text[0], 'a', 'A') &&
					is_either(text[1], 't', 'T') &&
					is_either(text[2], 'l', 'L') &&
					is_either(text[3], 'a', 'A') &&
					is_either(text[4], 's', 'S') ) ||
				/*cosmos*/
				(text.length() == 6 &&
					is_either(text[0], 'c', 'C') &&
					is_either(text[1], 'o', 'O') &&
					is_either(text[2], 's', 'S') &&
					is_either(text[3], 'm', 'M') &&
					is_either(text[4], 'o', 'O') &&
					is_either(text[5], 's', 'S') ) ||
				/*bias*/
				(text.length() == 4 &&
					is_either(text[0], 'b', 'B') &&
					is_either(text[1], 'i', 'I') &&
					is_either(text[2], 'a', 'A') &&
					is_either(text[3], 's', 'S') ) ||
				/*andes*/
				(text.length() == 5 &&
					is_either(text[0], 'a', 'A') &&
					is_either(text[1], 'n', 'N') &&
					is_either(text[2], 'd', 'D') &&
					is_either(text[3], 'e', 'E') &&
					is_either(text[4], 's', 'S') ) )
				{
				return true;
				}
			return false;
			}

		//---------------------------------------------
		bool is_exception_post_step1a(std::basic_string<Tchar_type, Tchar_traits>& text)
			{
			//exception #2
			if (/*inning*/
				(text.length() == 6 &&
					is_either(text[0], 'i', 'I') &&
					is_either(text[1], 'n', 'N') &&
					is_either(text[2], 'n', 'N') &&
					is_either(text[3], 'i', 'I') &&
					is_either(text[4], 'n', 'N') &&
					is_either(text[5], 'g', 'G') ) ||
				/*outing*/
				(text.length() == 6 &&
					is_either(text[0], 'o', 'O') &&
					is_either(text[1], 'u', 'U') &&
					is_either(text[2], 't', 'T') &&
					is_either(text[3], 'i', 'I') &&
					is_either(text[4], 'n', 'N') &&
					is_either(text[5], 'g', 'G') ) ||
				/*canning*/
				(text.length() == 7 &&
					is_either(text[0], 'c', 'C') &&
					is_either(text[1], 'a', 'A') &&
					is_either(text[2], 'n', 'N') &&
					is_either(text[3], 'n', 'N') &&
					is_either(text[4], 'i', 'I') &&
					is_either(text[5], 'n', 'N') &&
					is_either(text[6], 'g', 'G') ) ||
				/*herring*/
				(text.length() == 7 &&
					is_either(text[0], 'h', 'H') &&
					is_either(text[1], 'e', 'E') &&
					is_either(text[2], 'r', 'R') &&
					is_either(text[3], 'r', 'R') &&
					is_either(text[4], 'i', 'I') &&
					is_either(text[5], 'n', 'N') &&
					is_either(text[6], 'g', 'G') ) ||
				/*earring*/
				(text.length() == 7 &&
					is_either(text[0], 'e', 'E') &&
					is_either(text[1], 'a', 'A') &&
					is_either(text[2], 'r', 'R') &&
					is_either(text[3], 'r', 'R') &&
					is_either(text[4], 'i', 'I') &&
					is_either(text[5], 'n', 'N') &&
					is_either(text[6], 'g', 'G') ) ||
				/*proceed*/
				(text.length() == 7 &&
					is_either(text[0], 'p', 'P') &&
					is_either(text[1], 'r', 'R') &&
					is_either(text[2], 'o', 'O') &&
					is_either(text[3], 'c', 'C') &&
					is_either(text[4], 'e', 'E') &&
					is_either(text[5], 'e', 'E') &&
					is_either(text[6], 'd', 'D') ) ||
				/*exceed*/
				(text.length() == 6 &&
					is_either(text[0], 'e', 'E') &&
					is_either(text[1], 'x', 'X') &&
					is_either(text[2], 'c', 'C') &&
					is_either(text[3], 'e', 'E') &&
					is_either(text[4], 'e', 'E') &&
					is_either(text[5], 'd', 'D') ) ||
				/*succeed*/
				(text.length() == 7 &&
					is_either(text[0], 's', 'S') &&
					is_either(text[1], 'u', 'U') &&
					is_either(text[2], 'c', 'C') &&
					is_either(text[3], 'c', 'C') &&
					is_either(text[4], 'e', 'E') &&
					is_either(text[5], 'e', 'E') &&
					is_either(text[6], 'd', 'D') ) )
				{
				return true;
				}
			return false;
			}
		/**Search for the longest among the following suffixes, and perform the action indicated. 

			-sses 
				-replace by ss 

			-ied+   ies* 
				-replace by ie if preceded by just one letter, otherwise by i (so ties -> tie, cries -> cri) 

			-s 
				-delete if the preceding word part contains a vowel not immediately before the s (so gas and this retain the s, gaps and kiwis lose it) 

			-us+   ss 
				-do nothing*/
		//---------------------------------------------
		void step_1a(std::basic_string<Tchar_type, Tchar_traits>& text)
			{
			if (is_suffix(text,/*sses*/'s', 'S', 's', 'S', 'e', 'E', 's', 'S') )
				{
				text.erase(text.end()-2, text.end() );
				update_r_sections(text);
				}
			else if (is_suffix(text,/*ied*/'i', 'I', 'e', 'E', 'd', 'D') ||
					is_suffix(text,/*ies*/'i', 'I', 'e', 'E', 's', 'S') )
				{
				if (text.length() == 4)
					{
					text.erase(text.end()-1, text.end() );
					update_r_sections(text);
					}
				else
					{
					text.erase(text.end()-2, text.end() );
					update_r_sections(text);
					}
				}
			else if (is_either(text[text.length()-1], 's', 'S') &&
					m_first_vowel < text.length()-2 &&
					!string_util::is_one_of(text[text.length()-2], "suSU") )
				{
				text.erase(text.end()-1, text.end() );
				update_r_sections(text);
				}
			}
		/**Search for the longest among the following suffixes, and perform the action indicated. 
		
			-eed   eedly+ 
				-replace by ee if in R1 

			-ed   edly+   ing   ingly+ 
				-delete if the preceding word part contains a vowel, and then 
				-if the word ends at, bl or iz add e (so luxuriat -> luxuriate), or 
				-if the word ends with a double remove the last letter (so hopp -> hop), or 
				-if the word is short, add e (so hop -> hope)*/
		//---------------------------------------------
		void step_1b(std::basic_string<Tchar_type, Tchar_traits>& text)
			{
			//if the preceding word contains a vowel
			bool regress_trim = false;

			if (is_suffix(text,/*eed*/'e', 'E', 'e', 'E', 'd', 'D') )
				{
				if (m_r1 <= text.length()-3)
					{
					text.erase(text.end()-1, text.end() );
					update_r_sections(text);
					}
				}
			else if (is_suffix(text,/*eedly*/'e', 'E', 'e', 'E', 'd', 'D', 'l', 'L', 'y', 'Y') )
				{
				if (m_r1 <= text.length()-5)
					{
					text.erase(text.end()-3, text.end() );
					update_r_sections(text);
					}
				}
			else if (is_suffix(text,/*ed*/'e', 'E', 'd', 'D') &&
				m_first_vowel < text.length()-2)
				{
				if (m_first_vowel == text.length()-3 &&
					is_either(text[m_first_vowel], 'e', 'E') )
					{/*no-op*/}
				else
					{
					text.erase(text.end()-2, text.end() );
					update_r_sections(text);
					regress_trim = true;
					}
				}
			else if (is_suffix(text,/*edly*/'e', 'E', 'd', 'D', 'l', 'L', 'y', 'Y') &&
				m_first_vowel < text.length()-4)
				{
				if (m_first_vowel == text.length()-3 &&
					is_either(text[m_first_vowel], 'e', 'E'))
					{/*no-op*/}
				else
					{
					text.erase(text.end()-4, text.end() );
					update_r_sections(text);
					regress_trim = true;
					}
				}
			else if (is_suffix(text,/*ing*/'i', 'I', 'n', 'N', 'g', 'G') &&
				m_first_vowel < text.length()-3)
				{
				text.erase(text.end()-3, text.end() );
				update_r_sections(text);
				regress_trim = true;
				}
			else if (is_suffix(text,/*ingly*/'i', 'I', 'n', 'N', 'g', 'G', 'l', 'L', 'y', 'Y') &&
				m_first_vowel < text.length()-5)
				{
				text.erase(text.end()-5, text.end() );
				update_r_sections(text);
				regress_trim = true;
				}
			if (regress_trim)
				{
				if (is_suffix(text,/*at*/'a', 'A', 't', 'T') ||
					is_suffix(text,/*bl*/'b', 'B', 'l', 'L') ||
					is_suffix(text,/*iz*/'i', 'I', 'z', 'Z') )
					{
					//move markers because string is being lengthened
					if (m_r2 == text.length() )
						{
						++m_r2;
						}
					text += 'e';
					}
				else if (is_suffix(text,/*bb*/'b', 'B', 'b', 'B') ||
						is_suffix(text,/*dd*/'d', 'D', 'd', 'D') ||
						is_suffix(text,/*ff*/'f', 'F', 'f', 'F') ||
						is_suffix(text,/*gg*/'g', 'G', 'g', 'G') ||
						is_suffix(text,/*mm*/'m', 'M', 'm', 'M') ||
						is_suffix(text,/*nn*/'n', 'N', 'n', 'N') ||
						is_suffix(text,/*pp*/'p', 'P', 'p', 'P') ||
						is_suffix(text,/*rr*/'r', 'R', 'r', 'R') ||
						is_suffix(text,/*tt*/'t', 'T', 't', 'T') )
					{
					text.erase(text.end()-1, text.end() );
					update_r_sections(text);
					}
				else if (is_short_word(text, text.length() ) )
					{
					//move markers because string is being lengthened
					if (m_r2 == text.length() )
						{
						++m_r2;
						}
					text += 'e';
					}
				}
			}
		/**replace suffix y or Y by i if preceded by a non-vowel which is
		not the first letter of the word (so cry -> cri, by -> by, say -> say)*/
		//---------------------------------------------
		void step_1c(std::basic_string<Tchar_type, Tchar_traits>& text)
			{
///@todo temporary testing hack
#define EITHER_LOOSE
			//proceeding consanant cannot be first letter in word
			if (text.length() > 2 &&
				!is_vowel(text[text.length()-2]) )
				{
				if (is_either(text[text.length()-1], 'y', LOWER_Y_HASH) )
					{
					text[text.length()-1] = 'i';
					}
				else if (is_either(text[text.length()-1], 'Y', UPPER_Y_HASH) )
					{
					text[text.length()-1] = 'I';
					}
				}
#undef EITHER_LOOSE
			}
		/**Search for the longest among the following suffixes, and, if found and in R1,
		perform the action indicated. 

			-tional:   replace by tion 
			-enci:   replace by ence 
			-anci:   replace by ance 
			-abli:   replace by able 
			-entli:   replace by ent 
			-izer   ization:   replace by ize 
			-ational   ation   ator:   replace by ate 
			-alism   aliti   alli:   replace by al 
			-fulness:   replace by ful 
			-ousli   ousness:   replace by ous 
			-iveness   iviti:   replace by ive 
			-biliti   bli+:   replace by ble 
			-ogi+:   replace by og if preceded by l 
			-fulli+:   replace by ful 
			-lessli+:   replace by less 
			-li+:   delete if preceded by a valid li-ending*/
		//---------------------------------------------
		void step_2(std::basic_string<Tchar_type, Tchar_traits>& text)
			{
			if (is_suffix(text,/*ization*/'i', 'I', 'z', 'Z', 'a', 'A', 't', 'T', 'i', 'I', 'o', 'O', 'n', 'N') )
				{
				if (m_r1 <= text.length()-7)
					{
					text.erase(text.end()-4, text.end() );
					text[static_cast<int>(text.length()-1)] = 'e';
					update_r_sections(text);
					}
				}
			else if (is_suffix(text,/*ational*/'a', 'A', 't', 'T', 'i', 'I', 'o', 'O', 'n', 'N', 'a', 'A', 'l', 'L') )
				{
				if (m_r1 <= text.length()-7)
					{
					text.erase(text.end()-4, text.end() );
					text[static_cast<int>(text.length()-1)] = 'e';
					update_r_sections(text);
					}
				}
			else if (is_suffix(text,/*fulness*/'f', 'F', 'u', 'U', 'l', 'L', 'n', 'N', 'e', 'E', 's', 'S', 's', 'S') )
				{
				if (m_r1 <= text.length()-7)
					{
					text.erase(text.end()-4, text.end() );
					update_r_sections(text);
					}
				}
			else if (is_suffix(text,/*ousness*/'o', 'O', 'u', 'U', 's', 'S', 'n', 'N', 'e', 'E', 's', 'S', 's', 'S') )
				{
				if (m_r1 <= text.length()-7)
					{
					text.erase(text.end()-4, text.end() );
					update_r_sections(text);
					}
				}
			else if (is_suffix(text,/*iveness*/'i', 'I', 'v', 'V', 'e', 'E', 'n', 'N', 'e', 'E', 's', 'S', 's', 'S') )
				{
				if (m_r1 <= text.length()-7)
					{
					text.erase(text.end()-4, text.end() );
					update_r_sections(text);
					}
				}
			else if (is_suffix(text,/*tional*/'t', 'T', 'i', 'I', 'o', 'O', 'n', 'N', 'a', 'A', 'l', 'L') )
				{
				if (m_r1 <= text.length()-6)
					{
					text.erase(text.end()-2, text.end() );
					update_r_sections(text);
					}
				}
			else if (is_suffix(text,/*lessli*/'l', 'L', 'e', 'E', 's', 'S', 's', 'S', 'l', 'L', 'i', 'I') )
				{
				if (m_r1 <= text.length()-6)
					{
					text.erase(text.end()-2, text.end() );
					update_r_sections(text);
					}
				}
			else if (is_suffix(text,/*biliti*/'b', 'B', 'i', 'I', 'l', 'L', 'i', 'I', 't', 'T', 'i', 'I') )
				{
				if (m_r1 <= text.length()-6)
					{
					text.erase(text.end()-3, text.end() );
					text[text.length()-2] = 'l';
					text[text.length()-1] = 'e';
					update_r_sections(text);
					}
				}
			else if (is_suffix(text,/*iviti*/'i', 'I', 'v', 'V', 'i', 'I', 't', 'T', 'i', 'I') )
				{
				if (m_r1 <= text.length()-5)
					{
					text.erase(text.end()-2, text.end() );
					text[text.length()-1] = 'e';
					update_r_sections(text);
					}
				}
			else if (is_suffix(text,/*fulli*/'f', 'F', 'u', 'U', 'l', 'L', 'l', 'L', 'i', 'I') )
				{
				if (m_r1 <= text.length()-5)
					{
					text.erase(text.end()-2, text.end() );
					update_r_sections(text);
					}
				}
			else if (is_suffix(text,/*ation*/'a', 'A', 't', 'T', 'i', 'I', 'o', 'O', 'n', 'N') )
				{
				if (m_r1 <= text.length()-5)
					{
					text.erase(text.end()-2, text.end() );
					text[text.length()-1] = 'e';
					update_r_sections(text);
					}
				}
			else if (is_suffix(text,/*alism*/'a', 'A', 'l', 'L', 'i', 'I', 's', 'S', 'm', 'M') )
				{
				if (m_r1 <= text.length()-5)
					{
					text.erase(text.end()-3, text.end() );
					update_r_sections(text);
					}
				}
			else if (is_suffix(text,/*aliti*/'a', 'A', 'l', 'L', 'i', 'I', 't', 'T', 'i', 'I') )
				{
				if (m_r1 <= text.length()-5)
					{
					text.erase(text.end()-3, text.end() );
					update_r_sections(text);
					}
				}
			else if (is_suffix(text,/*ousli*/'o', 'O', 'u', 'U', 's', 'S', 'l', 'L', 'i', 'I') )
				{
				if (m_r1 <= text.length()-5)
					{
					text.erase(text.end()-2, text.end() );
					update_r_sections(text);
					}
				}
			else if (is_suffix(text,/*entli*/'e', 'E', 'n', 'N', 't', 'T', 'l', 'L', 'i', 'I') )
				{
				if (m_r1 <= text.length()-5)
					{
					text.erase(text.end()-2, text.end() );
					update_r_sections(text);
					}
				}
			else if (is_suffix(text,/*alli*/'a', 'A', 'l', 'L', 'l', 'L', 'i', 'I') )
				{
				if (m_r1 <= text.length()-4)
					{
					text.erase(text.end()-2, text.end() );
					update_r_sections(text);
					}
				}
			else if (is_suffix(text,/*enci*/'e', 'E', 'n', 'N', 'c', 'C', 'i', 'I') )
				{
				if (m_r1 <= text.length()-4)
					{
					text[text.length()-1] = 'e';
					}
				}
			else if (is_suffix(text,/*anci*/'a', 'A', 'n', 'N', 'c', 'C', 'i', 'I') )
				{
				if (m_r1 <= text.length()-4)
					{
					text[text.length()-1] = 'e';
					}
				}
			else if (is_suffix(text,/*abli*/'a', 'A', 'b', 'B', 'l', 'L', 'i', 'I') )
				{
				if (m_r1 <= text.length()-4)
					{
					text[text.length()-1] = 'e';
					}
				}
			else if (is_suffix(text,/*izer*/'i', 'I', 'z', 'Z', 'e', 'E', 'r', 'R') )
				{
				if (m_r1 <= text.length()-4)
					{
					text.erase(text.end()-1, text.end() );
					update_r_sections(text);
					}
				}
			else if (is_suffix(text,/*ator*/'a', 'A', 't', 'T', 'o', 'O', 'r', 'R') )
				{
				if (m_r1 <= text.length()-4)
					{
					text.erase(text.end()-1, text.end() );
					text[text.length()-1] = 'e';
					update_r_sections(text);
					}
				}
			else if (m_r1 <= static_cast<int>(text.length()-3) &&
				is_suffix(text,/*bli*/'b', 'B', 'l', 'L', 'i', 'I') )
				{
				text[text.length()-1] = 'e';
				}
			else if (m_r1 <= static_cast<int>(text.length()-3) &&
				is_suffix(text,/*ogi*/'o', 'O', 'g', 'G', 'i', 'I') )
				{
				if (is_either(text[text.length()-4], 'l', 'L') )
					{
					text.erase(text.end()-1, text.end() );
					update_r_sections(text);
					}
				}
			else if (m_r1 <= static_cast<int>(text.length()-2) &&
					is_suffix(text,/*li*/'l', 'L', 'i', 'I') )
				{
				if (string_util::is_one_of(text[text.length()-3], "cdeghkmnrtCDEGHKMNRT") )
					{
					text.erase(text.end()-2, text.end() );
					update_r_sections(text);
					}
				}
			}
		/**Search for the longest among the following suffixes, and, if found and in R1, perform the action indicated. 

			-tional+:   replace by tion 
			-ational+:   replace by ate 
			-alize:   replace by al 
			-icate   iciti   ical:   replace by ic 
			-ful   ness:   delete 
			-ative*:   delete if in R2*/
		//---------------------------------------------
		void step_3(std::basic_string<Tchar_type, Tchar_traits>& text) 
			{
			if (is_suffix(text,/*ational*/'a', 'A', 't', 'T', 'i', 'I', 'o', 'O', 'n', 'N', 'a', 'A', 'l', 'L') )
				{
				if (m_r1 <= text.length()-7)
					{
					text.erase(text.end()-4, text.end() );
					text[text.length()-1] = 'e';
					update_r_sections(text);
					}
				}
			else if (is_suffix(text,/*tional*/'t', 'T', 'i', 'I', 'o', 'O', 'n', 'N', 'a', 'A', 'l', 'L') )
				{
				if (m_r1 <= text.length()-6)
					{
					text.erase(text.end()-2, text.end() );
					update_r_sections(text);
					}
				}
			else if (is_suffix(text,/*alize*/'a', 'A', 'l', 'L', 'i', 'I', 'z', 'Z', 'e', 'E') )
				{
				if (m_r1 <= text.length()-5)
					{
					text.erase(text.end()-3, text.end() );
					update_r_sections(text);
					}
				}
			else if (is_suffix(text,/*icate*/'i', 'I', 'c', 'C', 'a', 'A', 't', 'T', 'e', 'E') )
				{
				if (m_r1 <= text.length()-5)
					{
					text.erase(text.end()-3, text.end() );
					update_r_sections(text);
					}
				}
			else if (is_suffix(text,/*iciti*/'i', 'I', 'c', 'C', 'i', 'I', 't', 'T', 'i', 'I') )
				{
				if (m_r1 <= text.length()-5)
					{
					text.erase(text.end()-3, text.end() );
					update_r_sections(text);
					}
				}
			else if (is_suffix(text,/*ative*/'a', 'A', 't', 'T', 'i', 'I', 'v', 'V', 'e', 'E') )
				{
				if (m_r2 <= text.length()-5)
					{
					text.erase(text.end()-5, text.end() );
					update_r_sections(text);
					}
				}
			else if (is_suffix(text,/*ical*/'i', 'I', 'c', 'C', 'a', 'A', 'l', 'L') )
				{
				if (m_r1 <= text.length()-4)
					{
					text.erase(text.end()-2, text.end() );
					update_r_sections(text);
					}
				}
			else if (is_suffix(text,/*ness*/'n', 'N', 'e', 'E', 's', 'S', 's', 'S') )
				{
				if (m_r1 <= text.length()-4)
					{
					text.erase(text.end()-4, text.end() );
					update_r_sections(text);
					}
				}
			else if (is_suffix(text,/*ful*/'f', 'F', 'u', 'U', 'l', 'L') )
				{
				if (m_r1 <= text.length()-3)
					{
					text.erase(text.end()-3, text.end() );
					update_r_sections(text);
					}
				}
			}
		/**Search for the longest among the following suffixes, and, if found and in R2, perform the action indicated. 

			-al ance ence er ic able ible ant ement ment ent ism ate iti ous ive ize 
				-delete 
			-ion 
				-delete if preceded by s or t*/
		//---------------------------------------------
		void step_4(std::basic_string<Tchar_type, Tchar_traits>& text)
			{	
			if (is_suffix(text,/*ement*/'e', 'E', 'm', 'M', 'e', 'E', 'n', 'N', 't', 'T') )
				{
				if (m_r2 <= text.length()-5)
					{
					text.erase(text.end()-5, text.end() );
					update_r_sections(text);
					}
				}
			else if (is_suffix(text,/*ance*/'a', 'A', 'n', 'N', 'c', 'C', 'e', 'E') )
				{
				if (m_r2 <= text.length()-4)
					{
					text.erase(text.end()-4, text.end() );
					update_r_sections(text);
					}
				}
			else if (is_suffix(text,/*ence*/'e', 'E', 'n', 'N', 'c', 'C', 'e', 'E') )
				{
				if (m_r2 <= text.length()-4)
					{
					text.erase(text.end()-4, text.end() );
					update_r_sections(text);
					}
				}
			else if (is_suffix(text,/*ment*/'m', 'M', 'e', 'E', 'n', 'N', 't', 'T') )
				{
				if (m_r2 <= text.length()-4)
					{
					text.erase(text.end()-4, text.end() );
					update_r_sections(text);
					}
				}
			else if (is_suffix(text,/*sion*/'s', 'S', 'i', 'I', 'o', 'O', 'n', 'N') ||
					is_suffix(text,/*tion*/'t', 'T', 'i', 'I', 'o', 'O', 'n', 'N') )
				{
				if (m_r2 <= text.length()-3)
					{
					text.erase(text.end()-3, text.end() );
					update_r_sections(text);
					}
				}
			else if (is_suffix(text,/*able*/'a', 'A', 'b', 'B', 'l', 'L', 'e', 'E') )
				{
				if (m_r2 <= text.length()-4)
					{
					text.erase(text.end()-4, text.end() );
					update_r_sections(text);
					}
				}
			else if (is_suffix(text,/*ible*/'i', 'I', 'b', 'B', 'l', 'L', 'e', 'E') )
				{
				if (m_r2 <= text.length()-4)
					{
					text.erase(text.end()-4, text.end() );
					update_r_sections(text);
					}
				}
			else if (is_suffix(text,/*ant*/'a', 'A', 'n', 'N', 't', 'T') )
				{
				if (m_r2 <= text.length()-3)
					{
					text.erase(text.end()-3, text.end() );
					update_r_sections(text);
					}
				}
			else if (is_suffix(text,/*ent*/'e', 'E', 'n', 'N', 't', 'T') ||
					is_suffix(text,/*ism*/'i', 'I', 's', 'S', 'm', 'M') ||
					is_suffix(text,/*ate*/'a', 'A', 't', 'T', 'e', 'E') ||
					is_suffix(text,/*iti*/'i', 'I', 't', 'T', 'i', 'I') ||
					is_suffix(text,/*ous*/'o', 'O', 'u', 'U', 's', 'S') ||
					is_suffix(text,/*ive*/'i', 'I', 'v', 'V', 'e', 'E') ||
					is_suffix(text,/*ize*/'i', 'I', 'z', 'Z', 'e', 'E') )
				{
				if (m_r2 <= text.length()-3)
					{
					text.erase(text.end()-3, text.end() );
					update_r_sections(text);
					}
				}
			else if (is_suffix(text,/*al*/'a', 'A', 'l', 'L') )
				{
				if (m_r2 <= text.length()-2)
					{
					text.erase(text.end()-2, text.end() );
					update_r_sections(text);
					}
				}
			else if (is_suffix(text,/*er*/'e', 'E', 'r', 'R') )
				{
				if (m_r2 <= text.length()-2)
					{
					text.erase(text.end()-2, text.end() );
					update_r_sections(text);
					}
				}
			else if (is_suffix(text,/*ic*/'i', 'I', 'c', 'C') )
				{
				if (m_r2 <= text.length()-2)
					{
					text.erase(text.end()-2, text.end() );
					update_r_sections(text);
					}
				}
			}
		/**Search for the the following suffixes, and, if found, perform the action indicated. 

			-e 
				-delete if in R2, or in R1 and not preceded by a short syllable 
			-l 
				-delete if in R2 and preceded by l*/
		//---------------------------------------------
		void step_5(std::basic_string<Tchar_type, Tchar_traits>& text)
			{
			if (is_either(text[text.length()-1], 'e', 'E') )
				{
				if (m_r2 != text.length())
					{
					text.erase(text.end()-1, text.end() );
					update_r_sections(text);
					}
				else if (m_r1 != text.length() &&
					!is_short_syllable(text, text.length()-1))
					{
					text.erase(text.end()-1, text.end() );
					update_r_sections(text);
					}
				}
			else if (m_r2 != text.length() &&
				is_suffix(text,/*ll*/'l', 'L', 'l', 'L') )
				{
				text.erase(text.end()-1, text.end() );
				update_r_sections(text);
				}
			}
		//---------------------------------------------
		bool is_short_syllable(std::basic_string<Tchar_type, Tchar_traits>& text, size_t length)
			{
			if (length == 2)
				{
				if (is_vowel(text[0]) )
					{
					return (!is_vowel(text[1]) );
					}
				else
					{
					return false;
					}
				}
			else if (length > 2)
				{
				size_t start = text.find_first_of("aeiouyAEIOUY");
				if (start == std::basic_string<Tchar_type, Tchar_traits>::npos)
					{
					return false;
					}
				//if vowel is at the beginning, then quit
				if (start == 0)
					{
					return false;
					}
				if (start+2 == length &&
					!is_vowel(text[start+1]) &&
					!string_util::is_one_of(text[start+1], "wxWX") &&
					is_neither(text[start+1], LOWER_Y_HASH, UPPER_Y_HASH) )
					{
					return true;
					}
				else
					{
					return false;
					}
				}
			else
				{
				return false;
				}
			}
		//---------------------------------------------
		bool is_short_word(std::basic_string<Tchar_type, Tchar_traits>& text, size_t length)
			{
			if (length == 2)		
				{
				return is_short_syllable(text, length);
				}
			else if (length > 2)
				{
				size_t start = text.find_first_of("aeiouyAEIOUY");
				if (start == std::basic_string<Tchar_type, Tchar_traits>::npos)
					{
					return false;
					}
				//if vowel is at the beginning, then quit
				if (start == 0)
					{
					return false;
					}
				//make sure there are no more vowels in the word
				size_t end = text.find_first_of("aeiouyAEIOUY", start+1);
				if (end != std::basic_string<Tchar_type, Tchar_traits>::npos &&
					end < length)
					{
					return false;
					}
				if (start+1 != length &&
					!is_vowel(text[start+1]) &&
					!string_util::is_one_of(text[start+1], "wxWX") &&
					is_neither(text[start+1], LOWER_Y_HASH, UPPER_Y_HASH) )
					{
					return true;
					}
				else
					{
					return false;
					}
				}
			else
				{
				return false;
				}
			}
		//---------------------------------------------
		inline bool is_vowel(Tchar_type character)
			{
			return (string_util::is_one_of(character, "aeiouyAEIOUY") );
			}

		size_t m_first_vowel;
		};
	}

#endif //__ENGLISH_STEM_H__
