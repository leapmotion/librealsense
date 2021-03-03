# leapmotion/librealuvc

A fork of librealuvc to support the Rigel through UVC.

## Conan + CMake

These instructions are still a work-in-progress.

```sh
conan install . --install-folder build -r conan-center
conan build . --build-folder build
```

The `-r conan-center` part is necessary if you've modified your `conan remote` setup for building e.g. libtrack by changing your default remote to gitlab. If you don't have conan-center defined you may need to add it:
```sh
conan remote add conan-center https://conan.bintray.com
```

If `conan build` fails, you'll likely see CMake error output. You can re-attempt configuration/generation using your CMake interface of choice (e.g. cmake-gui) and debug from there.

# Original readme.md below

## Overview
This library provides a portable backend to UVC-compliant cameras and other
USB devices (e.g. motion sensors), with support for UVC Extension Units.

It is a modified version of the backend code from the IntelRealSense/librealsense
library.  In accordance with the Apache License v2.0, the complete source
code, including modifications by Leap Motion Inc, is licensed under the
Apache License, Version 2.0.

## License
This project is licensed under the [Apache License, Version 2.0](LICENSE).
Copyright 2018 Intel Corporation
