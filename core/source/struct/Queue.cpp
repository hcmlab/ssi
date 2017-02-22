// Queue.cpp
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2008/09/08
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

#include "struct/Queue.h"
#include "thread/Lock.h"

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif

namespace ssi {

Queue::Queue (ssi_size_t n)
: _n (ssi_cast (int, n)) {

	_first = 0;
    _last = _n-1;
    _count = 0;
	_q = new void *[_n + 1];
	for (int i = 0; i < _n; i++) {
		_q[i] = 0;
	}
}

Queue::~Queue () {

	delete[] _q;
}

bool Queue::enqueue (void *x) {

	Lock lock(_mutex);

	if (_count >= _n) {
		return false;
	}

	_last = (_last+1) % _n;
	_q[_last] = x;
	_count++;

	return true;
}

bool Queue::dequeue (void **x) {

	Lock lock(_mutex);

	if (_count <= 0) {
		*x = 0;
		return false;
	}

	{
		*x = _q[_first];
		_q[_first] = 0;
		_first = (_first+1) % _n;
		_count--;
	}

	return true;
}

bool Queue::empty () {

	int count;

	{
		Lock lock (_mutex);
		count = _count;
	}

	return count <= 0;
}

bool Queue::full () {

	int count;

	{
		Lock lock (_mutex);
		count = _count;
	}

	return count >= _n;
}

void Queue::print(void(*toString) (ssi_size_t size, ssi_char_t *str, void *x), FILE *fp) {

	if (empty()) {

		ssi_fprint(fp, "[]\n");

	} else {

		Lock lock (_mutex);

		ssi_fprint(fp, "[");

		int i;

		i = _first;
		ssi_char_t string[SSI_MAX_CHAR];
		while (i != _last) {
			toString (SSI_MAX_CHAR, string, _q[i]);
			ssi_fprint(fp, "%s ", string);
			i = (i+1) % _n;
		}
		toString (SSI_MAX_CHAR, string, _q[i]);
		ssi_fprint(fp, "%s ]\n", string);
	}
}

bool Queue::save(const ssi_char_t *filepath, void(*toFile) (const ssi_char_t *filepath, FILE *fp, void *x)) {

	FILE *fp = fopen(filepath, "wb");
	if (!fp) {
		ssi_wrn("could not open file '%s'", filepath);
		return false;
	}

	bool result = false;
	result = save(filepath, fp, toFile);
	fclose(fp);

	return result;
}

bool Queue::save(const ssi_char_t *filepath, FILE *fp, void(*toFile) (const ssi_char_t *filepath, FILE *fp, void *x)) {

	fwrite(&_n, sizeof(int), 1, fp);
	fwrite(&_first, sizeof(int), 1, fp);
	fwrite(&_last, sizeof(int), 1, fp);
	fwrite(&_count, sizeof(int), 1, fp);
	
	STATE::List empty = STATE::EMPTY;
	STATE::List filled = STATE::FILLED;
	for (int i = 0; i < _n; i++) {
		if (_q[i]) {
			fwrite(&filled, sizeof(STATE::List), 1, fp);
			toFile(filepath, fp, _q[i]);
		} else {
			fwrite(&empty, sizeof(STATE::List), 1, fp);
		}
	}	

	return true;
}

Queue *Queue::Load(const ssi_char_t *filepath, void(*fromFile) (const ssi_char_t *filepath, FILE *fp, void **x)) {

	FILE *fp = fopen(filepath, "rb");
	if (!fp) {
		ssi_wrn("could not open file '%s'", filepath);
		return false;
	}

	Queue *q = Load(filepath, fp, fromFile);
	fclose(fp);
	return q;
}

Queue *Queue::Load(const ssi_char_t *filepath, FILE *fp, void(*fromFile) (const ssi_char_t *filepath, FILE *fp, void **x)) {

	int n = 0;
	fread(&n, 1, sizeof(int), fp);

	Queue *q = new Queue(n);

	fread(&q->_first, sizeof(int), 1, fp);
	fread(&q->_last, sizeof(int), 1, fp);
	fread(&q->_count, sizeof(int), 1, fp);
	
	STATE::List state = STATE::EMPTY;
	for (int i = 0; i < n; i++) {		
		fread(&state, sizeof(STATE::List), 1, fp);
		if (state == STATE::FILLED) {
			fromFile(filepath, fp, &(q->_q[i]));
		}
	}

	return q;
}

}
