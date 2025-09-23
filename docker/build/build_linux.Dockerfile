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

ENV PATH=/opt/qt512/bin:$PATH

WORKDIR /workspace

CMD ["bash", "-lc", "qmake -v && qtpaths --query QT_INSTALL_PREFIX || true"]