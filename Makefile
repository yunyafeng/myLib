#CROSS	   	:= arm-none-linux-gnueabi-
CC			:= $(CROSS)g++
CXX			:= $(CROSS)g++
AR			:= $(CROSS)ar
LD         	:= $(CROSS)g++
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
LIBDIR		:= lib
INCLUDE		:= include

DEST		:= flib_test
ELF        	:= $(BINDIR)/$(DEST)
DIS        	:= $(BINDIR)/$(DEST).dis
MAP			:= $(BINDIR)/$(DEST).map.txt

DYNAMICLIB	:= $(LIBDIR)/lib$(DEST).so
STATICLIB	:= $(LIBDIR)/lib$(DEST).a

INCDIR 		:= src
SRCDIR 		:= src
C_SRC		+= $(wildcard $(SRCDIR)/*.c)
OBJS		+= $(patsubst $(SRCDIR)/%.c, $(OBJDIR)/%.o, $(C_SRC))

CXX_SRC		+= $(wildcard $(SRCDIR)/*.cpp)
OBJS		+= $(patsubst $(SRCDIR)/%.cpp, $(OBJDIR)/%.o, $(CXX_SRC))

INC 		:= -I$(INCDIR)
WARNINGS 	:= -Wall
CFLAGS 		:= $(WARNINGS) $(INC) -m32

ifeq ($(DEBUG), 1)
CFLAGS		+= -g
endif

all: check_directory $(ELF)
lib: check_directory $(DYNAMICLIB) $(STATICLIB) headers

check_directory:
	@$(TEST) $(BINDIR) || $(MKDIR) $(BINDIR)
	@$(TEST) $(OBJDIR) || $(MKDIR) $(OBJDIR)
	@$(TEST) $(LIBDIR) || $(MKDIR) $(LIBDIR)
	@$(TEST) $(INCLUDE) || $(MKDIR) $(INCLUDE)

$(DYNAMICLIB):$(OBJS)
	@$(PRINT) Creating $@ ....
	@$(CC) -shared -fPIC -o $@ $^ -m32

$(STATICLIB):$(OBJS)
	@$(PRINT) Creating $@ ....
	@$(AR) rc $@ $^

headers:
	@$(PRINT) Copying Headers ....
	@$(COPY) $(INCDIR)/*.h $(INCLUDE)

install:
	@$(PRINT) Installing Headers ....	
	@$(INSTALL) $(INCLUDE) $(Prefix)
	@$(PRINT) Installing Library ....
	@$(INSTALL) $(LIBDIR) $(Prefix)

$(ELF):$(OBJS)
	@echo Linking $@ from $^ ....
	@$(LD) -o $(ELF) $^ $(LDFLAGS) -Wl,-Map=$(MAP) -m32
	@echo Disassembling $(ELF) to $(DIS)
	@$(OBJDUMP) -D  $(ELF) > $(DIS)	

#$(OBJCOPY) -O binary -S $(ELF) $@		
    
$(OBJDIR)/%.o:$(SRCDIR)/%.c
	@$(PRINT) Compiling $< ....
	@$(CC) -c -o $@ $< $(CFLAGS)

$(OBJDIR)/%.o:$(SRCDIR)/%.cpp
	@$(PRINT) Compiling $< ....
	@$(CXX) -c -o $@ $< $(CFLAGS)

.PHONY: clean

clean:
	-$(DELDIR) $(OBJDIR) $(BINDIR) $(LIBDIR) $(INCLUDE)
