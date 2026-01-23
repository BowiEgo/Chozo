#!/bin/bash
# ================================================================
#  Chozo Engine Environment Setup Script (Linux/macOS)
# ================================================================

set -e

echo "[Log] Checking for xmake..."

# Check if xmake command exists
if ! command -v xmake &> /dev/null
then
    echo "[Log] xmake not found. Starting installation..."

    # Run the official installation script via curl
    curl -fsSL https://xmake.io/shget.sh | bash

    # Source the profile to ensure xmake is available in the current session
    source ~/.xmake/profile 2>/dev/null || source ~/.bashrc 2>/dev/null || source ~/.zshrc 2>/dev/null

    echo "[Log] xmake installed successfully."
else
    echo "[Log] xmake is already installed."
fi

echo "[Log] Initializing project configuration..."

# Configure the project (e.g., set build mode to debug)
xmake f -m debug -y

echo "[Log] Setup complete! Run 'xmake' to build the project."