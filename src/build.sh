#!/bin/bash

# Configuration
SRC_DIR="dlls"
TARGET_SO="monster_mm_i386.so"
DEST_DIR="/home/diablo-dev/serverfiles/cstrike/addons/monster/dlls"

echo "-----------------------------------"
echo "Starting Build Process"
echo "-----------------------------------"

# Step 1: Clean previous build artifacts
echo "Running make clean in ${SRC_DIR}..."
make -C "${SRC_DIR}" clean || {
    echo "WARNING: make clean failed or failed silently. Continuing anyway."
    # Note: Not all Makefiles support 'make clean'. If your Makefile lacks this target, this step will fail.
    # You can comment this out if your Makefile doesn't have a 'clean' target.
}

# Step 2: Run make to build the library
echo "Running make in ${SRC_DIR}..."
make -C "${SRC_DIR}" -j4 || {
    echo "ERROR: Make failed. Build aborted."
    exit 1
}

# Step 3: Verify the .so file exists
if [ ! -f "${SRC_DIR}/${TARGET_SO}" ]; then
    echo "ERROR: Build completed, but ${TARGET_SO} was not found in ${SRC_DIR}."
    exit 1
fi

# Step 4: Copy the file to the destination
echo "Copying ${TARGET_SO} to ${DEST_DIR}..."

mkdir -p "${DEST_DIR}"
cp -p "${SRC_DIR}/${TARGET_SO}" "${DEST_DIR}/" || {
    echo "ERROR: Failed to copy file."
    exit 1
}

chmod 755 "${DEST_DIR}/${TARGET_SO}"

echo "-----------------------------------"
echo "Build and Copy Successful!"
echo "-----------------------------------"