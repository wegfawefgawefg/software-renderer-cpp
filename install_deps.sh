#!/bin/bash

# Function to print a message and install a package
install_package() {
    echo "----------------------------------------"
    echo "Installing $1..."
    sudo apt install -y "$1"
    if [ $? -eq 0 ]; then
        echo "$1 installed successfully!"
    else
        echo "Failed to install $1!"
        exit 1
    fi
    echo "----------------------------------------"
}

# Update package lists
echo "Updating package lists..."
sudo apt update
if [ $? -eq 0 ]; then
    echo "Package lists updated successfully!"
else
    echo "Failed to update package lists!"
    exit 1
fi
echo "----------------------------------------"

# Install build-essential (gcc, make, etc.)
install_package build-essential

# Install SDL2 development library
install_package libsdl2-dev

# Install SDL2_ttf development library
install_package libsdl2-ttf-dev

# Optional: Install SDL2_image, SDL2_mixer, and SDL2_net if needed
install_package libsdl2-image-dev
install_package libsdl2-mixer-dev
install_package libsdl2-net-dev

# do libgif
install_package libgif-dev

echo "All required packages installed successfully!"
