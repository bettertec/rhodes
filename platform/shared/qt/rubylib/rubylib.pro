QT -= core

TARGET = rubylib
TEMPLATE = lib

CONFIG += staticlib warn_on

INCLUDEPATH += ../../ruby/include\
../../ruby\
../../ruby/generated\
../..

macx {
  DESTDIR = ../../../osx/bin/rubylib
  OBJECTS_DIR = ../../../osx/bin/rubylib/tmp
  INCLUDEPATH += ../../ruby/iphone
  HEADERS += ../../ruby/ext/socket/constants.h\
../../ruby/iphone/ruby/config.h\
../../ruby/iphone/crt_externs.h\
../../ruby/iseq.h\
../../ruby/thread_pthread.h
  SOURCES += ../../ruby/miniprelude.c\
../../ruby/newline.c\
../../ruby/thread_pthread.c
}

win32 {
  DESTDIR = ../../../win32/bin/rubylib
  OBJECTS_DIR = ../../../win32/bin/rubylib/tmp
  INCLUDEPATH += ../../ruby/win32
  DEFINES -= _UNICODE UNICODE
  DEFINES += _NDEBUG NDEBUG WIN32 _WINDOWS _LIB BUFSIZ=512 STATIC_LINKED
  HEADERS += ../../ruby/win32/ruby/config.h\
../../ruby/win32/dir.h
  SOURCES += ../../ruby/missing/acosh.c\
../../ruby/missing/cbrt.c\
../../ruby/missing/crypt.c\
../../ruby/missing/dup2.c\
../../ruby/missing/erf.c\
../../ruby/missing/hypot.c\
../../ruby/missing/tgamma.c\
../../ruby/missing/strlcpy.c\
../../ruby/missing/strlcat.c\
../../ruby/win32/miniprelude.c\
../../ruby/win32/newline.c\
../../ruby/win32/win32.c
}

unix:!macx {
  DESTDIR = ../../../linux/bin/rubylib
  OBJECTS_DIR = ../../../linux/bin/rubylib/tmp
  INCLUDEPATH += ../../ruby/linux
  HEADERS += ../../ruby/linux/ruby/config.h
  SOURCES += ../../ruby/miniprelude.c\
../../ruby/missing/acosh.c\
../../ruby/missing/cbrt.c\
../../ruby/missing/crypt.c\
../../ruby/missing/dup2.c\
../../ruby/missing/erf.c\
../../ruby/missing/hypot.c\
../../ruby/missing/stdlib.c\
../../ruby/missing/strlcat.c\
../../ruby/missing/strlcpy.c\
../../ruby/missing/tgamma.c\
../../ruby/newline.c
}

DEFINES += RHODES_EMULATOR

!win32 {
  QMAKE_CFLAGS_WARN_ON += -Wno-extra -Wno-unused -Wno-sign-compare -Wno-format -Wno-parentheses
}
win32 {
  QMAKE_CFLAGS_WARN_ON += /wd4244 /wd4133 /wd4996 /wd4554 /wd4018 /wd4101 /wd4005 /wd4146 /wd4047 /wd4100 /wd4189 /wd4646 /wd4645
  QMAKE_CFLAGS_RELEASE += /O2
}

HEADERS += ../../ruby/ext/rho/rhoruby.h\
../../ruby/ext/socket/addrinfo.h\
../../ruby/ext/socket/sockport.h\
../../ruby/ext/calendar/event.h\
../../ruby/debug.h\
../../ruby/dln.h\
../../ruby/eval_intern.h\
../../ruby/gc.h\
../../ruby/id.h\
../../ruby/node.h\
../../ruby/regenc.h\
../../ruby/regint.h\
../../ruby/regparse.h\
../../ruby/revision.h\
../../ruby/transcode_data.h\
../../ruby/version.h\
../../ruby/vm_core.h\
../../ruby/vm_opts.h

SOURCES += ../../ruby/ext/datetimepicker/datetimepicker_wrap.c\
../../ruby/ext/rhoconf/rhoconf_wrap.c\
../../ruby/ext/nativebar/nativebar_wrap.c\
../../ruby/ext/sqlite3_api/sqlite3_api_wrap.c\
../../ruby/ext/rho/rhoruby.c\
../../ruby/ext/rho/rhosupport.c\
../../ruby/ext/stringio/stringio.c\
../../ruby/ext/geolocation/geolocation_wrap.c\
../../ruby/ext/asynchttp/asynchttp_wrap.c\
../../ruby/ext/socket/socket.c\
../../ruby/ext/ringtones/ringtones_wrap.c\
../../ruby/ext/calendar/calendar_wrap.c\
../../ruby/ext/calendar/event_wrap.c\
../../ruby/ext/navbar/navbar_wrap.c\
../../ruby/ext/alert/alert_wrap.c\
../../ruby/ext/system/system_wrap.c\
../../ruby/ext/phonebook/phonebook_wrap.c\
../../ruby/ext/camera/camera_wrap.c\
../../ruby/ext/webview/webview_wrap.c\
../../ruby/array.c\
../../ruby/bignum.c\
../../ruby/class.c\
../../ruby/compar.c\
../../ruby/compile.c\
../../ruby/complex.c\
../../ruby/cont.c\
../../ruby/debug.c\
../../ruby/dir.c\
../../ruby/dln.c\
../../ruby/dln_find.c\
../../ruby/dmyencoding.c\
../../ruby/dmyext.c\
../../ruby/enum.c\
../../ruby/enumerator.c\
../../ruby/error.c\
../../ruby/eval.c\
../../ruby/file.c\
../../ruby/gc.c\
../../ruby/hash.c\
../../ruby/inits.c\
../../ruby/io.c\
../../ruby/iseq.c\
../../ruby/load.c\
../../ruby/marshal.c\
../../ruby/math.c\
../../ruby/node.c\
../../ruby/numeric.c\
../../ruby/object.c\
../../ruby/pack.c\
../../ruby/proc.c\
../../ruby/process.c\
../../ruby/random.c\
../../ruby/range.c\
../../ruby/rational.c\
../../ruby/re.c\
../../ruby/regcomp.c\
../../ruby/regenc.c\
../../ruby/regerror.c\
../../ruby/regexec.c\
../../ruby/regparse.c\
../../ruby/regsyntax.c\
../../ruby/ruby.c\
../../ruby/safe.c\
../../ruby/signal.c\
../../ruby/sprintf.c\
../../ruby/st.c\
../../ruby/strftime.c\
../../ruby/string.c\
../../ruby/struct.c\
../../ruby/thread.c\
../../ruby/time.c\
../../ruby/transcode.c\
../../ruby/util.c\
../../ruby/variable.c\
../../ruby/version.c\
../../ruby/vm.c\
../../ruby/vm_dump.c\
../../ruby/enc/ascii.c\
../../ruby/enc/iso_8859_9.c\
../../ruby/enc/unicode.c\
../../ruby/enc/us_ascii.c\
../../ruby/enc/utf_8.c\
../../ruby/ext/strscan/strscan.c\
../../ruby/ext/syncengine/syncengine_wrap.c\
../../ruby/generated/parse.c\
../../ruby/missing/lgamma_r.c\
../../ruby/ext/mapview/mapview_wrap.c\
../../ruby/ext/signature/signature_wrap.c\
../../ruby/ext/nativeviewmanager/nativeviewmanager_wrap.c\
../../ruby/ext/bluetooth/bluetooth_wrap.c\
../../ruby/enc/encdb.c
