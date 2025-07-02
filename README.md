# 🔧 Custom DLL Injector (Windows)

A lightweight GUI-based DLL Injector built in C++ using the Windows API. This tool allows users to:

- Select a `.dll` file from disk.
- Choose a running process from the system.
- Inject the selected DLL into the chosen process.
- Designed for educational and ethical use only.

---

## 🚀 Features

- Simple Windows GUI — no external dependencies
- Dynamically lists running processes
- Manual DLL selection
- Safe and stable injection using `LoadLibrary`
- Error reporting via message boxes

---

## 🧠 Requirements

- Windows 10 or later (x86)
- Microsoft Visual Studio (2019 or later recommended)
- C++17 or later
- Optional: Target app (e.g. AssaultCube) running for injection

---

## 🔨 Building the Project

### Using Visual Studio:

1. Clone or download this repository.
2. Open the `.sln` file in Visual Studio.
3. Go to **Build → Configuration Manager**:
   - Set **Configuration** to `Release`
   - Set **Platform** to `x86` (for 32-bit processes like AssaultCube)
4. Build the solution (`Ctrl + Shift + B`)

> 💡 If Release gives build errors, see [Build Notes](#-build-notes).

---

## 📦 Distributing the Injector

1. After a successful **Release build**, locate the `.exe`:
