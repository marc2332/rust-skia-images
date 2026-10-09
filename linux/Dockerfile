# Ubuntu 24.04 provides FreeType 2.13 headers for stable COLRv1 support.
FROM --platform=linux/amd64 ubuntu:24.04
LABEL org.opencontainers.image.source=https://github.com/pragmatrix/rust-skia-images

# For github actions we need a newer git version
RUN apt-get update \
 && apt-get install -y software-properties-common \
 && add-apt-repository ppa:git-core/ppa -y

RUN apt-get install -y wget gnupg

# Add the LLVM 20 repository for Ubuntu 24.04.
RUN wget -O /tmp/llvm-snapshot.gpg.key https://apt.llvm.org/llvm-snapshot.gpg.key \
 && gpg --dearmor -o /usr/share/keyrings/llvm.gpg /tmp/llvm-snapshot.gpg.key \
 && rm /tmp/llvm-snapshot.gpg.key \
 && echo "deb [arch=amd64 signed-by=/usr/share/keyrings/llvm.gpg] https://apt.llvm.org/noble/ llvm-toolchain-noble-20 main" > /etc/apt/sources.list.d/llvm.list

# unzip: for installing the Android NDK
# libgl1 libgl1-mesa-dev mesa-common-dev: for builds that need OpenGL
# libgles2-mesa-dev for egl support.
# Clang 20 uses the GCC 13 standard library headers.
# make is needed for supplemental builds.
# ninja-build for building Skia
RUN apt-get update && apt-get install -y \
	curl \
	clang-20 \
	gcc \
	g++-13 \
	git \
	libgl1 \
	libgl1-mesa-dev \
	libgles2-mesa-dev \
	libssl-dev \
	libfontconfig1-dev \
	libfreetype-dev \
	mesa-common-dev \
	llvm-20-dev \
	pkg-config \
	python3 \
	python-is-python3 \
	unzip \
	make \
	libwayland-dev \
	ninja-build \
	libstdc++-13-dev

RUN curl https://sh.rustup.rs -sSf | sh -s -- -y
ENV PATH="/root/.cargo/bin:${PATH}"
RUN cargo install --locked just

# Install Android NDK

ENV ANDROID_NDK_VERSION=r26d
ENV ANDROID_HOST=linux-x86_64

# HTTP/1.1, because of spurious HTTP/2 framing errors in the Azure / GitHub cloud.

RUN curl --http1.1 -sSf -o android-ndk.zip https://dl.google.com/android/repository/android-ndk-${ANDROID_NDK_VERSION}-linux.zip \
 && unzip android-ndk.zip \
 && rm android-ndk.zip

ENV ANDROID_NDK=/android-ndk-${ANDROID_NDK_VERSION}
ENV PATH=${PATH}:/android-ndk-${ANDROID_NDK_VERSION}/toolchains/llvm/prebuilt/${ANDROID_HOST}/bin

# wasm32-unknown-emscripten

SHELL ["/bin/bash", "-c"]

ENV EMSCRIPTEN_VER=3.1.59

RUN git clone https://github.com/emscripten-core/emsdk.git \
	&& (cd emsdk && ./emsdk install ${EMSCRIPTEN_VER}) \
	&& (cd emsdk && ./emsdk activate ${EMSCRIPTEN_VER})

# Needed for cross compilation to aarch64-unknown-linux-gnu
# Enable multiarch for arm64 to install cross-compilation libraries
RUN dpkg --add-architecture arm64
# Configure apt to use ports.ubuntu.com for arm64 packages
RUN sed -i '/^Types:/a Architectures: amd64' /etc/apt/sources.list.d/ubuntu.sources \
	&& printf '%s\n' \
		"deb [arch=arm64] http://ports.ubuntu.com/ubuntu-ports noble main restricted universe multiverse" \
		"deb [arch=arm64] http://ports.ubuntu.com/ubuntu-ports noble-updates main restricted universe multiverse" \
		"deb [arch=arm64] http://ports.ubuntu.com/ubuntu-ports noble-backports main restricted universe multiverse" \
		"deb [arch=arm64] http://ports.ubuntu.com/ubuntu-ports noble-security main restricted universe multiverse" \
		> /etc/apt/sources.list.d/ubuntu-arm64.list
RUN apt-get update && apt-get install -y --no-install-recommends \
	gcc-13-aarch64-linux-gnu \
	g++-13-aarch64-linux-gnu \
	libstdc++-13-dev:arm64 \
	libgcc-13-dev:arm64

# Set up alternatives after all packages are installed
RUN update-alternatives \
	--install /usr/bin/clang clang /usr/bin/clang-20 90 \
	--slave /usr/bin/clang++ clang++ /usr/bin/clang++-20 \
	--slave /usr/bin/llvm-config llvm-config /usr/bin/llvm-config-20
RUN update-alternatives --install /usr/bin/c++ c++ /usr/bin/clang++-20 100

# Set up alternatives for aarch64 cross-compiler
RUN update-alternatives \
	--install /usr/bin/aarch64-linux-gnu-gcc aarch64-linux-gnu-gcc /usr/bin/aarch64-linux-gnu-gcc-13 100 \
	--slave /usr/bin/aarch64-linux-gnu-g++ aarch64-linux-gnu-g++ /usr/bin/aarch64-linux-gnu-g++-13

# Add script to test C++ include paths
COPY test_includes.sh /usr/local/bin/test_includes.sh
RUN chmod +x /usr/local/bin/test_includes.sh

# Run the include path tests
RUN /usr/local/bin/test_includes.sh
