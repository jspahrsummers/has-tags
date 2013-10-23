#import <Foundation/Foundation.h>
#import <sys/xattr.h>

static const char * const TAGS_XATTR_NAME = "com.apple.metadata:_kMDItemUserTags";
static const int TAGS_XATTR_OPTIONS = XATTR_NOFOLLOW;

int main (int argc, const char **argv) {
	@autoreleasepool {
		if (argc < 3) {
			NSLog(@"Usage: has-tags path tag [tag ...]");
			return EXIT_FAILURE;
		}

		const char *path = argv[1];

		const char **tags = argv + 2;
		int tagCount = argc - 2;

		ssize_t len = getxattr(path, TAGS_XATTR_NAME, NULL, 0, 0, TAGS_XATTR_OPTIONS);
		if (len == 0) {
			NSLog(@"%s does not have any tags", path);
			return EXIT_FAILURE;
		} else if (len < 0) {
			NSLog(@"Error reading xattrs for %s: %s", path, strerror(errno));
			return EXIT_FAILURE;
		}

		void *buffer = calloc(1, (size_t)len);
		ssize_t newLen = getxattr(path, TAGS_XATTR_NAME, buffer, (size_t)len, 0, TAGS_XATTR_OPTIONS);
		if (newLen == 0) {
			NSLog(@"%s does not have any tags", path);
			return EXIT_FAILURE;
		} else if (newLen < 0) {
			NSLog(@"Error reading xattrs for %s: %s", path, strerror(errno));
			return EXIT_FAILURE;
		} else if (newLen > len) {
			NSLog(@"Found greater length (%lli) for xattr than originally returned (%lli)", (long long)newLen, (long long)len);
			return EXIT_FAILURE;
		}

		NSData *data = [[NSData alloc] initWithBytesNoCopy:buffer length:(NSUInteger)newLen freeWhenDone:YES];

		NSError *error = nil;
		NSArray *rawTags = [NSPropertyListSerialization propertyListWithData:data options:NSPropertyListImmutable format:NULL error:&error];
		if (rawTags == nil) {
			NSLog(@"Could not parse tags from xattr: %@", error);
			return EXIT_FAILURE;
		}

		NSMutableArray *tagNames = [[NSMutableArray alloc] initWithCapacity:rawTags.count];
		for (NSString *tag in rawTags) {
			NSArray *components = [tag componentsSeparatedByString:@"\n"];
			[tagNames addObject:components[0]];
		}

		for (int i = 0; i < tagCount; i++) {
			NSString *tag = @(tags[i]);
			if (![tagNames containsObject:tag]) {
				NSLog(@"%s does not have tag \"%@\"", path, tag);
				return EXIT_FAILURE;
			}
		}

		return EXIT_SUCCESS;
	}
}
