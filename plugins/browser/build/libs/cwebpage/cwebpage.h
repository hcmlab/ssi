/*
 * This include file is meant to be included with any C/C++ source you
 * write which uses the cwebpage DLL.
 */

#ifndef __CWEBPAGE_H_INCLUDED
#define __CWEBPAGE_H_INCLUDED

#ifdef UNICODE
#define _UNICODE
#endif
#include <windows.h>
#include <exdisp.h>		/* Defines of stuff like IWebBrowser2. This is an include file with Visual C 6 and above */
#include <mshtml.h>		/* Defines of stuff like IHTMLDocument2. This is an include file with Visual C 6 and above */
#include <mshtmhst.h>	/* Defines of stuff like IDocHostUIHandler. This is an include file with Visual C 6 and above */

#ifdef __cplusplus
extern "C" {
#endif

// Passed to an app's window procedure (as a WM_NOTIFY message) whenever an
// action has occurred on the web page (and the app has asked to be informed
// of that specific action)
typedef struct {
	NMHDR			nmhdr;
	IHTMLEventObj *	htmlEvent;
	LPCTSTR			eventStr;
} WEBPARAMS; 

// Our _IDispatchEx struct. This is just an IDispatch with some
// extra fields appended to it for our use in storing extra
// info we need for the purpose of reacting to events that happen
// to some element on a web page.
typedef struct {
	IDispatch		*dispatchObj;	// The mandatory IDispatch.
	DWORD			refCount;		// Our reference count.
	IHTMLWindow2 *	htmlWindow2;	// Where we store the IHTMLWindow2 so that our IDispatch's Invoke() can get it.
	HWND			hwnd;			// The window hosting the browser page. Our IDispatch's Invoke() sends messages when an event of interest occurs.
	short			id;				// Any numeric value of your choosing that you wish to associate with this IDispatch.
	unsigned short	extraSize;		// Byte size of any extra fields prepended to this struct.
	IUnknown		*object;		// Some object associated with the web page element this IDispatch is for.
	void			*userdata;		// An extra pointer.
} _IDispatchEx;

#if UNICODE
BSTR WINAPI TStr2BStr(HWND, const WCHAR *);
typedef BSTR WINAPI TStr2BStrPtr(HWND, const WCHAR *);
#else
BSTR WINAPI TStr2BStr(HWND, const char *);
typedef BSTR WINAPI TStr2BStrPtr(HWND, const char *);
#endif
#define TSTR2BSTR TStr2BStr
#define TSTR2BSTRNAME "TStr2BStr"

void * WINAPI BStr2TStr(HWND, BSTR);
typedef void * WINAPI BStr2TStrPtr(HWND, BSTR);
#define BSTR2TSTR BStr2TStr
#define BSTR2TSTRNAME "BStr2TStr"

long WINAPI EmbedBrowserObject(HWND);
#define EMBEDBROWSEROBJECT EmbedBrowserObject
typedef long WINAPI EmbedBrowserObjectPtr(HWND);
#define EMBEDBROWSEROBJECTNAME "EmbedBrowserObject"

void WINAPI UnEmbedBrowserObject(HWND);
#define UNEMBEDBROWSEROBJECT UnEmbedBrowserObject
typedef long WINAPI UnEmbedBrowserObjectPtr(HWND);
#define UNEMBEDBROWSEROBJECTNAME "UnEmbedBrowserObject"

#ifdef UNICODE
long WINAPI DisplayHTMLPage(HWND, const WCHAR *);
typedef long WINAPI DisplayHTMLPagePtr(HWND, const WCHAR *);
#else
long WINAPI DisplayHTMLPage(HWND, const char *);
typedef long WINAPI DisplayHTMLPagePtr(HWND, const char *);
#endif
#define DISPLAYHTMLPAGE DisplayHTMLPage
#define DISPLAYHTMLPAGENAME "DisplayHTMLPage"

#ifdef UNICODE
long WINAPI DisplayHTMLStr(HWND, const WCHAR *);
typedef long WINAPI DisplayHTMLStrPtr(HWND, const WCHAR *);
#else
long WINAPI DisplayHTMLStr(HWND, const char *);
typedef long WINAPI DisplayHTMLStrPtr(HWND, const char *);
#endif
#define DISPLAYHTMLSTR DisplayHTMLStr
#define DISPLAYHTMLSTRNAME "DisplayHTMLStr"

#ifdef UNICODE
IHTMLElement * WINAPI GetWebElement(HWND, IHTMLDocument2 *, const WCHAR *, INT);
typedef IHTMLElement * WINAPI GetWebElementPtr(HWND, IHTMLDocument2 *, const WCHAR *, INT);
#else
IHTMLElement * WINAPI GetWebElement(HWND, IHTMLDocument2 *, const char *, INT);
typedef IHTMLElement * WINAPI GetWebElementPtr(HWND, IHTMLDocument2 *, const char *, INT);
#endif
#define GETWEBELEMENT GetWebElement
#define GETWEBELEMENTNAME "GetWebElement"

IHTMLElement * WINAPI GetWebSrcElement(IHTMLEventObj *);
typedef IHTMLElement * WINAPI GetWebSrcElementPtr(IHTMLEventObj *);
#define GETWEBSRCELEMENT GetWebSrcElement
#define GETWEBSRCELEMENTNAME "GetWebSrcElement"

void WINAPI ResizeBrowser(HWND, DWORD, DWORD);
#define RESIZEBROWSER ResizeBrowser
typedef void WINAPI ResizeBrowserPtr(HWND, DWORD, DWORD);
#define RESIZEBROWSERNAME "ResizeBrowser"

#define WEBPAGE_GOBACK		0
#define WEBPAGE_GOFORWARD	1
#define WEBPAGE_GOHOME		2
#define WEBPAGE_SEARCH		3
#define WEBPAGE_REFRESH		4
#define WEBPAGE_STOP		5

void WINAPI DoPageAction(HWND, DWORD);
#define DOPAGEACTION DoPageAction
typedef void WINAPI DoPageActionPtr(HWND, DWORD);
#define DOPAGEACTIONNAME "DoPageAction"

#define WORS_SUCCESS	0
#define WORS_TIMEOUT	-1
#define WORS_DESTROYED	-2

HRESULT WINAPI WaitOnReadyState(HWND, READYSTATE, DWORD, IWebBrowser2 *);
typedef HRESULT WINAPI WaitOnReadyStatePtr(HWND, READYSTATE, DWORD, IWebBrowser2 *);
#define WAITONREADYSTATE WaitOnReadyState
#define WAITONREADYSTATENAME "WaitOnReadyState"

void * WINAPI BStr2TStr(HWND, BSTR);
typedef void * WINAPI BStr2TStrPtr(HWND, BSTR);
#define BSTR2TSTR BStr2TStr
#define BSTR2TSTRNAME "BStr2TStr"

#if UNICODE
BSTR WINAPI TStr2BStr(HWND, const WCHAR *);
typedef BSTR WINAPI TStr2BStrPtr(HWND, const WCHAR *);
#else
BSTR WINAPI TStr2BStr(HWND, const char *);
typedef BSTR WINAPI TStr2BStrPtr(HWND, const char *);
#endif
#define TSTR2BSTR TStr2BStr
#define TSTR2BSTRNAME "TStr2BStr"

HRESULT WINAPI GetWebPtrs(HWND, IWebBrowser2 **, IHTMLDocument2 **);
typedef HRESULT WINAPI GetWebPtrsPtr(HWND, IWebBrowser2 **, IHTMLDocument2 **);
#define GETWEBPTRS GetWebPtrs
#define GETWEBPTRSNAME "GetWebPtrs"

HRESULT WINAPI SetWebReturnValue(IHTMLEventObj *, BOOL);
typedef HRESULT WINAPI SetWebReturnValuePtr(IHTMLEventObj *, BOOL);
#define SETWEBRETURNVALUE SetWebReturnValue
#define SETWEBRETURNVALUENAME "SetWebReturnValue"

void WINAPI FreeWebEvtHandler(IDispatch *);
typedef void WINAPI FreeWebEvtHandlerPtr(IDispatch *);
#define FREEWEBEVTHANDLER FreeWebEvtHandler
#define FREEWEBEVTHANDLERNAME "FreeWebEvtHandler"

IDispatch * WINAPI CreateWebEvtHandler(HWND, IHTMLDocument2 *, DWORD, long, IUnknown *, void *);
typedef IDispatch * WINAPI CreateWebEvtHandlerPtr(HWND, IHTMLDocument2 *, DWORD, long, IUnknown *, void *);
#define CREATEWEBEVTHANDLER CreateWebEvtHandler
#define CREATEWEBEVTHANDLERNAME "CreateWebEvtHandler"

#ifdef __cplusplus
}
#endif

#endif /* __CWEBPAGE_H_INCLUDED */
