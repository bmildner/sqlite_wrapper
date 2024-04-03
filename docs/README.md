## Required Tools

### All OS:
- cmake:
  - minimum version 3.20

- vcpkg:
  - install latest version via git clone 
  - environment variable VCPKG_ROOT must be set 

### Linux only:
- gcovr: (optional for code coverage) 
  - install latest version via pip/pipx install gcovr 

## Environment Variables

- VCPKG_ROOT
  - must always point to vcpkg installation, used in CMakePresets.json
- CC
  - to set C compiler, like gcc-12
- CXX
  - to set C++ compiler, like g++-12 
- CLANG_TIDY
  - to set your clang-tidy, like clang-tidy-15
- GCOV
  - like GCOV=gcov-12 if your compiler is not g++ or clang, like using g++-12 on Ubuntu 22.04 for example

## Hints

- If test_runner or test_runner_mocked crash directly at startup with `AddressSanitizer:DEADLYSIGNAL` ... 
  `Segmentation fault (core dumped)` or enters an endless loop printing `AddressSanitizer:DEADLYSIGNAL` you may have hit
   an incompatibility between asan and an high-entropy ASLR in you linux kernel!
   See [https://reviews.llvm.org/D148280](https://reviews.llvm.org/D148280) or 
   [https://github.com/tpm2-software/tpm2-tss/issues/2792](https://github.com/tpm2-software/tpm2-tss/issues/2792).
  - You can try to temporarily reduce entropy using `sudo sysctl vm.mmap_rnd_bits=28`.