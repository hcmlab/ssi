#include "pch.h"
#include "ssized.h"
#include "base/Factory.h"

#ifndef DLLEXP
#define DLLEXP extern "C" __declspec(dllexport)
#endif

DLLEXP bool Register(ssi::Factory *factory, FILE *logfile, ssi::IMessage *message) {

    ssi::Factory::SetFactory(factory);

    if (logfile) {
        ssiout = logfile;
    }
    if (message) {
        ssimsg = message;
    }

    bool result = true;

    result = ssi::Factory::Register(ssi::ZedCamera::GetCreateName(), ssi::ZedCamera::Create) && result;

    return result;
}