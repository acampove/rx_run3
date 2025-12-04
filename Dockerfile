FROM gitlab-registry.cern.ch/lhcb-rd/cal-rx-run3:v3

COPY  . /workspace
WORKDIR /workspace

RUN pip install --no-deps -r requirements.txt
