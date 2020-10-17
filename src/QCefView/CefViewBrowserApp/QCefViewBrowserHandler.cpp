#pragma region std_headers
#include <sstream>
#include <string>
#pragma endregion std_headers

#pragma region cef_headers
#include <include/cef_app.h>
#include <include/wrapper/cef_closure_task.h>
#include <include/wrapper/cef_helpers.h>
#pragma endregion cef_headers

#pragma region qt_headers
#include <QRect>
#include <QWindow>
#include <QVariant>
#pragma endregion qt_headers

#include <QCefProtocol.h>

#include "QCefViewBrowserHandler.h"

// Custom menu command Ids.
enum client_menu_ids {
	CLIENT_ID_SHOW_DEVTOOLS = MENU_ID_USER_FIRST,
	CLIENT_ID_CLOSE_DEVTOOLS,
	CLIENT_ID_INSPECT_ELEMENT,
	CLIENT_ID_TESTMENU_SUBMENU,
	CLIENT_ID_TESTMENU_CHECKITEM,
	CLIENT_ID_TESTMENU_RADIOITEM1,
	CLIENT_ID_TESTMENU_RADIOITEM2,
	CLIENT_ID_TESTMENU_RADIOITEM3,
};

QCefViewBrowserHandler::QCefViewBrowserHandler(CCefWindow* pQCefWin)
	: is_closing_(false)
	, pQCefWindow_(pQCefWin)
	, cefquery_handler_(new QCefQueryHandler(pQCefWin))
	, resource_manager_(new CefResourceManager())
	, message_router_(nullptr)
	, browser_count_(0) {
}

QCefViewBrowserHandler::~QCefViewBrowserHandler() {
}

bool QCefViewBrowserHandler::OnProcessMessageReceived(CefRefPtr<CefBrowser> browser,
	CefProcessId source_process,
	CefRefPtr<CefProcessMessage> message) {
	CEF_REQUIRE_UI_THREAD();
	if (message_router_->OnProcessMessageReceived(browser, source_process, message))
		return true;

	if (DispatchNotifyRequest(browser, source_process, message))
		return true;

	return false;
}

void QCefViewBrowserHandler::OnBeforeContextMenu(CefRefPtr<CefBrowser> browser,
	CefRefPtr<CefFrame> frame,
	CefRefPtr<CefContextMenuParams> params,
	CefRefPtr<CefMenuModel> model) {
	CEF_REQUIRE_UI_THREAD();

	// Add a separator if the menu already has items.
	if (model->GetCount() > 0)
		model->AddSeparator();

	// Add DevTools items to all context menus.
	model->AddItem(CLIENT_ID_SHOW_DEVTOOLS, "&Show DevTools");
	model->AddItem(CLIENT_ID_CLOSE_DEVTOOLS, "Close DevTools");
	model->AddSeparator();
	model->AddItem(CLIENT_ID_INSPECT_ELEMENT, "Inspect Element");

	// Test context menu features.
	BuildTestMenu(model);
}

bool QCefViewBrowserHandler::OnContextMenuCommand(CefRefPtr<CefBrowser> browser,
	CefRefPtr<CefFrame> frame,
	CefRefPtr<CefContextMenuParams> params,
	int command_id,
	EventFlags event_flags) {
	CEF_REQUIRE_UI_THREAD();

	switch (command_id) {
	case CLIENT_ID_SHOW_DEVTOOLS:
		ShowDevTools(browser, CefPoint());
		return true;
	case CLIENT_ID_CLOSE_DEVTOOLS:
		CloseDevTools(browser);
		return true;
	case CLIENT_ID_INSPECT_ELEMENT:
		ShowDevTools(browser, CefPoint(params->GetXCoord(), params->GetYCoord()));
		return true;
	default:  // Allow default handling, if any.
		return ExecuteTestMenu(command_id);
	}
}


void QCefViewBrowserHandler::BuildTestMenu(CefRefPtr<CefMenuModel> model) {
	if (model->GetCount() > 0)
		model->AddSeparator();

	// Build the sub menu.
	CefRefPtr<CefMenuModel> submenu =
		model->AddSubMenu(CLIENT_ID_TESTMENU_SUBMENU, "Context Menu Test");
	submenu->AddCheckItem(CLIENT_ID_TESTMENU_CHECKITEM, "Check Item");
	submenu->AddRadioItem(CLIENT_ID_TESTMENU_RADIOITEM1, "Radio Item 1", 0);
	submenu->AddRadioItem(CLIENT_ID_TESTMENU_RADIOITEM2, "Radio Item 2", 0);
	submenu->AddRadioItem(CLIENT_ID_TESTMENU_RADIOITEM3, "Radio Item 3", 0);

	// Check the check item.
	if (test_menu_state_.check_item)
		submenu->SetChecked(CLIENT_ID_TESTMENU_CHECKITEM, true);

	// Check the selected radio item.
	submenu->SetChecked(
		CLIENT_ID_TESTMENU_RADIOITEM1 + test_menu_state_.radio_item, true);
}

bool QCefViewBrowserHandler::ExecuteTestMenu(int command_id) {
	if (command_id == CLIENT_ID_TESTMENU_CHECKITEM) {
		// Toggle the check item.
		test_menu_state_.check_item ^= 1;
		return true;
	} else if (command_id >= CLIENT_ID_TESTMENU_RADIOITEM1 &&
		command_id <= CLIENT_ID_TESTMENU_RADIOITEM3) {
		// Store the selected radio item.
		test_menu_state_.radio_item = (command_id - CLIENT_ID_TESTMENU_RADIOITEM1);
		return true;
	}

	// Allow default handling to proceed.
	return false;
}


void QCefViewBrowserHandler::ShowDevTools(CefRefPtr<CefBrowser> browser,
	const CefPoint& inspect_element_at) {
	CefWindowInfo windowInfo;
	CefBrowserSettings settings;
	windowInfo.SetAsPopup(NULL, "DevTools");
	browser->GetHost()->ShowDevTools(windowInfo, this, settings, CefPoint());
}

void QCefViewBrowserHandler::CloseDevTools(CefRefPtr<CefBrowser> browser) {
	browser->GetHost()->CloseDevTools();
}


void QCefViewBrowserHandler::OnAddressChange(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, const CefString& url) {
	CEF_REQUIRE_UI_THREAD();
}

void QCefViewBrowserHandler::OnTitleChange(CefRefPtr<CefBrowser> browser, const CefString& title) {
	CEF_REQUIRE_UI_THREAD();
}

bool QCefViewBrowserHandler::OnConsoleMessage(CefRefPtr<CefBrowser> browser,
	const CefString& message,
	const CefString& source,
	int line) {
	CEF_REQUIRE_UI_THREAD();
	if (source.empty() || message.empty())
		return false;

	std::string src = source.ToString();
	std::size_t found = src.find_last_of("/\\");
	if (found != std::string::npos && found < src.length() - 1)
		src = src.substr(found + 1);

	__noop(src, message.ToString());
	return false;
}

bool QCefViewBrowserHandler::OnDragEnter(CefRefPtr<CefBrowser> browser,
	CefRefPtr<CefDragData> dragData,
	CefDragHandler::DragOperationsMask mask) {
	CEF_REQUIRE_UI_THREAD();

	return true;
}

bool QCefViewBrowserHandler::OnJSDialog(CefRefPtr<CefBrowser> browser,
	const CefString& origin_url,
	const CefString& accept_lang,
	JSDialogType dialog_type,
	const CefString& message_text,
	const CefString& default_prompt_text,
	CefRefPtr<CefJSDialogCallback> callback,
	bool& suppress_message) {
	CEF_REQUIRE_UI_THREAD();

	return false;
}

bool QCefViewBrowserHandler::OnBeforeUnloadDialog(CefRefPtr<CefBrowser> browser,
	const CefString& message_text,
	bool is_reload,
	CefRefPtr<CefJSDialogCallback> callback) {
	CEF_REQUIRE_UI_THREAD();

	return false;
}

void QCefViewBrowserHandler::OnResetDialogState(CefRefPtr<CefBrowser> browser) {
	CEF_REQUIRE_UI_THREAD();
}

bool QCefViewBrowserHandler::OnPreKeyEvent(CefRefPtr<CefBrowser> browser,
	const CefKeyEvent& event,
	CefEventHandle os_event,
	bool* is_keyboard_shortcut) {
	CEF_REQUIRE_UI_THREAD();

	return false;
}

bool QCefViewBrowserHandler::OnBeforePopup(CefRefPtr<CefBrowser> browser,
	CefRefPtr<CefFrame> frame,
	const CefString& target_url,
	const CefString& target_frame_name,
	CefLifeSpanHandler::WindowOpenDisposition target_disposition,
	bool user_gesture,
	const CefPopupFeatures& popupFeatures,
	CefWindowInfo& windowInfo,
	CefRefPtr<CefClient>& client,
	CefBrowserSettings& settings,
	bool* no_javascript_access) {
	//TODO Ϊʲô��������ʱ�������ĵ��ûᱨ��
	//CEF_REQUIRE_UI_THREAD();

	// If the browser is closing, block the popup
	if (is_closing_)
		return true;

	// Allow the popup
	return false;

	// Redirect all popup page into the source frame forcefully
	frame->LoadURL(target_url);
	// Don't allow new window or tab
	return true;
}

void QCefViewBrowserHandler::OnAfterCreated(CefRefPtr<CefBrowser> browser) {
	CEF_REQUIRE_UI_THREAD();

	// If the browser is closing, return immediately to release the new created browser
	if (is_closing_)
		return;

	if (!message_router_) {
		// Create the browser-side router for query handling.
		CefMessageRouterConfig config;
		config.js_query_function = QCEF_QUERY_NAME;
		config.js_cancel_function = QCEF_QUERY_CANCEL_NAME;

		// Register handlers with the router.
		message_router_ = CefMessageRouterBrowserSide::Create(config);
		message_router_->AddHandler(cefquery_handler_.get(), false);
	}

	if (!main_browser_) {
		// If this is the main browser then keep it
		{
			std::unique_lock<std::mutex> lock(mtx_);
			// We need to keep the main child window, but not popup windows
			main_browser_ = browser;
		}

		// Set the cef window handle to the QcefWindow
		if (pQCefWindow_)
			pQCefWindow_->setCefBrowserWindow(browser->GetHost()->GetWindowHandle());
	} else if (browser->IsPopup()) {
		// Add to the list of popup browsers.
		popup_browser_list_.push_back(browser);

		// Give focus to the popup browser. Perform asynchronously because the
		// parent window may attempt to keep focus after launching the popup.
		CefPostTask(TID_UI, CefCreateClosureTask(base::Bind(&CefBrowserHost::SetFocus, browser->GetHost().get(), true)));
	}

	// Increase the browser count
	{
		std::unique_lock<std::mutex> lock(mtx_);
		browser_count_++;
	}
}

bool QCefViewBrowserHandler::DoClose(CefRefPtr<CefBrowser> browser) {
	CEF_REQUIRE_UI_THREAD();

	// Closing the main window requires special handling. See the DoClose()
	// documentation in the CEF header for a detailed description of this
	// process.
	if (main_browser_ && main_browser_->IsSame(browser))
		// Set a flag to indicate that the window close should be allowed.
		is_closing_ = true;

	// Allow the close. For windowed browsers this will result in the OS close
	// event being sent.
	return false;
}

void QCefViewBrowserHandler::OnBeforeClose(CefRefPtr<CefBrowser> browser) {
	CEF_REQUIRE_UI_THREAD();

	message_router_->OnBeforeClose(browser);

	if (main_browser_ && main_browser_->IsSame(browser)) {
		// if the main browser is closing, we need to close all the pop up browsers.
		if (!popup_browser_list_.empty()) {
			for (auto& popup : popup_browser_list_) {
				if (popup) {
					popup->StopLoad();
					popup->GetHost()->CloseBrowser(true);
				}
			}
		}

		main_browser_->StopLoad();
		main_browser_ = nullptr;
	} else if (browser->IsPopup()) {
		// Remove from the browser popup list.
		for (auto it = popup_browser_list_.begin(); it != popup_browser_list_.end(); ++it) {
			if ((*it)->IsSame(browser)) {
				popup_browser_list_.erase(it);
				break;
			}
		}
	}

	// Decrease the browser count
	{
		std::unique_lock<std::mutex> lock(mtx_);
		if (--browser_count_ == 0) {
			message_router_->RemoveHandler(cefquery_handler_.get());
			cefquery_handler_ = nullptr;
			message_router_ = nullptr;

			// Notify the waiting thread if any
			//close_cv_.notify_all();
		}
	}
}

void QCefViewBrowserHandler::OnLoadingStateChange(CefRefPtr<CefBrowser> browser,
	bool isLoading,
	bool canGoBack,
	bool canGoForward) {
	CEF_REQUIRE_UI_THREAD();
	if (pQCefWindow_)
		pQCefWindow_->loadingStateChanged(isLoading, canGoBack, canGoForward);
}

void QCefViewBrowserHandler::OnLoadStart(CefRefPtr<CefBrowser> browser,
	CefRefPtr<CefFrame> frame) {
	CEF_REQUIRE_UI_THREAD();
	if (pQCefWindow_) {
		pQCefWindow_->loadStart();
	}
}

void QCefViewBrowserHandler::OnLoadEnd(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, int httpStatusCode) {
	CEF_REQUIRE_UI_THREAD();
	if (pQCefWindow_)
		pQCefWindow_->loadEnd(httpStatusCode);
}

void QCefViewBrowserHandler::OnLoadError(CefRefPtr<CefBrowser> browser,
	CefRefPtr<CefFrame> frame,
	ErrorCode errorCode,
	const CefString& errorText,
	const CefString& failedUrl) {
	CEF_REQUIRE_UI_THREAD();
	if (errorCode == ERR_ABORTED)
		return;

	QString msg = QString::fromStdString(errorText.ToString());
	QString url = QString::fromStdString(failedUrl.ToString());
	QString content = QString("<html><body bgcolor=\"white\">"
		"<h2>Failed to load URL: %1 </h2>"
		"<h2>Error: %2(%3)</h2>"
		"</body></html>")
		.arg(url)
		.arg(msg)
		.arg(errorCode);

	frame->LoadString(content.toStdString(), failedUrl);
	if (pQCefWindow_)
		pQCefWindow_->loadError(errorCode, msg, url);
}

bool QCefViewBrowserHandler::OnBeforeBrowse(CefRefPtr<CefBrowser> browser,
	CefRefPtr<CefFrame> frame,
	CefRefPtr<CefRequest> request,
	bool is_redirect) {
	CEF_REQUIRE_UI_THREAD();

	message_router_->OnBeforeBrowse(browser, frame);
	return false;
}

bool QCefViewBrowserHandler::OnOpenURLFromTab(CefRefPtr<CefBrowser> browser,
	CefRefPtr<CefFrame> frame,
	const CefString& target_url,
	CefRequestHandler::WindowOpenDisposition target_disposition,
	bool user_gesture) {
	CEF_REQUIRE_UI_THREAD();

	return false; // return true to cancel this navigation.
}

//CefRefPtr<CefResourceRequestHandler>
//QCefViewBrowserHandler::GetResourceRequestHandler(CefRefPtr<CefBrowser> browser,
//                                                  CefRefPtr<CefFrame> frame,
//                                                  CefRefPtr<CefRequest> request,
//                                                  bool is_navigation,
//                                                  bool is_download,
//                                                  const CefString& request_initiator,
//                                                  bool& disable_default_handling)
//{
//  CEF_REQUIRE_IO_THREAD();
//  return this;
//}

bool QCefViewBrowserHandler::OnQuotaRequest(CefRefPtr<CefBrowser> browser,
	const CefString& origin_url,
	int64 new_size,
	CefRefPtr<CefRequestCallback> callback) {
	CEF_REQUIRE_IO_THREAD();
	static const int maxSize = 10 * 1024 * 1024;
	callback->Continue(new_size <= maxSize);
	return true;
}

void QCefViewBrowserHandler::OnRenderProcessTerminated(CefRefPtr<CefBrowser> browser, TerminationStatus status) {
	CEF_REQUIRE_UI_THREAD();

	message_router_->OnRenderProcessTerminated(browser);

	if (main_browser_) {
		CefString url = main_browser_->GetMainFrame()->GetURL();
		if (!url.empty()) {
			main_browser_->GetMainFrame()->LoadURL(url);
		}
	}
}

CefRequestHandler::ReturnValue QCefViewBrowserHandler::OnBeforeResourceLoad(CefRefPtr<CefBrowser> browser,
	CefRefPtr<CefFrame> frame,
	CefRefPtr<CefRequest> request,
	CefRefPtr<CefRequestCallback> callback) {
	return resource_manager_->OnBeforeResourceLoad(browser, frame, request, callback);
}

CefRefPtr<CefResourceHandler> QCefViewBrowserHandler::GetResourceHandler(CefRefPtr<CefBrowser> browser,
	CefRefPtr<CefFrame> frame,
	CefRefPtr<CefRequest> request) {
	return resource_manager_->GetResourceHandler(browser, frame, request);
}

void QCefViewBrowserHandler::OnProtocolExecution(CefRefPtr<CefBrowser> browser,
	const CefString& url,
	bool& allow_os_execution) {
}


CefRefPtr<CefBrowser> QCefViewBrowserHandler::GetBrowser() const {
	std::unique_lock<std::mutex> lock(mtx_);
	return main_browser_;
}

void QCefViewBrowserHandler::CloseAllBrowsers(bool force_close) {
	// If all browsers had been closed, then return
	std::unique_lock<std::mutex> lock(mtx_);
	if (!browser_count_) {
		return;
	}

	// Flip the closing flag
	is_closing_ = true;

	// Close all popup browsers if any
	if (!popup_browser_list_.empty()) {
		for (auto it = popup_browser_list_.begin(); it != popup_browser_list_.end(); ++it)
			(*it)->GetHost()->CloseBrowser(force_close);
	}

	if (main_browser_)
		// Request that the main browser close.
		main_browser_->GetHost()->CloseBrowser(force_close);

	// Wait for the browser to be closed
	//close_cv_.wait(lock);
}

bool QCefViewBrowserHandler::TriggerEvent(const int64_t frame_id, const CefRefPtr<CefProcessMessage> msg) {
	if (msg->GetName().empty())
		return false;

	CefRefPtr<CefBrowser> browser = GetBrowser();
	if (browser) {
		std::vector<int64> frameIds;
		if (MAIN_FRAME == frame_id) {
			frameIds.push_back(browser->GetMainFrame()->GetIdentifier());
		} else if (ALL_FRAMES == frame_id) {
			browser->GetFrameIdentifiers(frameIds);
		} else {
			frameIds.push_back(frame_id);
		}

		for (auto id : frameIds) {
			auto frame = browser->GetFrame(id);
			browser->SendProcessMessage(PID_RENDERER, msg);
			//browser->SendProcessMessage(PID_BROWSER, msg);
			return true;
		}
	}

	return false;
}

bool QCefViewBrowserHandler::ResponseQuery(const int64_t query, bool success, const CefString& response, int error) {
	if (cefquery_handler_)
		return cefquery_handler_->Response(query, success, response, error);

	return false;
}

bool QCefViewBrowserHandler::DispatchNotifyRequest(CefRefPtr<CefBrowser> browser,
	CefProcessId source_process,
	CefRefPtr<CefProcessMessage> message) {
	if (pQCefWindow_ && message->GetName() == INVOKEMETHOD_NOTIFY_MESSAGE) {
		CefRefPtr<CefListValue> messageArguments = message->GetArgumentList();
		if (messageArguments && (messageArguments->GetSize() >= 2)) {
			int idx = 0;
			if (CefValueType::VTYPE_INT == messageArguments->GetType(idx)) {
				int browserId = browser->GetIdentifier();
				int frameId = messageArguments->GetInt(idx++);

				if (CefValueType::VTYPE_STRING == messageArguments->GetType(idx)) {
					CefString functionName = messageArguments->GetString(idx++);
					if (functionName == QCEF_INVOKEMETHOD) {
						QString method;
						if (CefValueType::VTYPE_STRING == messageArguments->GetType(idx)) {
#if defined(CEF_STRING_TYPE_UTF16)
							method = QString::fromWCharArray(messageArguments->GetString(idx++).c_str());
#elif defined(CEF_STRING_TYPE_UTF8)
							method = QString::fromUtf8(messageArguments->GetString(idx++).c_str());
#elif defined(CEF_STRING_TYPE_WIDE)
							method = QString::fromWCharArray(messageArguments->GetString(idx++).c_str());
#endif
						}

						QVariantList arguments;
						QString qStr;
						for (idx; idx < messageArguments->GetSize(); idx++) {
							if (CefValueType::VTYPE_NULL == messageArguments->GetType(idx))
								arguments.push_back(0);
							else if (CefValueType::VTYPE_BOOL == messageArguments->GetType(idx))
								arguments.push_back(QVariant::fromValue(messageArguments->GetBool(idx)));
							else if (CefValueType::VTYPE_INT == messageArguments->GetType(idx))
								arguments.push_back(QVariant::fromValue(messageArguments->GetInt(idx)));
							else if (CefValueType::VTYPE_DOUBLE == messageArguments->GetType(idx))
								arguments.push_back(QVariant::fromValue(messageArguments->GetDouble(idx)));
							else if (CefValueType::VTYPE_STRING == messageArguments->GetType(idx)) {
#if defined(CEF_STRING_TYPE_UTF16)
								qStr = QString::fromWCharArray(messageArguments->GetString(idx).c_str());
#elif defined(CEF_STRING_TYPE_UTF8)
								qStr = QString::fromUtf8(messageArguments->GetString(idx).c_str());
#elif defined(CEF_STRING_TYPE_WIDE)
								qStr = QString::fromWCharArray(messageArguments->GetString(idx).c_str());
#endif
								arguments.push_back(qStr);
							} else {
							}
						}
						pQCefWindow_->invokeMethodNotify(browserId, frameId, method, arguments);
						return true;
					}
				}
			}
		}
	}

	return false;
}
