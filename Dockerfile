FROM acampove/rx_run3:v4.7

COPY . /workspace
WORKDIR /workspace

RUN pip install --no-deps -r requirements.txt
