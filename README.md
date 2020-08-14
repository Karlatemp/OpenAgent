# OpenAgent

## 什么是OpenAgent

OpenAgent, 是一款能够让你挂载 `-javaagent` 的程序,
他拥有和 `-javaagent` 一样的效果。

## 编写背景

此项目的编写背景为
1. 各种面板服务商可能不支持/不允许添加特定参数, 对于需要使用
   javaagent 的程序来说极其不友好, 此时, 这个项目并随之产生
2. 为了练习写 C++ 与 JNI

## 如何使用

首先前往Releases页面下载最后一个版本

如果你的服务商支持自定义启动核心名字

> 我们直接把核心名字改成 `OpenAgent-XXX.jar`,
> 记下原来的启动核心名字, 比如 `waterfall.jar`
>
> 直接启动服务器, 等待第一次执行 "崩溃"

如果你的服务商不支持自定义核心名字
> 首先找到我们的启动核心, 比如 `server.jar`
>
> 然后把 `server.jar` 改名成 `realserver.jar`
>
> 然后把 `OpenAgent-XXX.jar` 改名成 `server.jar`
>
> 直接启动服务器, 等待第一次执行 "崩溃"
>
> 此时你的 `你的核心jar` 为 `realserver.jar`



第一次崩溃之后, 直接中断服务器运行, 打开 OpenAgent.conf,
```hocon
bootstrap {
  jar = "waterfall.jar" # 你的核心jar
  mode = "jar" # 不需要改
  class = "org.bukkit.craftbukkit.Main" # 不需要改
}
agents = [
  # 一组 javaagent 启动参数对
  {path = "authlib-injector.jar", opt = "https://example.yggdrasil.com"},
  # 更多的 agent
  {path = "agent2.jar"}
  # ....
]
```

配置全部完成后, 重启服务器即可

## 支持的工作平台

我们支持了下面的平台, 如果你有需要, 可以自行编译 `natives`
然后给我们发出 `Pull Request`

- Windows
  - [x] Windows x64 (Compiled on `Microsoft Windows 10`)
- Linux
  - [x] Linux x64 (Compiled on `CentOS-8.1.1911-x86_64`(`VMware`))

未在上方列出的均为不支持.
