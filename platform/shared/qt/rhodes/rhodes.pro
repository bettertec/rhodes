QT += core gui network webkit

TARGET = RhoSimulator
TEMPLATE = app

CONFIG += warn_on

INCLUDEPATH += ../..\
../../ruby\
../../ruby/include

macx {
  ICON = resources/rho.icns
  QMAKE_INFO_PLIST = resources/Info.plist
  DESTDIR = ../../../osx/bin/RhoSimulator
  MOC_DIR = ../../../osx/bin/RhoSimulator/generated_files
  UI_DIR = ../../../osx/bin/RhoSimulator/generated_files
  OBJECTS_DIR = ../../../osx/bin/RhoSimulator/tmp
  RCC_DIR =  ../../../osx/bin/RhoSimulator/resources
  HEADERS += impl/SSLImpl.h
  SOURCES += impl/SSLImpl.cpp
  LIBS += -lcrypto -lssl -lz -lldap
  LIBS += -L../../../osx/bin/curl -lcurl
  LIBS += -L../../../osx/bin/rubylib -lrubylib
  LIBS += -L../../../osx/bin/rholib -lrholib
  LIBS += -L../../../osx/bin/sqlite3 -lsqlite3
  LIBS += -L../../../osx/bin/syncengine -lsyncengine
  PRE_TARGETDEPS += ../../../osx/bin/rubylib/librubylib.a\
../../../osx/bin/rholib/librholib.a\
../../../osx/bin/sqlite3/libsqlite3.a\
../../../osx/bin/syncengine/libsyncengine.a
  exists("../../../osx/bin/extensions/extensions.pri") {
    include("../../../osx/bin/extensions/extensions.pri")
  }
}

win32 {
  CONFIG += embed_manifest_exe
  DESTDIR = ../../../win32/bin/RhoSimulator
  MOC_DIR = ../../../win32/bin/RhoSimulator/generated_files
  UI_DIR = ../../../win32/bin/RhoSimulator/generated_files
  OBJECTS_DIR = ../../../win32/bin/RhoSimulator/tmp
  RCC_DIR =  ../../../win32/bin/RhoSimulator/resources
  HEADERS += ../../../wm/rhodes/rho/net/NetRequestImpl.h\
impl/RhoThreadImpl.h
  SOURCES += ../../../wm/rhodes/rho/net/NetRequestImpl.cpp
  RESOURCES += resources/simulator.qrc
  INCLUDEPATH += ../../../wm/rhodes\
../../wtl80/include
  DEFINES -= _UNICODE
  DEFINES += _NDEBUG NDEBUG WIN32 _WINDOWS UNICODE QT_LARGEFILE_SUPPORT QT_NO_DEBUG QT_CORE_LIB QT_GUI_LIB QT_WEBKIT_LIB _CRT_SECURE_NO_WARNINGS _CRT_NON_CONFORMING_SWPRINTFS
  LIBS += ../../../win32/bin/rubylib/rubylib.lib\
../../../win32/bin/rholib/rholib.lib\
../../../win32/bin/sqlite3/sqlite3.lib\
../../../win32/bin/syncengine/syncengine.lib\
wininet.lib comsuppwd.lib ws2_32.lib Crypt32.lib gdiplus.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib
  PRE_TARGETDEPS += ../../../win32/bin/rubylib/rubylib.lib\
../../../win32/bin/rholib/rholib.lib\
../../../win32/bin/sqlite3/sqlite3.lib\
../../../win32/bin/syncengine/syncengine.lib
}

unix:!macx {
  DESTDIR = ../../../linux/bin/RhoSimulator
  MOC_DIR = ../../../linux/bin/RhoSimulator/generated_files
  UI_DIR = ../../../linux/bin/RhoSimulator/generated_files
  OBJECTS_DIR = ../../../linux/bin/RhoSimulator/tmp
  RCC_DIR =  ../../../linux/bin/RhoSimulator/resources
  # INCLUDEPATH += ../../curl/include
  LIBS += -lcurl
  # LIBS += -L../../../osx/bin/curl -lcurl
  LIBS += -L../../../linux/bin/rubylib -lrubylib
  LIBS += -L../../../linux/bin/rholib -lrholib
  LIBS += -L../../../linux/bin/sqlite3 -lsqlite3
  LIBS += -L../../../linux/bin/syncengine -lsyncengine
  PRE_TARGETDEPS += ../../../linux/bin/curl/libcurl.a\
../../../linux/bin/rubylib/librubylib.a\
../../../linux/bin/rholib/librholib.a\
../../../linux/bin/sqlite3/libsqlite3.a\
../../../linux/bin/syncengine/libsyncengine.a
}

DEFINES += RHODES_EMULATOR=1

!win32 {
  QMAKE_CFLAGS_WARN_ON += -Wno-extra -Wno-unused -Wno-sign-compare -Wno-format -Wno-parentheses
  QMAKE_CXXFLAGS_WARN_ON += -Wno-extra -Wno-unused -Wno-sign-compare -Wno-format -Wno-parentheses
}
win32 {
  QMAKE_CFLAGS_WARN_ON += /wd4996 /wd4100
  QMAKE_CXXFLAGS_WARN_ON += /wd4996 /wd4100
  QMAKE_CFLAGS_RELEASE += /O2
  QMAKE_CXXFLAGS_RELEASE += /O2
}

HEADERS += ExternalWebView.h\
MainWindowCallback.h\
QtMainWindow.h\
QtNativeTabBar.h\
QtWebInspector.h\
impl/RhoClassFactoryImpl.h\
impl/MainWindowImpl.h\
impl/NativeToolbarImpl.h\
impl/NativeTabbarImpl.h\
../../rubyext/NativeToolbarExt.h\
DateTimeDialog.h\
impl/DateTimePickerImpl.h\
impl/AlertImpl.h\
RhoSimulator.h

SOURCES += main.cpp\
ExternalWebView.cpp\
QtMainWindow.cpp\
QtNativeTabBar.cpp\
QtWebInspector.cpp\
impl/AlertImpl.cpp\
impl/BluetoothImpl.cpp\
impl/CalendarImpl.cpp\
impl/CameraImpl.cpp\
impl/DateTimePickerImpl.cpp\
impl/GeoLocationImpl.cpp\
impl/MapViewImpl.cpp\
impl/NativeToolbarImpl.cpp\
impl/PhonebookImpl.cpp\
impl/RhoClassFactoryImpl.cpp\
impl/RhodesImpl.cpp\
impl/RhoFileImpl.cpp\
impl/RhoNativeViewManagerImpl.cpp\
impl/RhoThreadImpl.cpp\
impl/RingtoneManagerImpl.cpp\
impl/SignatureImpl.cpp\
impl/SystemImpl.cpp\
impl/WebViewImpl.cpp\
impl/MainWindowImpl.cpp\
impl/NativeTabbarImpl.cpp\
DateTimeDialog.cpp

FORMS += ExternalWebView.ui\
QtMainWindow.ui\
QtWebInspector.ui\
DateTimeDialog.ui
