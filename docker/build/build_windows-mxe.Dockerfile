FROM ubuntu:22.04

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y --no-install-recommends \
    ca-certificates gnupg software-properties-common \
    build-essential make git pkg-config \
    curl wget unzip zip tar xz-utils \
    binutils-mingw-w64 mingw-w64 wine64 \
    file python3 \
    && rm -rf /var/lib/apt/lists/*

# Build selected MXE packages (qtbase/qtsvg/qtmultimedia)
# 首次构建时间较长；后续可换用 GHCR 预构建镜像以提速
RUN git clone https://github.com/mxe/mxe.git /opt/mxe && \
    make -C /opt/mxe -j"$(nproc)" \
      MXE_TARGETS='x86_64-w64-mingw32.shared' \
      qtbase qtsvg qtmultimedia

ENV PATH=/opt/mxe/usr/bin:$PATH \
    MXE_PREFIX=/opt/mxe/usr \
    MXE_TARGET=x86_64-w64-mingw32.shared

WORKDIR /workspace

CMD ["bash", "-lc", "${MXE_TARGET}-qmake-qt5 -v || true"]