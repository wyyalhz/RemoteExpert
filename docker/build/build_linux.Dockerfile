FROM ubuntu:18.04

ENV DEBIAN_FRONTEND=noninteractive

# Base tools
RUN apt-get update && apt-get install -y --no-install-recommends \
    ca-certificates software-properties-common gnupg \
    build-essential make git pkg-config \
    curl wget unzip tar xz-utils \
    && rm -rf /var/lib/apt/lists/*

# Qt 5.12.8 (beineri PPA)
RUN add-apt-repository -y ppa:beineri/opt-qt-5.12.8-bionic && \
    apt-get update && apt-get install -y --no-install-recommends \
    qt512base qt512tools qt512svg \
    && rm -rf /var/lib/apt/lists/*

## Provide OpenGL headers and libGL.so needed by Qt (fixes: missing /usr/lib/x86_64-linux-gnu/libGL.so)
RUN apt-get update && apt-get install -y --no-install-recommends \
    libgl1-mesa-dev libglu1-mesa-dev \
    libx11-dev libxext-dev libxrender-dev libxi-dev libxrandr-dev libxfixes-dev \
    libxcb1-dev libxcb-keysyms1-dev libxcb-image0-dev libxcb-render0-dev libxcb-shape0-dev libxcb-xfixes0-dev \
    zlib1g-dev libssl-dev \
    libasound2-dev libpulse-dev \
    gstreamer1.0-tools gstreamer1.0-plugins-base gstreamer1.0-plugins-good gstreamer1.0-libav \
    libgstreamer1.0-dev libgstreamer-plugins-base1.0-dev \
    && rm -rf /var/lib/apt/lists/*

ENV PATH=/opt/qt512/bin:$PATH

WORKDIR /workspace

CMD ["bash", "-lc", "qmake -v && qtpaths --query QT_INSTALL_PREFIX || true"]