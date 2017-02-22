// ElanDocument.cpp
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2013/04/08
// Copyright (C) University of Augsburg, Lab for Human Centered Multimedia
//
// *************************************************************************************************
//
// This file is part of Social Signal Interpretation (SSI) developed at the 
// Lab for Human Centered Multimedia of the University of Augsburg
//
// This library is free software; you can redistribute itand/or
// modify it under the terms of the GNU General Public
// License as published by the Free Software Foundation; either
// version 3 of the License, or any laterversion.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FORA PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public
// License along withthis library; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
//
//*************************************************************************************************

#include "ElanDocument.h"
#include "ioput/xml/tinyxml.h"

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif

namespace ssi {

ssi_char_t *ElanDocument::ssi_log_name = "elandoc___";

	ElanDocument::ElanDocument () 
	: _filepath (0) {
		_meta.author = "unspecified";
		ssi_char_t now[ssi_time_size_friendly];
		ssi_now_friendly (now);
		now[4] = '-';
		now[7] = '-';
		now[10] = 'T';
		now[13] = ':';
		now[16] = ':';		
		_meta.date = now;
		_meta.date += "+00:00";
	}

	ElanDocument::~ElanDocument () {
		delete[] _filepath;
	};

	ElanDocument *ElanDocument::Read (const ssi_char_t *filepath) {

		ElanDocument *me = new ElanDocument ();

		if (!ssi_exists (filepath)) {
			ssi_wrn ("file not found '%s'", filepath);
            return nullptr;
		}

		me->_filepath = ssi_strcpy (filepath);

		ssi_msg (SSI_LOG_LEVEL_BASIC, "read elan file '%s'", filepath);

		TiXmlDocument doc;
		if (!doc.LoadFile (filepath)) {
			ssi_wrn ("failed loading trainer from file '%s'", filepath);
            return nullptr;
		}

		TiXmlElement *body = doc.FirstChildElement();	
		if (!body || strcmp (body->Value (), "ANNOTATION_DOCUMENT") != 0) {
			ssi_wrn ("child <ANNOTATION_DOCUMENT> missing");
            return nullptr;
		}

		me->_meta.author = body->Attribute ("AUTHOR");
		if (!me->_meta.author.str ()) {
			ssi_wrn ("attribute AUTHOR missing");
            return nullptr;
		}				
		me->_meta.date = body->Attribute ("DATE");
		if (!me->_meta.date.str ()) {
			ssi_wrn ("attribute DATE missing");
            return nullptr;
		}

		TiXmlElement *header = body->FirstChildElement ("HEADER");
		if (!header) {
			ssi_wrn ("child <HEADER> missing");
            return nullptr;
		}

		TiXmlElement *media = header->FirstChildElement ("MEDIA_DESCRIPTOR");
		if (media) {
			SSI_DBG (SSI_LOG_LEVEL_DEBUG, "media");
			do {
				const ssi_char_t *url = media->Attribute ("MEDIA_URL");
				if (!url) {
					ssi_wrn ("attribute MEDIA_URL missing");
                    return nullptr;
				}				
				const ssi_char_t *type = media->Attribute ("MIME_TYPE");
				if (!type) {
					ssi_wrn ("attribute MIME_TYPE missing");
                    return nullptr;
				}
				const ssi_char_t *url_rel = media->Attribute ("RELATIVE_MEDIA_URL");
				if (!url_rel) {
					ssi_wrn ("attribute RELATIVE_MEDIA_URL missing");
                    return nullptr;
				}
				SSI_DBG (SSI_LOG_LEVEL_DEBUG, ">%s [%s]\n", url, type);
				ElanMedia media;
				media.url = url;
				media.type = type;
				media.url_rel = url_rel;
				me->_media.push_back (media);
			} while (media = media->NextSiblingElement ("MEDIA_DESCRIPTOR"));

		} else {
			ssi_wrn ("no media found");
		}

		TiXmlElement *time_order = body->FirstChildElement ("TIME_ORDER");
		if (!time_order) {
			ssi_wrn ("child <TIME_ORDER> missing");
            return nullptr;
		}

		TiXmlElement *time_slot = time_order->FirstChildElement ("TIME_SLOT");
		std::map<String,int> slot;
		if (time_slot) {
			SSI_DBG (SSI_LOG_LEVEL_DEBUG, "time slots:");
			do {
				const ssi_char_t *slot_id = time_slot->Attribute ("TIME_SLOT_ID");				
				if (!slot_id) {
					ssi_wrn ("attribute TIME_SLOT_ID missing");
                    return nullptr;
				}
				int slot_value;
				const ssi_char_t *slot_value_s = time_slot->Attribute ("TIME_VALUE", &slot_value);				
				if (!slot_value_s) {
					ssi_wrn ("attribute TIME_VALUE missing");
                    return nullptr;
				}
				SSI_DBG (SSI_LOG_LEVEL_DEBUG, ">%s [%d]\n", slot_id, slot_value);
				slot[String (slot_id)] = slot_value;
			} while (time_slot = time_slot->NextSiblingElement ("TIME_SLOT"));

		} else {
			ssi_wrn ("no time slot found");
		}

		TiXmlElement *tier = body->FirstChildElement ("TIER");		
		if (tier) {
			do {
				const ssi_char_t *tier_id = tier->Attribute ("TIER_ID"); 
				if (!tier_id) {
					ssi_wrn ("attribute TIER_ID missing");
                    return nullptr;
				}
				SSI_DBG (SSI_LOG_LEVEL_DEBUG, "annotations for tier '%s':\n", tier_id);
				TiXmlElement *annnotation = tier->FirstChildElement ("ANNOTATION");				
				std::map<String,int>::iterator slot_from, slot_to;
				ElanTier t (tier_id);
				if (annnotation) {														
					do {
						TiXmlElement *alignable = annnotation->FirstChildElement ("ALIGNABLE_ANNOTATION");						
						do {
							const ssi_char_t *slot_id_1 = alignable->Attribute ("TIME_SLOT_REF1");				
							if (!slot_id_1) {
								ssi_wrn ("attribute TIME_SLOT_REF1 missing");
                                return nullptr;
							}
							slot_from = slot.find (String (slot_id_1));							
							if (slot_from == slot.end ()) {
								ssi_wrn ("slot id '%s' not found", slot_id_1);
                                return nullptr;
							}
							const ssi_char_t *slot_id_2 = alignable->Attribute ("TIME_SLOT_REF2");				
							if (!slot_id_2) {
								ssi_wrn ("attribute TIME_SLOT_REF2 missing");
                                return nullptr;
							}			
							slot_to = slot.find (String (slot_id_2));
							if (slot_to == slot.end ()) {
								ssi_wrn ("slot id '%s' not found", slot_id_2);
                                return nullptr;
							}
							TiXmlElement *annnotation_value = alignable->FirstChildElement ("ANNOTATION_VALUE");
							do {
								const ssi_char_t *value = annnotation_value->GetText ();								
								SSI_DBG (SSI_LOG_LEVEL_DEBUG, "%d-%d : %s", slot_from->second, slot_to->second, value);
								ElanAnnotation a;
								a.from = slot_from->second;
								a.to = slot_to->second;
								a.value = value;
								t.push_back (a);
							} while (annnotation_value = annnotation_value->NextSiblingElement ("ANNOTATION_VALUE"));												
						} while (alignable = alignable->NextSiblingElement ("ALIGNABLE_ANNOTATION"));
					} while (annnotation = annnotation->NextSiblingElement ("ANNOTATION"));					
					me->push_back (t);
				} else {
					ssi_wrn ("no annotations found for %s", tier_id);
					me->push_back (t);
				}
			} while (tier = tier->NextSiblingElement ("TIER"));
		} else {
			ssi_wrn ("child <TIER> missing");
            return nullptr;
		}

		return me;

	}

	ElanTier &ElanDocument::operator[] (const ssi_char_t *name) {

		ElanDocument::iterator it;
		for (it = begin (); it != end (); it++) {		
			if (strcmp (it->name (), name) == 0) {
				return *it;
			}
		}
		ssi_err ("tier '%s' not found", name);		
	}

	std::vector<ElanMedia> &ElanDocument::media () {
		return _media;
	}

	ElanMeta &ElanDocument::meta () {
		return _meta;
	}

	void ElanDocument::print (FILE *file) {

		ssi_fprint (file, "META\n\tauthor=%s\n\tdate=%s\n", _meta.author.str (), _meta.date.str ());

		std::vector<ElanMedia>::iterator media;
		int i = 1;
		for (media = _media.begin (); media != _media.end (); media++) {
			ssi_fprint (file, "MEDIA#%d\n", i++);
			ssi_fprint (file, "\turl=%s\n\turl (relative)=%s\n\ttype=%s\n", media->url.str (), media->url_rel.str (), media->type.str ());
		}

		ElanDocument::iterator tier;		
		for (tier = begin (); tier != end (); tier++) {
			tier->print (file);
		}
	}

	bool ElanDocument::write (const ssi_char_t *filepath) {

		TiXmlDocument doc;

		TiXmlDeclaration head ("1.0", "", "");
		doc.InsertEndChild (head);
	
		// body
		TiXmlElement bodyElement ("ANNOTATION_DOCUMENT");		
		bodyElement.SetAttribute ("AUTHOR", _meta.author.str ());
		bodyElement.SetAttribute ("DATE", _meta.date.str ());
		bodyElement.SetAttribute ("FORMAT", "2.7");
		bodyElement.SetAttribute ("VERSION", "2.7");
		bodyElement.SetAttribute ("xmlns:xsi", "http://www.w3.org/2001/XMLSchema-instance");
		bodyElement.SetAttribute ("xsi:noNamespaceSchemaLocation", "http://www.mpi.nl/tools/elan/EAFv2.7.xsd");

		// header
		TiXmlElement headerElement ("HEADER");
		headerElement.SetAttribute ("MEDIA_FILE", "");
		headerElement.SetAttribute ("TIME_UNITS", "milliseconds");
		std::vector<ElanMedia>::iterator media;		
		for (media = _media.begin (); media != _media.end (); media++) {
			TiXmlElement mediaElement ("MEDIA_DESCRIPTOR");
			mediaElement.SetAttribute ("MEDIA_URL", media->url.str ());
			mediaElement.SetAttribute ("MIME_TYPE", media->type.str ());
			mediaElement.SetAttribute ("RELATIVE_MEDIA_URL", media->url_rel.str ());
			headerElement.InsertEndChild (mediaElement);
		}
		bodyElement.InsertEndChild (headerElement);
		
		// time order
		TiXmlElement timeOrderElement ("TIME_ORDER");
		std::map<ssi_size_t, ssi_size_t> slot;
		ElanDocument::iterator tier;
		ssi_size_t slotCounter = 1;
		for (tier = begin (); tier != end (); tier++) {
			ElanTier::iterator anno;
			for (anno = tier->begin (); anno != tier->end (); anno++) {
				slot.insert (std::pair<ssi_size_t,ssi_size_t>(anno->from, slotCounter++));
				slot.insert (std::pair<ssi_size_t,ssi_size_t>(anno->to, slotCounter++));
			}
		}		
		ssi_char_t slotId[SSI_MAX_CHAR];
		for (std::map<ssi_size_t,ssi_size_t>::iterator it=slot.begin(); it!=slot.end(); ++it) {
			TiXmlElement timeSlotElement ("TIME_SLOT");
			ssi_sprint (slotId, "st%u", it->second);
			timeSlotElement.SetAttribute ("TIME_SLOT_ID", slotId);
			timeSlotElement.SetAttribute ("TIME_VALUE", it->first);
			timeOrderElement.InsertEndChild (timeSlotElement);
		}
		bodyElement.InsertEndChild (timeOrderElement);

		// tier		
		ssi_size_t annoCounter = 1;
		ssi_char_t annoId[SSI_MAX_CHAR];
		for (tier = begin (); tier != end (); tier++) {
			TiXmlElement tierElement ("TIER");
			tierElement.SetAttribute ("DEFAULT_LOCALE", "us");
			tierElement.SetAttribute ("LINGUISTIC_TYPE_REF", "default");
			tierElement.SetAttribute ("TIER_ID", tier->name ());
			ElanTier::iterator anno;			
			for (anno = tier->begin (); anno != tier->end (); anno++) {				
				TiXmlElement annotationElement ("ANNOTATION");
				TiXmlElement alignableAnnotationElement ("ALIGNABLE_ANNOTATION");
				ssi_sprint (annoId, "a%u", annoCounter++);
				alignableAnnotationElement.SetAttribute ("ANNOTATION_ID", annoId);
				ssi_sprint (slotId, "st%u", slot[anno->from]);				
				alignableAnnotationElement.SetAttribute ("TIME_SLOT_REF1", slotId);
				ssi_sprint (slotId, "st%u", slot[anno->to]);
				alignableAnnotationElement.SetAttribute ("TIME_SLOT_REF2", slotId);
				TiXmlElement annotationValueElement ("ANNOTATION_VALUE");
				annotationValueElement.InsertEndChild  (TiXmlText (anno->value.str () ? anno->value.str () : ""));
				alignableAnnotationElement.InsertEndChild (annotationValueElement);
                annotationElement.InsertEndChild(alignableAnnotationElement);
                tierElement.InsertEndChild (annotationElement);
			}
			bodyElement.InsertEndChild (tierElement);
		}
		
		TiXmlElement linguisticTypeElement ("LINGUISTIC_TYPE");
		linguisticTypeElement.SetAttribute ("GRAPHIC_REFERENCES", "false");
		linguisticTypeElement.SetAttribute ("LINGUISTIC_TYPE_ID", "default");
		linguisticTypeElement.SetAttribute ("TIME_ALIGNABLE", "true");
		bodyElement.InsertEndChild (linguisticTypeElement);

		TiXmlElement linguisticLocaleElement ("LOCALE");
		linguisticLocaleElement.SetAttribute ("COUNTRY_CODE", "EN");
		linguisticLocaleElement.SetAttribute ("LANGUAGE_CODE", "us");
		bodyElement.InsertEndChild (linguisticLocaleElement);
		
		doc.InsertEndChild (bodyElement);

		if (!doc.SaveFile (filepath)) {
			ssi_wrn ("saving elan document to file '%s' failed", filepath);	
			return false;
		}

		ssi_msg (SSI_LOG_LEVEL_BASIC, "saved elan document to file '%s'", filepath);
		
		return true;
	}
}

