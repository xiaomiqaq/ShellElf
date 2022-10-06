以Tests/hello_a64测试程序为例

```
mkdir build 
cd build 
cmake ..
make

cd build
./Packer ../Tests/hello_a64

成功后，在Tests下生成hello_a64_文件，运行它需要改变权限
cd ../Tests
chmod 777 hello_a64_

ok
```

