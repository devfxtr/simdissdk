# IMPORTANT: if you are getting linker errors, make sure that
# "SIMDIS_SDK_LIB_EXPORT_SHARED" is not in your test's Preprocessor Definitions

if(NOT ENABLE_UNIT_TESTING)
    return()
endif()

project(SimCore_UnitTests)

create_test_sourcelist(SimCoreTestFiles SimCoreTests.cpp
    TokenizerTest.cpp
    StringUtilsTest.cpp
    CoordConvertLibTest.cpp
    VersionTest.cpp
    CoreCommonTest.cpp
    CalculationTest.cpp
    MathTest.cpp
    MgrsTest.cpp
    SquareMatrixTest.cpp
    TimeClassTest.cpp
    TimeManagerTest.cpp
    TimeStringTest.cpp
    TimeUtilsTest.cpp
    TimeJulianTest.cpp
    InterpolationTest.cpp
    EMTest.cpp
    ValidNumberTest.cpp
    GeoFenceTest.cpp
    MultiFrameCoordTest.cpp
    AngleTest.cpp
    UnitsTest.cpp
    UnitsFormatter.cpp
    GogToGeoFenceTest.cpp
    CalculateLibTest.cpp
)

add_executable(SimCoreTests ${SimCoreTestFiles})
target_link_libraries(SimCoreTests PRIVATE simCore)
# If simUtil is built, then add it as a dependency for UnitsFormatter test and add a -D define
if(TARGET simUtil)
    target_link_libraries(SimCoreTests PRIVATE simUtil)
    target_compile_definitions(SimCoreTests PRIVATE HAVE_SIMUTIL)
endif()
set_target_properties(SimCoreTests PROPERTIES
    FOLDER "Unit Tests"
    PROJECT_LABEL "Unit Tests - SimCore"
)

add_test(NAME VersionTest COMMAND SimCoreTests VersionTest)
add_test(NAME TimeManager COMMAND SimCoreTests TimeManagerTest)
add_test(NAME TokenizerTest COMMAND SimCoreTests TokenizerTest)
add_test(NAME StringUtilsTest COMMAND SimCoreTests StringUtilsTest)
add_test(NAME CoordConvertLibTest COMMAND SimCoreTests CoordConvertLibTest)
add_test(NAME CoreCommonTest COMMAND SimCoreTests CoreCommonTest)
add_test(NAME CalculationTest COMMAND SimCoreTests CalculationTest)
add_test(NAME CoreMathTest COMMAND SimCoreTests MathTest)
add_test(NAME CoreSquareMatrixTest COMMAND SimCoreTests SquareMatrixTest)
add_test(NAME CoreMgrsTest COMMAND SimCoreTests MgrsTest)
add_test(NAME CoreTimeClassTest COMMAND SimCoreTests TimeClassTest)
add_test(NAME CoreInterpolationTest COMMAND SimCoreTests InterpolationTest)
add_test(NAME CoreValidNumberTest COMMAND SimCoreTests ValidNumberTest)
add_test(NAME CoreTimeStringTest COMMAND SimCoreTests TimeStringTest)
add_test(NAME CoreTimeUtilsTest COMMAND SimCoreTests TimeUtilsTest)
add_test(NAME CoreTimeJulianTest COMMAND SimCoreTests TimeJulianTest)
add_test(NAME CoreGeoFenceTest COMMAND SimCoreTests GeoFenceTest)
add_test(NAME MultiFrameCoordTest COMMAND SimCoreTests MultiFrameCoordTest)
add_test(NAME AngleTest COMMAND SimCoreTests AngleTest)
add_test(NAME CoreUnitsTest COMMAND SimCoreTests UnitsTest)
add_test(NAME CoreUnitsFormatter COMMAND SimCoreTests UnitsFormatter)
add_test(NAME GogToGeoFenceTest COMMAND SimCoreTests GogToGeoFenceTest)
add_test(NAME CalculateLibTest COMMAND SimCoreTests CalculateLibTest ${SimCore_UnitTests_SOURCE_DIR}/CalculateInput.txt)

# Try to locate the correct file for the RCS test...
set(FILE_LOCATIONS
    $ENV{SIMDIS_SDK_FILE_PATH}/data/rcs/
    ${INSTALLSOURCE_DATA_DIR}/TestData/UpdateData/
)
find_file(RCSFILE fake_rcs_3.rcs PATHS ${FILE_LOCATIONS})
mark_as_advanced(RCSFILE)
if(EXISTS ${RCSFILE})
    add_test(NAME CoreEMTest COMMAND SimCoreTests EMTest ${RCSFILE})
endif(EXISTS ${RCSFILE})
