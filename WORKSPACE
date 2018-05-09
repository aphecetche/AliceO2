local_repository(
    name = "com_github_nelhage_rules_boost",
    path = "/Users/laurent/github.com/nelhage/rules_boost",
)

load("@com_github_nelhage_rules_boost//:boost/boost.bzl", "boost_deps")

boost_deps()

MS_GSL_BUILD = """
cc_library(
    name = "ms_gsl",
    hdrs = glob(["include/gsl/*.h"]),
    includes = ["include"],
    visibility=["//visibility:public"],
)
""" 

new_http_archive(
    name = "ms_gsl",
    urls= ["https://github.com/Microsoft/GSL/archive/v1.0.0.tar.gz"],
    sha256 = "9694b04cd78e5b1a769868f19fdd9eea2002de3d4c3a81a1b769209364543c36",
    strip_prefix = "GSL-1.0.0",
    build_file_content = MS_GSL_BUILD,
)

FAIRLOGGER_BUILD = """
cc_library(
    name="libFairLogger",
    hdrs=["logger/Logger.h"],
    srcs=["logger/Logger.cxx"],
    copts=["-O2 -g -fPIC -std=c++11 -DNDEBUG -Iexternal/fairlogger/logger"],
   visibility=["//visibility:public"], 
)
"""
new_http_archive(
    name = "fairlogger",
    urls = ["https://github.com/FairRootGroup/FairLogger/archive/v1.1.0.tar.gz"],
    sha256 = "e185e5bd07df648224f85e765d18579fae0de54adaab9a194335e3ad6d3d29f7",
    strip_prefix = "FairLogger-1.1.0",
    build_file_content = FAIRLOGGER_BUILD,
)
