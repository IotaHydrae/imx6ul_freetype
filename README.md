# imx6ul_freetype

#### 介绍
freetype的一些基本操作，支持多行显示，自定义显示位置和行距,字体文件因为大于10MB无法上传，可以找寻Windows文件夹下Fonts目录的字体文件。

#### 安装教程
```shell
arm-linux-gnueabihf-gcc -o show_line show_line.c -lfreetyoe
```
#### 使用说明
```shell
Usage: ./show_line <font-file> <lcd_x> <lcd_y> [font-size] [row-spacing]
```
#### 参与贡献
@[hz2](https://gitee.com/JensenHua/)

1.  Fork 本仓库
2.  新建 Feat_xxx 分支
3.  提交代码
4.  新建 Pull Request


#### 特技

1.  使用 Readme\_XXX.md 来支持不同的语言，例如 Readme\_en.md, Readme\_zh.md
2.  Gitee 官方博客 [blog.gitee.com](https://blog.gitee.com)
3.  你可以 [https://gitee.com/explore](https://gitee.com/explore) 这个地址来了解 Gitee 上的优秀开源项目
4.  [GVP](https://gitee.com/gvp) 全称是 Gitee 最有价值开源项目，是综合评定出的优秀开源项目
5.  Gitee 官方提供的使用手册 [https://gitee.com/help](https://gitee.com/help)
6.  Gitee 封面人物是一档用来展示 Gitee 会员风采的栏目 [https://gitee.com/gitee-stars/](https://gitee.com/gitee-stars/)
