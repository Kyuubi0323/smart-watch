#
# This is a project Makefile. It is assumed the directory this Makefile resides in is a
# project subdirectory.
#

PROJECT_NAME := smart-watch

EXTRA_COMPONENT_DIRS = ../../../SensorLib

include $(IDF_PATH)/make/project.mk
