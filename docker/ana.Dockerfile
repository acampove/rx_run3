FROM gitlab-registry.cern.ch/lhcb-rd/cal-rx-run3:v6.0
WORKDIR /workspace
COPY src ./src
RUN pip install --no-deps src/*
