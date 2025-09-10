# kernel_dev_diary

本仓库用于记录个人学习嵌入式Linux设备驱动程序的笔记。

目前的工作主要是进行交换机应用层面的协议开发，使用到的是C语言。对于Linux驱动的部分用之甚少，只能在工作之余自己学习，希望能够坚持！

## 开发环境

本项目使用RPi5，内核版本6.12.25+rpt-rpi-2712，架构aarch64

项目文件结构如下：

```
.
├── code
│   ├── examples        # 基础示例代码
│   ├── experiments     # 实验性代码
│   └── projects        # 完整驱动项目
├── notes
│   ├── errors          # 错误调试记录
│   ├── references      # 参考资料和文档
│   ├── study_log       # 学习日志
│   └── turtorials      # 教程式笔记
└── README.md
```