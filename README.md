# has-tags

`has-tags` is a small command line utility for OS X Mavericks that tests whether
a file or folder matches a set of tags.

## Installation

```sh
make
[sudo] make install
```

## Usage

```sh
has-tags path tag [tag ...]
```

This tests that `path` has _all_ of the given tags applied to it. The program
will exit with code 0 if the path matches, or a non-zero code if it doesn't.

Only one path can be specified at a time. Each `tag` is case-sensitive.

## Example

```sh
has-tags README.md Red && echo "This README is totes red"
```

To find all files matching a set of tags within a folder:

```sh
find . -exec has-tags {} Red \; -print
```
