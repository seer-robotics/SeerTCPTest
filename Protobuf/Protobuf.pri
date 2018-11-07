
INCLUDEPATH +=  $$PWD \
                $$PWD/include


CONFIG(debug,debug|release){#debug

    macx:{
        # macOS only
        LIBS += -L$$PWD/lib/macOS \
            -lprotobuf
    }
    linux-g++:{
        # linux only
        LIBS += -L$$PWD/lib/linux_x64 \
            -lprotobuf
    }

    win32:{
   contains(QT_ARCH, i386) {
        win32-g++{
            # windows only
            LIBS += -L$$PWD/lib/mingw_x86 \
            -lprotobuf
        }else{
            LIBS += -L$$PWD/lib/msvc_x86 \
            -lprotobuf
        }
    }else{
        win32-g++{
        # windows only
            LIBS += -L$$PWD/lib/mingw_x64 \
            -lprotobuf
        }else{
            LIBS += -L$$PWD/lib/msvc2015_x64/Debug \
            -llibprotobuf
        }
    }
  }
}else{#release

    macx:{
        # macOS only
        LIBS += -L$$PWD/lib/macOS \
            -lprotobuf
    }
    linux-g++:{
        # linux only
        LIBS += -L$$PWD/lib/linux_x64 \
            -lprotobuf
    }
    win32:{
    contains(QT_ARCH, i386) {
        win32-g++{
            # windows only
            LIBS += -L$$PWD/lib/mingw_x86 \
            -lprotobuf
        }else{
            LIBS += -L$$PWD/lib/msvc_x86 \
            -lprotobuf
        }
    }else{
        win32-g++{
        # windows only
            LIBS += -L$$PWD/lib/mingw_x64 \
            -lprotobuf
        }else{

            LIBS += -L$$PWD/lib/msvc2015_x64/Release \
            -llibprotobuf
        }
    }
  }
}
