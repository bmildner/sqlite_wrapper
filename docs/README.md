# Required Tools

All OS:
- cmake:
- - minimum version 3.20

- vcpkg:
- - install latest version via git clone 
- - environment variable VCPKG_ROOT must be set 

Linux only:
- gcovr: (optional for code coverage) 
- - install latest version via pip/pipx install gcovr 

# Environment Variables

- CC
- CXX
- CLANG_TIDY

# Hints

- If test_runner or test_runner_mocked crash directly at startup with `AddressSanitizer:DEADLYSIGNAL` ... 
  `Segmentation fault (core dumped)` or enters an endless loop printing `AddressSanitizer:DEADLYSIGNAL` you may have hit
   an incompatibility between asan and an high-entropy ASLR in you linux kernel!
- - You can try to temporarily reduce entropy using `sudo sysctl vm.mmap_rnd_bits=28`.