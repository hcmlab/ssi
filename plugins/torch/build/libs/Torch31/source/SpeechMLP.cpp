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

#include "Allocator.h"
#include "SpeechMLP.h"
#include "Tanh.h"
#include "Sigmoid.h"
#include "SoftMax.h"
#include "LogSoftMax.h"
#include "DiskXFile.h"
#include "string_stuff.h"


namespace Torch {


SpeechMLP::SpeechMLP( char *quicknet_mlpw_filename , int n_cw_vecs_ , 
                      char *quicknet_norms_filename , bool online_norm_ ,
                      real alpha_m_ , real alpha_v_ , bool lna8_outputs_ )
{
    DiskXFile *mlpw_fd ;
    int magic , ver_code , net_type , n_layers_ , n_sections ;
    int sec_type , n_weights , data_type , bytes_per_weight , exponent ;
    int *n_units ;
    bool type_is_float ;
    real *hid_weights=NULL , *hid_bias=NULL , *out_weights=NULL , *out_bias=NULL , *real_ptr ;
    void *vals ;
    MLPNonLinTransformType nl_type ;
    
#ifdef DEBUG
    if ( (sizeof(int) != 4) || (sizeof(float) != 4) || (sizeof(double) != 8) || 
         (sizeof(short) != 2) )
        error("SpeechMLP::SpeechMLP(2) - data types are not expected sizes\n") ;
#endif

    lna8_outputs = lna8_outputs_ ;
    n_cw_vecs = n_cw_vecs_ ;
    if ( (n_cw_vecs % 2) == 0 )
        error("SpeechMLP::SpeechMLP(2) - n_cw_vecs is not odd\n") ;

    mlpw_fd = new DiskXFile( quicknet_mlpw_filename , "r" ) ;

    // Read the magic number and make sure that it is ok
    if ( mlpw_fd->read( &magic , sizeof(int) , 1 ) != 1 )
        error("SpeechMLP::SpeechMLP(2) - error reading magic number\n") ;
    if ( magic != 0x4d4c5057 )
        error("SpeechMLP::SpeechMLP(2) - magic number 0x%X incorrect\n",magic) ;

    // Read the version code and ignore
    if ( mlpw_fd->read( &ver_code , sizeof(int) , 1 ) != 1 )
        error("SpeechMLP::SpeechMLP(2) - error reading version code\n") ;

    // Read the net type
    if ( mlpw_fd->read( &net_type , sizeof(int) , 1 ) != 1 )
        error("SpeechMLP::SpeechMLP(2) - error reading net type code\n") ;

    // Interpret the net type according to the QuickNet "QN_MLPW_ntype" enumeration
    if ( (net_type == 0) || (net_type == 3) )
        nl_type = MLP_NONE ;
    else if ( net_type == 1 )
        nl_type = MLP_SOFTMAX ;
    else if ( net_type == 2 )
        nl_type = MLP_SIGMOID ;
    else
        nl_type = MLP_SOFTMAX ;   // default MLPW nettype
    
    // Read the number of layers in the neural network
    if ( mlpw_fd->read( &n_layers_ , sizeof(int) , 1 ) != 1 )
        error("SpeechMLP::SpeechMLP(2) - error reading number of layers code\n") ;
    // We assume that the number of layers will be 3
    if ( n_layers_ != 3 )
        error("SpeechMLP::SpeechMLP(2) - number of layers is not 3\n") ;
    n_units = (int *)Allocator::sysAlloc( n_layers_ * sizeof(int) ) ;
    
    // Read the number of sections in the neural network
    if ( mlpw_fd->read( &n_sections , sizeof(int) , 1 ) != 1 )
        error("SpeechMLP::SpeechMLP(2) - error reading number of sections\n") ;
    // We assume that the number of sections will be 4
    if ( n_sections != 4 )
        error("SpeechMLP::SpeechMLP(2) - number of sections is not 4\n") ;
    
    // Read the number of units in each layer of the network
    for ( int i=0 ; i<n_layers_ ; i++ )
    {
        if ( mlpw_fd->read( n_units+i , sizeof(int) , 1 ) != 1 )
            error("SpeechMLP::SpeechMLP(2) - error reading number of units in %dth layer\n",i) ;
    }

    // We have read all of the information required to "Create" the neural net.
    // QuickNet neural nets always use a Sigmoid transformation in the hidden layer.
    createMLP( n_units[0] , n_units[1] , n_units[2] , MLP_SIGMOID , nl_type ) ;
    
    // Now read the data for each section
    for ( int i=0 ; i<n_sections ; i++ )
    {
        // Read and process the header information
        // Section Type
        if ( mlpw_fd->read( &sec_type , sizeof(int) , 1 ) != 1 )
            error("SpeechMLP::SpeechMLP(2) - error reading section type\n") ;
        
        // Number of weights in the section
        if ( mlpw_fd->read( &n_weights , sizeof(int) , 1 ) != 1 )
            error("SpeechMLP::SpeechMLP(2) - error reading number of weights\n") ;

        // Read the 'datatype' field and determine what the data type is
        if ( mlpw_fd->read( &data_type , sizeof(int) , 1 ) != 1 )
            error("SpeechMLP::SpeechMLP(2) - error reading data type\n") ;
        
        if ( data_type > 32 )
        {
            // The data type is a floating point type
            type_is_float = true ;
            bytes_per_weight = data_type - 32 ;
            if ( (bytes_per_weight != 4) && (bytes_per_weight != 8) )
                error("SpeechMLP::SpeechMLP(2) - invalid bytes_per_weight for float type\n") ;
        }
        else
        {
            type_is_float = false ;
            bytes_per_weight = data_type ;
            if ( (bytes_per_weight != 1) && (bytes_per_weight != 2) && (bytes_per_weight != 4) )
                error("SpeechMLP::SpeechMLP(2) - invalid bytes_per_weight for fixed type\n") ;
            
            // The data type is fixed point - read the 'exponent value'
            if ( mlpw_fd->read( &exponent , sizeof(int) , 1 ) != 1 )
                error("SpeechMLP::SpeechMLP(2) - error reading fixed point exponent\n") ;
        }

        // Allocate a temporary place to store values read from file
        vals = Allocator::sysAlloc( n_weights * bytes_per_weight ) ;

        // Now read the weights themselves
        if ( mlpw_fd->read( vals , bytes_per_weight , n_weights ) != n_weights )
            error("SpeechMLP::SpeechMLP(2) - error reading weights\n") ;
            
        switch( sec_type )
        {
            // Interpret the section type as per the QuickNet QN_SectionSelector enumeration.
        case 0:
            // Input-hidden layer weights.
            // First check that the total number of weights is correct
            if ( n_weights != (n_units[0] * n_units[1]) )
            {
                error( "SpeechMLP::SpeechMLP(2) - n_weights=%d wrong for inp-hid weights\n" , 
                       n_weights ) ;
            }
            if ( hid_weights != NULL )
                error( "SpeechMLP::SpeechMLP(2) - duplicate hidden layer weights section\n" ) ;
             
            // Allocate an array of floats to store the weights
            hid_weights = (real *)Allocator::sysAlloc( n_weights * sizeof(real) ) ;

            // Convert the weights to real format
            convertWeightsToReal( n_weights , bytes_per_weight , type_is_float , exponent ,
                                  vals , hid_weights ) ;
            break ;
        case 1:
            // Hidden layer bias.
            // First check that the total number of weights is correct
            if ( n_weights != n_units[1] )
            {
                error( "SpeechMLP::SpeechMLP(2) - n_weights=%d wrong for hid bias weights\n" , 
                       n_weights ) ;
            }
            if ( hid_bias != NULL )
                error( "SpeechMLP::SpeechMLP(2) - duplicate hidden layer bias section\n" ) ;
             
            // Allocate an array of floats to store the weights
            hid_bias = (real *)Allocator::sysAlloc( n_weights * sizeof(real) ) ;

            // Convert the weights to real format
            convertWeightsToReal( n_weights , bytes_per_weight , type_is_float , exponent ,
                                  vals , hid_bias ) ;
            break ;
        case 2:
            // Hidden-output layer weights
            // First check that the total number of weights is correct
            if ( n_weights != (n_units[1] * n_units[2]) )
            {
                error( "SpeechMLP::SpeechMLP(2) - n_weights=%d wrong for hid-out weights\n" , 
                       n_weights ) ;
            }
            if ( out_weights != NULL )
                error( "SpeechMLP::SpeechMLP(2) - duplicate output layer weights section\n" ) ;
             
            // Allocate an array of floats to store the weights
            out_weights = (real *)Allocator::sysAlloc( n_weights * sizeof(real) ) ;

            // Convert the weights to real format
            convertWeightsToReal( n_weights , bytes_per_weight , type_is_float , exponent ,
                                  vals , out_weights ) ;
            break ;
        case 3:
            // Output layer bias.
            // First check that the total number of weights is correct
            if ( n_weights != n_units[2] )
            {
                error( "SpeechMLP::SpeechMLP(2) - n_weights=%d wrong for out bias weights\n" , 
                       n_weights ) ;
            }
            if ( out_bias != NULL )
                error( "SpeechMLP::SpeechMLP(2) - duplicate output layer bias section\n" ) ;
             
            // Allocate an array of floats to store the weights
            out_bias = (real *)Allocator::sysAlloc( n_weights * sizeof(real) ) ;

            // Convert the weights to real format
            convertWeightsToReal( n_weights , bytes_per_weight , type_is_float , exponent ,
                                  vals , out_bias ) ;
            break ;
        default:
            error("SpeechMLP::SpeechMLP(2) - invalid section type\n") ;
        }
        
        free( vals ) ;
    }
    
    // Make sure that we read all sections
    if ( (hid_weights==NULL)||(hid_bias==NULL)||(out_weights==NULL)||(out_bias==NULL) )
         error("SpeechMLP::SpeechMLP(2) - not all sections were read correctly\n") ;

    // Now fill in the weights and biases of our hidden layer linear transform
    real_ptr = hidden_layer_lin->params->data[0];
    for ( int i=0 ; i<n_units[1]*n_units[0] ; i++ )
      real_ptr[i] = hid_weights[i];

    real_ptr += n_units[1]*n_units[0];
    for ( int i=0 ; i<n_units[1] ; i++ )
        real_ptr[i] = hid_bias[i];
            
    // Now fill in the weights and biases of our output layer linear transform
    real_ptr = output_layer_lin->params->data[0] ;
    for ( int i=0 ; i<n_units[2]*n_units[1] ; i++ )
      real_ptr[i] = out_weights[i];

    real_ptr += n_units[2]*n_units[1];
    for ( int i=0 ; i<n_units[1] ; i++ )
        real_ptr[i] = out_bias[i];
    
    free( n_units ) ;
    free( hid_weights ) ;
    free( hid_bias ) ;
    free( out_weights ) ;
    free( out_bias ) ;

    delete mlpw_fd ;

    // Check that the number of MLP inputs corresponds to the number of features and
    //   the context window size that we are using.
    if ( (n_mlp_inputs % n_cw_vecs) != 0 )
        error("SpeechMLP::SpeechMLP(2) - n_mlp_inputs is not a multiple of n_cw_vecs\n") ;
    n_features = n_mlp_inputs / n_cw_vecs ;

    // Allocate memory for the feature vectors in the context window.
    // Create the 'List' object that can be passed to the 'forward' methods
    mlp_input_seq = new Sequence(1, n_mlp_inputs);
    context_window = mlp_input_seq->frames[0];
    for ( int i=0 ; i<n_mlp_inputs ; i++ )
        context_window[i] = 0.0 ;

    // If a norms file was specified, read it
    ftr_norms_means = NULL ;
    ftr_norms_inv_stddevs = NULL ;
    ftr_norms_vars = NULL ;
    if ( (quicknet_norms_filename != NULL) && (strcmp(quicknet_norms_filename,"")!=0) )
        loadFeatureNorms( quicknet_norms_filename ) ;

    // Setup everything related to online normalisation of feature vectors
    //   ie. adapting the means & stddevs used for normalisation.
    online_norm = online_norm_ ;
    if ( online_norm==true )
    {
        if ( (quicknet_norms_filename==NULL) || (strcmp(quicknet_norms_filename,"")==0) )
            error("SpeechMLP::SpeechMLP(2) - cannot do online norm without a norms file\n") ;

        alpha_m = alpha_m_ ;
        alpha_v = alpha_v_ ;
        
        // Save the means and inv stddevs we read from file, so that we can re-init between
        //   input files.
        orig_ftr_norms_means = (real *)Allocator::sysAlloc( n_features * sizeof(real) ) ;
        orig_ftr_norms_inv_stddevs = (real *)Allocator::sysAlloc( n_features * sizeof(real) ) ;
        orig_ftr_norms_vars = (real *)Allocator::sysAlloc( n_features * sizeof(real) ) ;
        
        memcpy( orig_ftr_norms_means , ftr_norms_means , n_features*sizeof(real) ) ;
        memcpy( orig_ftr_norms_inv_stddevs , ftr_norms_inv_stddevs , n_features*sizeof(real) ) ;
        memcpy( orig_ftr_norms_vars , ftr_norms_vars , n_features*sizeof(real) ) ;
    }
    else
    {
        orig_ftr_norms_means = NULL ;
        orig_ftr_norms_inv_stddevs = NULL ;
        orig_ftr_norms_vars = NULL ;
    }
}


void SpeechMLP::feedForwardOneFrame( real *features , real *mlp_outputs )
{
    int x ;
    
    // 'mlp_outputs' is assumed to be pre-allocated and assumed to have
    //   enough (ie. n_mlp_outputs) memory allocated.

    // Do we have means and stddevs so that we can normalise the input
    //   feature vector ?
    if ( ftr_norms_means != NULL )
        normaliseFeatures( features ) ;

    // Assemble the new context window.
    // Shuffle the existing context window contents down to make room for the
    //   new input vector.
    memmove( context_window , context_window+n_features , 
             (n_mlp_inputs-n_features)*sizeof(real) ) ;

    // Copy the new input feature vector
    memcpy( context_window+((n_cw_vecs-1)*n_features) , features , n_features*sizeof(real) ) ;

    // Calculate the output of the MLP
    forward( mlp_input_seq ) ;
    
    // Copy the outputs to the 'mlp_outputs' buffer.
    memcpy( mlp_outputs , outputs->frames[0] , n_mlp_outputs*sizeof(real) ) ;

    // Calculate the log of the output values if we haven't already get them from a
    //   LOG_SOFTMAX nonlinear output layer transformation
    if ( output_nl_transf != MLP_LOGSOFTMAX )
    {
        for ( int i=0 ; i<n_mlp_outputs ; i++ )
        {
            if ( lna8_outputs == true )
            {
                x = (int)floor( -24.0 * log( mlp_outputs[i] + 1e-37 ) ) ;
                if ( x > 255 ) x = 255 ;
                if ( x < 0 ) x = 0 ;
                mlp_outputs[i] = -((real)x + 0.5) / 24.0 ;
            }
            else
                mlp_outputs[i] = log( mlp_outputs[i] ) ;
        }
    }
    else if ( lna8_outputs == true )
    {
        for ( int i=0 ; i<n_mlp_outputs ; i++ )
        {
            x = (int)floor( -24.0 * mlp_outputs[i] ) ;
            if ( x > 255 ) x = 255 ;
            if ( x < 0 ) x = 0 ;
            mlp_outputs[i] = -((real)x + 0.5) / 24.0 ;
        }
    }
}
    

void SpeechMLP::feedForward( int n_frames_ , real **features , int *n_out_frames , 
                             real ***mlp_outputs )
{
    // Allocate memory for the MLP outputs here.
    // The number of output frames is less than the number of input frames
    //   if the context window size is greater than 1 (we wait until we have
    //   a full context window before starting the MLP).

    int j , start_index ;
    
    if ( n_frames_ < n_cw_vecs )
        error("SpeechMLP::feedForward - not enough input frames to fill context window\n") ;
        
    *n_out_frames = n_frames_ - n_cw_vecs + 1 ;
    *mlp_outputs = (real **)Allocator::sysAlloc( (*n_out_frames) * sizeof(real *) ) ;
    
    // Initialise the context window and online normalisation.
    start_index = initContextWindow( features ) ;

    for ( j=0 ; j<(*n_out_frames) ; j++ )
    {
        (*mlp_outputs)[j] = (real *)Allocator::sysAlloc( n_mlp_outputs * sizeof(real) ) ;
        feedForwardOneFrame( features[start_index++] , (*mlp_outputs)[j] ) ;
    }
}


void SpeechMLP::convertWeightsToReal( int n_weights , int bytes_per_weight , 
                                      bool weights_are_float , int exponent , 
                                      void *inputs_ , real *outputs_ )
{
    // If the inputs are fixed point, figure out how each value will be 
    //   scaled using the exponent value.
    real scale=0.0 ;
    if ( weights_are_float == false )
    {
        if ( bytes_per_weight == 1 )
            scale = (real)pow( 2.0 , exponent - 7 ) ;
        else if ( bytes_per_weight == 2 )
            scale = (real)pow( 2.0 , exponent - 15 ) ;
        else if ( bytes_per_weight == 4 )
            scale = (real)pow( 2.0 , exponent - 31 ) ;
        else
            scale = 0.0 ;
    }

    // Convert the input values.
    for ( int j=0 ; j<n_weights ; j++ )
    {
        if ( weights_are_float == true )
        {
            if ( bytes_per_weight == 4 )
                outputs_[j] = (real)((float *)inputs_)[j] ;
            else if ( bytes_per_weight == 8 )
                outputs_[j] = (real)((double *)inputs_)[j] ;
        }
        else
        {
            if ( bytes_per_weight == 1 )
                outputs_[j] = scale * (int)(((char *)inputs_)[j]) ;
            else if ( bytes_per_weight == 2 )
                outputs_[j] = scale * (int)(((short *)inputs_)[j]) ;
            else if ( bytes_per_weight == 4 )
                outputs_[j] = scale * ((int *)inputs_)[j] ;
        }
    }
}


void SpeechMLP::createMLP( int n_inputs_ , int n_hidden_ , int n_outputs_ ,
                           MLPNonLinTransformType hidden_nl_transf_ ,
                           MLPNonLinTransformType output_nl_transf_ ) 
{
    n_mlp_inputs = n_inputs_ ;
    n_mlp_hidden = n_hidden_ ;
    n_mlp_outputs = n_outputs_ ;
    hidden_nl_transf = hidden_nl_transf_ ;
    output_nl_transf = output_nl_transf_ ;
    
    // Setup the linear transformation associated with the hidden layer.
    hidden_layer_lin = new Linear( n_mlp_inputs , n_mlp_hidden ) ;
    addFCL( hidden_layer_lin ) ;

    // Setup the non-linear transformation associated with the hidden layer
    //   and connect it to the linear transformation.
    switch ( hidden_nl_transf )
    {
    case MLP_TANH:
        hidden_layer_nonlin = new Tanh( n_mlp_hidden ) ;
        break ;
    case MLP_SIGMOID:
        hidden_layer_nonlin = new Sigmoid( n_mlp_hidden ) ;
        break ;
    case MLP_SOFTMAX:
        hidden_layer_nonlin = new SoftMax( n_mlp_hidden ) ;
        break ;
    case MLP_LOGSOFTMAX:
        hidden_layer_nonlin = new LogSoftMax( n_mlp_hidden ) ;
        break ;
    case MLP_NONE:
        error("SpeechMLP::SpeechMLP - must have a hidden layer non-linear transformation\n") ;
        break ;
    default:
        error("SpeechMLP::SpeechMLP - invalid hidden_nl_transf\n") ;
    }
    
    addFCL( hidden_layer_nonlin ) ;

    // Setup the linear transformation associated with the output layer
    output_layer_lin = new Linear( n_mlp_hidden , n_mlp_outputs ) ;
    addFCL( output_layer_lin ) ;

    // Setup the non-linear transformation associated with the output layer
    //   and connect it to the linear transformation.
    switch ( output_nl_transf )
    {
    case MLP_TANH:
        output_layer_nonlin = new Tanh( n_mlp_outputs ) ;
        break ;
    case MLP_SIGMOID:
        output_layer_nonlin = new Sigmoid( n_mlp_outputs ) ;
        break ;
    case MLP_SOFTMAX:
        output_layer_nonlin = new SoftMax( n_mlp_outputs ) ;
        break ;
    case MLP_LOGSOFTMAX:
        output_layer_nonlin = new LogSoftMax( n_mlp_outputs ) ;
        break ;
    case MLP_NONE:
        output_layer_nonlin = NULL ;
        break ;
    default:
        error("SpeechMLP::SpeechMLP - invalid output_nl_transf\n") ;
    }
    
    if ( output_layer_nonlin != NULL )
        addFCL( output_layer_nonlin ) ;
    
    ConnectedMachine::build() ;
}


int SpeechMLP::initContextWindow( real **frames )
{
    // There are assumed to be at least 'n_cw_vecs' frames in 'frames'.
    // Copy the first '(n_cw_vecs-1)/2' vectors into 'context_window'
    //   and return the index into 'frames' for the next vector.
    //   (ie. the first vector we will input into the MLP)
 
    // Reset the means, inv stddevs and vars used for feature normalisation
    if ( online_norm == true )
    {
        memcpy( ftr_norms_means , orig_ftr_norms_means , n_features*sizeof(real) ) ;
        memcpy( ftr_norms_inv_stddevs , orig_ftr_norms_inv_stddevs , n_features*sizeof(real) ) ;
        memcpy( ftr_norms_vars , orig_ftr_norms_vars , n_features*sizeof(real) ) ;
    }

    for ( int i=1 ; i<n_cw_vecs ; i++ )
    {
        if ( ftr_norms_means != NULL )
            normaliseFeatures( frames[i-1] ) ;
        memcpy( context_window+(i*n_features) , frames[i-1] , n_features*sizeof(real) ) ;
    }

    return (n_cw_vecs-1) ;
}


void SpeechMLP::loadFeatureNorms( char *norms_filename )
{
    FILE *norms_fd ;
    char line[1000] , str[100] ;
    int n_vals ;
    
    // The input file is in the format as output by the QuickNet qnnorm utility.
    if ( (norms_filename == NULL) || (strcmp(norms_filename,"")==0) )
        return ;

    if ( n_features <= 0 )
        error("SpeechMLP::loadFeatureNorms - n_features not defined\n") ;
    
    // Open the input file
    if ( (norms_fd = fopen( norms_filename , "r" )) == NULL )
        error("SpeechMLP::loadFeatureNorms - error opening norms file\n") ;

    // Load the means header line "vec <num_features>" and check validity
    fgets( line , 1000 , norms_fd ) ;
    if ( sscanf( line , "%s %d" , str , &n_vals ) != 2 )
        error("SpeechMLP::loadFeatureNorms - error reading means header line\n") ;
    if ( (strcmp( str , "VEC" ) != 0) && (strcmp( str , "vec" ) != 0) )
        error("SpeechMLP::loadFeatureNorms - VEC not found on means header line\n") ;
    if ( n_vals != n_features )
        error("SpeechMLP::loadFeatureNorms - feature vector size does not match norms file\n") ;
    
    // Allocate memory for the means and inv stddevs
    ftr_norms_means = (real *)Allocator::sysAlloc( n_features * sizeof(real) ) ;
    ftr_norms_inv_stddevs = (real *)Allocator::sysAlloc( n_features * sizeof(real) ) ;
    ftr_norms_vars = (real *)Allocator::sysAlloc( n_features * sizeof(real) ) ;

    // Read in the means
    for ( int i=0 ; i<n_vals ; i++ )
    {
        fgets( line , 1000 , norms_fd ) ;
#ifdef USE_DOUBLE
        if ( sscanf( line , "%lf" , ftr_norms_means+i ) != 1 )
#else
        if ( sscanf( line , "%f" , ftr_norms_means+i ) != 1 )
#endif
            error("SpeechMLP::loadFeatureNorms - error reading means value\n") ;
    }

    // Read the inv stddevs header line
    fgets( line , 1000 , norms_fd ) ;
    if ( sscanf( line , "%s %d" , str , &n_vals ) != 2 )
        error("SpeechMLP::loadFeatureNorms - error reading inv stddevs header line\n") ;
    if ( (strcmp( str , "VEC" ) != 0) && (strcmp( str , "vec" ) != 0) )
        error("SpeechMLP::loadFeatureNorms - VEC not found on inv stddevs header line\n") ;
    if ( n_vals != n_features )
        error("SpeechMLP::loadFeatureNorms - feature vector size does not match norms file\n") ;

    // Read in the inv stddevs
    for ( int i=0 ; i<n_vals ; i++ )
    {
        fgets( line , 1000 , norms_fd ) ;
#ifdef USE_DOUBLE
        if ( sscanf( line , "%lf" , ftr_norms_inv_stddevs+i ) != 1 )
#else
        if ( sscanf( line , "%f" , ftr_norms_inv_stddevs+i ) != 1 )
#endif
            error("SpeechMLP::loadFeatureNorms - error reading inv stddev value\n") ;
        ftr_norms_vars[i] = 1.0 / (ftr_norms_inv_stddevs[i] * ftr_norms_inv_stddevs[i]) ;
    }

    fclose( norms_fd ) ;
}


SpeechMLP::~SpeechMLP()
{
    if ( hidden_layer_lin != NULL )
        delete hidden_layer_lin ;
    if ( hidden_layer_nonlin != NULL )
        delete hidden_layer_nonlin ;
    if ( output_layer_lin != NULL )
        delete output_layer_lin ; 
    if ( output_layer_nonlin != NULL )
        delete output_layer_nonlin ;
    if ( ftr_norms_means != NULL )
        free( ftr_norms_means ) ;
    if ( ftr_norms_inv_stddevs != NULL )
        free( ftr_norms_inv_stddevs ) ;
    if ( ftr_norms_vars != NULL )
        free( ftr_norms_vars ) ;
    if ( orig_ftr_norms_means != NULL )
        free( orig_ftr_norms_means ) ;
    if ( orig_ftr_norms_vars != NULL )
        free( orig_ftr_norms_vars ) ;
    if ( orig_ftr_norms_inv_stddevs != NULL )
        free( orig_ftr_norms_inv_stddevs ) ;
    if ( mlp_input_seq != NULL )
        delete mlp_input_seq ;
}


void SpeechMLP::normaliseFeatures( real *features )
{
    real mean , var , x ;
    
    for ( int i=0 ; i<n_features ; i++ )
    {
        if ( online_norm == true )
        {
            mean = ftr_norms_means[i] ;
            var = ftr_norms_vars[i] ;
            x = features[i] ;

            // update recursive estimate of mean 
            mean = (1.0 - alpha_m) * mean + alpha_m * x ;

            // subtract latest mean from the value 
            x -= mean ;

            // update recursive estimate of variance 
            var = (1.0 - alpha_v) * var + alpha_v * x * x ;

            // save the new bias and scale estimates (for the next frame) 
            ftr_norms_means[i] = mean ;
            ftr_norms_vars[i] = var ;
            ftr_norms_inv_stddevs[i] = 1.0 / sqrt(var) ;
            x *= ftr_norms_inv_stddevs[i] ;
            features[i] = x ;
        }
        else
        {
            // Subtract the mean
            features[i] -= ftr_norms_means[i] ;

            // Scale the difference by the inverse stddev
            features[i] *= ftr_norms_inv_stddevs[i] ;
        }
    }
}


#ifdef DEBUG
void SpeechMLP::outputText()
{
    real *real_ptr ;

    printf("num input units = %d\n",n_mlp_inputs);
    printf("num hidden units = %d\n",n_mlp_hidden);
    printf("num output units = %d\n",n_mlp_outputs);
    printf("\n");
    printf("hidden layer non-linear transformation is: ");
    switch ( hidden_nl_transf )
    {
    case MLP_TANH:
        printf("TANH\n") ;
        break ;
    case MLP_SIGMOID:
        printf("SIGMOID\n") ;
        break ;
    case MLP_SOFTMAX:
        printf("SOFTMAX\n") ;
        break ;
    case MLP_LOGSOFTMAX:
        printf("LOGSOFTMAX\n") ;
        break ;
    case MLP_NONE:
        printf("NONE\n") ;
        break ;
    default:
        printf("UNKNOWN!!\n") ;
    }
    printf("output layer non-linear transformation is: ");
    switch ( output_nl_transf )
    {
    case MLP_TANH:
        printf("TANH\n") ;
        break ;
    case MLP_SIGMOID:
        printf("SIGMOID\n") ;
        break ;
    case MLP_SOFTMAX:
        printf("SOFTMAX\n") ;
        break ;
    case MLP_LOGSOFTMAX:
        printf("LOGSOFTMAX\n") ;
        break ;
    case MLP_NONE:
        printf("NONE\n") ;
        break ;
    default:
        printf("UNKNOWN!!\n") ;
    }

    printf("HIDDEN LAYER WEIGHTS\n\n") ;
    if ( hidden_layer_lin != NULL )
    {
        real_ptr = hidden_layer_lin->params->data[0] ;
        for ( int i=0 ; i<n_mlp_hidden ; i++ )
        {
            for ( int j=0 ; j<n_mlp_inputs ; j++ )
                printf("%f\n",*(real_ptr++));
        }
        printf("\nHIDDEN LAYER BIASES\n\n") ;
        for ( int i=0 ; i<n_mlp_hidden ; i++ )
            printf("%f\n",*(real_ptr++));
    }
    if ( output_layer_lin != NULL )
    {
        printf("OUTPUT LAYER WEIGHTS\n\n") ;
        real_ptr = output_layer_lin->params->data[0] ;
        for ( int i=0 ; i<n_mlp_outputs ; i++ )
        {
            for ( int j=0 ; j<n_mlp_hidden ; j++ )
                printf("%f\n",*(real_ptr++));
        }
        printf("\nOUTPUT LAYER BIASES\n\n") ;
        for ( int i=0 ; i<n_mlp_outputs ; i++ )
            printf("%f\n",*(real_ptr++));
    }
    fflush(stdout) ;
}
#endif

}

