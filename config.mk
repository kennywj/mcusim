#
# system configuration
# update this file to select included modules
#

# include FreeRTOS module
FREERTOS_EN ?= 1

#
# following options are application module
#
GNSS_EN ?=0			# gnss module
JSON_EN ?=0			# simple JSON module
DUKTAPE_EN ?=0 		# simple javascript interperator
