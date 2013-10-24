all: has-tags

has-tags:
	xcrun clang -Wall -Ofast -framework CoreFoundation -o $@ has-tags.c

install:
	install -Ss has-tags /usr/local/bin

clean:
	rm -rf has-tags
