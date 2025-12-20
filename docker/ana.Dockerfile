FROM gitlab-registry.cern.ch/lhcb-rd/cal-rx-run3:v6.2
WORKDIR /workspace
COPY src ./src

USER 0
RUN pip install --no-deps src/*
