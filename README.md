FlashCS6ForCocos2d-x
===================

该项目的目标是使用FlashCS工具来设计COCOS2D-X动画。

设计思路可参照我得博客文章：http://blog.csdn.net/ctbinzi/article/details/19649213

有问题可以联系我，QQ 185514913

--------------------------------
项目结构说明

/FlashTransform 是一个基于FlashBuilder的FlashAir项目，主要功能是从FlashCS6存档文件读取我们感兴趣的数据，并转存为自定义的xml文档 和 一个用于宏定义动画ID的.h文件。自定义的xml文档格式可参见 Document/Template.xml 文件。用于宏定义动画ID的.h文件可参见 Cocos2d-x/Classes/FlashEffectDefine.h。另外 FlashTransform 程序启动项可接受两个字符串参数，分别为 FlashCS6 工程的xfl格式存档（可参见Art/Flash/Actions/）路径 和 生成后的自定义xml文件存储路径。

/Cocos2d-x 是一个基于cocos2d-x3.0的C++项目，使用VS2012及其以上版本可以打开。该工程主要演示在cocos2d-x工程里面如何读取FlashTransform生产的xml文件，并生成Cocos动画。编译运行该工程，鼠标点击游戏屏幕，可以在点击处看到Flash里面设计好的动画效果。

/Document/卡牌游戏动作编辑器功能说明.docx 该文档描述了Flash关键帧动画与Cocos2d-x的Action动画之间的转换关系。

/Art/Flash/Actions 是一个FlashCS6项目的xlf格式存档
