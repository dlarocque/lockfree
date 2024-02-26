cc_test(
    name = "lockfree_test",
    srcs = ["spsc_tests.cc"],
    deps = [
        "//lib:lockfree",
        "@googletest//:gtest_main", # gtest_main does not inlcude mocking
    ],
)
