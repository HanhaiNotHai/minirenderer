# minirenderer

使用 C++20、CMake 和 SDL3 的 CPU 软件渲染器。当前版本在 64×64 黑色图像上绘制
`(7, 3)`、`(12, 37)`、`(62, 53)` 三个白点，坐标原点位于左上角。

## 构建并运行 viewer（默认）

需要 CMake 3.25+、支持 C++20 的编译器，以及 SDL 所需的 Linux 图形开发依赖。
WSL 中通过 WSLg 显示窗口。SDL 使用仓库中固定版本的子模块：

```bash
git submodule update --init --recursive
cmake --preset debug
cmake --build --preset debug --parallel
./build-debug/apps/viewer/viewer
```

窗口初始尺寸为 640×640，以最近邻方式放大图像。调整窗口大小时保持比例并留黑边。
按 Esc 或关闭窗口退出。当前采集键鼠状态，相机控制将在后续阶段加入。

也可以直接执行 `cmake -S . -B build`、`cmake --build build --parallel`，然后运行
`./build/apps/viewer/viewer`。Release 配置对应 `release` preset 和 `build-release/`。

## 离线输出

默认构建不生成 offline；可以显式构建同一配置中的 target：

```bash
cmake --build build-debug --target offline --parallel
./build-debug/apps/offline/offline framebuffer.ppm
```

无需窗口和 SDL 的独立构建：

```bash
cmake --preset offline
cmake --build --preset offline --parallel
./build-offline/apps/offline/offline framebuffer.ppm
```

输出路径可省略，默认为当前工作目录下的 `framebuffer.ppm`。文件为二进制 PPM（P6），
与窗口使用同一份渲染逻辑。`MR_BUILD_VIEWER=OFF` 时不会配置或编译 SDL。

## 模块与接口

| target | 职责 | 依赖 |
| --- | --- | --- |
| `core` | 帧缓冲、像素绘制；后续扩展光栅化算法 | C++ 标准库 |
| `sdl` | RAII 窗口资源、输入采集、颜色缓冲上传与显示 | `core`、SDL3 |
| `viewer` | 交互主循环 | `core`、`sdl` |
| `offline` | 渲染并写入 PPM | `core` |

`core/` 和 `platform/sdl/` 各自维护公开头文件、实现和 CMake 配置。
应用位于 `apps/viewer/`、`apps/offline/`，共享的三点示例位于 `apps/common/demo.hpp`。

应用持有 `Framebuffer`，其中分别保存 RGBA32 颜色与 float 深度；A 初期固定为 255。
`Renderer::draw_point` 借用帧缓冲，越界时返回 false。当前不执行深度测试。
显示层借用只读 `ColorView`，按源 stride 和 SDL 返回的 pitch 逐行复制到 streaming texture。
视图不拥有像素，使用期间宿主必须存活，且不能通过赋值替换其存储。

`SdlPlatform::poll_input()` 返回不含 SDL 类型的 `InputState`：W/A/S/D/Escape/F1、右键、
相对鼠标位移、焦点和退出请求。按键区分 down/pressed/released，失焦清理状态。
SDL 资源创建、输入轮询和显示均在主线程执行。初始化、显示或文件输出失败时，程序打印错误并非零退出。

## 测试

测试默认关闭；启用后需要 Python 3，仅使用其标准库：

```bash
cmake --preset test
cmake --build --preset test --parallel
ctest --preset test
```

覆盖帧缓冲与绘制边界、PPM 的全部像素、文件输出错误、输入状态和不同 stride 的行复制。
SDL 集成测试使用 dummy 视频驱动和 software renderer，无需显示服务器。
测试配置会同时构建 offline，供端到端输出测试使用。

仅运行不依赖 SDL 的测试：

```bash
cmake -S . -B build-core-tests -DMR_BUILD_VIEWER=OFF -DBUILD_TESTING=ON
cmake --build build-core-tests --parallel
ctest --test-dir build-core-tests --output-on-failure
```
