NAME = stereo-mixer
BUNDLE = $(NAME).lv2
LIBS = `pkg-config --cflags --libs lv2`
ALL_TTL = manifest.ttl $(NAME).ttl
CXX = gcc
CXX_FLAGS = -std=c11 -fPIC

ifeq ($(DEBUG),Y)
	DEBUG_C_FLAGS = -g -DDEBUG
endif

$(BUNDLE): clean $(NAME).so
	mkdir -- $(BUNDLE)
	cp -- $(ALL_TTL) $(NAME).so $(BUNDLE)

$(NAME).so:
	$(CXX) -shared $(CXX_FLAGS) $(DEBUG_C_FLAGS) $(LIBS) \
		src/$(NAME).c -o $(NAME).so

clean:
	rm -rf -- $(BUNDLE) $(NAME).so
