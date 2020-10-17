#include <QCoreApplication>
#include <QDir>

#include <QCefProtocol.h>

#include "CCefSetting.h"

CefString CCefSetting::browser_sub_process_path;

CefString CCefSetting::resource_directory_path;

CefString CCefSetting::locales_directory_path;

CefString CCefSetting::user_agent;

CefString CCefSetting::cache_path;

CefString CCefSetting::user_data_path;

int CCefSetting::persist_session_cookies;

int CCefSetting::persist_user_preferences;

CefString CCefSetting::locale;

int CCefSetting::remote_debugging_port;

cef_color_t CCefSetting::background_color;

CefString CCefSetting::accept_language_list;

void
CCefSetting::initializeInstance()
{
  static CCefSetting s_instance;
}

CCefSetting::CCefSetting()
{
	QDir::setCurrent(QCoreApplication::applicationDirPath());
	QDir ExeDir = QDir::current();

  QString strExePath = ExeDir.filePath(RENDER_PROCESS_NAME);
  browser_sub_process_path.FromString(QDir::toNativeSeparators(strExePath).toStdString());

  QString strResPath = ExeDir.filePath(RESOURCE_DIRECTORY_NAME);
  resource_directory_path.FromString(QDir::toNativeSeparators(strResPath).toStdString());

  QDir ResPath(strResPath);
  locales_directory_path.FromString(QDir::toNativeSeparators(ResPath.filePath(LOCALES_DIRECTORY_NAME)).toStdString());

  user_agent.FromString(QCEF_USER_AGENT);
}
