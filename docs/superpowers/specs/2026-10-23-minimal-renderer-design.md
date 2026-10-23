# 最小软件渲染器设计

## 目标与已确认方案

在 WSL/Linux 上建立可逐步学习和扩展的现代 C++ CPU 渲染器。用户已在架构访谈及本轮 Q1–Q3 中确认以下规格；沿用已批准的模块边界。

- C++20，CMake 3.25+，使用已有 SDL 3.4.18 子模块。
- `core` 定义 framebuffer 和 renderer，不依赖 SDL。
- `sdl` 管理窗口、输入状态、CPU 图像上传与显示；公开头文件无 SDL 类型。
- `viewer` 编排交互程序，`offline` 复用核心并输出二进制 PPM (P6)。
- 64×64 图像，左上角原点，黑色背景；白点位于 (7,3)、(12,37)、(62,53)。
- 初始窗口 640×640，保持比例缩放，留黑边，最近邻采样。
- 颜色按内存 R/G/B/A 四字节排列，A 固定 255；另存 float 深度，本次不执行深度测试。
- 默认构建 viewer；offline 为显式构建 target。关闭 viewer 后默认构建 offline，不配置 SDL。
- 采集独立于 SDL 的按住、按下、释放、右键及鼠标增量状态，本次应用只响应关闭和 Esc。

## 方案选择

用户已选择应用持有 CPU 帧缓冲、SDL streaming texture 接收逐行复制的方案。
直接在锁定的 SDL 纹理内渲染会让核心生命周期依赖窗口，不满足离屏复用。
单独调用 SDL_UpdateTexture 也可显示，但此处按已确认方案使用 lock/copy/unlock。

## 接口与所有权

命名空间 `minirenderer`。Framebuffer 拥有颜色与深度 vector，禁止非正尺寸；颜色默认不透明黑，深度默认正无穷。
`Rgba8 { uint8_t r,g,b,a; }` 大小恰为 4，alpha 默认 255。
`ColorView { const Rgba8* pixels; int width,height; size_t stride_bytes; }` 为只读借用视图。
Framebuffer 提供尺寸、mutable/const spans、clear 和 color_view，视图不可长于宿主生命周期。
保持固定尺寸，不提供 resize；允许复制，禁止移动以避免移后尺寸与存储不一致。

Renderer 是无状态像素写入器：`bool draw_point(Framebuffer&, int x, int y, Rgba8) const noexcept`。
返回是否写入成功；越界坐标（包括负数）返回 false 且不改变图像，不做钳制或回绕。
示例白点属于应用，放在 apps/common 的共享函数，core 不包含示例坐标。

SdlPlatform 使用私有实现和 RAII 管理 SDL 初始化、窗口、renderer、纹理，禁止复制及移动。
公开 `poll_input()` 返回 InputState、`present(ColorView)`、`set_relative_mouse_mode(bool)`。
输入枚举仅 W/A/S/D/Escape/F1；ButtonState 含 down/pressed/released。
每帧清除边沿与位移；事件保留同帧按下和释放，忽略重复按下。
失焦清除按键和鼠标并关闭相对模式；每帧仅一次读取 SDL 相对增量，不重复累计 motion 事件。
当前 viewer 不开启相对模式；接口为下一阶段保留。

## 显示和错误处理

纹理格式 SDL_PIXELFORMAT_RGBA32，STREAMING，BLENDMODE_NONE，NEAREST。
锁定后按源 stride 和目标 pitch 逐行复制有效像素，再解锁、clear、render texture、present。
显示视图必须非空，尺寸与固定纹理一致，stride 至少 width*4；错误以异常报告。
SDL 调用在主线程执行；初始化失败自动回收已创建资源，main 捕获异常输出错误并非零退出。
viewer 每帧渲染同一示例并短暂让出 CPU，避免静态画面忙循环。

offline 接受零或一个输出路径参数，默认 framebuffer.ppm；多余参数报错。
输出 P6、64 64、255 头部和 12288 字节 RGB 数据，检查打开与写入错误。

## CMake 与目录

core/{include/minirenderer,src}、platform/sdl/{include/minirenderer/platform,src}、apps/{viewer,offline,common}。
库 core 和 sdl 为 STATIC；sdl PUBLIC 链接 core、PRIVATE 链接 SDL3::SDL3；应用 PRIVATE 链接所用库。
target 声明 cxx_std_20 和各自 include 路径；关闭编译器扩展。
MR_BUILD_VIEWER/MR_BUILD_OFFLINE 默认 ON；offline 在 viewer 开启时 EXCLUDE_FROM_ALL。
BUILD_TESTING 默认 OFF；开启测试时测试所需 offline 加入默认构建。
Presets 提供 debug、release、offline、test，使用可用的 Unix Makefiles；生成独立 build-* 目录。

## 实现者裁定

- 默认输出 PPM、测试可选且不引入第三方框架；Python 仅作为测试工具。
- 维度及大小运算溢出在分配前拒绝；越界点忽略并返回 false。
- 在当前工作目录实现，不切换分支或改动用户暂存区，不自动提交。

## 验收

1. 不构建 SDL 的 offline 配置能构建并运行；生成图像仅上述三个坐标为白，其余为不透明黑的 RGB 输出。
2. 默认 build 产生 viewer，不产生 offline；显式 offline target 可用。
3. viewer 可在 SDL dummy/software 下自动冒烟测试；输入用 SDL 真实事件队列验证。
4. core 测试覆盖清屏、边界、坐标方向、alpha/深度及尺寸验证；输出路径失败非零退出。
5. 实际 WSLg 窗口连接若不可用，明确区分编译/无头测试与人工窗口交互验证。
