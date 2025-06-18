# UGLHook-MinGW
An universal graphic library hook for D3D12 (D3D11, D3D10 and maybe Vulkan will be added later) based on 
[Universal-Dear-ImGui-Hook](https://github.com/Sh0ckFR/Universal-Dear-ImGui-Hook).

## Getting Started

- This project is based on [kiero](https://github.com/Rebzzel/kiero).
- To use it, you need to compile it into a static library `libuglhook.a` and link to your dll,
then inject your dll file into an application process.
- Switch into and compile the libraries in `\libraries`, and then use `mingw32-make` and `mingw32-make test` to compile the demo dll.

![alt text](imgui.png)

## Built With

* [Kiero](https://github.com/Rebzzel/kiero) - Kiero: Universal graphical hook for a D3D9-D3D12, OpenGL and Vulcan based games.
* [MinHook](https://github.com/TsudaKageyu/minhook) - The Minimalistic x86/x64 API Hooking Library for Windows.
* [ImGui](https://github.com/ocornut/imgui) - Dear ImGui: Bloat-free Immediate Mode Graphical User interface for C++ with minimal dependencies.

## Authors

* **Rebzzel** - *Initial work* - [Rebzzel](https://github.com/Rebzzel)
* **Sh0ckFR** | **Revan600** - *Updated version with ImGui + InputHook* - [Sh0ckFR](https://github.com/Sh0ckFR) - [Revan600](https://github.com/Revan600)
* **HTMonkeyG** - *Updated namespaces and APIs* - [HTMonkeyG](https://github.com/HTMonkeyG)

## Contributors

* **primemb** - *Fixed some bugs, active member* - [primemb](https://github.com/primemb)

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

The original licence of the Kiero library can be found here: [LICENSE](https://github.com/Rebzzel/kiero/blob/master/LICENSE)