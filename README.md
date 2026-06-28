# APCpp-C

A C wrapper for [APCpp](https://github.com/N00byKing/APCpp).

By design, APCpp-C is intended to mirror the original C++ public API via thin wrappers, including function names, types, and general behavior. These are the following things you should keep in mind when using this library:

# Ownership

- Returned pointers should not be freed, as they are views into internal static storage used by the library.
- Data returned is only valid until the next call of the same function. Any important data should be copied beforehand if it must persist.

# Building
APCpp-C utilizes the CMake build system.

## Linux
 ```
 mkdir build && cd build`
 cmake ..
 cmake --build .
 ```
## Windows
- Create a folder `build`
- Enter the folder
- `cmake .. -DWIN32=1` (If on MinGW, also add `-DMINGW=1`)
- `cmake --build .`

## Wii U
For the Wii U version, this wrapper instead targets [my own port of APCpp for said platform.](https://github.com/techmuse8/APCpp-wiiu/tree/wiiu-port)
> [!IMPORTANT]  
> In order to build APCpp-C as an RPL (dynamic library) for Wii U, you'll have to build a custom fork of the wut homebrew toolchain from source, as RPLs are currently broken in upstream wut as of writing this. [See here for more information.](https://github.com/techmuse8/wut/tree/rpl-upstream)

You'll also need to have the latest version of [wiiu-mbedtls](https://github.com/dkosmari/wiiu-mbedtls-package/releases/latest) installed.
After all of the required dependencies are installed, run the following commands to build the library: 

```
mkdir build && cd build
/opt/devkitpro/portlibs/wiiu/bin/powerpc-eabi-cmake ..
make
```
The output RPL can be found in the `platform/wiiu` directory in the build folder.

