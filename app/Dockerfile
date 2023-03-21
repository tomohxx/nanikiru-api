FROM ubuntu:20.04
ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get update && \
  apt-get install -y \
  git \
  cmake \
  gcc \
  g++ \
  libcpprest-dev \
  libssl-dev \
  libspdlog-dev && \
  apt-get clean && rm -rf /var/lib/apt/lists/*
RUN git clone https://github.com/nlohmann/json.git && \
  cd json && \
  mkdir build && \
  cd build && \
  cmake .. && \
  make && \
  make install
RUN git clone https://github.com/pboettch/json-schema-validator.git && \
  cd json-schema-validator && \
  mkdir build && \
  cd build && \
  cmake .. && \
  make && make install
COPY src/ /src/
WORKDIR /src
RUN mkdir build && \
  cd build && \
  cmake .. -DCMAKE_BUILD_TYPE=Release && \
  make
CMD ["./build/main"]
