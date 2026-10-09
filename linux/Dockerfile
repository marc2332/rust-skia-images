# Keep the glibc 2.31 baseline while building newer FreeType below.
FROM --platform=linux/amd64 ubuntu:20.04
LABEL org.opencontainers.image.source=https://github.com/pragmatrix/rust-skia-images

# For github actions we need a newer git version
RUN apt-get update \
 && apt-get install -y software-properties-common \
 && add-apt-repository ppa:git-core/ppa -y

RUN apt-get install -y wget gnupg

# Add the LLVM 20 repository for Ubuntu 20.04.
RUN wget -O /tmp/llvm-snapshot.gpg.key https://apt.llvm.org/llvm-snapshot.gpg.key \
 && gpg --dearmor -o /usr/share/keyrings/llvm.gpg /tmp/llvm-snapshot.gpg.key \
 && rm /tmp/llvm-snapshot.gpg.key \
 && echo "deb [arch=amd64 signed-by=/usr/share/keyrings/llvm.gpg] https://apt.llvm.org/focal/ llvm-toolchain-focal-20 main" > /etc/apt/sources.list.d/llvm.list

# unzip: for installing the Android NDK
# libgl1 libgl1-mesa-dev mesa-common-dev: for builds that need OpenGL
# libgles2-mesa-dev for egl support.
# Clang 20 uses the GCC 10 standard library headers.
# make is needed for supplemental builds.
# ninja-build for building Skia
RUN apt-get update && apt-get install -y \
	curl \
	clang-20 \
	gcc \
	g++-10 \
	git \
	libgl1 \
	libgl1-mesa-dev \
	libgles2-mesa-dev \
	libssl-dev \
	libfontconfig1-dev \
	libfreetype-dev \
	libpng-dev \
	libbz2-dev \
	libbrotli-dev \
	zlib1g-dev \
	xz-utils \
	mesa-common-dev \
	llvm-20-dev \
	pkg-config \
	python3 \
	python-is-python3 \
	unzip \
	make \
	libwayland-dev \
	ninja-build \
	libstdc++-10-dev

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

RUN git clone --depth 1 --branch ${EMSCRIPTEN_VER} https://github.com/emscripten-core/emsdk.git \
	&& (cd emsdk && ./emsdk install ${EMSCRIPTEN_VER}) \
	&& (cd emsdk && ./emsdk activate ${EMSCRIPTEN_VER})

# Needed for cross compilation to aarch64-unknown-linux-gnu
# Enable multiarch for arm64 to install cross-compilation libraries
RUN dpkg --add-architecture arm64
# Configure apt to use ports.ubuntu.com for arm64 packages
RUN sed -i 's/^deb /deb [arch=amd64] /' /etc/apt/sources.list \
	&& printf '%s\n' \
		"deb [arch=arm64] http://ports.ubuntu.com/ubuntu-ports focal main restricted universe multiverse" \
		"deb [arch=arm64] http://ports.ubuntu.com/ubuntu-ports focal-updates main restricted universe multiverse" \
		"deb [arch=arm64] http://ports.ubuntu.com/ubuntu-ports focal-backports main restricted universe multiverse" \
		"deb [arch=arm64] http://ports.ubuntu.com/ubuntu-ports focal-security main restricted universe multiverse" \
		> /etc/apt/sources.list.d/ubuntu-arm64.list
RUN apt-get update && apt-get install -y --no-install-recommends \
	gcc-10-aarch64-linux-gnu \
	g++-10-aarch64-linux-gnu \
	libstdc++-10-dev:arm64 \
	libgcc-10-dev:arm64

# Set up alternatives after all packages are installed
RUN update-alternatives \
	--install /usr/bin/clang clang /usr/bin/clang-20 90 \
	--slave /usr/bin/clang++ clang++ /usr/bin/clang++-20 \
	--slave /usr/bin/llvm-config llvm-config /usr/bin/llvm-config-20
RUN update-alternatives --install /usr/bin/c++ c++ /usr/bin/clang++-20 100

# Set up alternatives for aarch64 cross-compiler
RUN update-alternatives \
	--install /usr/bin/aarch64-linux-gnu-gcc aarch64-linux-gnu-gcc /usr/bin/aarch64-linux-gnu-gcc-10 100 \
	--slave /usr/bin/aarch64-linux-gnu-g++ aarch64-linux-gnu-g++ /usr/bin/aarch64-linux-gnu-g++-10

ARG FREETYPE_VERSION=2.14.3
ARG FREETYPE_SHA256=36bc4f1cc413335368ee656c42afca65c5a3987e8768cc28cf11ba775e785a5f

# Replace distro FreeType after apt installs so Skia's fixed include paths use the new headers.
RUN curl --fail --location --retry 3 \
	"https://download-mirror.savannah.gnu.org/releases/freetype/freetype-${FREETYPE_VERSION}.tar.xz" \
	-o /tmp/freetype.tar.xz \
	&& echo "${FREETYPE_SHA256}  /tmp/freetype.tar.xz" | sha256sum --check - \
	&& tar -xf /tmp/freetype.tar.xz -C /tmp \
	&& cd /tmp/freetype-${FREETYPE_VERSION} \
	&& ./configure --prefix=/usr --libdir=/usr/lib/x86_64-linux-gnu \
		--enable-shared --disable-static --with-harfbuzz=no \
		--with-zlib=yes --with-bzip2=yes --with-png=yes --with-brotli=yes \
	&& make -j"$(nproc)" \
	&& make install \
	&& ldconfig \
	&& rm -rf /tmp/freetype.tar.xz /tmp/freetype-${FREETYPE_VERSION}

COPY test_freetype.c /tmp/test_freetype.c
RUN clang /tmp/test_freetype.c -I/usr/include/freetype2 -lfreetype -o /tmp/test_freetype \
	&& /tmp/test_freetype \
	&& clang /tmp/test_freetype.c $(pkg-config --cflags --libs freetype2) -o /tmp/test_freetype \
	&& /tmp/test_freetype \
	&& rm /tmp/test_freetype /tmp/test_freetype.c

# Add script to test C++ include paths
COPY test_includes.sh /usr/local/bin/test_includes.sh
RUN chmod +x /usr/local/bin/test_includes.sh

# Run the include path tests
RUN /usr/local/bin/test_includes.sh
