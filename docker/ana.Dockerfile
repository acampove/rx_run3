FROM gitlab-registry.cern.ch/lhcb-rd/cal-rx-run3:v6.0
RUN pip install --no-deps src/*
