################################################################
# Qwt Widget Library
# Copyright (C) 1997   Josef Wilgen
# Copyright (C) 2002   Uwe Rathmann
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the Qwt License, Version 1.0
################################################################

include( $${PWD}/../examples.pri )

TARGET   = spectrogram

HEADERS = \
     plot.h \
     variables.h

INCLUDEPATH += /Users/jsvirzi/utils/root/include

SOURCES = \
    plot.cpp \
    main.cpp \
    variables.cpp

LIBS += -L/Users/jsvirzi/utils/root/lib -lCore -lCint -lRIO -lNet -lHist -lGraf -lGraf3d -lGpad -lTree -lRint -lPostscript -lMatrix -lPhysics -lMathCore -lThread -lpthread -Wl,-rpath,/Users/jsvirzi/utils/root/lib -stdlib=libc++ -lm -ldl
