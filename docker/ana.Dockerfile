FROM gitlab-registry.cern.ch/lhcb-rd/cal-rx-run3:v4
RUN pip install --no-deps src/*
