ARCH		= $(shell uname -m | sed s,i[3456789]86,ia32,)

LIB_PATH	= /usr/lib64

EFI_INCLUDE	= /usr/include/efi
EFI_INCLUDES	= -nostdinc -I$(EFI_INCLUDE) -I$(EFI_INCLUDE)/$(ARCH) -I$(EFI_INCLUDE)/protocol
EFI_PATH	= /usr/lib64/gnuefi

LIB_GCC		= $(shell $(CC) -print-libgcc-file-name)
EFI_LIBS	= -lefi -lgnuefi $(LIB_GCC) 

EFI_CRT_OBJS 	= $(EFI_PATH)/crt0-efi-$(ARCH).o
EFI_LDS		= /usr/lib64/gnuefi/elf_$(ARCH)_efi.lds

CFLAGS		= -ggdb -O0 -fno-stack-protector -fno-strict-aliasing -fpic \
		  -fshort-wchar -Wall -mno-red-zone -maccumulate-outgoing-args \
		  -mno-mmx -mno-sse -Werror \
		  $(EFI_INCLUDES)
ifeq ($(ARCH),x86_64)
	CFLAGS	+= -DEFI_FUNCTION_WRAPPER -DGNU_EFI_USE_MS_ABI
endif
ifneq ($(origin VENDOR_CERT_FILE), undefined)
	CFLAGS += -DVENDOR_CERT_FILE=\"$(VENDOR_CERT_FILE)\"
endif
ifneq ($(origin VENDOR_DBX_FILE), undefined)
	CFLAGS += -DVENDOR_DBX_FILE=\"$(VENDOR_DBX_FILE)\"
endif

LDFLAGS		= -nostdlib -znocombreloc -T $(EFI_LDS) -shared -Bsymbolic -L$(EFI_PATH) -L$(LIB_PATH) $(EFI_CRT_OBJS)

VERSION		= 0.1

TARGETS	= clear.efi fill.efi qvi.efi

all: $(TARGETS)

%.o : %.c
	$(CC) $(CFLAGS) -c -o $@ $<

%.so : %.o
	$(LD) -o $@ $(LDFLAGS) $^ $(EFI_LIBS)

%.efi: %.so
	objcopy -j .text -j .sdata -j .data \
		-j .dynamic -j .dynsym  -j .rel \
		-j .rela -j .reloc -j .eh_frame \
		-j .vendor_cert \
		--target=efi-app-$(ARCH) $^ $@
	objcopy -j .text -j .sdata -j .data \
		-j .dynamic -j .dynsym  -j .rel \
		-j .rela -j .reloc -j .eh_frame \
		-j .debug_info -j .debug_abbrev -j .debug_aranges \
		-j .debug_line -j .debug_str -j .debug_ranges \
		--target=efi-app-$(ARCH) $^ $@.debug

clean:
	rm -rf $(TARGET) $(OBJS)
	rm -f *.debug *.so *.efi *.o

GITTAG = $(VERSION)

test-archive:
	@rm -rf /tmp/vars-$(VERSION) /tmp/vars-$(VERSION)-tmp
	@mkdir -p /tmp/vars-$(VERSION)-tmp
	@git archive --format=tar $(shell git branch | awk '/^*/ { print $$2 }') | ( cd /tmp/vars-$(VERSION)-tmp/ ; tar x )
	@git diff | ( cd /tmp/vars-$(VERSION)-tmp/ ; patch -s -p1 -b -z .gitdiff )
	@mv /tmp/vars-$(VERSION)-tmp/ /tmp/vars-$(VERSION)/
	@dir=$$PWD; cd /tmp; tar -c --bzip2 -f $$dir/vars-$(VERSION).tar.bz2 vars-$(VERSION)
	@rm -rf /tmp/vars-$(VERSION)
	@echo "The archive is in vars-$(VERSION).tar.bz2"

archive:
	git tag $(GITTAG) refs/heads/master
	@rm -rf /tmp/vars-$(VERSION) /tmp/vars-$(VERSION)-tmp
	@mkdir -p /tmp/vars-$(VERSION)-tmp
	@git archive --format=tar $(GITTAG) | ( cd /tmp/vars-$(VERSION)-tmp/ ; tar x )
	@mv /tmp/vars-$(VERSION)-tmp/ /tmp/vars-$(VERSION)/
	@dir=$$PWD; cd /tmp; tar -c --bzip2 -f $$dir/vars-$(VERSION).tar.bz2 vars-$(VERSION)
	@rm -rf /tmp/vars-$(VERSION)
	@echo "The archive is in vars-$(VERSION).tar.bz2"

