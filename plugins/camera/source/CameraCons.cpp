// link libraries
#ifdef _MSC_VER 
#pragma comment (lib, "strmiids.lib")
#pragma comment (lib, "quartz.lib")
#pragma comment (lib, "ole32.lib")
#pragma comment (lib, "comsuppw.lib")
#pragma comment (lib, "vfw32.lib")
#pragma comment (lib, "winmm.lib")
#ifdef _DEBUG
#pragma comment (lib, "msvcrtd.lib")
#pragma comment (lib, "strmbasd.lib")
#else
#pragma comment (lib, "msvcrt.lib")
#pragma comment (lib, "strmbase.lib")
#endif
#endif