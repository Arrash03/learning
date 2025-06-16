#include <iostream>
// #include <bits/stdc++.h>
// // #include <clang/18.1.3/include/__clang_cuda_builtin_vars.h>
// // #include <linux/cuda.h>
// #include <linux/securebits.h>
// #include <linux/selinux_netlink.h>
// #include <linux/ethtool_netlink.h>
// #include <linux/netlink.h>
// #include <linux/cryptouser.h>
// #include <linux/const.h>

inline int static_var = 12;




#include "class.h"


void print_static()
{
    std::cout << static_var << std::endl;
}

void print_const()
{
    std::cout << const_var << std::endl;
}

void A::print_my_self()
{
    std::cout << a << b << c << std::endl;
}
