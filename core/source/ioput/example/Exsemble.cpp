// Exsemble.h
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2007/11/13
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

#include "ioput/example/Exsemble.h"
#include "base/Factory.h"
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

Exsemble::Exsemble(ssi_size_t width) {

	WIDTH = width;
}

Exsemble::~Exsemble () {
}

void Exsemble::console(int top, int left, int width, int height) {
	Console *console = ssi_pcast(Console, Factory::GetObjectFromId(SSI_FACTORY_CONSOLE_ID));
	console->setPosition(ssi_rect(top, left, width, height));
	printf("\r");
}

void Exsemble::add(Example::example_fptr_t func, void *arg, const char *name, const char *info) {

	push_back(Example(func, arg, name, info));
}

void Exsemble::runExample(FILE *fp, ssi_size_t num, Example &ex) {

	drawTop(fp);
	drawEmpty(fp);
	drawExample(fp, num, ex._name, ex._info);
	drawEmpty(fp);
	drawBottom(fp);

	bool result = ex.run();
	drawResult(fp, result);

	ssi_print("\n\n");
	ssi_print_off("press enter to continue\n\n");
	getchar();

	Factory::ClearObjects();
}

void Exsemble::show(FILE *fp) {

	while (true) {

		drawTop(fp);
		drawEmpty(fp);

		std::vector<Example>::iterator it;
		ssi_size_t count = 1;
		for (it = begin(); it != end(); it++) {
			drawExample (fp, count++, it->_name, it->_info);			
			drawEmpty(fp);
		}
		drawEmpty(fp);
		drawName(fp, ssi_cast (ssi_size_t, size() + 1), "RUN ALL");
		drawName(fp, 0, "QUIT");
		drawEmpty(fp);
		drawBottom(fp);

		ssi_size_t selection = 0;
		ssi_print("> ");
		int result = scanf("%u", &selection);
		getchar();
		if (result == 1) {
			if (selection == 0) {
				return;
			}
			else if (selection <= size()) {
				runExample(fp, selection, at(selection - 1));
			} else if (selection == size()+1) {
				ssi_size_t num = 1;
				for (it = begin(); it != end(); it++, num++) {
					runExample(fp, num, *it);
				}
				return;
			}
		}
	}
}

void Exsemble::drawEmpty(FILE *fp) {

	ssi_fprint(fp, "%c", BORDER::VERTICAL);
	drawChars(fp, WIDTH - 2, ' ');
	ssi_fprint(fp, "%c\n", BORDER::VERTICAL);
}

void Exsemble::drawExample(FILE *fp, ssi_size_t num, const ssi_char_t *name, const ssi_char_t *info) {

	drawName(fp, num, name);
	drawInfo(fp, info);
}

void Exsemble::drawResult(FILE *fp, bool result) {

	drawTop(fp);
	drawEmpty(fp);
	ssi_fprint(fp, "%c", BORDER::VERTICAL);
	drawChars(fp, BORDER::INDENT_LEFT, ' ');
	ssi_fprint(fp, "%s", result ? "OK" : "FAILED");
	drawChars(fp, WIDTH - BORDER::INDENT_LEFT - BORDER::INDENT_RIGHT - BORDER::BORDER_WIDTH - (result ? 2 : 6), ' ');
	ssi_fprint(fp, "%c\n", BORDER::VERTICAL);
	drawEmpty(fp);
	drawBottom(fp);
}

void Exsemble::drawName(FILE *fp, ssi_size_t num, const ssi_char_t *name) {

	ssi_fprint(fp, "%c %2u  %s", BORDER::VERTICAL, num, name);
	drawChars(fp, WIDTH - BORDER::INDENT_LEFT - BORDER::INDENT_RIGHT - 2 * BORDER::BORDER_WIDTH - ssi_strlen(name), ' ');
	ssi_fprint(fp, " %c\n", BORDER::VERTICAL);	
}

void Exsemble::drawInfo(FILE *fp, const ssi_char_t *info) {

	ssi_size_t n_words = ssi_split_string_count(info, ' ');
	ssi_char_t **words = new ssi_char_t *[n_words];
	ssi_split_string(n_words, words, info, ' ');

	ssi_size_t rem = WIDTH - BORDER::INDENT_LEFT - BORDER::INDENT_RIGHT - 2 * BORDER::BORDER_WIDTH;
	ssi_fprint(fp, "%c", BORDER::VERTICAL);
	drawChars(fp, BORDER::INDENT_LEFT, ' ');
	ssi_size_t n;
	for (ssi_size_t i = 0; i < n_words; i++) {
		n = ssi_strlen(words[i]);
		if (n+1 > rem) {
			drawChars(fp, rem, ' ');
			ssi_fprint(fp, " %c\n%c", BORDER::VERTICAL, BORDER::VERTICAL);			
			drawChars(fp, BORDER::INDENT_LEFT, ' ');
			rem = WIDTH - BORDER::INDENT_LEFT - BORDER::INDENT_RIGHT - 2 * BORDER::BORDER_WIDTH;
		}
		ssi_fprint(fp, "%s ", words[i]);
		rem -= n + 1;
	}
	drawChars(fp, rem, ' ');
	ssi_fprint(fp, " %c\n", BORDER::VERTICAL);
	rem = WIDTH - BORDER::INDENT_LEFT - BORDER::INDENT_RIGHT - 2 * BORDER::BORDER_WIDTH;	

	for (ssi_size_t i = 0; i < n_words; i++) {
		delete[] words[i];
	}
	delete[] words;
}

void Exsemble::drawChars(FILE *fp, ssi_size_t n, char c) {

	for (ssi_size_t i = 0; i < n; i++) {
		ssi_fprint(fp, "%c", c);
	}
}

void Exsemble::drawTop(FILE *fp) {

	ssi_fprint(fp, "%c", BORDER::TOP_LEFT);
	drawChars(fp, WIDTH - 2, BORDER::HORIZONTAL);
	ssi_fprint(fp, "%c\n", BORDER::TOP_RIGHT);
}

void Exsemble::drawBottom(FILE *fp) {

	ssi_fprint(fp, "%c", BORDER::BOTTOM_LEFT);
	drawChars(fp, WIDTH - 2, BORDER::HORIZONTAL);
	ssi_fprint(fp, "%c\n", BORDER::BOTTOM_RIGHT);
}

}
