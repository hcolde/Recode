# 乱码转换器

（Windows x64）下载地址：https://github.com/hcolde/recoding/releases/download/1/recode.zip

解决中文乱码问题项目，就不用蹩脚的英文来介绍了。（目前仅支持64位操作系统）

## 背景

1.公司一个产品的后端PHP代码，不知道谁修改的时候加上了BOM头，导致部署在Linux系统时可能会报错500。然后一个同事就手动把所有PHP代码的BOM头给去掉。

2.最近在github上下载了一些代码来观摩，用某些编辑器打开后发现是一堆乱码。原来是因为编辑器不支持GB2312编码，虽然可以下载插件解决，但是爱好简洁却又懒惰的我，懒得下插件。于是乎乱码转换器孕育而生。

## 功能

支持从GBK（向下兼容GB2312）、UCS-2 Big Endian、UCS-2 Little Endian及UTF8-BOM编码转换为UTF8编码。

## 使用到的技术

1. 转换文件编码的功能使用Go来实现，原因是Go来操作byte很方便:)；
2. UI用Win32实现，因为编码问题一般出现在Windows上（贫穷的我没用过Mac）；
3. c++调用Go生成的动态链接库；
4. c++与Go之间用zmq通信，并使用rapidjson解析json。

## 如何使用

1. 确保recode.exe与recoding.dll在同一个目录下；
2. 端口12138未被占用；
3. 目前未作多开限制，请不要多开软件并操作转换；
4. 转换后的文件保存在桌面的output目录下。