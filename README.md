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
    **Sample output:**
    ```plaintext
    2025-01-22 09:00:26 [DEBUG] Parsing Coinbase Pro order book
    2025-01-22 09:00:26 [INFO] Successfully parsed Coinbase Pro order book - Bids: 4194, Asks: 1257
    2025-01-22 09:00:29 [DEBUG] Parsing Gemini order book
    2025-01-22 09:00:29 [INFO] Successfully parsed Gemini order book - Bids: 1000, Asks: 1000
    2025-01-22 09:00:31 [DEBUG] Parsing Kraken order book
    2025-01-22 09:00:31 [INFO] Successfully parsed Kraken order book - Bids: 100, Asks: 100
    2025-01-22 09:00:31 [INFO] Starting order book merge
    2025-01-22 09:00:31 [INFO] Merging order book from Coinbase Pro
    2025-01-22 09:00:31 [DEBUG] Merging bids from Coinbase Pro: 4194 entries (Price range: 0.01 - 105164)
    2025-01-22 09:00:31 [DEBUG] Merging asks from Coinbase Pro: 1257 entries (Price range: 105177 - 1e+07)
    2025-01-22 09:00:31 [INFO] Merging order book from Gemini
    2025-01-22 09:00:31 [DEBUG] Merging bids from Gemini: 1000 entries (Price range: 95331.9 - 105158)
    2025-01-22 09:00:31 [DEBUG] Merging asks from Gemini: 1000 entries (Price range: 105159 - 120054)
    2025-01-22 09:00:31 [INFO] Merging order book from Kraken
    2025-01-22 09:00:31 [DEBUG] Merging bids from Kraken: 100 entries (Price range: 104957 - 105176)
    2025-01-22 09:00:31 [DEBUG] Merging asks from Kraken: 100 entries (Price range: 105176 - 105353)
    2025-01-22 09:00:31 [INFO] Order book merge completed. Total entries - Bids: 5294, Asks: 2357
    Buy cost (10.00 BTC): $1051754.03
    Sell proceeds (10.00 BTC): $1051504.21
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

## TODO
- Enable live data using websocket.
- Enable other coins/tokens.
