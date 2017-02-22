// WekaServerJNI.cpp
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2014/03/04
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

#include "WekaServerJNI.h"

namespace ssi {

void WekaServerJNI::enter () {

	JavaVMInitArgs vm_args; /* JDK/JRE 6 VM initialization arguments */
    JavaVMOption options[3];
	options[0].optionString = _class_path;     
	options[1].optionString = _lib_path;  
	//options[2].optionString = "-verbose:jni";                 
	options[2].optionString = "";                 

    vm_args.version = JNI_VERSION_1_6;
    vm_args.nOptions = 3;
    vm_args.options = options;
	vm_args.ignoreUnrecognized = JNI_TRUE;

	ssi_msg (SSI_LOG_LEVEL_BASIC, "create java virtual machine");

    /* load and initialize a Java VM, return a JNI interface
     * pointer in _env */
    jint res = JNI_CreateJavaVM (&_jvm, (void**)&_env, &vm_args);
	if (res != JNI_OK) {
		ssi_err ("ERROR: JNI_CreateJavaVM() failed (%d)", res);
	}

	ssi_msg (SSI_LOG_LEVEL_BASIC, "created java virtual machine");
}

void WekaServerJNI::run () {

	jclass StringClass = _env->FindClass("java/lang/String");
    int numOfArguments = 3;
    int argumentIndex = 0;
	char string[255];
 
    jobjectArray jargs = _env->NewObjectArray (numOfArguments, StringClass, NULL);
	_env->SetObjectArrayElement (jargs, argumentIndex++, _env->NewStringUTF (_model));
	ssi_sprint (string, "%d", _port);
	_env->SetObjectArrayElement (jargs, argumentIndex++, _env->NewStringUTF (string));
	ssi_sprint (string, "%d", _n_buffer);
	_env->SetObjectArrayElement (jargs, argumentIndex++, _env->NewStringUTF (string));

    /* invoke the Main.test method using the JNI */
    jclass cls = _env->FindClass("WekaServer");
	CheckForJNIException (_env);

    jmethodID mid = _env->GetStaticMethodID (cls, "main", "([Ljava/lang/String;)V");
	CheckForJNIException (_env);

    _env->CallStaticVoidMethod (cls, mid, jargs);
	CheckForJNIException (_env);

}

void WekaServerJNI::flush () {

	ssi_msg (SSI_LOG_LEVEL_BASIC, "destroy java virtual machine");

    /* We are done. */
    _jvm->DestroyJavaVM ();
	_jvm = 0;
	_env = 0;
}

void WekaServerJNI::CheckForJNIException (JNIEnv *_env) {

    jthrowable expt = _env->ExceptionOccurred();
    if (expt != NULL) {
        _env->ExceptionClear();
        jmethodID toString = _env->GetMethodID(_env->FindClass("java/lang/Object"), "toString", "()Ljava/lang/String;");
        jstring estring = (jstring) _env->CallObjectMethod(expt, toString);
        jboolean isCopy;
        std::string message = _env->GetStringUTFChars(estring, &isCopy);
        ssi_err (message.c_str ());
	}
}

}
