#!/usr/bin/env bash

# GBE Fork Quick Build Launcher for Linux
# Choose your build method

clear
echo
echo "  ==============================================="
echo "   GBE Fork Professional Build System"
echo "  ==============================================="
echo
echo "  Choose your build method:"
echo
echo "  [1] Interactive GUI Builder    (Recommended)"
echo "  [2] Quick Standard Build       (Fastest)"
echo "  [3] Dependencies Only          (Setup)"
echo "  [4] Classic Command Line       (Advanced)"
echo "  [5] Exit"
echo

read -p "Enter your choice (1-5): " choice

case $choice in
    1)
        echo
        echo "Starting Interactive GUI Builder..."
        ./build_linux_interactive.sh
        ;;
    2)
        echo
        echo "Starting Quick Standard Build..."
        ./build_linux_premake.sh
        ;;
    3)
        echo
        echo "Building Dependencies..."
        ./build_linux_premake.sh --deps
        ;;
    4)
        echo
        echo "Starting Classic Build..."
        ./build_linux_premake.sh
        ;;
    5)
        echo "Goodbye!"
        exit 0
        ;;
    *)
        echo
        echo "Invalid choice. Please try again."
        read -p "Press Enter to continue..."
        exec "$0"
        ;;
esac

echo
echo "Build process completed!"
read -p "Press Enter to exit..."