# Use a lightweight Alpine Linux base image
FROM alpine:3.21.2

# Set environment variables
ENV DEBIAN_FRONTEND=noninteractive
ENV LANG=C.UTF-8

# Install dependencies
RUN apk add --no-cache \
    build-base \
    cmake \
    curl-dev \
    jsoncpp-dev \
    openssl-dev \
    boost-dev \
    gtest-dev \
    git

# RUN apk add --no-cache clang clang-libs libc++ libc++abi libc-dev
RUN apk add --no-cache clang g++ libstdc++

RUN apk add --no-cache libgcc

ENV CC=clang
ENV CXX=clang++

# Set the working directory inside the container
WORKDIR /app

# Copy the source code into the container
COPY . .

RUN mkdir -p /usr/local/lib/cmake/JsonCpp && \
    echo "include(FindPackageHandleStandardArgs)" > /usr/local/lib/cmake/JsonCpp/JsonCppConfig.cmake && \
    echo "find_package_handle_standard_args(JsonCpp REQUIRED_VARS JsonCpp_INCLUDE_DIR JsonCpp_LIBRARIES)" >> /usr/local/lib/cmake/JsonCpp/JsonCppConfig.cmake && \
    echo "set(JsonCpp_INCLUDE_DIR /usr/include/jsoncpp)" >> /usr/local/lib/cmake/JsonCpp/JsonCppConfig.cmake && \
    echo "set(JsonCpp_LIBRARIES jsoncpp)" >> /usr/local/lib/cmake/JsonCpp/JsonCppConfig.cmake

# Build the project
RUN mkdir -p build && \
    cd build && \
    cmake -DCMAKE_BUILD_TYPE=Release \
					-DCMAKE_C_COMPILER=clang \
					-DCMAKE_CXX_COMPILER=clang++ \
					-DJsonCpp_INCLUDE_DIR=/usr/include/jsoncpp \
          -DJsonCpp_LIBRARIES=/usr/lib/libjsoncpp.so \
					-DCMAKE_LINKER=/usr/bin/ld \
          .. && \
    make

# Set the container's default command to a shell
CMD ["/bin/sh"]

