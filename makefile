#VERSION = 3.02
VERBOSE=0
CXX      = gcc

CPPFLAGS  = -I./ -Iinclude/
LDFLAGS =  -L/lib/arm-linux-gnueabi -lpthread -lasound  

OBJDIR=obj
BINDIR=bin

include inputs.mk		
-include $(OBJ:.o=.d)

.PHONY: clean includes check

all: $(BINDIR)/syncedSoundClient

includes:
	@ls src/*.c | sed 's/src\/\(.*\)\.c/OBJ+=obj\/\1.o/' > inputs.mk	
	@cat inputs.mk

$(OBJDIR)/%.o: src/%.c
	@echo "compiling.." $<
	@$(CXX) -c $(CFLAGS) $(CPPFLAGS) $< -o $@
	@$(CXX) -MM $(CFLAGS) $(CPPFLAGS) $< > $(OBJDIR)/$*.d
	@sed -i '1 s/^/obj\//' $(OBJDIR)/$*.d

$(BINDIR)/syncedSoundClient: $(OBJ)
	@echo "linking.."
	@$(CXX) $(CPPFLAGS) -o $@ $(OBJ) $(LDFLAGS)

clean:
	rm -rf $(OBJDIR)/*.o $(BINDIR)/server $(OBJDIR)/*.d
	
check:
	@echo checking...
	@cppcheck --enable=all src/*
