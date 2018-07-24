chino-os
===
[English](README.md) [中文文档](README-zh.md)

[![travis-ci](https://travis-ci.org/chino-os/chino-os.svg?branch=master)](https://travis-ci.org/chino-os/chino-os) [![appveyor](https://ci.appveyor.com/api/projects/status/ff0xqvr439d0780v?svg=true)](https://ci.appveyor.com/project/sunnycase/chino-os)

![Screenshots](screenshots/2.png)

## 简介

`chino` 是一个用 C++ 实现的、专为物联网设计的实时操作系统。

## 特性

- 多任务式
- 动态链接
- 支持多种架构
- 支持在 Windows 下的开发与调试

## 支持的架构与开发板

架构          | 开发板           | 固件类型 |
------------- | ----------------|----------------
win-x86_64    | win32-simulator | exe           |
x86_64		  | pc			    | iso           |
cortex-m3	  | stm32f103rc     | hex           |

## 构建

### Win32 模拟器

1. 把 `CMakeSettings-template.json` 重命名为 `CMakeSettings.json`。
2. 用 Visual Studio 2017 （或者更高版本）打开项目根目录。
3. 生成 CMake Cache 然后构建。

### 其它

1. 下载 [chino-gnu-toolchain-preview3.tar.gz](https://github.com/chino-os/chino-gnu-toolchain/releases/download/preview3/chino-gnu-toolchain-preview3.tar.gz) 并解压到 `/opt/` 目录
```bash
wget https://github.com/chino-os/chino-gnu-toolchain/releases/download/preview3/chino-gnu-toolchain-preview3.tar.gz
sudo tar xvzf chino-gnu-toolchain-preview3.tar.gz /opt/
```
2. 安装依赖
```bash
git clone git://git.code.sf.net/p/gnu-efi/code gnu-efi-code
cd gnu-efi-code
make && sudo make install
sudo apt install xorriso cmake -y
```
3. 克隆源码编译
```bash
git clone https://github.com/chino-os/chino-os.git
mkdir build && cd build
../build.sh <arch> <board>
make firmware
```

## 运行

### exe 程序
1. 直接双击 `kernel.exe` （在 build_i/src/kernel/Debug 目录下）。

### iso 固件
1. 下载 [VirtualBox](https://www.virtualbox.org/wiki/Downloads) 并运行。
2. `文件` -> `导入应用`, 导入 `chino-os/vms/` 目录下的 `Chino.ova`。
3. `设置` -> `储存`, 选择 `空` 设备然后导入在 `build` 目录下的 `firmware.iso`。
4. 开始运行。

### hex 固件
1. 将 `kernel.hex` 烧录到开发板。
2. 重启开发板。

## [许可证 (MIT)](https://raw.githubusercontent.com/chino-os/chino-os/master/LICENSE)

	MIT License

	Copyright (c) 2018 chino-os

	Permission is hereby granted, free of charge, to any person obtaining a copy
	of this software and associated documentation files (the "Software"), to deal
	in the Software without restriction, including without limitation the rights
	to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
	copies of the Software, and to permit persons to whom the Software is
	furnished to do so, subject to the following conditions:

	The above copyright notice and this permission notice shall be included in all
	copies or substantial portions of the Software.

	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
	OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
	SOFTWARE.


