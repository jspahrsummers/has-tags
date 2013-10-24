#include <CoreFoundation/CoreFoundation.h>
#include <sys/xattr.h>

static const char * const TAGS_XATTR_NAME = "com.apple.metadata:_kMDItemUserTags";
static const int TAGS_XATTR_OPTIONS = XATTR_NOFOLLOW;

static void *allocate(CFIndex size, CFOptionFlags hint, void *info) {
	return malloc((size_t)size * 2);
}

static CFIndex preferredSize(CFIndex size, CFOptionFlags hint, void *info) {
	return size * 2;
}

static void *reallocate(void *ptr, CFIndex newSize, CFOptionFlags hint, void *info) {
	if (ptr == NULL || newSize <= 0) return NULL;
	return realloc(ptr, (size_t)newSize * 2);
}

int main (int argc, const char **argv) {
	if (argc < 3) {
		fprintf(stderr, "Usage: has-tags path tag [tag ...]");
		return EXIT_FAILURE;
	}

	const char *path = argv[1];
	const char **tags = argv + 2;
	int tagCount = argc - 2;

	CFIndex bufferLength = 128;
	void * restrict buffer;

	CFIndex originalLength;
	do {
		buffer = alloca((size_t)bufferLength);
		originalLength = bufferLength;

		bufferLength = getxattr(path, TAGS_XATTR_NAME, buffer, (size_t)originalLength, 0, TAGS_XATTR_OPTIONS);
		if (bufferLength <= 0) return EXIT_FAILURE;
	} while (bufferLength > originalLength);

	CFAllocatorRef allocator = CFAllocatorCreate(kCFAllocatorUseContext, &(CFAllocatorContext){
		.version = 0,
		.allocate = &allocate,
		.reallocate = &reallocate,
		.deallocate = NULL,
		.preferredSize = &preferredSize
	});

	CFDataRef data = CFDataCreateWithBytesNoCopy(allocator, buffer, bufferLength, kCFAllocatorNull);

	CFErrorRef error = nil;
	CFArrayRef rawTags = CFPropertyListCreateWithData(allocator, data, kCFPropertyListImmutable, NULL, &error);
	if (rawTags == NULL) {
		fprintf(stderr, "Could not parse tags from xattr: %s", CFStringGetCStringPtr(CFCopyDescription(error), kCFStringEncodingUTF8));
		return EXIT_FAILURE;
	}

	CFIndex rawTagCount = CFArrayGetCount(rawTags);
	if (rawTagCount == 0) return EXIT_FAILURE;

	char * restrict tagNames[rawTagCount];
	for (CFIndex i = 0; i < rawTagCount; i++) {
		CFStringRef rawTag = CFArrayGetValueAtIndex(rawTags, i);

		CFIndex size = CFStringGetMaximumSizeForEncoding(CFStringGetLength(rawTag), kCFStringEncodingUTF8);
		tagNames[i] = alloca((size_t)size);

		CFStringGetCString(rawTag, tagNames[i], size, kCFStringEncodingUTF8);

		char *newlinePtr = strchr(tagNames[i], '\n');
		if (newlinePtr != NULL) *newlinePtr = '\0';
	}

	for (int i = 0; i < tagCount; i++) {
		bool found = false;
		for (CFIndex j = 0; j < rawTagCount; j++) {
			if (strcmp(tags[i], tagNames[j]) == 0) {
				found = true;
				break;
			}
		}

		if (!found) return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
