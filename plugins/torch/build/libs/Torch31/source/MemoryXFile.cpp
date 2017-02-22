// Copyright (C) 2003--2004 Ronan Collobert (collober@idiap.ch)
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

#include "MemoryXFile.h"

namespace Torch {

IMPLEMENT_NEW_LIST(MemoryXFileList, MemoryXFileNode);

char MemoryXFile::petit_message_pour_melanie[10000];

MemoryXFile::MemoryXFile(MemoryXFileList *memory_, int size_, int buffer_format_size_)
{
  addIOption("buffer size", &buffer_size, 65536, "buffer size for writing");

  buffer_format_size = buffer_format_size_;
  buffer_format = (char *)allocator->alloc(buffer_format_size);

  // Copy the list, but not inside
  memory = new(allocator) MemoryXFileList;
  for(int i = 0; i < memory_->n_nodes; i++)
  {
    MemoryXFileNode *node_ = (MemoryXFileNode *)memory->allocator->alloc(sizeof(MemoryXFileNode));
    node_->size = memory_->nodes[i]->size;
    node_->mem = memory_->nodes[i]->mem;
    memory->addNode(node_);
  }

  /*** Remarque:
       Si je me trompe pas, size c'est ce qui est ecrit.
       (genre si je fais des seek, je sais ce qu'il y a).
       total_size, c'est ce qui est alloue.
  ****/

  // Check the size
  if(size_ < 0)
  {
    total_size = 0;
    for(int i = 0; i < memory->n_nodes; i++)
      total_size += memory->nodes[i]->size;
    size = total_size;
  }
  else
  {
    size = size_;
    total_size = size_;
  }

  // Boxon
  position = 0;
  internal_memory_node_index = 0;
  internal_position_in_the_node = 0;
  is_eof = false;
}

MemoryXFile::MemoryXFile(void *memory_, int size_, int buffer_format_size_)
{
  addIOption("buffer size", &buffer_size, 65536, "buffer size for writing");

  buffer_format_size = buffer_format_size_;
  buffer_format = (char *)allocator->alloc(buffer_format_size);

  // Copy the memory_, but not inside
  memory = new(allocator) MemoryXFileList;
  MemoryXFileNode *node_ = (MemoryXFileNode *)memory->allocator->alloc(sizeof(MemoryXFileNode));
  node_->size = size_;
  node_->mem = memory_;
  memory->addNode(node_);

  // Check the size
  size = size_;
  total_size = size_;

  // Boxon
  position = 0;
  internal_memory_node_index = 0;
  internal_position_in_the_node = 0;
  is_eof = false;
}

MemoryXFile::MemoryXFile(int buffer_format_size_)
{
  addIOption("buffer size", &buffer_size, 65536, "buffer size for writing");

  buffer_format_size = buffer_format_size_;
  buffer_format = (char *)allocator->alloc(buffer_format_size);

  // The list...
  memory = new(allocator) MemoryXFileList;

  // The size...
  size = 0;
  total_size = 0;

  // Le boxon...
  position = 0;
  internal_memory_node_index = 0;
  internal_position_in_the_node = 0;
  is_eof = false;
}

int MemoryXFile::read(void *ptr, int block_size, int n_blocks)
{
  // Check eof
  if(position == size)
  {
    is_eof = true;
    return 0;
  }

  // Check what to read
  int size_to_read = block_size*n_blocks;
  if((size-position) < size_to_read)
  {
    // On va tomber sur la fin du fichier.
    is_eof = true;
    size_to_read = size-position;
  }

  if(!size_to_read)
    return 0;

  // Read it
  int size_read = size_to_read;
  char *w_ptr = (char *)ptr;
  char *r_ptr = (char *)memory->nodes[internal_memory_node_index]->mem;
  while(size_to_read--)
  {
    if(internal_position_in_the_node == memory->nodes[internal_memory_node_index]->size)
    {
      internal_position_in_the_node = 0;
      r_ptr = (char *)memory->nodes[++internal_memory_node_index]->mem;
    }
    *w_ptr++ = r_ptr[internal_position_in_the_node++];
  }

  // Tchao boy
  position += size_read;
  return(size_read);
}

int MemoryXFile::write(void *ptr, int block_size, int n_blocks)
{
  int size_to_write = block_size*n_blocks;
  if(!size_to_write)
    return 0;

  char *r_ptr = (char *)ptr;

  // If there is still some space...
  if((total_size-position) > 0)
  {
    int size_to_write_now;
    if(size_to_write < (total_size-position))
      size_to_write_now = size_to_write;
    else
      size_to_write_now = total_size-position;

    size_to_write -= size_to_write_now;
    
    char *w_ptr = (char *)memory->nodes[internal_memory_node_index]->mem;
    while(size_to_write_now--)
    {
      if(internal_position_in_the_node == memory->nodes[internal_memory_node_index]->size)
      {
        internal_position_in_the_node = 0;
        w_ptr = (char *)memory->nodes[++internal_memory_node_index]->mem;
      }
      w_ptr[internal_position_in_the_node++] = *r_ptr++;
    }
  }

  // Still something to write ?
  if(size_to_write)
  {
    int new_block_size = (size_to_write < buffer_size ? buffer_size : size_to_write);
    MemoryXFileNode *node_ = (MemoryXFileNode *)memory->allocator->alloc(sizeof(MemoryXFileNode));
    node_->size = new_block_size;
    node_->mem = memory->allocator->alloc(new_block_size);
    memory->addNode(node_);
    total_size += new_block_size;
    internal_memory_node_index = memory->n_nodes-1;
    memcpy(memory->nodes[internal_memory_node_index]->mem, r_ptr, size_to_write);
    internal_position_in_the_node = size_to_write;
  }

  // Tchao girl
  position += block_size*n_blocks;

  // Check si on depasse...
  if(position > size)
    size = position;
  return(block_size*n_blocks);
}

int MemoryXFile::eof()
{
  return is_eof;
}

int MemoryXFile::flush()
{
  return 0;
}

void MemoryXFile::rewind()
{
  seek(0L, SEEK_SET);
}

int MemoryXFile::printf(const char *format, ...)
{
  va_list args;
  va_start(args, format);
  int res = vsprintf(petit_message_pour_melanie, format, args);
  va_end(args);

  write(petit_message_pour_melanie, 1, strlen(petit_message_pour_melanie));  
  return res;
}

int MemoryXFile::scanf(const char *format, void *ptr)
{
  // DEBUG: ne sette pas le eof si on tombe sur la fin du fichier en lisant.
  // A CORRIGER!
  // DEBUG: correction faite le 19/05/2004
  // DEBUG: n_lus recupere par %n, ce qui n'est pas conseille. arg.

  // Check eof
  if(position == size)
  {
    is_eof = true;
    return 0;
  }

  concat();

  strcpy(buffer_format, format);
  strcat(buffer_format, "%n");
  int n_lus;
  int res = sscanf(((char *)memory->nodes[0]->mem)+position, buffer_format, ptr, &n_lus);

  if(res == EOF)
  {
    position = size;
    internal_position_in_the_node = size;
  }
  else
  {
    position += n_lus;
    internal_position_in_the_node += n_lus;
  }

  if(position == size)
    is_eof = true;

  if(position > size)
    error("MemoryXFile: fatal read error, you have a bug in your code!");

  return res;
}

void MemoryXFile::concat()
{
  int the_size = size;
  if(!the_size)
    return;

  if(memory->n_nodes < 1)
    return;

  if(memory->n_nodes == 1)
  {
    if(size > 0)
    {
      if(((char *)memory->nodes[0]->mem)[size-1] == '\0')
        return;
    }
  }

  char *big_buffer = (char *)Allocator::sysAlloc(the_size+1);
  big_buffer[the_size] = '\0';

  char *dest = big_buffer;
  for(int i = 0; i < memory->n_nodes; i++)
  {
    int size_ = memory->nodes[i]->size;
    if(size_ < the_size)
    {
      memcpy(dest, memory->nodes[i]->mem, size_);
      the_size -= size_;
      dest += size_;      
    }
    else
    {
      memcpy(dest, memory->nodes[i]->mem, the_size);
      break;
    }
  }

  // Scrappe the list
  allocator->free(memory);

  // Copy the list, but not inside
  memory = new(allocator) MemoryXFileList;

  MemoryXFileNode *node_ = (MemoryXFileNode *)memory->allocator->alloc(sizeof(MemoryXFileNode));
  node_->size = size+1;
  node_->mem = big_buffer;
  memory->addNode(node_);
  memory->allocator->retain(big_buffer);

  // Check the size
  total_size = size+1;

  // Boxon
  internal_memory_node_index = 0;
  internal_position_in_the_node = position;
}

long MemoryXFile::tell()
{
  return((long)position);
}

int MemoryXFile::seek(long offset, int whence)
{
  int new_pos = 0;
  switch(whence)
  {
    case SEEK_SET:
      new_pos = (int)offset;
      break;
    case SEEK_CUR:
      new_pos = position + (int)offset;
      break;
    case SEEK_END:
      new_pos = size - (int)offset;
      break;
  }

  if( (new_pos > size) || (new_pos < 0) )
    return -1;

  int new_pos_ = new_pos;
  int internal_memory_node_index_ = 0;
  while(new_pos_ >= memory->nodes[internal_memory_node_index_]->size)
    new_pos_ -= memory->nodes[internal_memory_node_index_]->size;

  // Boxon
  position = new_pos;
  internal_memory_node_index = internal_memory_node_index_;
  internal_position_in_the_node = new_pos_;
  is_eof = false; // Efface le flag de fin.

  return 0;
}

char *MemoryXFile::gets(char *dest, int size_)
{
  // Check eof
  if(position == size)
  {    
    is_eof = true;
    return NULL;
  }

  // Check what to read
  // Faire gaffe au '\0' en plus a mettre a la fin...
  int size_to_read = size_-1;
  if((size-position) < size_to_read)
  {
    size_to_read = size-position;
    // On *risque* de tomber sur la fin du fichier.
    is_eof = true;
  }

  if(!size_to_read)
    return NULL;

  // Read it
  int size_read = 0;
  char *w_ptr = dest;
  char *r_ptr = (char *)memory->nodes[internal_memory_node_index]->mem;
  while(size_to_read--)
  {
    if(internal_position_in_the_node == memory->nodes[internal_memory_node_index]->size)
    {
      internal_position_in_the_node = 0;
      r_ptr = (char *)memory->nodes[++internal_memory_node_index]->mem;
    }

    char z = r_ptr[internal_position_in_the_node++];
    size_read++;
    *w_ptr++ = z;

    if(z == '\n')
    {
      // On n'est pas tombe sur la fin du fichier.
      is_eof = false;
      break;
    }
  }

  *w_ptr++ = '\0';

  // Ye Ye Ye
  position += size_read;

  return dest;
}

MemoryXFile::~MemoryXFile()
{
}

}
