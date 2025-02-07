#!/bin/sh

set -eu
IFS=$'\n\t'

DEBUG=false
DRY_RUN=false

log() {
    echo "[INFO] $1"
}

debug_log() {
    if [ "$DEBUG" = true ]; then
        echo "[DEBUG] $1"
    fi
}

log "Starting Vulkan SDK installation script."

TEMP_DIR="./tmp"

if [ ! -d "$TEMP_DIR" ]; then
    log "Creating temp directory: $TEMP_DIR"
    mkdir -p "$TEMP_DIR"
fi

log "Downloading Vulkan SDK..."
curl -L "https://sdk.lunarg.com/sdk/download/latest/mac/vulkan-sdk.zip" -o "$TEMP_DIR/vulkan-sdk.zip"
log "Download complete: $TEMP_DIR/vulkan-sdk.zip"

log "Extracting Vulkan SDK..."
unzip -o "$TEMP_DIR/vulkan-sdk.zip" -d "$TEMP_DIR"
log "Extraction complete."

if [ "$DEBUG" = true ]; then
    debug_log "Checking for InstallVulkan directories in $TEMP_DIR:"
    find "$TEMP_DIR" -maxdepth 1 -type d -name "InstallVulkan-*.app"
fi

INSTALLER_APP=$(find "$TEMP_DIR" -maxdepth 1 -type d -name "InstallVulkan-*.app" | head -n 1)

if [ -z "$INSTALLER_APP" ]; then
    log "No versioned installer found, checking for InstallVulkan.app..."
    INSTALLER_APP="$TEMP_DIR/InstallVulkan.app"
    if [ ! -d "$INSTALLER_APP" ]; then
        echo "[ERROR] No Vulkan installer found in $TEMP_DIR."
        exit 1
    fi
    log "Using fallback installer: $INSTALLER_APP"
else
    log "Found Vulkan installer: $INSTALLER_APP"
fi

log "Found Vulkan installer: $INSTALLER_APP"

if echo "$INSTALLER_APP" | grep -q "InstallVulkan-"; then
    VERSION=$(echo "$INSTALLER_APP" | sed 's/.*InstallVulkan-//;s/.app//')
    INSTALLER_EXEC="$INSTALLER_APP/Contents/MacOS/InstallVulkan-$VERSION"
else
    VERSION="fallback"
    INSTALLER_EXEC="$INSTALLER_APP/Contents/MacOS/InstallVulkan"
fi

log "Using Vulkan version: $VERSION"

debug_log "Looking for installer executable at: $INSTALLER_EXEC"
if [ ! -x "$INSTALLER_EXEC" ]; then
    echo "[ERROR] Installer executable not found or not executable: $INSTALLER_EXEC"
    exit 1
fi

CMD="\"$INSTALLER_EXEC\" --accept-licenses --default-answer --confirm-command install"

if [ "$DRY_RUN" = true ]; then
    log "[DRY-RUN] Command that would be executed: $CMD"
else
    log "Executing Vulkan installer..."
    eval "$CMD"
    log "Installation complete."
fi

log "Cleaning up temporary files..."
rm -rf "$INSTALLER_APP"
rm -f "$TEMP_DIR/vulkan-sdk.zip"

log "Vulkan SDK version $VERSION installed successfully! You can now build Blazium by running 'scons'."
