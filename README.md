# Rust-Skia Build Images

This repository builds and publishes the Linux image used to compile Rust-Skia project binaries.

The amd64 image uses Ubuntu 20.04, Clang 20, and GCC 10 standard library headers to retain the glibc 2.31 build baseline. Android, Emscripten, and ARM64 cross-compilation tools remain included.

FreeType 2.14.3 is built from a SHA-256-verified source archive after all apt installs. It replaces the distro headers and native shared library under `/usr`, including `/usr/include/freetype2`, which Skia uses directly. The image build checks both direct and pkg-config-based header and runtime selection.

This does not enable embedded FreeType in Skia. Applications still use their runtime system libraries, so test the resulting Skia archives on each supported Linux baseline, including COLRv1 rendering on Ubuntu 22.04 and 24.04. Avoid subsequent apt upgrades of FreeType in build jobs, which could replace the pinned installation.

Build locally with:

```sh
docker build linux --tag rust-skia:linux
```

CI publishes only `ghcr.io/<owner>/rust-skia-linux:latest`. The experimental Linux, native ARM64, and Windows image definitions have been removed.
