##遮罩图片说明##

示例：
[GBA 3倍大小]
image_name="gba_2x.png"
viewport_rotate=0
viewport_width=720
viewport_height=480
viewport_x=120
viewport_y=20
[4:3铺满屏幕 (水原ありさ)]
image_name="full_4-3.png"
viewport_width=726
viewport_height=544
[SFC 4:3铺满屏幕]
image_name="sfc_full_4-3.png"
viewport_width=720
viewport_height=520
viewport_y=10

配置文件说明:
[]                 --（必有）遮罩条目名称（[要显示的名称]）
image_name         --（必有）遮罩图文件名称（遮罩图放于配置文件同目录下）
viewport_rotate    --（可无）画面旋转（顺时针：0 无旋转，1 旋转90度， 2 旋转180度， 3 旋转270度）（无时：随图形设置）
viewport_width     --（可无）画面宽度（无时：随图形设置）
viewport_height    --（可无）画面高度（无时：随图形设置）
viewport_x         --（可无）画面x轴（无时：居中）
viewport_y         --（可无）画面y轴（无时：居中）

遮罩图和配置文件路径：
优先路径： ux0:data/EMU4VITA/[程序文件夹名]/overlays/ （例gPsp：ux0:data/EMU4VITA/gPsp/overlays/）
第二路径： 程序被调用启动时设置的私有assets/overlays (详见程序源码的boot.c，非程序员用不到这个)
第三路径： 程序被调用启动时设置的通用assets/overlays (详见程序源码的boot.c，非程序员用不到这个，默认为app0:assets/overlays)
（配置文件与遮罩图会依优先路径逐个尝试读取，如果未读取到的话）