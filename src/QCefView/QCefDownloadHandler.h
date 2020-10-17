#pragma once

#include "include/cef_base.h"
#include "include/cef_client.h"
#include "include/wrapper/cef_helpers.h"

class QCefDownloadHandler: public CefDownloadHandler {
public:
	QCefDownloadHandler();

	// CefDownloadHandler methods
	void OnBeforeDownload(
		CefRefPtr<CefBrowser> browser,
		CefRefPtr<CefDownloadItem> download_item,
		const CefString& suggested_name,
		CefRefPtr<CefBeforeDownloadCallback> callback) OVERRIDE;
	void OnDownloadUpdated(
		CefRefPtr<CefBrowser> browser,
		CefRefPtr<CefDownloadItem> download_item,
		CefRefPtr<CefDownloadItemCallback> callback) OVERRIDE;
	// Include the default reference counting implementation.

	IMPLEMENT_REFCOUNTING(QCefDownloadHandler);
};

