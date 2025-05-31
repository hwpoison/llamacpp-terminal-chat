CC = g++
CPPFLAGS= -std=c++17 -s -Os -fno-ident -fno-ident -fno-asynchronous-unwind-tables -fvisibility=hidden -fmerge-all-constants -fno-exceptions -Iinclude -I./yyjson/src
SFLAGS= -static-libgcc -static-libstdc++ -static
LDFLAGS += -Lyyjson  
OBJECTS := build/minipost.o build/terminal.o build/completion.o build/sjson.o build/utils.o build/logging.o build/chat.o build/yyjson.o

COPY_FILES = $(if $(filter Windows_NT, $(OS)), \
			   copy $(subst /,\,$(1)) $(subst /,\,$(2)), \
			   cp $(1) $(2))

RM_FILES = $(if $(filter Windows_NT, $(OS)), \
			   del $(subst /,\,$(1)), \
			   rm $(1) || true) 


# copy function for windows and linux
ifeq ($(OS),Windows_NT)
	LDFLAGS += -lws2_32
	CLEAN_COMMAND := del /F /Q
	MKDIR = if not exist $(subst /,\\,$1) mkdir $(subst /,\\,$1)
else
	CLEAN_COMMAND := rm -f
	MKDIR = mkdir -p $1
endif

copy_config:
	$(call COPY_FILES, config/*.*, dist/)

build/%.o: src/%.cpp
	$(call MKDIR,build)
	$(CC) -c $< -o $@ $(CPPFLAGS)

build/yyjson.o: yyjson/src/yyjson.c
	$(call MKDIR,build)
	$(CC) -c $< -o $@ $(CPPFLAGS)

chat: $(OBJECTS)
	$(call MKDIR,dist)
	$(CC) src/main.cpp $(OBJECTS) -o dist/chat $(CPPFLAGS) $(LDFLAGS)
	$(MAKE) copy_config

static: $(OBJECTS)
	$(call MKDIR,dist)
	$(CC) src/main.cpp $(OBJECTS) -o dist/chat $(CPPFLAGS) $(SFLAGS) $(LDFLAGS)
	$(MAKE) copy_config

clean:
	$(call RM_FILES, build/*.o)
	$(call RM_FILES, dist/chat*)
	$(call RM_FILES, dist/*.json)
	$(call RM_FILES, dist/*.sh)
	$(call RM_FILES, dist/*.bat)