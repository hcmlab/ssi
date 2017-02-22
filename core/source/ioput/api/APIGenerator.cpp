// TheFramework.cpp
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2011/04/01
// Copyright (C) University of Augsburg, Lab for Human Centered Multimedia
//
// *************************************************************************************************
//
// This file is part of Social Signal Interpretation (SSI) developed at the 
// Lab for Human Centered Multimedia of the University of Augsburg
//
// This library is free software; you can redistribute itand/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 3 of the License, or any laterversion.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FORA PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along withthis library; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
//
//*************************************************************************************************

#include "ioput/api/APIGenerator.h"
#include "ioput/file/FilePath.h"
#include "ioput/file/FileTools.h"
#include "ioput/option/OptionList.h"
#include "base/ISensor.h"
#include "frame/include/TheFramework.h"
#include "event/include/TheEventBoard.h"
#include "graphic/Console.h"

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif

namespace ssi {

ssi_char_t *APIGenerator::ssi_log_name = "apigen____";
int APIGenerator::ssi_log_level = SSI_LOG_LEVEL_DEFAULT;
APIGenerator::VERSION DEFAULT_VERSION = APIGenerator::V1;
ssi_size_t APIGenerator::_n_dlls = 0;
ssi_char_t **APIGenerator::_dll_names = 0;
ssi_size_t APIGenerator::_n_objects = 0;
ssi_char_t **APIGenerator::_object_names = 0;

void APIGenerator::SaveCurrentComponentList () {	 
	_n_dlls = Factory::GetDllNames (&_dll_names);
	_n_objects = Factory::GetObjectNames (&_object_names);
}

void APIGenerator::ResetCurrentComponentList () {
	for (ssi_size_t i = 0; i < _n_dlls; i++) {
		delete[] _dll_names[i];
	}
	delete[] _dll_names;

	for (ssi_size_t i = 0; i < _n_objects; i++) {
		delete[] _object_names[i];
	}
	delete[] _object_names;
}

bool APIGenerator::CreateAPIIndex (const ssi_char_t *apidir) {

	ssi_msg (SSI_LOG_LEVEL_BASIC, "create index '%s'", apidir);

	CreateIndex (apidir);
	CreateStart (apidir);
	CreateMenu (apidir);

	return true;
}

bool APIGenerator::CreateIndex (const ssi_char_t *apidir) {

	ssi_char_t *filename = 0;
	if (apidir[0] == '\0') {
		filename = ssi_strcpy (".\\index.html");
	} else {
		filename = ssi_strcat (apidir, "\\index.html");
	}
	FILE *fp = fopen (filename, "w");

	if (!fp) {
		ssi_wrn ("could not create file '%s'", filename);
		return false;
	}

	ssi_fprint (fp, "%s", "<?xml version=\"1.0\" ?>\n\
<HTML>\n\
	<HEAD>\n\
		<TITLE>Social Signal Interpretation (SSI)\n\
		</TITLE>\n\
	</HEAD>\n\
	<FRAMESET COLS=\"250pixel,*\">\n\
		<FRAME SRC=\"menu.html\">\n\
		<FRAME SRC=\"start.html\" name=\"MAIN\">\n\
	</FRAMESET>\n\
</HTML>\n");
	fclose (fp);

	delete[] filename;

	return true;
}

bool APIGenerator::CreateStart (const ssi_char_t *apidir) {

	ssi_char_t *filename = 0;
	if (apidir[0] == '\0') {
		filename = ssi_strcpy (".\\start.html");
	} else {
		filename = ssi_strcat (apidir, "\\start.html");
	}
	FILE *fp = fopen (filename, "w");

	if (!fp) {
		ssi_wrn ("could not create file '%s'", filename);
		return false;
	}

	ssi_fprint (fp, "%s", "<?xml version=\"1.0\" ?>\n\
<HTML>\n\
	<HEAD>\n\
		<TITLE>Social Signal Interpretation (SSI) API\n\
		</TITLE>\n\
	</HEAD>\n\
	<BODY>\n\
		<TABLE WIDTH=\"100%\" HEIGHT=\"100%\">\n\
			<TR>\n\
				<TD ALIGN=\"CENTER\" VALIGN=\"MIDDLE\">\n\
					<IMG SRC=\"ssi-logo.png\"><BR>\n\
					<P>Social Signal Interpretation (SSI)\n\
						<BR />(c) University of Augsburg\n\
						<BR />\n\
						<A HREF=\"http://openssi.net\" TARGET=\"new\">http://openssi.net</A>\n\
					</P>\n\
				</TD>\n\
			</TR>\n\
		</TABLE>\n\
	</BODY>\n\
</HTML>\n"
);
	fclose (fp);

	delete[] filename;

	return true;
}

bool APIGenerator::CreateMenu (const ssi_char_t *apidir) {

	TiXmlDocument doc;	

	TiXmlDeclaration decl ("1.0", "", "");
	doc.InsertEndChild (decl);

	TiXmlElement html ("HTML");
	
	TiXmlElement head ("HEAD");
	TiXmlElement head_title ("TITLE");
	head_title.InsertEndChild (TiXmlText ("Social Signal Interpretation (SSI) API ["));
	head_title.InsertEndChild (TiXmlText (SSI_VERSION));
	head_title.InsertEndChild (TiXmlText ("]"));
	head.InsertEndChild (head_title);
	html.InsertEndChild (head);

	TiXmlElement body ("BODY");

	TiXmlElement table ("TABLE"); 
	table.SetAttribute ("WIDTH", "100%");
	table.SetAttribute ("CELLPADDING", "3");
	table.SetAttribute ("CELLSPACING", "0");

	{
		TiXmlElement table_tr ("TR");	
		TiXmlElement table_tr_th ("TD");	
		table_tr_th.SetAttribute ("ALIGN", "left");	
		TiXmlElement table_tr_th_a ("A");
		table_tr_th_a.SetAttribute ("HREF", "start.html");
		table_tr_th_a.SetAttribute ("TARGET", "MAIN");
		TiXmlElement table_tr_th_img ("IMG");
		table_tr_th_img.SetAttribute ("BORDER", "0");
		table_tr_th_img.SetAttribute ("WIDTH", "100");
		table_tr_th_img.SetAttribute ("SRC", "ssi-logo.png");	
		table_tr_th_a.InsertEndChild (table_tr_th_img);
		table_tr_th.InsertEndChild (table_tr_th_a);
		table_tr.InsertEndChild (table_tr_th);
		table.InsertEndChild (table_tr);
	}

	{
		TiXmlElement table_tr ("TR");	
		TiXmlElement table_tr_th ("TD");			
		table_tr_th.InsertEndChild (TiXmlText ("&nbsp;"));
		table_tr.InsertEndChild (table_tr_th);
		table.InsertEndChild (table_tr);
	}

	StringList files;
	FileTools::ReadFilesFromDir (files, apidir, "ssi*.html");
	
	const ssi_char_t *file = 0;
	for (ssi_size_t i = 0; i < files.size (); i++) {

		file = files.get (i);
		FilePath fp (file);

		TiXmlElement table_tr ("TR");

		{
			TiXmlElement table_tr_td ("TD");
			table_tr_td.SetAttribute ("ALIGN", "right");
			TiXmlElement table_tr_td_code ("CODE");
			TiXmlElement table_tr_td_code_a ("A");		
			table_tr_td_code_a.SetAttribute ("HREF", fp.getNameFull ());
			table_tr_td_code_a.SetAttribute ("TARGET", "MAIN");	
			ssi_char_t *name = ssi_strcpy(fp.getName());
			table_tr_td_code_a.InsertEndChild (TiXmlText (name+3));
			delete[] name;
			table_tr_td_code.InsertEndChild (table_tr_td_code_a);
			table_tr_td.InsertEndChild (table_tr_td_code);
			table_tr.InsertEndChild (table_tr_td);
		}

		table.InsertEndChild (table_tr);
	}

	body.InsertEndChild (table);	

	html.InsertEndChild (body);
	doc.InsertEndChild (html);

	ssi_char_t *filename = 0;
	if (apidir[0] == '\0') {
		filename = ssi_strcpy (".\\menu.html");
	} else {
		filename = ssi_strcat (apidir, "\\menu.html");
	}
	
	if (!doc.SaveFile (filename)) {
		ssi_wrn ("could not save '%s'", filename);
		delete[] filename;
		return false;
	}
	delete[] filename;

	return true;
}

bool APIGenerator::CreateAPI (const ssi_char_t *filepath) {
	
	ssi_msg (SSI_LOG_LEVEL_BASIC, "parse '%s'", filepath);

	TiXmlDocument doc;	

	TiXmlDeclaration decl ("1.0", "", "");
	doc.InsertEndChild (decl);

	TiXmlElement html ("HTML");
	
	TiXmlElement head ("HEAD");
	TiXmlElement head_title ("TITLE");

	if (_n_objects == 0) {
		SaveCurrentComponentList ();
	}
	
	for (ssi_size_t i = 0; i < _n_dlls; i++) {	
		head_title.InsertEndChild (TiXmlText (_dll_names[i]));	
		head_title.InsertEndChild (TiXmlText (" "));
	}

	head_title.InsertEndChild (TiXmlText (" - Social Signal Interpretation (SSI) API ["));
	head_title.InsertEndChild (TiXmlText (SSI_VERSION));
	head_title.InsertEndChild (TiXmlText ("]"));
	head.InsertEndChild (head_title);
	html.InsertEndChild (head);

	TiXmlElement body ("BODY");
	CreateSource (body);
	CreateSummary (body);
	CreateDetails (body);

	TiXmlElement cp ("FONT");
	cp.SetAttribute ("SIZE", "-1");	
	TiXmlElement cp_p ("P");
	cp_p.InsertEndChild (TiXmlText ("Built with Social Signal Interpretation (SSI) ["));
	cp_p.InsertEndChild (TiXmlText (SSI_VERSION));
	cp_p.InsertEndChild (TiXmlText ("]"));
	TiXmlElement cp_p_br ("BR");
	cp_p.InsertEndChild (cp_p_br);
	cp_p.InsertEndChild (TiXmlText ("(c) University of Augsburg"));
	cp_p.InsertEndChild (cp_p_br);
	TiXmlElement cp_p_a ("A");
	cp_p_a.SetAttribute ("HREF", "http://openssi.net");
	cp_p_a.SetAttribute ("TARGET", "new");
	cp_p_a.InsertEndChild (TiXmlText ("http://openssi.net"));
	cp_p.InsertEndChild (cp_p_a);
	cp.InsertEndChild (cp_p);
	body.InsertEndChild (cp);

	html.InsertEndChild (body);
	doc.InsertEndChild (html);

	FilePath fp (filepath);
	ssi_char_t *filepath_with_ext = 0;
	if (strcmp (fp.getExtension (), ".html") != 0) {
		filepath_with_ext = ssi_strcat (filepath, ".html");
	} else {
		filepath_with_ext = ssi_strcpy (filepath);
	}
	ssi_msg (SSI_LOG_LEVEL_BASIC, "save '%s'", filepath_with_ext);
	if (!doc.SaveFile (filepath_with_ext)) {
		ssi_wrn ("could not save '%s'", filepath_with_ext);
		delete[] filepath_with_ext;
		return false;
	}
	delete[] filepath_with_ext;

	return true;
}

bool APIGenerator::CreateSource (TiXmlElement &body) {

	body.InsertEndChild (TiXmlComment ());
	body.InsertEndChild (TiXmlComment ("========== SOURCE ==========="));
	body.InsertEndChild (TiXmlComment ());

	TiXmlElement p ("P"); 
	TiXmlElement p_dl ("DL");

	TiXmlElement p_dl_dt ("DT");
	TiXmlElement p_dl_dt_font ("FONT");
	p_dl_dt_font.SetAttribute ("SIZE", "+2");
	TiXmlElement p_dl_dt_font_b ("B");
	p_dl_dt_font_b.InsertEndChild (TiXmlText ("SOURCE"));
	p_dl_dt_font.InsertEndChild (p_dl_dt_font_b);
	p_dl_dt.InsertEndChild (p_dl_dt_font);
	p_dl.InsertEndChild (p_dl_dt);

	for (ssi_size_t i = 0; i < _n_dlls; i++) {	
		TiXmlElement p_dl_dd ("DD");
		TiXmlElement p_dl_dd_font ("FONT");
		p_dl_dd_font.SetAttribute ("SIZE", "+2");
		TiXmlElement p_dl_dd_font_code ("CODE");
		TiXmlElement p_dl_dd_font_code_a ("A");
		p_dl_dd_font_code_a.SetAttribute ("NAME", _dll_names[i]);
		p_dl_dd_font_code_a.InsertEndChild (TiXmlText (_dll_names[i]));
		p_dl_dd_font_code.InsertEndChild (p_dl_dd_font_code_a);
		p_dl_dd_font.InsertEndChild (p_dl_dd_font_code);
		p_dl_dd.InsertEndChild (p_dl_dd_font);
		p_dl.InsertEndChild (p_dl_dd);
	}
	
	p.InsertEndChild (p_dl);
	body.InsertEndChild (p);

	return true;
}

bool APIGenerator::CreateSummary (TiXmlElement &body) {

	body.InsertEndChild (TiXmlComment ());
	body.InsertEndChild (TiXmlComment ("========== SUMMARY ==========="));
	body.InsertEndChild (TiXmlComment ());

	TiXmlElement table ("TABLE"); 
	table.SetAttribute ("BORDER", "1");
	table.SetAttribute ("WIDTH", "100%");
	table.SetAttribute ("CELLPADDING", "3");
	table.SetAttribute ("CELLSPACING", "0");

	TiXmlElement table_tr ("TR");	
	TiXmlElement table_tr_th ("TH");
	table_tr_th.SetAttribute ("BGCOLOR", "#CCCCFF");
	table_tr_th.SetAttribute ("ALIGN", "left");
	table_tr_th.SetAttribute ("COLSPAN", "2");
	TiXmlElement table_tr_th_font ("FONT");
	table_tr_th_font.SetAttribute ("SIZE", "+2");
	TiXmlElement table_tr_th_font_b ("B");
	table_tr_th_font_b.InsertEndChild (TiXmlText ("SUMMARY"));
	table_tr_th_font.InsertEndChild (table_tr_th_font_b);
	table_tr_th.InsertEndChild (table_tr_th_font);
	table_tr.InsertEndChild (table_tr_th);
	table.InsertEndChild (table_tr);
 
	for (ssi_size_t i = 0; i < _n_objects; i++) {	

		if (ssi_strcmp(_object_names[i], Console::GetCreateName()))
		{
			continue;
		}

		TiXmlElement table_tr ("TR");

		{			
			TiXmlElement table_tr_td ("TD");
			table_tr_td.SetAttribute ("ALIGN", "right");
			table_tr_td.SetAttribute ("VALIGN", "top");
			table_tr_td.SetAttribute ("WIDTH", "1%");
			TiXmlElement table_tr_td_code ("CODE");
			table_tr_td_code.InsertEndChild (TiXmlText (_object_names[i]));	
			table_tr_td.InsertEndChild (table_tr_td_code);
			table_tr.InsertEndChild (table_tr_td);
		}

		{
			TiXmlElement table_tr_td ("TD");
			table_tr_td.SetAttribute ("ALIGN", "left");
			table_tr_td.SetAttribute ("VALIGN", "top");
			table_tr_td.SetAttribute ("WIDTH", "100%");
			TiXmlElement table_tr_td_code ("CODE");
			TiXmlElement table_tr_td_code_a ("A");
			ssi_char_t *create = _object_names[i];
			ssi_char_t *anchor = ssi_strcat ("#", create);
			table_tr_td_code_a.SetAttribute ("HREF", anchor);
			delete[] anchor;
			size_t pos = strlen (create) - 1;
			while (create[pos] != '_' && pos > 0) {
				--pos;
			}
			table_tr_td_code_a.InsertEndChild (TiXmlText (pos > 0 ? create+pos+1 : create));
			table_tr_td_code.InsertEndChild (table_tr_td_code_a);
			table_tr_td.InsertEndChild (table_tr_td_code);
			table_tr.InsertEndChild (table_tr_td);
		}

		table.InsertEndChild (table_tr);
	}

	body.InsertEndChild (table);	
	body.InsertEndChild (TiXmlElement ("P"));

	return true;

}

bool APIGenerator::CreateDetails (TiXmlElement &body) {

	body.InsertEndChild (TiXmlComment ());
	body.InsertEndChild (TiXmlComment ("========== DETAILS ==========="));
	body.InsertEndChild (TiXmlComment ());

	TiXmlElement table ("TABLE"); 
	table.SetAttribute ("BORDER", "1");
	table.SetAttribute ("WIDTH", "100%");
	table.SetAttribute ("CELLPADDING", "3");
	table.SetAttribute ("CELLSPACING", "0");

	TiXmlElement table_tr ("TR");	
	TiXmlElement table_tr_th ("TH");
	table_tr_th.SetAttribute ("BGCOLOR", "#CCCCFF");
	table_tr_th.SetAttribute ("ALIGN", "left");
	table_tr_th.SetAttribute ("COLSPAN", "2");
	TiXmlElement table_tr_th_font ("FONT");
	table_tr_th_font.SetAttribute ("SIZE", "+2");
	TiXmlElement table_tr_th_font_b ("B");
	table_tr_th_font_b.InsertEndChild (TiXmlText ("DETAILS"));
	table_tr_th_font.InsertEndChild (table_tr_th_font_b);
	table_tr_th.InsertEndChild (table_tr_th_font);
	table_tr.InsertEndChild (table_tr_th);
	table.InsertEndChild (table_tr);
	
	for (ssi_size_t i = 0; i < _n_objects; i++) {

		if (ssi_strcmp(_object_names[i], Console::GetCreateName()))
		{
			continue;
		}

		TiXmlElement table_tr ("TR");
		TiXmlElement table_tr_td ("TD");
		table_tr_td.SetAttribute ("ALIGN", "right");
		table_tr_td.SetAttribute ("VALIGN", "top");
		table_tr_td.SetAttribute ("WIDTH", "1%");
		TiXmlElement table_tr_td_code ("CODE");
		table_tr_td_code.SetAttribute ("SIZE", "+2");
		TiXmlElement table_tr_td_code_a ("A");		
		table_tr_td_code_a.SetAttribute ("NAME", _object_names[i]);
		table_tr_td_code_a.InsertEndChild (TiXmlText (_object_names[i]));
		table_tr_td_code.InsertEndChild (table_tr_td_code_a);
		table_tr_td.InsertEndChild (table_tr_td_code);
		table_tr.InsertEndChild (table_tr_td);
		TiXmlElement table_tr_td2 ("TD");
		CreateClass (table_tr_td2, _object_names[i]);
		table_tr.InsertEndChild (table_tr_td2);
		table.InsertEndChild (table_tr);
	}

	body.InsertEndChild (table);

	TiXmlElement hr ("HR");
	body.InsertEndChild (hr);

	return true;
}

bool APIGenerator::CreateClass (TiXmlElement &body, const ssi_char_t *create) {

	body.InsertEndChild (TiXmlComment ());
	body.InsertEndChild (TiXmlComment ("========== CLASS ==========="));
	body.InsertEndChild (TiXmlComment ());

	IObject *object = Factory::Create (create, 0, false);

	TiXmlElement table ("TABLE"); 
	table.SetAttribute ("BORDER", "0");
	table.SetAttribute ("WIDTH", "100%");
	table.SetAttribute ("CELLPADDING", "0");
	table.SetAttribute ("CELLSPACING", "0");
	
	{
		TiXmlElement table_tr ("TR");
		TiXmlElement table_tr_td ("TD");
		table_tr_td.SetAttribute ("WIDTH", "100%");
		table_tr_td.SetAttribute ("COLSPAN", "7");		
		table_tr_td.InsertEndChild (TiXmlText ("&nbsp;"));
		TiXmlElement table_tr_td_b ("B");	
		size_t pos = strlen (create) - 1;
		while (create[pos] != '_' && pos > 0) {
			--pos;
		}
		table_tr_td_b.InsertEndChild (TiXmlText (pos > 0 ? create+pos+1 : create));
		table_tr_td.InsertEndChild (table_tr_td_b);
		table_tr_td.InsertEndChild (TiXmlText ("&nbsp;"));
		TiXmlElement table_tr_td_code ("CODE");	
		table_tr_td_code.InsertEndChild (TiXmlText (SSI_OBJECT_NAMES[object->getType ()]));
		table_tr_td.InsertEndChild (table_tr_td_code);
		table_tr.InsertEndChild (table_tr_td);
		table.InsertEndChild (table_tr);
	}

	{
		TiXmlElement table_tr ("TR");
		TiXmlElement table_tr_td ("TD");
		table_tr_td.SetAttribute ("WIDTH", "100%");
		table_tr_td.SetAttribute ("COLSPAN", "7");
		table_tr_td.InsertEndChild (TiXmlText ("&nbsp;"));
		table_tr.InsertEndChild (table_tr_td);
		table.InsertEndChild (table_tr);

	}

	{
		TiXmlElement table_tr ("TR");
		TiXmlElement table_tr_td ("TD");
		table_tr_td.SetAttribute ("WIDTH", "1%");		
		table_tr_td.InsertEndChild (TiXmlText ("&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;"));
		table_tr.InsertEndChild (table_tr_td);
		TiXmlElement table_tr_td2 ("TD");
		table_tr_td2.SetAttribute ("WIDTH", "100%");
		table_tr_td2.SetAttribute ("COLSPAN", "7");
		TiXmlText table_tr_td_text2 (object->getInfo ());
		table_tr_td2.InsertEndChild (table_tr_td_text2);
		table_tr.InsertEndChild (table_tr_td2);
		table.InsertEndChild (table_tr);
	}

	{
		TiXmlElement table_tr ("TR");
		TiXmlElement table_tr_td ("TD");
		table_tr_td.SetAttribute ("WIDTH", "100%");
		table_tr_td.SetAttribute ("COLSPAN", "7");		
		table_tr_td.InsertEndChild (TiXmlText ("&nbsp;"));
		table_tr.InsertEndChild (table_tr_td);
		table.InsertEndChild (table_tr);

	}

	CreateOptions (table, *object);

	{
		TiXmlElement table_tr ("TR");
		TiXmlElement table_tr_td ("TD");
		table_tr_td.SetAttribute ("WIDTH", "100%");
		table_tr_td.SetAttribute ("COLSPAN", "7");
		table_tr_td.InsertEndChild (TiXmlText ("&nbsp;"));
		table_tr.InsertEndChild (table_tr_td);
		table.InsertEndChild (table_tr);
	}

	if (object->getType () == SSI_SENSOR) {

		ISensor *sensor = ssi_pcast (ISensor, object);

		{
			TiXmlElement table_tr ("TR");
			TiXmlElement table_tr_td ("TD");
			table_tr_td.SetAttribute ("WIDTH", "1%");		
			table_tr_td.InsertEndChild (TiXmlText ("&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;"));
			table_tr.InsertEndChild (table_tr_td);
			TiXmlElement table_tr_td2 ("TD");
			table_tr_td2.SetAttribute ("WIDTH", "100%");
			table_tr_td2.SetAttribute ("COLSPAN", "6");			
			table_tr_td2.InsertEndChild (TiXmlText ("Available channels:"));
			table_tr.InsertEndChild (table_tr_td2);
			table.InsertEndChild (table_tr);
		}

		{
			TiXmlElement table_tr ("TR");
			TiXmlElement table_tr_td ("TD");
			table_tr_td.SetAttribute ("WIDTH", "100%");
			table_tr_td.SetAttribute ("COLSPAN", "7");
			table_tr_td.InsertEndChild (TiXmlText ("&nbsp;"));
			table_tr.InsertEndChild (table_tr_td);
			table.InsertEndChild (table_tr);
		}

		for (ssi_size_t i = 0; i < sensor->getChannelSize (); i++) {			
			
			IChannel *channel = sensor->getChannel (i);

			TiXmlElement table_tr ("TR");

			{
				TiXmlElement table_tr_td ("TD");
				table_tr_td.SetAttribute ("WIDTH", "1%");			
				table_tr_td.InsertEndChild (TiXmlText ("&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;"));
				table_tr.InsertEndChild (table_tr_td);
			}
			{
				TiXmlElement table_tr_td ("TD");
				table_tr_td.SetAttribute ("WIDTH", "1%");
				table_tr_td.InsertEndChild (TiXmlText ("&nbsp;&nbsp;&#43;"));
				table_tr.InsertEndChild (table_tr_td);
			}	

			{
				TiXmlElement table_tr_td ("TD");
				table_tr_td.SetAttribute ("WIDTH", "1%");
				table_tr_td.SetAttribute ("style", "white-space: nowrap");
				TiXmlElement table_tr_td_code ("CODE");
				table_tr_td_code.InsertEndChild (TiXmlText ("&nbsp;"));
				table_tr_td_code.InsertEndChild (TiXmlText (channel->getName ()));
				table_tr_td_code.InsertEndChild (TiXmlText ("&nbsp;"));
				table_tr_td.InsertEndChild (table_tr_td_code);
				table_tr.InsertEndChild (table_tr_td);
			}

			{
				TiXmlElement table_tr_td ("TD");
				table_tr_td.SetAttribute ("WIDTH", "100%");
				table_tr_td.SetAttribute ("COLSPAN", "6");
				TiXmlElement table_tr_td_code ("CODE");
				table_tr_td_code.InsertEndChild (TiXmlText ("&nbsp;"));
				table_tr_td_code.InsertEndChild (TiXmlText (channel->getInfo ()));
				table_tr_td_code.InsertEndChild (TiXmlText ("&nbsp;"));
				table_tr_td.InsertEndChild (table_tr_td_code);
				table_tr.InsertEndChild (table_tr_td);
			}

			table.InsertEndChild (table_tr);

		}

		{
			TiXmlElement table_tr ("TR");
			TiXmlElement table_tr_td ("TD");
			table_tr_td.SetAttribute ("WIDTH", "100%");
			table_tr_td.SetAttribute ("COLSPAN", "7");
			table_tr_td.InsertEndChild (TiXmlText ("&nbsp;"));
			table_tr.InsertEndChild (table_tr_td);
			table.InsertEndChild (table_tr);
		}
	}

	body.InsertEndChild (table);

	const ssi_char_t *oname = object->getName ();
	if (strcmp (oname, TheEventBoard::GetCreateName ()) != 0 &&
		strcmp (oname, TheFramework::GetCreateName ()) != 0) {
		delete object;
	}

	return true;
}

bool APIGenerator::CreateOptions (TiXmlElement &table, IObject &object) {

	IOptions *options = object.getOptions ();
	if (options == 0) {
		return true;
	}
	ssi_char_t string[SSI_MAX_CHAR];	

	for (ssi_size_t i = 0; i < options->getSize (); i++) {

		TiXmlElement table_tr ("TR");

		{
			TiXmlElement table_tr_td ("TD");
			table_tr_td.SetAttribute ("WIDTH", "1%");			
			table_tr_td.InsertEndChild (TiXmlText ("&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;"));
			table_tr.InsertEndChild (table_tr_td);
		}
		{
			TiXmlElement table_tr_td ("TD");
			table_tr_td.SetAttribute ("WIDTH", "1%");
			table_tr_td.InsertEndChild (TiXmlText ("&nbsp;&nbsp;&#43;"));
			table_tr.InsertEndChild (table_tr_td);
		}	

		ssi_option_t *option = options->getOption (i);

		{
			TiXmlElement table_tr_td ("TD");
			table_tr_td.SetAttribute ("WIDTH", "1%");
			table_tr_td.SetAttribute ("style", "white-space: nowrap");
			TiXmlElement table_tr_td_code ("CODE");
			ssi_sprint (string, "&nbsp;%s&nbsp;", option->name);
			TiXmlText table_tr_td_code_text (string);
			table_tr_td_code.InsertEndChild (table_tr_td_code_text);
			table_tr_td.InsertEndChild (table_tr_td_code);
			table_tr.InsertEndChild (table_tr_td);
		}

		{
			TiXmlElement table_tr_td ("TD");
			table_tr_td.SetAttribute ("WIDTH", "1%");
			table_tr_td.SetAttribute ("style", "white-space: nowrap");
			TiXmlElement table_tr_td_code ("CODE");
			ssi_sprint (string, "&nbsp;%s&nbsp;", SSI_TYPE_NAMES[option->type]);
			TiXmlText table_tr_td_code_text (string);
			table_tr_td_code.InsertEndChild (table_tr_td_code_text);
			table_tr_td.InsertEndChild (table_tr_td_code);
			table_tr.InsertEndChild (table_tr_td);
		}

		{
			TiXmlElement table_tr_td ("TD");
			table_tr_td.SetAttribute ("WIDTH", "1%");
			table_tr_td.SetAttribute ("style", "white-space: nowrap");
			TiXmlElement table_tr_td_code ("CODE");
			ssi_sprint (string, "&nbsp;%u&nbsp;", option->num);
			TiXmlText table_tr_td_code_text (string);
			table_tr_td_code.InsertEndChild (table_tr_td_code_text);
			table_tr_td.InsertEndChild (table_tr_td_code);
			table_tr.InsertEndChild (table_tr_td);
		}

		{
			TiXmlElement table_tr_td ("TD");
			table_tr_td.SetAttribute ("WIDTH", "1%");
			table_tr_td.SetAttribute ("style", "white-space: nowrap");
			TiXmlElement table_tr_td_code ("CODE");
			ssi_char_t *value = 0;
			options->getOptionValueAsString(option->name, &value, 5);
			ssi_sprint (string, "&nbsp;'%s'&nbsp;", value);
			delete[] value;
			TiXmlText table_tr_td_code_text (string);
			table_tr_td_code.InsertEndChild (table_tr_td_code_text);
			table_tr_td.InsertEndChild (table_tr_td_code);
			table_tr.InsertEndChild (table_tr_td);
		}

		{
			TiXmlElement table_tr_td("TD");
			table_tr_td.SetAttribute("WIDTH", "1%");
			TiXmlElement table_tr_td_code("CODE");
			ssi_sprint(string, "&nbsp;%s&nbsp;", option->lock ? "LOCK" : "FREE");
			TiXmlText table_tr_td_code_text(string);
			table_tr_td_code.InsertEndChild(table_tr_td_code_text);
			table_tr_td.InsertEndChild(table_tr_td_code);
			table_tr.InsertEndChild(table_tr_td);
		}

		{
			TiXmlElement table_tr_td ("TD");
			table_tr_td.SetAttribute ("WIDTH", "100%");
			TiXmlElement table_tr_td_code ("CODE");
			ssi_sprint (string, "&nbsp;%s&nbsp;", option->help);
			TiXmlText table_tr_td_code_text (string);
			table_tr_td_code.InsertEndChild (table_tr_td_code_text);
			table_tr_td.InsertEndChild (table_tr_td_code);
			table_tr.InsertEndChild (table_tr_td);
		}

		table.InsertEndChild (table_tr);

	}

	return true;
}

}
