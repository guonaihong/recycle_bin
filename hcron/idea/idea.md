# hcron

## 缘由
以前曾写过定时执行任务的代码,每隔一段时间做一些事,以及每个小时的开头，或者每天的开头或者每月的开头做一些事。
写这种代码就发现特别烧脑子，看着很简单的事，写起来累，跑一段时间就会有以前没考虑到情况发生。

## 自己想做成什么样子
* 使用cron使用的表达式达到定时的效果
* 支持中文

## 必须通过的case
* 每个月最后一个星期六
* 春节,端午,清明,等农历假日
* cron所使用的表达式