# Parent project does not export its library target, so this CML implicitly
# depends on being added from it, i.e. the testing is done only from the build
# tree and is not feasible from an install location

project(httpfileserverTests LANGUAGES CXX)

# ---- Tests ----

add_executable(httpfileserver_test source/httpfileserver_test.cpp)
target_link_libraries(httpfileserver_test PRIVATE httpfileserver_lib)
target_compile_features(httpfileserver_test PRIVATE cxx_std_20)

add_test(NAME httpfileserver_test COMMAND httpfileserver_test)

# ---- End-of-file commands ----

add_folders(Test)
