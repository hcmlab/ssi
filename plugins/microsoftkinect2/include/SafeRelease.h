#ifndef SSI_MICROSOFTKINECT2_SAFERELEASE
#define SSI_MICROSOFTKINECT2_SAFERELEASE

template<class Interface>
static inline void SafeRelease(Interface *& pInterfaceToRelease)
{
	if (pInterfaceToRelease != NULL) {
		pInterfaceToRelease->Release();
		pInterfaceToRelease = NULL;
	}

	::ShowWindow(0, 0);
}

#endif