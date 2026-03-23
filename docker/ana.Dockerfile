FROM gitlab-registry.cern.ch/lhcb-rd/cal-rx-run3:v3.0.0
WORKDIR /workspace
COPY src                ./src
COPY requirements-dev.txt .

USER 0
RUN pip install --no-deps -r requirements-dev.txt
