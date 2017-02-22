// Copyright (C) 2003--2004 Johnny Mariethoz (Johnny.Mariethoz@idiap.ch)
//                
// This file is part of Torch 3.1.
//
// All rights reserved.
// 
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
// 1. Redistributions of source code must retain the above copyright
//    notice, this list of conditions and the following disclaimer.
// 2. Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the
//    documentation and/or other materials provided with the distribution.
// 3. The name of the author may not be used to endorse or promote products
//    derived from this software without specific prior written permission.
// 
// THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
// IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
// OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
// INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
// NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
// THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include "HTKDataSet.h"
#include "IOHTK.h"
#include "IOHTKTarget.h"
#include "IOSub.h"
#include "IOMulti.h"

namespace Torch {

HTKDataSet::HTKDataSet(const char *inputs_filename, bool one_file_is_one_sequence, int max_load, const char *targets_filename, LexiconInfo* lex_, bool words)
{
  io_allocator = new Allocator;

	IOHTK* io_inputs = new(io_allocator) IOHTK(inputs_filename, one_file_is_one_sequence, max_load,true);
	
  IOHTKTarget* io_targets = NULL;
	if(targets_filename) {
    n_per_frame = (int)(io_inputs->header->sample_period);
		io_targets = new(io_allocator) IOHTKTarget(targets_filename, lex_, n_per_frame, words);
  }

  MemoryDataSet::init(io_inputs, io_targets);
  message("HTKDataSet: %d examples loaded", n_examples);
	
	delete io_allocator;
}

HTKDataSet::HTKDataSet(char **inputs_filenames, char ** targets_filenames, int n_files_, bool one_file_is_one_sequence, int max_load, LexiconInfo* lex_, bool words)
{

  io_allocator = new Allocator;
	
	if(n_files_ <= 0)
    error("HTKDataSet: check the number of files!");

	//inputs
	int n_files = n_files_;

  IOHTK **io_files = (IOHTK **)io_allocator->alloc(sizeof(IOHTK *)*n_files_);
  if(max_load > 0)
  {
    int i = 0;
    while( (max_load > 0) && (i < n_files_) )
    {
        io_files[i] = new(io_allocator) IOHTK(inputs_filenames[i], one_file_is_one_sequence, max_load, true);
				max_load -= io_files[i]->n_sequences;
				i++;
    }
    n_files = i;
  }
  else
  {
      for(int i = 0; i < n_files_; i++) 
        io_files[i] = new(io_allocator) IOHTK(inputs_filenames[i], one_file_is_one_sequence, -1, true);
  }
  
	IOMulti* io_inputs = new(io_allocator) IOMulti((IOSequence**)io_files, n_files);
	
	//targets 
	IOMulti* io_targets = NULL;
	if(targets_filenames){
    n_per_frame = (int)(io_files[0]->header->sample_period);
		IOHTKTarget **io_files_targets = (IOHTKTarget **)io_allocator->alloc(sizeof(IOHTKTarget *)*n_files);
		for ( int i=0; i < n_files;i++)
			io_files_targets[i] = new(io_allocator) IOHTKTarget(targets_filenames[i], lex_, n_per_frame, words);
		io_targets = new(io_allocator) IOMulti((IOSequence**)io_files_targets, n_files);
	}

  
	MemoryDataSet::init(io_inputs, io_targets);
  message("HTKDataSet: %d examples loaded", n_examples);

	delete io_allocator;
}



HTKDataSet::~HTKDataSet()
{
}

}
