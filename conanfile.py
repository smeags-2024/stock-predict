from conan import ConanFile
from conan.tools.cmake import CMakeDeps, CMakeToolchain, cmake_layout


class StockPredictConan(ConanFile):
    name = "stock-predict"
    version = "1.0.0"
    
    # Package metadata
    description = "Advanced C++ Stock Price Prediction System"
    topics = ("stock-prediction", "machine-learning", "finance", "cpp20")
    url = "https://github.com/smeags-2024/stock-predict"
    license = "MIT"
    
    # Configuration
    settings = "os", "compiler", "build_type", "arch"
    
    # Requirements
    def requirements(self):
        # Core dependencies (commonly available in Conan Center)
        self.requires("eigen/3.4.0")
        self.requires("nlohmann_json/3.11.2") 
        self.requires("spdlog/1.12.0")
        self.requires("fmt/10.1.1")
        self.requires("cli11/2.3.2")
        
        # Testing and benchmarking
        self.requires("gtest/1.14.0")
        self.requires("benchmark/1.8.3")
    
    def build_requirements(self):
        self.tool_requires("cmake/[>=3.20]")
    
    def layout(self):
        cmake_layout(self)
    
    def generate(self):
        # Generate CMake files
        deps = CMakeDeps(self)
        deps.generate()
        
        tc = CMakeToolchain(self)
        tc.variables["CMAKE_CXX_STANDARD"] = "20"
        tc.variables["CMAKE_CXX_STANDARD_REQUIRED"] = "ON"
        tc.generate()
    
    def configure(self):
        # Ensure C++20 is used
        if self.settings.compiler.get_safe("cppstd"):
            self.settings.compiler.cppstd = "20"
