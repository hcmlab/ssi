// Selection.cpp
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2010/05/20
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

#include "Selection.h"

namespace ssi {

int Selection::Compare (const void * a, const void * b)
{
	return ssi_pcast (const ISelection::score, a)->value < ssi_pcast (const ISelection::score, b)->value ? 1 : -1;
}

Selection::Selection ()
	: _n_scores (0),
	_scores (0),
	_selected (0),
	_n_selected (0),
	_pre_select (0) {
};

Selection::Selection (Selection *pre_select)
	: _n_scores (0),
	_scores (0),
	_selected (0),
	_n_selected (0),
	_pre_select (pre_select) {
};

Selection::~Selection () {
	release ();
}

void Selection::set (ssi_size_t n_scores,
	const ISelection::score *scores,
	bool sort) {
	
	release ();

	_n_scores = n_scores;	
	_scores = new ISelection::score[_n_scores];
	for (ssi_size_t i = 0; i < _n_scores; i++) {
		if (_pre_select) {
			_scores[i].index = _pre_select->_scores[scores[i].index].index;			
			_scores[i].value = scores[i].value;
		} else {
			_scores[i] = scores[i];		
		}
	}

	if (sort) {
		qsort (_scores, _n_scores, sizeof (ISelection::score), Compare);
	}

	_n_selected = n_scores;
	_selected = new ssi_size_t[_n_scores];
	for (ssi_size_t i = 0; i < _n_scores; i++) {
		_selected[i] = _scores[i].index;		
	}

}

void Selection::release () {

	delete[] _selected; _selected = 0;
	_n_selected = 0;
	delete[] _scores; _scores = 0;
	_n_scores = 0;
}

ssi_size_t Selection::selNFirst (ssi_size_t n) {

	if (!_scores) {
		ssi_wrn ("scores not set");
		return 0;
	}

	if (n > _n_scores) {
		ssi_wrn ("n exceeds number of scores");
		return 0;
	}
	
	_n_selected = n == 0 ? _n_scores : n;

	return _n_selected;
}

ssi_size_t Selection::selByScore (ssi_real_t score) {

	if (!_scores) {
		ssi_wrn ("scores not set");
		return 0;
	}

	_n_selected = 1; //return at least one
	for (ssi_size_t i = 1; i < _n_scores; i++) {
		if (_scores[i].value > score) {
			break;
		}
		++_n_selected;
	}

	return _n_selected;
}

ssi_size_t Selection::selNBest () {

	if (!_scores) {
		ssi_wrn ("scores not set");
		return 0;
	}

	ssi_real_t max_val = _scores[0].value;
	ssi_size_t max_ind = 0;
	for (ssi_size_t i = 1; i < _n_scores; i++) {
		if (_scores[i].value > max_val) {
			max_val = _scores[i].value;
			max_ind = i;
		}		
	}
	_n_selected = max_ind + 1;

	return _n_selected;
}

void Selection::print (FILE *file) {
		
	ssi_fprint (file, "#total:\t\t%u\n", _n_scores);
	ssi_fprint (file, "#selected:\t%u\n", _n_selected);

	ISelection::score *ptr = _scores;
	for (ssi_size_t i = 0; i < _n_selected; i++) {
		ssi_fprint (file, "%u: %.5f\n", ptr->index, ptr->value);
		++ptr;
	}
}

bool Selection::load (const ssi_char_t *filename, File::TYPE type) {

	release ();

	File *file = File::CreateAndOpen (type, File::READ, filename);
	if (!file) {
		ssi_wrn ("could not load selection from '%s'", filename);
		return false;
	}

	if (type == File::BINARY) {

		FILE *fp = file->getFile ();

		fread (&_n_scores, sizeof (_n_scores), 1, fp);
		fread (&_n_selected, sizeof (_n_selected), 1, fp);
		_scores = new ISelection::score[_n_scores];
		_selected = new ssi_size_t[_n_scores];
		ISelection::score *ptr = _scores;
		ssi_size_t *ptr2 = _selected;
		for (ssi_size_t i = 0; i < _n_scores; i++) {
			fread (&(ptr->index), sizeof (ssi_size_t), 1, fp);
			fread (&(ptr->value), sizeof (ssi_real_t), 1, fp);
			*ptr2++ = ptr->index;
			++ptr;
		}

	} else {

		file->setType (SSI_SIZE);
		file->read (&_n_scores, 1, 1);
		file->read (&_n_selected, 1, 1);
		_scores = new ISelection::score[_n_scores];
		_selected = new ssi_size_t[_n_scores];
		ISelection::score *ptr = _scores;
		ssi_size_t *ptr2 = _selected;
		for (ssi_size_t i = 0; i < _n_scores; i++) {
			file->setType (SSI_SIZE);
			file->read (&(ptr->index), 0, 1);
			file->setType (SSI_REAL);
			file->read (&(ptr->value), 1, 1);
			*ptr2++ = ptr->index;
			++ptr;
		}
	}

	delete file;
	return true;
}

bool Selection::save (const ssi_char_t *filename, File::TYPE type) {

	if (!_scores) {
		ssi_wrn ("scores not set");
		return false;
	}

	File *file = File::CreateAndOpen (type, File::WRITE, filename);
	if (!file) {
		ssi_wrn ("could not save selection to '%s'", filename);
		return false;
	}

	if (type == File::BINARY) {

		FILE *fp = file->getFile ();

		fwrite (&_n_scores, sizeof (_n_scores), 1, fp);
		fwrite (&_n_selected, sizeof (_n_selected), 1, fp);
		ISelection::score *ptr = _scores;
		for (ssi_size_t i = 0; i < _n_scores; i++) {
			fwrite (&(ptr->index), sizeof (ssi_size_t), 1, fp);
			fwrite (&(ptr->value), sizeof (ssi_real_t), 1, fp);
			++ptr;
		}

	} else {

		file->setType (SSI_SIZE);
		file->write (&_n_scores, 1, 1);
		file->write (&_n_selected, 1, 1);

		ISelection::score *ptr = _scores;
		for (ssi_size_t i = 0; i < _n_scores; i++) {
			file->setType (SSI_SIZE);
			file->write (&(ptr->index), 0, 1);
			file->setType (SSI_REAL);
			file->write (&(ptr->value), 1, 1);
			++ptr;
		}
	}

	delete file;
	return true;
}

}


