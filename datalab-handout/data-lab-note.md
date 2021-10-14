## CSAPP Lab1:data-lab-Note

CSAPP这本经典著作里不仅有重要的基础知识，里面的7个Lab也是非常的经典，以下为进行这个Lab所遇困难的笔记。

Lab1-data-lab是仅使用位操作符来实现一些简单基本的操作，实际写起来非常的有趣。也通过这个对二进制补码(2-complement)有更深的理解.

先是rating为1、2的几个简单练手:

1. 使用按位取反(~)与按位与(&)实现按位异或(^).这个比较简单思考

2. 获取Tmin.这个是这个lab最简单的东西，只要熟悉补码就知道0x8000000就是Tmin也就是1<<31.

3. 判断Tmax，Tmax=(1<<31)-1或0x7fffffff即可.

4.  

5. 求某数的负数（这个也是太简单），解释:任意数的补码加上本身刚好是0xffffffff(或者说补码+本体+1刚好算术溢出且变为0)，也就是a+(~a)+1=0;由于标志位可以解释为
   $$
   -2^{w-1}
   $$
   (w为二进制补码的位数)，那么某数的表示可以看作为
   $$
   B2T_w(\vec{x})=-x_{w-1}2^{w-1}+\sum_{i=0}^{w-2}{x_i2^i}
   $$
   详见书中第二章补码部分.

```c
//1
/*
 * bitXor - x^y using only ~ and &
 *   Example: bitXor(4, 5) = 1
 *   Legal ops: ~ &
 *   Max ops: 14
 *   Rating: 1
 */
int bitXor(int x, int y) {
    int a=(~x)&(~y);
    int b=x&y;
    int c=(~a)&(~b);
    return c;
  return 2;
}
/*
 * tmin - return minimum two's complement integer
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 4
 *   Rating: 1
 */
int tmin(void) {
    return 1<<31;
  return 2;

}
//2
/*
 * isTmax - returns 1 if x is the maximum, two's complement number,
 *     and 0 otherwise
 *   Legal ops: ! ~ & ^ | +
 *   Max ops: 10
 *   Rating: 1
 */
int isTmax(int x) {
    int a = !((~x)^(x+1));
    int b = !!(x+1);
    return a&b;
}
/*
 * allOddBits - return 1 if all odd-numbered bits in word set to 1
 *   where bits are numbered from 0 (least significant) to 31 (most significant)
 *   Examples allOddBits(0xFFFFFFFD) = 0, allOddBits(0xAAAAAAAA) = 1
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 12
 *   Rating: 2
 */
int allOddBits(int x) {
    //x=x&(x>>16)&(x>>8)&(x>>4);
    x=x&(x>>16);
    x=x&(x>>8);
    x=x&(x>>4);
    return !((x&0xa)^0xa);
}
/*
 * negate - return -x
 *   Example: negate(1) = -1.
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 5
 *   Rating: 2
 */
int negate(int x) {
    return ~x+1;
}
```

rating3:

```c
/*
 * isAsciiDigit - return 1 if 0x30 <= x <= 0x39 (ASCII codes for characters '0' to '9')
 *   Example: isAsciiDigit(0x35) = 1.
 *            isAsciiDigit(0x3a) = 0.
 *            isAsciiDigit(0x05) = 0.
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 15
 *   Rating: 3
 */
int isAsciiDigit(int x) {
    int y = !((x>>4)^3);
    int z = !((x&8)^0);
    int t = !(((x>>1)&7)^4);
    return y&(z|t);
  return 2;
}
/*
 * conditional - same as x ? y : z
 *   Example: conditional(2,4,5) = 4
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 16
 *   Rating: 3
 */
int conditional(int x, int y, int z) {
    x=!!x;
    return (((~x)+1)&y) |(((~(!x))+1)&z);
  return 2;
}
/*
 * isLessOrEqual - if x <= y  then return 1, else return 0
 *   Example: isLessOrEqual(4,5) = 1.
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 24
 *   Rating: 3
 */
int isLessOrEqual(int x, int y) {
    int f=(x^y)>>31;
    int p=(y+1+(~x))>>31;
    y = y>>31;
    return (!p&!f)|(f&!y);
  return 2;
}
```

