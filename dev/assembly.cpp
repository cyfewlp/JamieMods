#include <iostream>
#include <vector>

void
stackAllocationExample()
{
    std::vector<int> vec(6); // vec ��������ջ��
    vec.push_back(1);     // Ԫ�� 1 �����䵽����
    vec.push_back(2);     // Ԫ�� 2 �����䵽����
}

void
heapAllocationExample()
{
    std::vector<int>* vec = new std::vector<int>(5); // vec �������ڶ���
    vec->push_back(1);                            // Ԫ�� 1 �����䵽����
    vec->push_back(2);                            // Ԫ�� 2 �����䵽����
    delete vec;                                   // �ͷ� vec ���������Ԫ�ص��ڴ�
}

int
main()
{
    stackAllocationExample();
    heapAllocationExample();
    return 0;
}
