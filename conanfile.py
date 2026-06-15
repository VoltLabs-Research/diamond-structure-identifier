from conan import ConanFile
from conan.tools.cmake import CMake, CMakeToolchain, CMakeDeps, cmake_layout


class IdentifyDiamondConan(ConanFile):
    name = "identify-diamond"
    version = "1.0.0"
    package_type = "static-library"
    license = "MIT"
    settings = "os", "arch", "compiler", "build_type"
    default_options = {"hwloc/*:shared": True}
    requires = (
        "boost/1.88.0",
        "onetbb/2021.12.0",
        "coretoolkit/[>=2.0]",
        "common-neighbor-analysis/[>=2.0]",
        "structure-identification/[>=2.0]",
        "spdlog/1.14.1",
        "nlohmann_json/3.11.3",
    )
    exports_sources = "CMakeLists.txt", "include/*", "src/*"

    def layout(self):
        cmake_layout(self)

    def generate(self):
        CMakeToolchain(self).generate()
        CMakeDeps(self).generate()

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

    def package(self):
        cmake = CMake(self)
        cmake.install()

    def package_info(self):
        self.cpp_info.set_property("cmake_target_name", "identify-diamond::identify-diamond")
        self.cpp_info.libs = ["identify-diamond_lib"]
        self.cpp_info.requires = [
            "boost::headers",
            "onetbb::onetbb",
            "coretoolkit::coretoolkit",
            "common-neighbor-analysis::common-neighbor-analysis",
            "structure-identification::structure-identification",
            "nlohmann_json::nlohmann_json",
            "spdlog::spdlog",
        ]
