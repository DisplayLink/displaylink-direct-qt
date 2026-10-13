FROM debian:trixie-slim

# Enable Debian dbgsym repository so we can install Qt debug symbols and fetch
# matching Qt source for gdb.
RUN printf '%s\n' \
        'Types: deb' \
        'URIs: http://deb.debian.org/debian-debug' \
        'Suites: trixie-debug' \
        'Components: main' \
        'Signed-By: /usr/share/keyrings/debian-archive-keyring.pgp' \
        '' \
        'Types: deb-src' \
        'URIs: http://deb.debian.org/debian' \
        'Suites: trixie' \
        'Components: main' \
        'Signed-By: /usr/share/keyrings/debian-archive-keyring.pgp' \
        > /etc/apt/sources.list.d/debug-src.sources

# Install utilities and Qt6 binaries and demos
RUN apt-get update && apt-get install -y --no-install-recommends \
        build-essential \
        gdb \
        cmake \
        ninja-build \
        pkg-config \
        git \
        dbus \
        libdbus-1-dev \
        libudev-dev \
        libinput-dev \
        libx11-dev \
        libxext-dev \
        libxfixes-dev \
        libxrandr-dev \
        libts-dev \
        qt6-base-dev \
        qt6-base-private-dev \
        qt6-base-examples \
        qt6-declarative-examples \
        libqt6core6 \
        libqt6gui6 \
        libqt6widgets6 \
        libqt6quick6 \
        libqt6quickwidgets6 \
        libqt6qml6 \
        libqt6gui6-dbgsym \
        libqt6core6t64-dbgsym \
        libqt6widgets6-dbgsym \
    && rm -rf /var/lib/apt/lists/*

# Install Qt6 source code for debugging purposes
RUN apt-get update \
    && cd /usr/src \
    && apt-get source qt6-base \
    && mv qt6-base-*/ qt6-base \
    && rm -f qt6-base_* \
    && rm -rf /var/lib/apt/lists/*

# Configure gdb to detect the installed Qt source code
RUN printf '%s\n' \
    'set substitute-path ./ /usr/src/qt6-base' \
    >> /etc/gdb/gdbinit

WORKDIR /workspace
