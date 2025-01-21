# Running Merger in a Docker Container

This guide explains how to build a lightweight Docker image for the Merger project, get a shell in the container, and execute commands.

---

## Prerequisites
- Docker installed on your system.
- The `Dockerfile` and project source code are in the same directory.

---

## Build the Docker Image
To build the Docker image, run the following command in the project directory:
```bash
docker build -t merger .
```

## Run the Docker Container
Start the Docker container with an interactive shell:
```bash
docker run -it --name merger-inst merger /bin/sh
```
You will now have a shell inside the container.

## Run the Merger Application
To execute the application inside the container, follow these steps:
- Compile the project (optional) or checkout build directory:
```bash
mkdir -p build && cd build
cmake ..
make
```
- Run the application:
```bash
./Merger --help
```
- Example commands:
    - Fetch order book with default settings:
    ```bash
    ./Merger
    ```
    - Enable live mode with a custom interval:
    ```bash
    ./Merger --live --interval 2000
    ```
    - Calculate costs for a specific BTC quantity:
    ```bash
    ./Merger --quantity 5.0
    ```
    - Stop and Remove the Container
    ```bash
    docker stop merger-inst 
    docker rm merger-inst 
    ```

## Notes
To rebuild the Docker image after making changes to the source code, rerun the docker build command.
You can modify the Dockerfile to include any additional dependencies or settings for your application.
