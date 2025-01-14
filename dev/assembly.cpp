#include <iostream>
#include <vector>

void
stackAllocationExample()
{
    std::vector<int> vec(6); // vec 对象本身在栈上
    vec.push_back(1);     // 元素 1 被分配到堆上
    vec.push_back(2);     // 元素 2 被分配到堆上
}

void
heapAllocationExample()
{
    std::vector<int>* vec = new std::vector<int>(5); // vec 对象本身在堆上
    vec->push_back(1);                            // 元素 1 被分配到堆上
    vec->push_back(2);                            // 元素 2 被分配到堆上
    delete vec;                                   // 释放 vec 对象本身和其元素的内存
}

int
main()
{
    stackAllocationExample();
    heapAllocationExample();
    return 0;
}
