// Copyright (C) 2003--2004 Darren Moore (moore@idiap.ch)
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

#include "PhoneInfo.h"

namespace Torch {


PhoneInfo::PhoneInfo()
{
    n_phones = 0 ;
    phone_names = NULL ;
    sil_index = -1 ;
    pause_index = -1 ;
}


PhoneInfo::PhoneInfo( const char *phones_fname , const char *sil_name , const char *pause_name )
{
    DiskXFile phones_fd( phones_fname , "r" ) ;
    char *str , line[1000] ;
    int total_n_phones ;
    
    if ( (phones_fname == NULL) || (strcmp(phones_fname,"")==0) )
        error("PhoneInfo::PhoneInfo(2) - phones_fname undefined\n") ;
        
    n_phones = 0 ;
    phone_names = NULL ;
    sil_index = -1 ;
    pause_index = -1 ;

    // read the first line of the file and use it to determine the file type
    phones_fd.gets( line , 1000 ) ;
    if ( strstr( line , "PHONE" ) ) 
    {
        // This is a NOWAY format phone models file
        readPhonesFromNoway( &phones_fd , sil_name , pause_name ) ;
    }
    else if ( strstr( line , "~o" ) )
    {
        // This is a HTK model definition file
        readPhonesFromHTK( &phones_fd , sil_name , pause_name ) ;
    }
    else
    {
        // Assume that the file contains just a list of phone names
        //   with 1 phone name per line.
        // Do a first pass to determine the number of phones
        total_n_phones = 0 ;
        do
        {
            if ( (line[0] == '#') || ((str = strtok( line , " \r\n\t" )) == NULL) ) 
                continue ;
            total_n_phones++ ;
        }
        while ( phones_fd.gets( line , 1000 ) != NULL ) ;

        // Allocate some memory for the list of phone names.
        phone_names = (char **)allocator->alloc( total_n_phones * sizeof(char *) ) ;
        phones_fd.seek( 0 , SEEK_SET ) ;
        
        while ( phones_fd.gets( line , 1000 ) != NULL )
        {
            if ( (line[0] == '#') || ((str = strtok( line , " \r\n\t" )) == NULL) ) 
                continue ;

            if ( n_phones >= total_n_phones )
                error("PhoneInfo::PhoneInfo - n_phones exceeds expected\n") ;
                
            phone_names[n_phones] = (char *)allocator->alloc( (strlen(str)+1) * sizeof(char) ) ;
            strcpy( phone_names[n_phones] , str ) ;

            if ( (sil_name != NULL) && (strcmp(sil_name,str)==0) )
            {
                if ( sil_index >= 0 )
                    error("PhoneInfo::PhoneInfo(2) - sil_index already defined\n") ;
                sil_index = n_phones ;
            }
            if ( (pause_name != NULL) && (strcmp(pause_name,str)==0) )
            {
                if ( pause_index >= 0 )
                    error("PhoneInfo::PhoneInfo(2) - pause_index already defined\n") ;
                pause_index = n_phones ;
            }

            n_phones++ ;
        }

        if ( n_phones != total_n_phones )
            error("PhoneInfo::PhoneInfo(2) - unexpected n_phones\n") ;
    }

    if ( (sil_name != NULL) && (strcmp(sil_name,"")!=0) && (sil_index<0) )
        error("PhoneInfo::PhoneInfo(2) - silence phone not found\n") ;
    if ( (pause_name != NULL) && (strcmp(pause_name,"")!=0) && (pause_index<0) ) 
        error("PhoneInfo::PhoneInfo(2) - pause phone not found\n") ;
}
            

PhoneInfo::~PhoneInfo()
{
}


void PhoneInfo::addPhone( char *phone_name , bool is_sil , bool is_pause )
{
    if ( phone_name == NULL )
        return ;

    phone_names = (char **)allocator->realloc( phone_names , (n_phones+1)*sizeof(char *) ) ;
    phone_names[n_phones] = (char *)allocator->alloc( (strlen(phone_name)+1)*sizeof(char) ) ;
    strcpy( phone_names[n_phones] , phone_name ) ;

    if ( is_sil == true )
    {
        if ( sil_index >= 0 )
            error("PhoneInfo::addPhone - silence phone already defined\n") ;
        sil_index = n_phones ;
    }
    if ( is_pause == true )
    {
        if ( pause_index >= 0 )
            error("PhoneInfo::addPhone - pause phone already defined\n") ;
        pause_index = n_phones ;
    }
    
    n_phones++ ;
}


char *PhoneInfo::getPhone( int index )
{
    if ( (index < 0) || (index >= n_phones) )
        error("PhoneInfo::getPhone - index out of range\n") ;

    return phone_names[index] ;
}


int PhoneInfo::getIndex( char *phone_name )
{
    if ( phone_name == NULL )
        error("PhoneInfo::getIndex - phone_name is NULL\n") ;

    // Just do a linear search.
    for ( int i=0 ; i<n_phones ; i++ )
    {
        if ( strcmp( phone_name , phone_names[i] ) == 0 )
            return i ;
    }

    return -1 ;
}


void PhoneInfo::readPhonesFromNoway( DiskXFile *phones_fd , const char *sil_name , const char *pause_name )
{
    char line[1000] , str[1000] ;
    int cnt=0 , n_states , index ;
    
    // Assume the first line of the file has already been read.
    // The second line contains the number of phones in the file.
    phones_fd->gets( line , 1000 ) ;
    if ( sscanf( line , "%d" , &n_phones ) != 1 )
        error("PhoneInfo::readPhonesFromNoway - error reading n_phones\n") ;

    phone_names = (char **)allocator->alloc( n_phones * sizeof(char *) ) ;
    
    while ( phones_fd->gets( line , 1000 ) != NULL )
    {
        // interpret the line containing the index, n_states, name fields
        if ( sscanf( line , "%d %d %s" , &index , &n_states , str ) != 3 )
            error("PhoneInfo::readPhonesFromNoway - error reading index,n_st,name line\n") ;
        if ( index != (cnt+1) )
            error("PhoneInfo::readPhonesFromNoway - phone index mismatch\n") ;
        
        // add the phone to our list
        phone_names[cnt] = (char *)allocator->alloc( (strlen(str)+1)*sizeof(char) ) ;
        strcpy( phone_names[cnt] , str ) ;

        if ( (sil_name != NULL) && (strcmp(sil_name,str)==0) )
        {
            if ( sil_index >= 0 )
                error("PhoneInfo::readPhonesFromNoway - sil_index already defined\n") ;
            sil_index = cnt ;
        }
        if ( (pause_name != NULL) && (strcmp(pause_name,str)==0) )
        {
            if ( pause_index >= 0 )
                error("PhoneInfo::readPhonesFromNoway - pause_index already defined\n") ;
            pause_index = cnt ;
        }

        // There are (n_states+1) lines before the next line containing a phone name.
        // Read and discard.
        for ( int i=0 ; i<(n_states+1) ; i++ )
            phones_fd->gets( line , 1000 ) ;

        cnt++ ;
    }

    if ( cnt != n_phones )
        error("PhoneInfo::readPhonesFromNoway - n_phones mismatch\n") ;
}
        
        
void PhoneInfo::readPhonesFromHTK( DiskXFile *phones_fd , const char *sil_name , const char *pause_name )
{
    char line[1000] , *str ;
    int total_n_phones=0 ;
    
    // Assume the first line of the file has already been read.
    // Do a first pass of the file to determine the number of phones.
    while ( phones_fd->gets( line , 1000 ) != NULL )
    {
        if ( strstr( line , "~h" ) != NULL )
            total_n_phones++ ;
    }

    // Allocate memory
    phone_names = (char **)allocator->alloc( total_n_phones * sizeof(char *) ) ;
    phones_fd->seek( 0 , SEEK_SET ) ;
    
    n_phones = 0 ;
    while ( phones_fd->gets( line , 1000 ) != NULL )
    {
        if ( strstr( line , "~h" ) != NULL )
        {
            strtok( line , "\"" ) ; // get past the ~h
            if ( (str = strtok( NULL , "\"" )) == NULL )
                error("PhoneInfo::readPhonesFromHTK - could not locate phone name\n") ;
           
            if ( n_phones >= total_n_phones )
                error("PhoneInfo::readPhonesFromHTK - n_phones exceeds expected\n") ;
            
            phone_names[n_phones] = (char *)allocator->alloc( (strlen(str)+1)*sizeof(char) ) ;
            strcpy( phone_names[n_phones] , str ) ;

            if ( (sil_name != NULL) && (strcmp(sil_name,str)==0) )
            {
                if ( sil_index >= 0 )
                    error("PhoneInfo::readPhonesFromHTK - sil_index already defined\n") ;
                sil_index = n_phones ;
            }
            if ( (pause_name != NULL) && (strcmp(pause_name,str)==0) )
            {
                if ( pause_index >= 0 )
                    error("PhoneInfo::readPhonesFromHTK - pause_index already defined\n") ;
                pause_index = n_phones ;
            }

            n_phones++ ;
        }
    }

    if ( total_n_phones != n_phones )
        error("PhoneInfo::readPhonesFromHTK - unexpected n_phones\n") ;
}
    

#ifdef DEBUG
void PhoneInfo::outputText()
{
    printf("PhoneInfo: n_phones=%d sil_index=%d pause_index=%d\n",n_phones,sil_index,pause_index) ;
    for ( int i=0 ; i<n_phones ; i++ )
        printf("%s\n",phone_names[i]) ;
    printf("\n") ;
}
#endif


}
