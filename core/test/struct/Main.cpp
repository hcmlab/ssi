// Main.cpp
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2015/02/26
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

#include "ssi.h"
using namespace ssi;

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif

void toString(ssi_size_t n, ssi_char_t *str, void *x) {	
	ssi_size_t i = *ssi_pcast(ssi_size_t, x);
	ssi_sprint(str, "%u", i);
}

void toFile(const ssi_char_t *filepath, FILE *fp, void *x) {
	ssi_size_t i = *ssi_pcast(ssi_size_t, x);
	fwrite(&i, sizeof(ssi_size_t), 1, fp);
}

void fromFile(const ssi_char_t *filepath, FILE *fp, void **x) {
	*x = new ssi_size_t;
	fread(*x, sizeof(ssi_size_t), 1, fp);	
}

bool ex_queue (void *arg);
bool ex_bintree (void *arg);

int main () {

#ifdef USE_SSI_LEAK_DETECTOR
	{
#endif

	ssi_print ("%s\n\nbuild version: %s\n\n", SSI_COPYRIGHT, SSI_VERSION);

#if SSI_RANDOM_LEGACY_FLAG	
	ssi_random_seed();
#endif

	Exsemble ex;
	ex.add(&ex_queue, 0, "QUEUE", "How to use queue.");
	ex.add(&ex_bintree, 0, "BINTREE", "How to use a binary tree.");
	ex.show();

#ifdef USE_SSI_LEAK_DETECTOR
	}
	_CrtDumpMemoryLeaks();
#endif

	return 0;
}

bool ex_queue (void *arg) {

	ssi_size_t n_numbers = 20;
	ssi_size_t *numbers = new ssi_size_t[n_numbers];
	Randomi random100(0, 100);
	for (ssi_size_t i = 0; i < n_numbers; i++) {
		numbers[i] = random100.next();
	}

	Queue q(5);
	q.print(toString);
	
	for (ssi_size_t i = 0; i < n_numbers; i++) {
		q.enqueue(&numbers[i]);
	}
	q.print(toString);

	void *ptr;
	for (ssi_size_t i = 0; i < 2; i++) {
		q.dequeue(&ptr);
		if (ptr) {
			ssi_print("%u ", *ssi_pcast(ssi_size_t, ptr));
		}
	}
	ssi_print("\n");

	q.save("queue", toFile);
	Queue *qq = Queue::Load("queue", fromFile);
	qq->print(toString);	
	
	while (!qq->empty()) {
		qq->dequeue(&ptr);
		if (ptr) {
			ssi_print("%u ", *ssi_pcast (ssi_size_t, ptr));
		}
	}
	ssi_print("\n");
	qq->print(toString);

	delete qq;
	delete[] numbers;

	return true;
}

bool ex_bintree(void *arg) {

	ssi_size_t n_numbers = 20;
	ssi_size_t *numbers = new ssi_size_t[n_numbers];
	Randomi random100(0, 100);
	for (ssi_size_t i = 0; i < n_numbers; i++) {
		numbers[i] = random100.next();
	}

	BinTree tree(4);
	tree.print(toString, true);

	BinTree::Node *root = tree.root();
	Randomi random(0, n_numbers - 1);
	root->ptr = numbers + random.next();
	root->left->ptr = numbers + random.next();
	root->left->right->ptr = numbers + random.next();
	root->left->ptr = numbers + random.next();
	tree.get(1, 1)->ptr = numbers + random.next();
	tree.get(2, 3)->ptr = numbers + random.next();
	BinTree::Node *node = tree.get(2, 3);
	node->left->ptr = numbers + random.next();
	node->right->ptr = numbers + random.next();
	tree.print(toString);

	tree.save("tree", toFile);
	BinTree *t = BinTree::Load("tree", fromFile);
	t->print(toString);
	delete t;

	delete[] numbers;

	return true;
}
