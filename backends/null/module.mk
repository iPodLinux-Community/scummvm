MODULE := backends/null

MODULE_OBJS := \
	null.o

MODULE_DIRS += \
	backends/null/

# We don't use the rules.mk here on purpose
OBJS := $(addprefix $(MODULE)/, $(MODULE_OBJS)) $(OBJS)
