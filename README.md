# 快速开始 (Quick Start)

## 编译项目
### Unix(Linux/MacOS)
1. 确保已安装必要的依赖：
   - 如果使用系统 OpenCV：需要安装完整 OpenCV 开发包
   - 如果使用本地精简版：确保 lib 目录下有 OpenCV 库文件
2. 创建构建目录并进入：
    ```bash
   mkdir build
   cd build
3. 配置编译：
    ```bash
   cmake ..
   make
4. 编译完成后，可执行文件位于 build/bin/Image_Match：
    ```bash
    ./bin/Image_Match
### Windows（使用MSVC完成测试）
1. 确保已安装必要的依赖：
   - 如果使用系统 OpenCV：需要安装完整 OpenCV 开发包
   - 如果使用本地精简版：确保 lib 目录下有 OpenCV 库文件，提交版本已提供Release版本的动态库，已配置在运行时自动将lib目录下的dll文件复制到可执行程序目录下，如未成功，请自行将lib目录下的dll文件复制到可执行程序目录下
2. 创建构建目录并进入：
    ```bash
   mkdir build
   cd build
3. 配置编译：
    ```bash
   cmake ..
   cmake --build . --config=Release --parallel
4. 编译完成后，可执行文件位于 build/Release/Image_Match.exe，运行：
    ```bash
    Release\Image_Match.exe
# 实验结果对比
* 以下结果运行环境为（Mac Air M1 8G）
* 相关匹配阈值设置0.8
* 最小二乘匹配优化后阈值设置0.9

## hammer 数据

| 项目 | 使用梯度加权 | 不使用梯度加权 |
|------|-------------|---------------|
| Harris角点检测耗时 | 1053ms | 1212ms |
| 匹配耗时 | 16695ms | 15263ms |
| 匹配结果 | 19280对特征点 | 19280对特征点 |
| 单点最小二乘匹配优化耗时 | 3320ms | 2882ms |
| 单点最小二乘匹配结果 | 18386对特征点 | 18745对特征点 |

## town 数据

| 项目 | 使用梯度加权 | 不使用梯度加权 |
|------|-------------|---------------|
| Harris角点检测耗时 | 28ms | 37ms |
| 匹配耗时 | 1148ms | 1168ms |
| 匹配结果 | 3926对特征点 | 3926对特征点 |
| 单点最小二乘匹配优化耗时 | 671ms | 595ms |
| 单点最小二乘匹配结果 | 2902对特征点 | 3034对特征点 |
