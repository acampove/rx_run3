FROM gitlab-registry.cern.ch/lhcb-rd/cal-rx-run3:v1

COPY  . /workspace
WORKDIR /workspace

ENV http_proxy=
ENV HTTP_proxy=
ENV https_proxy=
ENV HTTPS_proxy=
RUN pip install "tabulate>=0.9.0"
RUN pip install "reana-client"
RUN pip install "pytest-split"
RUN pip install --no-deps -r requirements.txt
