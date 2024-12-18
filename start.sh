#!/bin/bash

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

echo -e "${YELLOW}Starting BistroSync setup...${NC}"

# Function to check if a command exists
command_exists() {
    command -v "$1" >/dev/null 2>&1
}

# Function to install packages based on the package manager
install_packages() {
    if command_exists apt-get; then
        # Debian/Ubuntu
        echo -e "${YELLOW}Installing dependencies using apt...${NC}"
        sudo apt-get update
        sudo apt-get install -y \
            build-essential \
            gcc \
            make \
            libgtk-3-dev \
            libpq-dev \
            postgresql \
            postgresql-contrib
    elif command_exists dnf; then
        # Fedora
        echo -e "${YELLOW}Installing dependencies using dnf...${NC}"
        sudo dnf install -y \
            gcc \
            make \
            gtk3-devel \
            postgresql-devel \
            postgresql-server \
            postgresql-contrib
    elif command_exists pacman; then
        # Arch Linux
        echo -e "${YELLOW}Installing dependencies using pacman...${NC}"
        sudo pacman -Sy --noconfirm \
            base-devel \
            gcc \
            make \
            gtk3 \
            postgresql \
            postgresql-libs
    else
        echo -e "${RED}No supported package manager found. Please install the required packages manually:${NC}"
        echo "- GCC"
        echo "- Make"
        echo "- GTK3 development files"
        echo "- PostgreSQL development files"
        echo "- PostgreSQL server"
        exit 1
    fi
}

# Create necessary directories
create_directories() {
    echo -e "${YELLOW}Creating necessary directories...${NC}"
    mkdir -p build logs
}

# Main installation process
main() {
    # Check if running as root
    if [ "$EUID" -eq 0 ]; then
        echo -e "${RED}Please do not run this script as root${NC}"
        exit 1
    }

    # Install required packages
    install_packages

    # Create directories
    create_directories

    # Build the project
    echo -e "${YELLOW}Building the project...${NC}"
    make clean
    make all

    if [ $? -eq 0 ]; then
        echo -e "${GREEN}Build completed successfully!${NC}"
        echo -e "${YELLOW}You can now run:${NC}"
        echo -e "  ${GREEN}./build/server${NC} - to start the server"
        echo -e "  ${GREEN}./build/client${NC} - to start the client"
    else
        echo -e "${RED}Build failed. Please check the error messages above.${NC}"
        exit 1
    fi
}

# Run the main function
main