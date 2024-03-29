################################################################
# Qwt Widget Library
# Copyright (C) 1997   Josef Wilgen
# Copyright (C) 2002   Uwe Rathmann
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the Qwt License, Version 1.0
################################################################

include( $${PWD}/../examples.pri )

TARGET       = dials

CONFIG += qwt

HEADERS = \
    attitude_indicator.h \
    speedo_meter.h \
    cockpit_grid.h \
    compass_grid.h

SOURCES = \
    attitude_indicator.cpp \
    speedo_meter.cpp \
    cockpit_grid.cpp \
    compass_grid.cpp \
    dials.cpp

