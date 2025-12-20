FROM python:3.13-slim AS base
RUN apt-get update && apt-get install -y --no-install-recommends \
    build-essential \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /workspace
COPY requirements-dep.txt .
RUN pip install --no-cache-dir -r requirements-dep.txt
