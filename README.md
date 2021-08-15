# 关于ESP32-SOLO

ESP32-SOLO主要特点是只有一个核心，这也使得使用上有点儿不一样

这一款的话和其他的相比较，它的最大特点就是便宜而且非常便宜只要5块钱，因为我是从那种类似于二手电子垃圾商那边买来的，我一共买了5块。

他们都是装在一个不知道是什么的板子上，我看了一下评论好像是说哪一家的无线遥控的RGB彩灯类似于这样子

# 开发板电路设计

我为这一个东西设计了电路，可以参考一下，https://oshwhub.com/sinzo/esp32-kuo-zhan-ban

目前来说具有以下特性

引出大量接口

具有SD卡接口

具有充电电路

具有Type-C

能任意下载程序

OLED

![](RackMultipart20210815-4-1hm5zsv_html_e6eab2f7186e3afd.png)

在实际测试时候，我发现BOOT和EN应该是可以去除，只是为了防止问题依然保留了下来，可以根据需要进行删减

充电电路选择的是TP4059

![](RackMultipart20210815-4-1hm5zsv_html_a99c871a5edda4c.png)

需要注意的是这里是自动复位/下载电路，它的电阻R5以及R6要接近10K，我一开始参数写成1K，焊接时候也焊了1K，结果就是直接失效了

![](RackMultipart20210815-4-1hm5zsv_html_fe1d7be50e452e88.png)

设计有一个RGB LED灯，WS2812B，属于机械键盘常用的一款，效果不错

![](RackMultipart20210815-4-1hm5zsv_html_9692f22f6f04b80d.png)

最终打出来的板子是黑色的PCB，我很喜欢。

![](RackMultipart20210815-4-1hm5zsv_html_d68a583150f64862.jpg)

这一个充电电路在充电时候会有50度左右，没问题，可以正常的运行。

![](RackMultipart20210815-4-1hm5zsv_html_728ced3997b672b4.jpg)

如果要省电，或许可以不安装PWR指示灯

![](RackMultipart20210815-4-1hm5zsv_html_e3c800aa548f58c9.jpg)

周身配置了m2的螺丝孔，需要的时候可以安装外壳或者亚克力面板

![](RackMultipart20210815-4-1hm5zsv_html_e24e5d51674b5567.jpg)

# ESP-32 SOLO快速上手

我用Arduino IDE来测试板子好坏以及下载，具体的编程和编译应该还是用PlatformIO（一个基于VSCode平台的插件）因为效率比较高。

在首选项的附加开发板管理中添加这一行

https://dl.espressif.com/dl/package\_esp32\_index.json

后重新启动，并在开发板管理器中搜索ESP32即可

![](RackMultipart20210815-4-1hm5zsv_html_387464302e61cd10.jpg)

之后在开发板那边点选ESP32 Dev Module

选了一个例子程序用来测试

![](RackMultipart20210815-4-1hm5zsv_html_95e270a3a209b267.jpg)

可以看到正常的进行下载，似乎毫无问题

![](RackMultipart20210815-4-1hm5zsv_html_f0fe104965b1afc5.jpg)

但是，在使用ESP-32 SOLO时候，因为这个单核特性有别于几乎所有成品板子，所以说程序是无法直接使用的。直接提示你错误，没法运行。

![](RackMultipart20210815-4-1hm5zsv_html_d80a2ee144f4eed8.jpg)

如何解决？最简单的方法就是使用这一个网友提供的这些文件

[https://github.com/lbernstone/arduino-esp32-solo](https://github.com/lbernstone/arduino-esp32-solo)去替换掉下载的sdk

![](RackMultipart20210815-4-1hm5zsv_html_816f940f6866e296.jpg)

方法是这样，先把上面这个下载下来后，替换arduino15中的packages目录下的esp32文件夹中的文件，具体的路径看图，版本要和实际的对应。比如我这里是1.05的版本。

![](RackMultipart20210815-4-1hm5zsv_html_938668cb695d6be5.jpg)

这个路径下的tools是不对的，我一开始替换这个文件夹发现依然编译的程序还是不能运行，感觉被骗了。后面发现是目录没搞对。

![](RackMultipart20210815-4-1hm5zsv_html_75da85b066b02d12.png)

错误路径

C:\Users\你的用户\AppData\Local\Arduino15\packages\esp32

正确路径

C:\Users\你的用户\AppData\Local\Arduino15\packages\esp32\hardware\esp32\1.0.5\tools

重新加载一个GetChipID例子，测试一下看来是没问题了

![](RackMultipart20210815-4-1hm5zsv_html_7df44fb89c3df1de.jpg)

现在可以自由的进行开发了。

# 如何使用PlatformIO开发ESP32

新建，板子的类型选择Espressif

![](RackMultipart20210815-4-1hm5zsv_html_3bbb63faa397c81e.jpg)

我写的程序是这样

![](RackMultipart20210815-4-1hm5zsv_html_c0385230a87378ff.jpg)

配置的platformio.ini是这样

![](RackMultipart20210815-4-1hm5zsv_html_4e19cba71b6b91f.jpg)

可以看到能正常的进行下载。如果说就插一个板子， 那么不需要配置COM口

![](RackMultipart20210815-4-1hm5zsv_html_afa4c982807de607.jpg)

但这不能运行，一样的问题，就是提示说你的程序是双核的。这就回到和之前ArduinoIDE一样的问题上面了。

先来找一下插件的所在文件夹，PlatformIO在安装好后，是在这样的几个文件内存有我们可能需要的文件：

C:\Users\用户名\.platformio\packages

C:\Users\用户名\.platformio\platforms

最终找到一个文件夹和我们之前折腾的ArduinoIDE的开发板资料文件夹最类似（一模一样），在这个位置

C:\Users\wzx13\.platformio\packages\framework-arduinoespressif32

我们需要下载这位老兄的文件，并且把这个文件夹覆盖掉原本的文件夹

![](RackMultipart20210815-4-1hm5zsv_html_6dddad47a161a866.jpg)

最终就可以使用了。

![](RackMultipart20210815-4-1hm5zsv_html_f3f64fcdcebacda9.png)

# 结束语

这一个ESP32-SOLO虽然是单核但是用起来确实挺爽的，价格便宜，然后它容量又大，并且看起来货还有非常多，总的来说如果觉得没什么问题的话，可以拿来用一用。

最后，祝玩的愉快。
