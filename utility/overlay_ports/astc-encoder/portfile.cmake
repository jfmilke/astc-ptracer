include(vcpkg_common_functions)

vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO ARM-software/astc-encoder
    REF 5ae5f420cabb03f16447dfb184b095ff4f67adc9 # post v2.0, master-commit
    SHA512 ee792ace62fa10210a445822a1cc163a5e818cef39fea84c752c9bdd856394337f7a2b7089aa9d1ed72f11d8ca9ac6d23d71092a769a81a343619c21ea2083da
    HEAD_REF master
)

file(REMOVE_RECURSE "${SOURCE_PATH}/Source/VS2019")
file(COPY ${CMAKE_CURRENT_LIST_DIR}/CMakeLists.txt DESTINATION ${SOURCE_PATH})

vcpkg_configure_cmake(
    SOURCE_PATH ${SOURCE_PATH}
    PREFER_NINJA
)


vcpkg_install_cmake()

vcpkg_fixup_cmake_targets(CONFIG_PATH cmake)

file(REMOVE_RECURSE ${CURRENT_PACKAGES_DIR}/debug/include)
if(NOT (NOT VCPKG_CMAKE_SYSTEM_NAME OR VCPKG_CMAKE_SYSTEM_NAME STREQUAL "WindowsStore") AND NOT VCPKG_CMAKE_SYSTEM_NAME STREQUAL "Darwin")
  file(REMOVE_RECURSE ${CURRENT_PACKAGES_DIR}/share/astc-encoder)
endif()

# Handle copyright
file(INSTALL ${SOURCE_PATH}/LICENSE.txt DESTINATION ${CURRENT_PACKAGES_DIR}/share/astc-encoder RENAME copyright)