cd /private/var/tmp/_bazel_laurent/76083efdcdc301e239deb4ec47cd88bd/execroot/__main__  
APPLE_SDK_PLATFORM='' APPLE_SDK_VERSION_OVERRIDE='' DEVELOPER_DIR=/Applications/Xcode.app/Contents/Developer \
PATH='/Users/laurent/.pyenv/shims:/usr/local/bin:/usr/local/sbin:/usr/bin:/bin:/usr/sbin:/sbin:/Applications/VMware Fusion.app/Contents/Public:/Library/TeX/texbin:/opt/X11/bin:/Library/Globus/bin:/Library/Globus/sbin:/Users/laurent/.pyenv/shims:/usr/local/Cellar/modules/4.1.1/bin:/Users/laurent/.fzf/bin:/Users/laurent/dotfiles/bin:/Users/laurent/Library/Python/2.7/bin:/Users/laurent/dotfiles/bin:/Users/laurent/Library/Python/2.7/bin' \
  SDKROOT=/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.13.sdk \
  TMPDIR=/var/folders/dq/s6x8gmp11rxc9rt8hq_7c3qc0000gn/T/ \
    XCODE_VERSION_OVERRIDE=9.3.0 \
  /usr/bin/sandbox-exec -f /private/var/tmp/_bazel_laurent/76083efdcdc301e239deb4ec47cd88bd/bazel-sandbox/2873096300521650529/sandbox.sb \
  /private/var/tmp/_bazel_laurent/76083efdcdc301e239deb4ec47cd88bd/execroot/__main__/_bin/process-wrapper \
  '--timeout=0' '--kill_delay=15' \
  external/local_config_cc/wrapped_clang '-D_FORTIFY_SOURCE=1' -fstack-protector -fcolor-diagnostics -Wall -Wthread-safety \
  -Wself-assign -fno-omit-frame-pointer -O0 -DDEBUG '-std=c++14' -iquote . -iquote bazel-out/darwin-fastbuild/genfiles -iquote \
  external/ms_gsl -iquote bazel-out/darwin-fastbuild/genfiles/external/ms_gsl -iquote external/bazel_tools \
  -iquote bazel-out/darwin-fastbuild/genfiles/external/bazel_tools -isystem external/ms_gsl/include \
  -isystem bazel-out/darwin-fastbuild/genfiles/external/ms_gsl/include -MD -MF \
  bazel-out/darwin-fastbuild/bin/Detectors/MUON/MCH/Mapping/Toto/_objs/toto/Detectors/MUON/MCH/Mapping/Toto/toto.d \
  '-frandom-seed=bazel-out/darwin-fastbuild/bin/Detectors/MUON/MCH/Mapping/Toto/_objs/toto/Detectors/MUON/MCH/Mapping/Toto/toto.o' \
  '-isysroot __BAZEL_XCODE_SDKROOT__' -no-canonical-prefixes -Wno-builtin-macro-redefined '-D__DATE__="redacted"' \
  '-D__TIMESTAMP__="redacted"' '-D__TIME__="redacted"' -c Detectors/MUON/MCH/Mapping/Toto/toto.cxx -o \
  bazel-out/darwin-fastbuild/bin/Detectors/MUON/MCH/Mapping/Toto/_objs/toto/Detectors/MUON/MCH/Mapping/Toto/toto.o