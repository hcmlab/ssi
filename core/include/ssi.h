// ssi.h
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2008/09/05
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

#pragma once

#ifndef SSI_H
#define	SSI_H

#include "SSI_Cons.h"

#include "base/Factory.h"
#include "base/Array1D.h"
#include "base/String.h"
#include "base/StringList.h"
#include "base/Random.h"

#include "struct/BinTree.h"
#include "struct/Queue.h"

#include "thread/Thread.h"
#include "thread/ThreadSafe.h"
#include "thread/ClockThread.h"
#include "thread/Event.h"
#include "thread/Lock.h"
#include "thread/Mutex.h"
#include "thread/Timer.h"
#include "thread/Condition.h"
#include "thread/ThreadPool.h"
#include "thread/RunAsThread.h"

#include "ioput/option/OptionList.h"
#include "ioput/option/CmdArgOption.h"
#include "ioput/option/CmdArgParser.h"
#include "ioput/file/File.h"
#include "ioput/file/FileAscii.h"
#include "ioput/file/FileMem.h"
#include "ioput/file/FileMemAscii.h"
#include "ioput/file/FileBinary.h"
#include "ioput/file/FileTools.h"
#include "ioput/file/FilePath.h"
#include "ioput/file/FileAnnotationWriter.h"
#include "ioput/file/FileProvider.h"
#include "ioput/file/FileStreamIn.h"
#include "ioput/file/FileStreamOut.h"
#include "ioput/file/FileSamplesIn.h"
#include "ioput/file/FileSamplesOut.h"
#include "ioput/file/FileAnnotationIn.h"
#include "ioput/file/FileAnnotationOut.h"
#include "ioput/file/FileEventsIn.h"
#include "ioput/file/FileEventsOut.h"
#include "ioput/file/FileMessage.h"
#include "ioput/file/FileCSV.h"
#include "ioput/socket/Socket.h"
#include "ioput/socket/SocketUdp.h"
#include "ioput/socket/SocketTcp.h"
#include "ioput/socket/SocketOsc.h"
#include "ioput/socket/SocketOscListener.h"
#include "ioput/socket/SocketOscEventWriter.h"
#include "ioput/socket/SocketImage.h"
#include "ioput/socket/SocketMessage.h"
#include "ioput/pipe/NamedPipe.h"
#include "ioput/wav/WavTools.h"
#include "ioput/example/Example.h"
#include "ioput/example/Exsemble.h"
#include "ioput/web/WebTools.h"

#include "buffer/Buffer.h"
#include "buffer/TimeBuffer.h"

#include "signal/SignalTools.h"

#include "event/EventAddress.h"
#include "event/EventList.h"
#include "event/IESelect.h"

#include "frame/include/ssiframe.h"
#include "event/include/ssievent.h"

#include "ioput/include/ssiioput.h"
#if __ANDROID__
#else
#include "mouse/include/ssimouse.h"
#endif
#if __gnu_linux__
#ifdef SSI_USE_SDL
#include "graphic/Console.h"
#include "graphic/Window.h"
#include "graphic/Canvas.h"
#include "graphic/Monitor.h"
#include "graphic/PaintData.h"
#include "graphic/Colormap.h"
#include "graphic/GraphicTools.h"
#include "graphic/include/ssigraphic.h"
#else
#include "graphic/Window.h"
#include "graphic/WindowFallback.h"
#include "graphic/Monitor.h"
#endif
#else

#include "graphic/Console.h"
#include "graphic/Window.h"
#include "graphic/Canvas.h"
#include "graphic/Monitor.h"
#include "graphic/PaintData.h"
#include "graphic/Colormap.h"
#include "graphic/GraphicTools.h"
#include "graphic/include/ssigraphic.h"

#include "graphic/Tab.h"
#include "graphic/Grid.h"
#include "graphic/Slider.h"
#include "graphic/ComboBox.h"
#include "graphic/CheckBox.h"
#include "graphic/TextBox.h"
#include "graphic/Button.h"
#include "control/include/ssicontrol.h"
#endif

#ifdef _DEBUG
#	pragma comment(lib, "ssid.lib")
#else
#	pragma comment(lib, "ssi.lib")
#endif

#endif
