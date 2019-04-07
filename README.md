## Usage

To clone **and** deploy the project in one command, use `mbed import` and skip to
the target and toolchain definition:

    mbed import https://gitlab.com/catie_6tron/mbed-os-app-template.git YOUR_PROJECT_NAME

Alternatively:

* Clone to `YOUR_PROJECT_NAME` and enter it:

    ```sh
    git clone https://gitlab.com/catie_6tron/mbed-os-app-template.git YOUR_PROJECT_NAME
    cd YOUR_PROJECT_NAME
    ```

* Create an empty Mbed configuration file, otherwise Mbed CLI commands won't work:

    On Linux/macOS:

    ```sh
    touch .mbed on Linux/macOS
    ```

    Or on Windows:

    ```sh
    echo.> .mbed
    ```

* Deploy Mbed OS with:

    ```sh
    mbed deploy
    ```

* Define your target (eg. `ZEST_CORE_STM32L496RG`) and toolchain:

    ```sh
    mbed target ZEST_CORE_STM32L496RG
    mbed toolchain GCC_ARM
    ```

* Export to Eclipse IDE with:

    ```sh
    mbed export -i eclipse_6tron
    ```

## Compiling and programming without IDE

* Compile the project:

    ```sh
    mbed compile
    ```

* Program the target device (eg. `STM32L496RG` for the Zest_Core_STM32L496RG) with a
  J-Link debug probe:

    ```sh
    python dist/program.py STM32L496RG BUILD/ZEST_CORE_STM32L496RG/GCC_ARM/YOUR_PROJECT_NAME.elf
    ```

# Projet-IoT

Plante connectée (Damien ORLANDO/Verner BOISSON)
