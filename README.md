# Lamphttp HTTP服务

Lamphttp是使用c语言实现的http服务，目前市面上有非常多的http服务，比如大名鼎鼎的Nginx
那么对于Lamphttp存在的意义是什么呢？对于Lamphttp主要是为了理解了tcp/ip到http的中间
这一层的实现，说白了就是当作学习用的.

虽然Lamphttp是作为学习用的项目，但其中间也用到了很多linux的编程技术，对于理解linux编
程非常有帮助. 比如，整个项目采用了反应堆模式，让项目有相当的可扩展空间. 当然免不了还有
linux的多线程编程，实现了一个简单的线程池，对于各个线程之间的通信实现了一个channel, 
当然最重要的是我们使用了epoll技术，让我们的服务支持高并发。下面是整个项目的架构图。

![lamphttp image](docs/lamphttp.jpg)

### 安装编译

项目使用CMake来管理依赖，使用如下步聚就可以安装服务：

```
git clone https://github.com/seepre/lamphttp.git
cd lamphttp
mkdir build
cmake ../
make && make install
```

如果没有安装cmake，可以使用如下命令安装：

```
# macOS
brew install cmake

# centOS
yum install cmake

# ubuntu
apt-get install cmake
```
