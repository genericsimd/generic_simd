#compiler definitions
CC	= gcc
OBJDUMP = objdump

RM = rm

#compiler options
INCLUDE_DIRS	:= -I./
CC_FLAGS   		:= -Wall -g -fno-strict-aliasing -Wall
LINK_FLAGS		:= -lpthread -lrt

#local variables
OBJS	= bsproc_cm.o
SRCS	= ${OBJS:%.o=%.c} 
TARGET	= bsproc_cm 
DEPEND	= depends

#compile rules
all: $(DEPEND) $(TARGET)

%.o:%.c
	$(CC) -c $(CC_FLAGS) $< -o $@

$(DEPEND):
	$(CC) $(CC_FLAGS) -MM $(SRCS)  > $@ 

#target
$(TARGET):$(OBJS)
	$(CC) $(LINK_FLAGS) $(OBJS) -o $@
#	$(OBJDUMP) -Dh $@ > asm

#clean rules
clean:
	$(RM) -rf $(OBJS)
	$(RM) -rf $(DEPEND)
	$(RM) -rf $(TARGET)

-include $(DEPEND)
