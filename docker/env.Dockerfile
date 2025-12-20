FROM quay.io/fedora/python-313 AS builder

USER 0
RUN dnf makecache \
    && dnf install -y gcc gcc-c++ make cmake libuuid-devel \
    && dnf clean all

WORKDIR /build
COPY requirements-dep.txt .

RUN pip install --no-cache-dir -r requirements-dep.txt

# ---

FROM quay.io/fedora/python-313 AS final

WORKDIR /workspace
COPY --from=builder /opt/app-root /opt/app-root

USER 0
RUN dnf install -y libuuid && dnf clean all
USER 1001
