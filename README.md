Usage: python jiagu.py xxx.apk

效果：实现dex文件整体加密、隐藏

(1)想体验具体效果可以访问我开发的工具：http://01hackcode.com

(2)加固原理，可以访问我写得一系列博客文章：

> * APP的安装过程   http://shuwoom.com/?p=60
> * DALVIK虚拟机启动过程  http://shuwoom.com/?p=112
> * APP的启动过程  http://shuwoom.com/?p=142
> * DEX文件结构分析   http://shuwoom.com/?p=179
> * DALVIK加载和解析DEX过程   http://shuwoom.com/?p=269
> * DALVIK查找类和方法的过程  http://shuwoom.com/?p=282
> * ELF文件结构分析   http://shuwoom.com/?p=286
> * SO加载解析过程   http://shuwoom.com/?p=351
> * 实战开发APK安全加固   http://shuwoom.com/?p=360
> * Android代码混淆技术总结 http://shuwoom.com/?p=506

安装环境:

(1)NDK：14.1

(2)gradle-3.3

(3)build-tools:android-25

(4)jdk1.8



sign文件夹：用于重打包签名

smali文件夹：当目标app没有自定义Application时，给该app添加一个自定义Application

TuokeApk文件夹：用于加密加固了的classes.dex文件

JiaguApk.jar：用于合并TuokeApk/bin/classes.dex和加密了的目标目标TargetApk.zip(只包含：classes.dex)


TODO:
###0.elf头破坏
1.自定义DexClassLoad实现无dex缓存

2.反编译模块：针对apktool、dex2jar、baksmali、idapro、01editor、shakaapktool、Androguard
参考：
http://www.freebuf.com/sectool/76884.html
https://github.com/wanchouchou/ManifestAmbiguity

3.修改dex文件，DexEducation-PracticingSafeDex.pdf

4.添加反调试、反模拟器模块

5.签名校验、反调试：http://www.jianshu.com/p/f17c60298e75

6.内存保护

7.字节码自修改

8.so、dex方法提取

9.内存加载so

10.独立加解密模块，总结整理几个常用的高效加密算法：rc4、凯撒加密+Base64、TEA加密、AES、RSA、白盒加密（常见算法c库）
参考：
http://blog.csdn.net/zhiqiangzhan/article/details/4658106
http://blog.csdn.net/doorxp/article/details/8763018
http://www.aichengxu.com/suanfa/6186233.htm
https://www.zhihu.com/question/35136485

11.添加密钥保存模块

http://www.hackdig.com/04/hack-20771.htm

12.合并函数加密功能

13.加密常量字符串

14.添加方法、字段代码提取

15.合并函数隐藏功能

16.自定义linker

17.dex代码混淆，修改https://github.com/strazzere/APKfuscator为python版本

18.so文件混淆，https://github.com/Fuzion24/AndroidObfuscation-NDK
