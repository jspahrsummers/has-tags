all: has-tags

has-tags:
	xcrun clang -Weverything -Ofast -ObjC -framework Foundation -o $@ has-tags.m

install:
	install -Ss has-tags /usr/local/bin

clean:
	rm -rf has-tags
