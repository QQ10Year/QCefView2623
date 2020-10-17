#include "QCefDownloadHandler.h"


QCefDownloadHandler::QCefDownloadHandler() {

}

void QCefDownloadHandler::OnBeforeDownload(
	CefRefPtr<CefBrowser> browser,
	CefRefPtr<CefDownloadItem> download_item,
	const CefString& suggested_name,
	CefRefPtr<CefBeforeDownloadCallback> callback) {
	CEF_REQUIRE_UI_THREAD();

	// Continue the download and show the "Save As" dialog.
	callback->Continue(suggested_name, true);
}

void QCefDownloadHandler::OnDownloadUpdated(
	CefRefPtr<CefBrowser> browser,
	CefRefPtr<CefDownloadItem> download_item,
	CefRefPtr<CefDownloadItemCallback> callback) {
	CEF_REQUIRE_UI_THREAD();

	if (download_item->IsComplete()) {
		/*test_runner::Alert(
			browser,
			"File \"" + download_item->GetFullPath().ToString() +
			"\" downloaded successfully.");*/
	}
}