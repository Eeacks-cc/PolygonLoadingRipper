# PolygonLoadingRipper

我使用了 [这张地图](https://vrchat.com/home/world/wrld_057b9b0f-a9c1-4f3c-b002-058a658e2217) 作为调试目标，逆向工程了其中的Polygon Loading

它可以从下载的模型文件中转储出对应的Mesh。

![截图](https://raw.githubusercontent.com/KeterTech/PolygonLoadingRipper/master/screenshots/blender_GWBcF6r26v.png)

贴图转储依旧在研究中...

# 使用方法
1. 运行 request_list.bat 文件下载模型列表(你必须在电脑上拥有curl)。
2. 下载完毕之后会在运行目录下出现一个 ModelList.json 里面可能包含大量预览图数据(未证实)，你需要提取其中的json。(建议使用vscode，对于大文件有优化)
3. 在清理后的 ModelList.json 中找到你喜欢的模型，其中元素"fn"是该模型的二进制文件在他们服务器上的链接。
4. 完整复制下来(例如:123456789.bin)，然后编辑 request_model.bat 将其中链接和-o参数的 x.bin 替换为复制的文件名。
5. 运行 request_model.bat 然后会在目录下下载得到对应的模型文件。
6. 运行 BinaryReader.exe 并且选择该文件。
7. 目录下应该会出现一个 x.bin.obj 文件，导入Blender，你现在有了它。

# 注意

我制作这个只是出于爱好，如果有任何问题请联系我。

mail@keter.tech
