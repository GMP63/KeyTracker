# The project name.
PROJECT := 'Hotspot Tracker'

# The "pure header" file list affecting all the source code.
templates = Message
htpls = $(patsubst %, include/%.h, $(templates))

# The source file list.
app_modules = messageserver ThreadedMessageQueue MapManager backupmanager
web_modules = http-connection listener webserver
hdrs = $(patsubst %, include/web/%.h, $(web_modules)) $(patsubst %, include/%.h, $(app_modules)) include/timer/timer.h
#srcs = $(patsubst %, src/web/%.cpp, $(web_modules)) $(patsubst %, src/%.cpp, $(app_modules)) src/timer/timer.cpp
test_modules = test-macros MapManager_Test ThreadedMessageQueue_Test
test_hdrs = $(patsubst %, test/src/%.h, $(test_modules))
#test_srcs = $(patsubst %, test/src/%.cpp, $(test_modules))
client_stress_test_file = client-stress-test
client_cmd_file = http-client-command

# The object file list.
objs = $(patsubst %, obj/web/%.o, $(web_modules)) $(patsubst %, obj/%.o, $(app_modules)) obj/timer/timer.o
test_objs = $(patsubst %, test/obj/%.o, $(test_modules))
client_stress_test_obj = $(patsubst %, obj/client/%.o, $(client_stress_test_file))
client_cmd_obj = $(patsubst %, obj/client/%.o, $(client_cmd_file))

# The executable filenames and their respective binary, object, and source files.
TARGET_APP = tracker
TARGET_TEST = test
TARGET_CLIENT_STRESS_TEST = bin/client_benchmark
TARGET_CLIENT_CMD = bin/http-client-command
TARGET_APP_BIN = bin/$(TARGET_APP)
TARGET_TEST_BIN = test/bin/$(TARGET_TEST)
TARGET_APP_OBJ = obj/$(TARGET_APP).o
TARGET_TEST_OBJ = test/obj/$(TARGET_TEST).o
TARGET_APP_SRC = src/$(TARGET_APP).cpp
TARGET_TEST_SRC = test/src/$(TARGET_TEST).cpp

#Dependent flags: release or debug (rod), address sanitizer or not.
#The selected target is release
ifneq (,$(findstring release,$(MAKECMDGOALS)))
  rod = -g0 -O3 -ftree-parallelize-loops=8 -floop-interchange # -fopt-info-optimized
# otherwise, debugging build is assumed
else
  rod = -g3 -O0
endif

ifneq (,$(findstring sanitizer,$(MAKECMDGOALS)))
  sanity_flags = -fno-omit-frame-pointer -fsanitize=address
endif

# The compiler flags.
CPPFLAGS = $(rod) $(sanity_flags) -Wall -Wextra -Wpedantic -Wno-trigraphs -fno-rtti -std=c++17 -Iinclude -I/home/guille/Proyecto/boost/include

# The linker flags for each target.
LDFLAGS = $(sanity_flags) -pthread -lstdc++
LDFLAGS_APP = -L ~/Proyecto/boost/lib $(LDFLAGS) -lboost_filesystem
LDFLAGS_TEST = -L ~/Proyecto/boost/lib $(LDFLAGS) -lm -lboost_filesystem

# The create directory action
make_dir = mkdir -p
# The compile action
compile = g++ $(CPPFLAGS) -c -o
# The link action (including target)
link = gcc -o


.PHONY: release sanitizer

all: dirs app webclient-cmd webclient-test test

app: $(TARGET_APP_BIN)

webclient-cmd: $(TARGET_CLIENT_CMD)

webclient-test: $(TARGET_CLIENT_STRESS_TEST)

test: test_dirs $(TARGET_TEST_BIN)

help:
	@echo ---------------------------------------------------------------------------------------
	@echo "USAGE:"
	@echo "make [release] [sanitizer] <app|webclient-cmd|webclient-test|test|clean|cleanapp|cleantest|cleanclient|help>"
	@echo ---------------------------------------------------------------------------------------


dirs:
	$(make_dir) bin
	$(make_dir) obj
	$(make_dir) obj/client
	$(make_dir) obj/timer
	$(make_dir) obj/web

test_dirs:
	$(make_dir) test/bin
	$(make_dir) test/obj

$(TARGET_APP_BIN): $(TARGET_APP_OBJ) $(objs)
	@echo ------------------------------------------------------------------------------
	@echo 'Linking file: $(TARGET_APP)'
	$(link) $(TARGET_APP_BIN) $(TARGET_APP_OBJ) $(objs) $(LDFLAGS_APP)
	@echo 'Finished linking: $(TARGET_APP)'
	@echo 'BUILD SUCCEEDED'
	@echo

$(TARGET_APP_OBJ): $(htpls) $(hdrs) $(TARGET_APP_SRC)

obj/timer/%.o: include/timer/%.h src/timer/%.cpp

obj/web/%.o: include/web/%.h src/web/%.cpp

obj/%.o: $(htpls) include/%.h src/%.cpp

test/obj/%.o: $(htpls) test/src/%.h test/src/%.cpp

$(TARGET_CLIENT_CMD): $(client_cmd_obj)
	@echo ------------------------------------------------------------------------
	@echo 'Linking file: $(TARGET_CLIENT_CMD)'
	$(link) $(TARGET_CLIENT_CMD) $(client_cmd_obj) $(LDFLAGS)
	@echo 'Finished linking: $(TARGET_CLIENT_CMD)'
	@echo 'BUILD SUCCEEDED'
	@echo 

$(TARGET_CLIENT_STRESS_TEST): $(client_stress_test_obj)
	@echo ------------------------------------------------------------------------
	@echo 'Linking file: $(TARGET_CLIENT_STRESS_TEST)'
	$(link) $(TARGET_CLIENT_STRESS_TEST) $(client_stress_test_obj) $(LDFLAGS)
	@echo 'Finished linking: $(TARGET_CLIENT_STRESS_TEST)'
	@echo 'BUILD SUCCEEDED'
	@echo 

$(TARGET_TEST_BIN): $(TARGET_TEST_OBJ) $(test_objs) $(objs)
	@echo ------------------------------------------------------------------------------
	@echo 'Linking file: $(TARGET_TEST)'
	$(link) $(TARGET_TEST_BIN) $(TARGET_TEST_OBJ) $(test_objs) $(objs) $(LDFLAGS_TEST)
	@echo 'Finished linking: $(TARGET_TEST)'
	@echo 'BUILD SUCCEEDED'

	@echo
	@echo Running tests...
	@echo
	(LD_LIBRARY_PATH=~/Proyecto/boost/lib $(TARGET_TEST_BIN))
	@echo

$(TARGET_TEST_OBJ): $(htpls) $(hdrs) $(test_hdrs) $(TARGET_TEST_SRC) $(test_srcs)


obj/client/%.o: src/client/%.cpp
	@echo ------------------------------------------------------------------------------
	@echo 'Compiling file: $<'
	$(compile) $@ $<
	@echo 'Finished building: $<'

obj/timer/%.o: src/timer/%.cpp
	@echo ------------------------------------------------------------------------------
	@echo 'Compiling file: $<'
	$(compile) $@ $<
	@echo 'Finished building: $<'

obj/web/%.o: src/web/%.cpp
	@echo ------------------------------------------------------------------------------
	@echo 'Compiling file: $<'
	$(compile) $@ $<
	@echo 'Finished building: $<'

obj/%.o: src/%.cpp
	@echo ------------------------------------------------------------------------------
	@echo 'Compiling file: $<'
	$(compile) $@ $<
	@echo 'Finished building: $<'

test/obj/%.o: test/src/%.cpp
	@echo ------------------------------------------------------------------------------
	@echo 'Compiling file: $<'
	$(compile) $@ $<
	@echo 'Finished building: $<'


clean:
	@echo ------------------------------------------------------------------------------
	@echo 'Cleaning whole project $(PROJECT) ...'
	rm -f  $(objs) $(TARGET_APP_OBJ) $(TARGET_APP_BIN) $(test_objs) $(TARGET_TEST_OBJ) $(TARGET_TEST_BIN)
	rm -f  $(client_stress_test_obj) $(client_cmd_obj) $(TARGET_CLIENT_STRESS_TEST) $(TARGET_CLIENT_CMD)
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

cleanclient:
	@echo ------------------------------------------------------------------------------
	@echo 'Cleaning client stress test and client command ...'
	rm -f  $(client_stress_test_obj) $(client_cmd_obj) $(TARGET_CLIENT_STRESS_TEST) $(TARGET_CLIENT_CMD)
	@echo Done.
