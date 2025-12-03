FROM gitlab-registry.cern.ch/lhcb-rd/cal-rx-run3:v1

COPY . /workspace
WORKDIR /workspace

ENV http_proxy=
ENV HTTP_proxy=
ENV https_proxy=
ENV HTTPS_proxy=
RUN pip install --no-deps -r requirements.txt
