
ifeq ($(ARM), 1)
CROSS	   	= arm-none-linux-gnueabi-
LIBJPEG		= depend/arm/libjpeg
else
LIBJPEG		= depend/x86/libjpeg
M32			= -m32
endif

CC			:= $(CROSS)gcc
CXX			:= $(CROSS)gcc
AR			:= $(CROSS)ar
LD         	:= $(CROSS)gcc
OBJCOPY    	:= $(CROSS)objcopy
OBJDUMP    	:= $(CROSS)objdump
DELFILE		:= rm -rf
DELDIR		:= rm -rf
INSTALL		:= cp -rf
COPY		:= cp -rf
PRINT		:= echo
MKDIR		:= mkdir -p
TEST		:= test -d

Prefix		:= ..
BINDIR 		:= bin
OBJDIR 		:= obj
OUTDIR		:= flib
LIBDIR		:= $(OUTDIR)/lib
INCLUDE		:= $(OUTDIR)/include

DEST		:= flib
ELF        	:= $(BINDIR)/$(DEST)
DIS        	:= $(BINDIR)/$(DEST).dis
MAP			:= $(BINDIR)/$(DEST).map.txt

DYNAMICLIB	:= $(LIBDIR)/lib$(DEST).so
STATICLIB	:= $(LIBDIR)/lib$(DEST).a

INCDIR 		:= src
SRCDIR 		:= src
PSRCDIR		:= src/private
PINCDIR		:= src/private

C_SRC		+= $(wildcard $(SRCDIR)/*.c)
P_SRC		+= $(wildcard $(PSRCDIR)/*.c)

OBJS		+= $(patsubst $(SRCDIR)/%.c, $(OBJDIR)/%.o, $(C_SRC))
OBJS		+= $(patsubst $(PSRCDIR)/%.c, $(OBJDIR)/%.o, $(P_SRC))

LIBPATH		:= -L$(LIBJPEG)/lib
INCPATH 	:= -I$(INCDIR) -I$(PINCDIR) -I$(LIBJPEG)/include
LIBS		:= -ljpeg
WARNINGS 	:= -Wall
CFLAGS 		:= $(WARNINGS) $(INCPATH) -O2 -fPIC $(M32)
LDFLAGS 	:= $(LIBPATH) $(LIBS) -fPIC $(M32)

ifeq ($(DEBUG), 1)
CFLAGS		+= -g
endif

all: check_directory lib
lib: $(DYNAMICLIB) $(STATICLIB) headers

check_directory:
	@$(TEST) $(BINDIR) || $(MKDIR) $(BINDIR)
	@$(TEST) $(OBJDIR) || $(MKDIR) $(OBJDIR)
	@$(TEST) $(LIBDIR) || $(MKDIR) $(LIBDIR)
	@$(TEST) $(INCLUDE) || $(MKDIR) $(INCLUDE)


$(DYNAMICLIB):$(OBJS)
	@$(PRINT) Creating $@ ....
	@$(CC) -shared -fPIC -o $@ $^ $(LDFLAGS)

$(STATICLIB):$(OBJS)
	@$(PRINT) Creating $@ ....
	@$(AR) rc $@ $^

headers:
	@$(PRINT) Copying Headers ....
	@$(COPY) $(INCDIR)/*.h $(INCLUDE)

install:
	@$(PRINT) Installing ....
	@$(INSTALL) $(LIBJPEG)/lib $(OUTDIR)
	@$(INSTALL) $(OUTDIR) $(Prefix)

$(ELF):$(OBJS)
	@echo Linking $@ from $^ ....
	@$(LD) -o $(ELF) $^ $(LDFLAGS) -Wl,-Map=$(MAP) -m32
	@echo Disassembling $(ELF) to $(DIS)
	@$(OBJDUMP) -D  $(ELF) > $(DIS)	

#$(OBJCOPY) -O binary -S $(ELF) $@		
    
$(OBJDIR)/%.o:$(SRCDIR)/%.c
	@$(PRINT) Compiling $< ....
	@$(CC) -c -o $@ $< $(CFLAGS)

$(OBJDIR)/%.o:$(PSRCDIR)/%.c
	@$(PRINT) Compiling $< ....
	@$(CC) -c -o $@ $< $(CFLAGS)

.PHONY: clean

clean:
	-$(DELDIR) $(OBJDIR) $(BINDIR) $(LIBDIR) $(INCLUDE)
