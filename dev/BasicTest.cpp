#include <algorithm>
#include <bitset>
#include <expected>
#include <iostream>
#include <random>
#include <spdlog/spdlog.h>

#include <boost/multi_index/composite_key.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/ranked_index.hpp>
#include <boost/multi_index/tag.hpp>
#include <boost/multi_index_container.hpp>
#include <iostream>
#include <string>

using namespace boost::multi_index;

// 数据结构
struct Record
{
    bool        flag;
    int         id;
    std::string name;
};

// 标签
struct by_rank
{
}; // 用于 ranked index

struct by_name
{
}; // 用于按名称排序

// 多索引容器定义
using RankedIndex =
    ranked_non_unique<tag<by_rank>,
                      composite_key<Record, member<Record, bool, &Record::flag>, member<Record, int, &Record::id>>,
                      composite_key_compare<std::greater<bool>, std::less<int>>>;

using NameIndex = ordered_non_unique<tag<by_name>, member<Record, std::string, &Record::name>>;

using RecordSet = multi_index_container<Record, indexed_by<RankedIndex, NameIndex>>;

std::string generate()
{
    return R"( student jamie("jamie");)";
}

struct student
{
    std::string_view name;

    student(const char *name) : name(std::move(name)) {}
};

struct teacher
{
    const student *m_student = nullptr;

    teacher operator|=(const student &student) const
    {
        return teacher{&student};
    }
};

enum error
{
    no_error,
};

std::string_view generateView()
{
    return "hello world";
}

int main()
{
    std::bitset<32> slots = 1024 ;
    slots |= 256;
    std::cout << slots.to_ulong()  << " " << slots.count() << std::endl;
    slots.set(20);
    std::cout << slots.to_ulong()  << " " << slots.count() << std::endl;
    return 0;
}