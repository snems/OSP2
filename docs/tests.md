# How to use tests

Install **git** on your system. GitHub has a nice [instruction](https://github.com/git-guides/install-git) how to do this for any OS.

## Testing

**CMake** configuration has `BUILD_TESTS` flag, which enables tests.

Follow [build instructions (README)](../README.md#how-to-build). After the mod has successfully built, configure it with the `BUILD_TESTS` flag enabled:

```bash
cmake -S . -B build -DBUILD_TESTS=ON
```

and build as usual:

```bash
cmake --build build
```
