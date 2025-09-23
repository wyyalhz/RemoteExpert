FROM mxe/mxe:latest

ENV DEBIAN_FRONTEND=noninteractive

# 与 Linux 构建镜像保持相似风格：注释与分步构建
# 说明：此镜像已包含 MXE 基础环境与构建工具

# 目标三方包（依据 client.pro 与脚本：需要 svg、multimedia、serialport、charts）
# 分步先下载源码，再编译；同时提高日志详细度（V=1），并控制并发避免 OOM
ARG MXE_JOBS=2
ENV PATH=/opt/mxe/usr/bin:$PATH \
    MXE_PREFIX=/opt/mxe/usr \
    MXE_TARGET=x86_64-w64-mingw32.shared

# 下载 Qt 源码（便于缓存和重试）
RUN make -C /opt/mxe V=1 MXE_TARGETS="${MXE_TARGET}" \
    download-qtbase download-qtsvg download-qtmultimedia download-qtserialport download-qtcharts

# 编译所需 Qt 组件
RUN make -C /opt/mxe -j"${MXE_JOBS}" V=1 MXE_TARGETS="${MXE_TARGET}" \
    qtbase qtsvg qtmultimedia qtserialport qtcharts

WORKDIR /workspace

CMD ["bash", "-lc", "${MXE_TARGET}-qmake-qt5 -v || true"]