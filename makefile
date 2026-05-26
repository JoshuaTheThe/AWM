override MAKEFLAGS += -rR
override OUTPUT := awm

define DEFAULT_VAR =
    ifeq ($(origin $1),default)
        override $(1) := $(2)
    endif
    ifeq ($(origin $1),undefined)
        override $(1) := $(2)
    endif
endef

override DEFAULT_KCC := cc
$(eval $(call DEFAULT_VAR,KCC,$(DEFAULT_KCC)))
override DEFAULT_KLD := cc
$(eval $(call DEFAULT_VAR,KLD,$(DEFAULT_KLD)))
override DEFAULT_KCFLAGS := -Wall -Wextra -Wpedantic -g
$(eval $(call DEFAULT_VAR,KCFLAGS,$(DEFAULT_KCFLAGS)))
override DEFAULT_KCPPFLAGS :=
$(eval $(call DEFAULT_VAR,KCPPFLAGS,$(DEFAULT_KCPPFLAGS)))
override DEFAULT_KNASMFLAGS :=
$(eval $(call DEFAULT_VAR,KNASMFLAGS,$(DEFAULT_KNASMFLAGS)))
override DEFAULT_KLDFLAGS :=  -static
$(eval $(call DEFAULT_VAR,KLDFLAGS,$(DEFAULT_KLDFLAGS)))

override KCFLAGS += \
    -Wall \
    -Wextra \
    -std=gnu11 \
    -march=native
override KLDFLAGS += \
    -m64 \
    -lgcc

override CFILES := $(shell cd src && find -L * -type f -name '*.c' | LC_ALL=C sort)
override OBJ := $(addprefix obj/,$(CFILES:.c=.c.o) $(ASFILES:.S=.S.o) $(NASMFILES:.asm=.asm.o))
override HEADER_DEPS := $(addprefix obj/,$(CFILES:.c=.c.d) $(ASFILES:.S=.S.d))
.PHONY: all
all: bin/$(OUTPUT)
bin/$(OUTPUT): $(OBJ)
	mkdir -p "$$(dirname $@)"
	cc -o $@ $(OBJ)
-include $(HEADER_DEPS)
obj/%.c.o: src/%.c
	mkdir -p "$$(dirname $@)"
	$(KCC) $(KCFLAGS) $(KCPPFLAGS) -c $< -o $@ -I src
.PHONY: clean
clean:
	rm -rf bin obj

