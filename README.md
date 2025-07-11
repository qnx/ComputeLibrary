> [!IMPORTANT]
> **Temporary Pause in Releases Due to Team Holidays**
>
> Releases of the Arm Compute Library will be paused until mid-August due to team holidays.


> [!IMPORTANT]
> **Versioning Change: CalVer → SemVer**
>
> We have updated our versioning scheme for the Arm Compute Library.
>
> - **v25.04** is the **last release** using Calendar Versioning (**CalVer**).
> - Starting from the next release, we are switching to **Semantic Versioning (**SemVer**).
> - The next version will be: **v52.0.0**
>
> This change provides clearer version semantics and better compatibility tracking for users and integrators.

<br>
<div align="center">
 <img src="https://raw.githubusercontent.com/ARM-software/ComputeLibrary/gh-pages/ACL_logo.png"/><br><br>
</div>

# Compute Library ![](https://img.shields.io/badge/latest_release-52.3.0-green)


The Compute Library is a collection of low-level machine learning functions optimized for Arm® Cortex®-A, Arm® Neoverse™ and Arm® Mali™ GPUs architectures.<br>

The library provides superior performance to other open source alternatives and immediate support for new Arm® technologies e.g. SVE2.

Key Features:

- Open source software available under a permissive MIT license
- Over 100 machine learning functions for CPU and GPU
- Multiple convolution algorithms (GeMM, Winograd, FFT, Direct and indirect-GeMM)
- Support for multiple data types: FP32, FP16, INT8, UINT8, BFLOAT16
- Micro-architecture optimization for key ML primitives
- Highly configurable build options enabling lightweight binaries
- Advanced optimization techniques such as kernel fusion, Fast math enablement and texture utilization
- Device and workload specific tuning using OpenCL tuner and GeMM optimized heuristics

<br>

| Repository  | Link                                                             |
| ----------- | ---------------------------------------------------------------- |
| Release     | https://github.com/arm-software/ComputeLibrary                   |
| Development | https://review.mlplatform.org/#/admin/projects/ml/ComputeLibrary |

<br>

## Documentation
[![Documentation](https://img.shields.io/badge/documentation-52.3.0-green)](https://artificial-intelligence.sites.arm.com/computelibrary/v52.3.0/index.xhtml)

> Note: The documentation includes the reference API, changelogs, build guide, contribution guide, errata, etc.

<br>

## Pre-built binaries
All the binaries can be downloaded from [here](https://github.com/ARM-software/ComputeLibrary/releases) or from the tables below.

<br>

| Platform       | Operating System | Release archive (Download) |
| -------------- | ---------------- | -------------------------- |
| Raspberry Pi 4 | Linux® 32bit      | [![](https://img.shields.io/badge/build-neon-orange)](https://github.com/ARM-software/ComputeLibrary/releases/download/v52.3.0/arm_compute-v52.3.0-linux-armv7a-cpu-bin.tar.gz) |
| Raspberry Pi 4 | Linux® 64bit      | [![](https://img.shields.io/badge/build-neon-orange)](https://github.com/ARM-software/ComputeLibrary/releases/download/v52.3.0/arm_compute-v52.3.0-linux-aarch64-cpu-bin.tar.gz) |
| Odroid N2      | Linux® 64bit      | [![](https://img.shields.io/badge/build-neon-orange)](https://github.com/ARM-software/ComputeLibrary/releases/download/v52.3.0/arm_compute-v52.3.0-linux-aarch64-cpu-bin.tar.gz) [![](https://img.shields.io/badge/build-neon+cl-yellowgreen)](https://github.com/ARM-software/ComputeLibrary/releases/download/v52.3.0/arm_compute-v52.3.0-linux-aarch64-cpu-gpu-bin.tar.gz) |
| HiKey960       | Linux® 64bit      | [![](https://img.shields.io/badge/build-neon-orange)](https://github.com/ARM-software/ComputeLibrary/releases/download/v52.3.0/arm_compute-v52.3.0-linux-aarch64-cpu-bin.tar.gz) [![](https://img.shields.io/badge/build-neon+cl-yellowgreen)](https://github.com/ARM-software/ComputeLibrary/releases/download/v52.3.0/arm_compute-v52.3.0-linux-aarch64-cpu-gpu-bin.tar.gz) |

<br>

| Architecture | Operating System | Release archive (Download) |
| ------------ | ---------------- | -------------------------- |
| armv7        | Linux®            | [![](https://img.shields.io/badge/build-neon-orange)](https://github.com/ARM-software/ComputeLibrary/releases/download/v52.3.0/arm_compute-v52.3.0-linux-armv7a-cpu-bin.tar.gz) [![](https://img.shields.io/badge/build-neon+cl-yellowgreen)](https://github.com/ARM-software/ComputeLibrary/releases/download/v52.3.0/arm_compute-v52.3.0-linux-armv7a-cpu-gpu-bin.tar.gz) |
| arm64-v8a    | Android™          | [![](https://img.shields.io/badge/build-neon-orange)](https://github.com/ARM-software/ComputeLibrary/releases/download/v52.3.0/arm_compute-v52.3.0-android-aarch64-cpu-bin.tar.gz) [![](https://img.shields.io/badge/build-neon+cl-yellowgreen)](https://github.com/ARM-software/ComputeLibrary/releases/download/v52.3.0/arm_compute-v52.3.0-android-aarch64-cpu-gpu-bin.tar.gz) |
| arm64-v8a    | Linux®            | [![](https://img.shields.io/badge/build-neon-orange)](https://github.com/ARM-software/ComputeLibrary/releases/download/v52.3.0/arm_compute-v52.3.0-linux-aarch64-cpu-bin.tar.gz) [![](https://img.shields.io/badge/build-neon+cl-yellowgreen)](https://github.com/ARM-software/ComputeLibrary/releases/download/v52.3.0/arm_compute-v52.3.0-linux-aarch64-cpu-gpu-bin.tar.gz) |

<br>

Please refer to the following link for more pre-built binaries: [![](https://img.shields.io/badge/v52.3.0-bins-yellowgreen)](https://github.com/ARM-software/ComputeLibrary/releases/tag/v52.3.0)

Pre-build binaries are generated with the following security / good coding practices related flags:
> -Wall, -Wextra, -Wformat=2, -Winit-self, -Wstrict-overflow=2, -Wswitch-default, -Woverloaded-virtual, -Wformat-security, -Wctor-dtor-privacy, -Wsign-promo, -Weffc++, -pedantic, -fstack-protector-strong

## Supported Architectures/Technologies

- Arm® CPUs:
    - Arm® Cortex®-A processor family using Arm® Neon™ technology
    - Arm® Neoverse™ processor family
    - Arm® Cortex®-R processor family with Armv8-R AArch64 architecture using Arm® Neon™ technology
    - Arm® Cortex®-X1 processor using Arm® Neon™ technology

- Arm® Mali™ GPUs:
    - Arm® Mali™-G processor family
    - Arm® Mali™-T processor family

- x86

<br>

## Supported Systems

- Android™
- Bare Metal
- Linux®
- OpenBSD®
- macOS®
- Tizen™
- QNX® (Experimental)

<br>

## Resources
- [Tutorial: Running AlexNet on Raspberry Pi with Compute Library](https://community.arm.com/processors/b/blog/posts/running-alexnet-on-raspberry-pi-with-compute-library)
- [Gian Marco's talk on Performance Analysis for Optimizing Embedded Deep Learning Inference Software](https://www.embedded-vision.com/platinum-members/arm/embedded-vision-training/videos/pages/may-2019-embedded-vision-summit)
- [Gian Marco's talk on optimizing CNNs with Winograd algorithms at the EVS](https://www.embedded-vision.com/platinum-members/arm/embedded-vision-training/videos/pages/may-2018-embedded-vision-summit-iodice)
- [Gian Marco's talk on using SGEMM and FFTs to Accelerate Deep Learning](https://www.embedded-vision.com/platinum-members/arm/embedded-vision-training/videos/pages/may-2016-embedded-vision-summit-iodice)

<br>

## Experimental builds

**⚠ Important** Bazel and CMake builds are experimental CPU only builds, please see the [documentation](https://artificial-intelligence.sites.arm.com/computelibrary/v52.3.0/how_to_build.xhtml) for more details.

<br>

## How to contribute

Contributions to the Compute Library are more than welcome. If you are interested on contributing, please have a look at our [how to contribute guidelines](https://artificial-intelligence.sites.arm.com/computelibrary/v52.3.0/contribution_guidelines.xhtml).

### Developer Certificate of Origin (DCO)
Before the Compute Library accepts your contribution, you need to certify its origin and give us your permission. To manage this process we use the Developer Certificate of Origin (DCO) V1.1 (https://developercertificate.org/)

To indicate that you agree to the the terms of the DCO, you "sign off" your contribution by adding a line with your name and e-mail address to every git commit message:

```Signed-off-by: John Doe <john.doe@example.org>```

You must use your real name, no pseudonyms or anonymous contributions are accepted.

### Public mailing list
For technical discussion, the ComputeLibrary project has a public mailing list: acl-dev@lists.linaro.org
The list is open to anyone inside or outside of Arm to self subscribe.  In order to subscribe, please visit the following website:
https://lists.linaro.org/mailman3/lists/acl-dev.lists.linaro.org/

<br>

## License and Contributions

The software is provided under MIT license. Contributions to this project are accepted under the same license.

### Other Projects
This project contains code from other projects as listed below. The original license text is included in those source files.

* The OpenCL header library is licensed under Apache License, Version 2.0, which is a permissive license compatible with MIT license.

* The half library is licensed under MIT license.

* The libnpy library is licensed under MIT license.

* The stb image library is either licensed under MIT license or is in Public Domain. It is used by this project under the terms of MIT license.

* The KleidiAI library is licensed under Apache License, Version 2.0.

* The GoogleTest library is used by KleidiAI and is licensed under BSD-3-Clause license.

* The Benchmark library is used by KleidiAI and is licensed under Apache License, Version 2.0.

<br>

## Trademarks and Copyrights

Android is a trademark of Google LLC.

Arm, Cortex, Mali and Neon are registered trademarks or trademarks of Arm Limited (or its subsidiaries) in the US and/or elsewhere.

Bazel is a trademark of Google LLC., registered in the U.S. and other
countries.

CMake is a trademark of Kitware, Inc., registered in the U.S. and other
countries.

Linux® is the registered trademark of Linus Torvalds in the U.S. and other countries.

Mac and macOS are trademarks of Apple Inc., registered in the U.S. and other
countries.

Tizen is a registered trademark of The Linux Foundation.

Windows® is a trademark of the Microsoft group of companies.

QNX® is a trademark of QNX, a division of BlackBerry.
