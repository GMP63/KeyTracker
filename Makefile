# The project name.
PROJECT = 'Hotspot Tracker'

# The "pure header" file list.
templates = Message ThreadedQueue
htpls = $(patsubst %, include/%.h, $(templates))

# The source file list.
app_modules = MapManager ThreadedMessageQueue
hdrs = $(patsubst %, include/%.h, $(app_modules))
srcs = $(patsubst %, src/%.cpp, $(app_modules))
test_modules = test-macros MapManager_Test ThreadedMessageQueue_Test
test_hdrs = $(patsubst %, test/src/%.h, $(test_modules))
test_srcs = $(patsubst %, test/src/%.cpp, $(test_modules))

# The object file list.
objs = $(patsubst %, obj/%.o, $(app_modules))
test_objs = $(patsubst %, test/obj/%.o, $(test_modules))

# The executable filenames and their respective binary, object, and source files.
TARGET_APP = tracker
TARGET_TEST = test
TARGET_APP_BIN = bin/$(TARGET_APP)
TARGET_TEST_BIN = test/bin/$(TARGET_TEST)
TARGET_APP_OBJ = obj/$(TARGET_APP).o
TARGET_TEST_OBJ = test/obj/$(TARGET_TEST).o
TARGET_APP_SRC = src/$(TARGET_APP).cpp
TARGET_TEST_SRC = test/src/$(TARGET_TEST).cpp

#Dependent flags: Release Or Debug
#The selected target is release
ifneq (,$(findstring release,$(MAKECMDGOALS)))
  rod = -g0 -O3 -ftree-parallelize-loops=8 -floop-interchange # -fopt-info-optimized
# otherwise, debugging build is assumed
else
  rod = -g3 -O0
endif

# The compiler flags.
CPPFLAGS = $(rod) -std=c++17 -Iinclude -I/home/guille/Proyecto/boost/include
# The linker flags.
LDFLAGS = -L ~/Proyecto/boost/lib -pthread -lstdc++ -lm

# The create directory action
make_dir = mkdir -p
# The compile action
compile = g++ $(CPPFLAGS) -c -o
# The link action (including target)
link = gcc -o
# The linker trailing options (after objs and custom libs list)
lnktrailopt = $(LDFLAGS)

.PHONY: release

all: app test

app: dirs $(TARGET_APP_BIN)

test: test_dirs $(TARGET_TEST_BIN)

dirs:
	$(make_dir) bin
	$(make_dir) obj

test_dirs:
	$(make_dir) test/bin
	$(make_dir) test/obj

$(TARGET_APP_BIN): $(TARGET_APP_OBJ) $(objs)
	@echo ------------------------------------------------------------------------------
	@echo 'Linking file: $(TARGET_APP)'
	$(link) $(TARGET_APP_BIN) $(TARGET_APP_OBJ) $(objs) $(lnktrailopt)
	@echo 'Finished linking: $(TARGET_APP)'
	@echo 'BUILD SUCCEEDED'
	@echo

$(TARGET_APP_OBJ): $(htpls) $(hdrs) $(TARGET_APP_SRC)

obj/%.o: $(htpls) include/%.h src/%.cpp

$(TARGET_TEST_BIN): $(TARGET_TEST_OBJ) $(test_objs) $(objs)
	@echo ------------------------------------------------------------------------------
	@echo 'Linking file: $(TARGET_TEST)'
	$(link) $(TARGET_TEST_BIN) $(TARGET_TEST_OBJ) $(test_objs) $(objs) $(lnktrailopt)
	@echo 'Finished linking: $(TARGET_TEST)'
	@echo 'BUILD SUCCEEDED'
	@echo
	@echo Running tests...
	@echo
	$(TARGET_TEST_BIN)
	@echo

$(TARGET_TEST_OBJ): $(htpls) $(hdrs) $(test_hdrs) $(TARGET_TEST_SRC) $(test_srcs)

test/obj/%.o: $(htpls) test/src/%.h test/src/%.cpp

obj/%.o: src/%.cpp
	@echo ------------------------------------------------------------------------------
	@echo 'Building file: $<'
	$(compile) $@ $<
	@echo 'Finished building: $<'

test/obj/%.o: test/src/%.cpp
	@echo ------------------------------------------------------------------------------
	@echo 'Building file: $<'
	$(compile) $@ $<
	@echo 'Finished building: $<'


clean:
	@echo ------------------------------------------------------------------------------
	@echo 'Cleaning whole project $(PROJECT) ...'
	rm -f  $(objs) $(TARGET_APP_OBJ) $(TARGET_APP_BIN) $(test_objs) $(TARGET_TEST_OBJ) $(TARGET_TEST_BIN)
	@echo Done.

cleanapp:
	@echo ------------------------------------------------------------------------------
	@echo 'Cleaning application ...'
	rm -f  $(objs) $(TARGET_APP_OBJ) $(TARGET_APP_BIN)
	@echo Done.

cleantest:
	@echo ------------------------------------------------------------------------------
	@echo 'Cleaning test ...'
	rm -f  $(test_objs) $(TARGET_TEST_OBJ) $(TARGET_TEST_BIN)
	@echo Done.
