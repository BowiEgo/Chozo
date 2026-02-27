#!/bin/bash

# Ensure xmake is available
# Check if xmake command exists
xmake --version >/dev/null 2>&1
if [ $? -ne 0 ]; then
    echo "[Error] xmake command not found."
    echo "[Log] Please install xmake first or check your PATH."
    read -p "Press enter to exit..."
    exit 1
fi

show_menu() {
    clear
    echo "================================================================"
    echo "  Chozo Engine Project Generator (Linux/macOS)"
    echo "================================================================"
    echo "  1. Clean Project"
    echo "  2. Generate Project        (Makefile/Ninja)"
    echo "  3. Generate VSCode Project (compile_commands)"
    echo "  4. Generate CLion Project  (xmake-config)"
    echo "  0. Exit"
    echo "================================================================"
    echo -n "Select option (0-4): "
}

clean_project() {
    echo "[Log] Cleaning..."
    # -a: all, -v: verbose
    xmake clean -a -v
    # Remove build and potential project files
    rm -rf build
    rm -rf .xmake
    echo "[Log] Clean finished."
    read -p "Press enter to continue..."
}

generate_generic() {
    echo "[Log] Configuring project..."
    # -y: yes to all prompts
    xmake f -m debug -y
    echo "[Log] Building project..."
    xmake
    echo "[Log] Done."
    read -p "Press enter to continue..."
}

generate_vscode() {
    echo "[Log] Generating compile_commands.json..."
    xmake project -k compile_commands
    echo "[Log] Done. compile_commands.json generated."
    read -p "Press enter to continue..."
}

refresh_clion() {
    echo "[Log] Refreshing environment for CLion..."
    xmake f -y
    echo "[Log] Done."
    read -p "Press enter to continue..."
}

while true; do
    show_menu
    read M
    case $M in
        1) clean_project ;;
        2) generate_generic ;;
        3) generate_vscode ;;
        4) refresh_clion ;;
        0) exit 0 ;;
        *) echo "Invalid option, try again." ; sleep 1 ;;
    esac
done