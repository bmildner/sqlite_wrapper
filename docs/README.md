## Required Tools

### All OS:
- cmake:
  - minimum version 3.20

- vcpkg:
  - install latest version via git clone 
  - environment variable VCPKG_ROOT must be set 

- ccache:
  - if installed and found by cmake ccache will be automatically used
  
- using non-std tool and compiler names
  - use cmake presets, see "linux-base-newest-toolset" for example

### Linux only:
- gcovr: (optional for code coverage) 
  - install latest version via pip/pipx install gcovr 

## Environment Variables
- VCPKG_ROOT
  - must always point to vcpkg installation, used in CMakePresets.json
- ASAN_OPTIONS
  - options for AddressSanitizer, set to "halt_on_error=true", "help=1" prints all supported options, separator for 
    multiple options is ":"  

## Hints
- If test_runner or test_runner_mocked crash directly at startup with `AddressSanitizer:DEADLYSIGNAL` ... 
  `Segmentation fault (core dumped)` or enter an endless loop printing `AddressSanitizer:DEADLYSIGNAL` you may have hit
   an incompatibility between asan and an high-entropy ASLR in you linux kernel!
   See [https://reviews.llvm.org/D148280](https://reviews.llvm.org/D148280) or 
   [https://github.com/tpm2-software/tpm2-tss/issues/2792](https://github.com/tpm2-software/tpm2-tss/issues/2792).
  - You can try to temporarily reduce entropy using `sudo sysctl vm.mmap_rnd_bits=28`.
  - To permanently change the entropy setting on Ubuntu add `vm.mmap_rnd_bits=28` in `/etc/sysctl.d/local.conf`.
- If test_runner or test_runner_mocked crash (SEGV) or return != 0 after all tests when run in gdb then this is due to
  an issues between asan/lsan and gdb. In some environments the text output of the sanitizer might not be visible!
  In CLion one has to uncheck "Use visual representation for Sanitizer's output"!
