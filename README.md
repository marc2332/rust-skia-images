# Rust-Skia Build Images

This repository builds and publishes the Linux image used to compile Rust-Skia project binaries.

The amd64 image uses Ubuntu 20.04, Clang 20, and GCC 10 standard library headers to retain the glibc 2.31 build baseline. Android, Emscripten, and ARM64 cross-compilation tools remain included.

The image uses Ubuntu 20.04's system FreeType 2.10.1 headers and library. Skia does not compile its COLRv1 rendering path with these headers.

Build locally with:

```sh
docker build linux --tag rust-skia:linux
```

CI publishes only `ghcr.io/<owner>/rust-skia-linux:latest`. The experimental Linux, native ARM64, and Windows image definitions have been removed.
