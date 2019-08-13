################################################################
# Qwt Widget Library
# Copyright (C) 1997   Josef Wilgen
# Copyright (C) 2002   Uwe Rathmann
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the Qwt License, Version 1.0
################################################################

# include( $${PWD}/../examples.pri )
include( /home/jsvirzi/projects/qwt-examples/examples.pri )

CONFIG += qwt

TARGET = spectrogram

HEADERS = \
     plot.h \
     variables.h

INCLUDEPATH += /Users/jsvirzi/utils/root/include
INCLUDEPATH += /usr/local/include/opencv
INCLUDEPATH += /usr/local/include
INCLUDEPATH += /home/jsvirzi/utils/qwt-6.1.4/src

SOURCES = \
    plot.cpp \
    main.cpp \
    variables.cpp

# /home/jsvirzi/projects/qwt-examples

LIBS += -L/Users/jsvirzi/utils/root/lib -lCore -lCint -lRIO -lNet -lHist -lGraf -lGraf3d -lGpad -lTree -lRint -lPostscript -lMatrix -lPhysics -lMathCore -lThread -lpthread -Wl,-rpath,/Users/jsvirzi/utils/root/lib -lm -ldl

# LIBS += -L/usr/local/lib -lopencv_calib3d -lopencv_contrib -lopencv_core -lopencv_features2d -lopencv_flann -lopencv_gpu -lopencv_highgui -lopencv_imgproc -lopencv_legacy -lopencv_ml -lopencv_nonfree -lopencv_objdetect -lopencv_ocl -lopencv_photo -lopencv_stitching -lopencv_superres -lopencv_ts -lopencv_video -lopencv_videostab -lrt -lpthread -lm -ldl
LIBS += -L/usr/local/lib -lopencv_calib3d -lopencv_contrib -lopencv_core -lopencv_features2d -lopencv_flann -lopencv_gpu -lopencv_highgui -lopencv_imgproc -lopencv_legacy -lopencv_ml -lopencv_nonfree -lopencv_objdetect -lopencv_ocl -lopencv_photo -lopencv_stitching -lopencv_superres -lopencv_ts -lopencv_video -lopencv_videostab -lrt -lpthread -lm -ldl
# LIBS += -pthread -m64 -I/usr/local/include

LIBS += -L/home/jsvirzi/utils/qwt-6.1.4/lib -lqwt
