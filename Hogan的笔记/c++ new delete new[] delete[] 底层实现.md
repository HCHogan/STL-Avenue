# c++ new delete new[] delete[] 底层实现
看了一些文章后觉得对下面几个概念没有一个清晰的讲解，我总结一下：
对于内置类型：
new []不会在首地址前4个字节定义数组长度。
delete 和 delete[]是一样的执行效果，都会删除整个数组，要删除的长度从new时即可知道。
对于自定义类型：
new []会在首地址前4个字节定义数组长度。
当delete[]时，会根据前4个字节所定义的长度来执行析构函数删除整个数组。
如果只是delete数组首地址，只会删除第一个对象的值。
==============我是分割线================
剩下的相关基础概念可以参考这篇文章：
原文地址：
http://blog.csdn.net/hazir/article/details/21413833
原文内容：
在 C++ 中，你也许经常使用 new 和 delete 来动态申请和释放内存，但你可曾想过以下问题呢？

new 和 delete 是函数吗？
new [] 和 delete [] 又是什么？什么时候用它们？
你知道 operator new 和 operator delete 吗？
为什么 new [] 出来的数组有时可以用 delete 释放有时又不行？
…
如果你对这些问题都有疑问的话，不妨看看我这篇文章。

new 和 delete 到底是什么？

如果找工作的同学看一些面试的书，我相信都会遇到这样的题：sizeof 不是函数，然后举出一堆的理由来证明 sizeof 不是函数。在这里，和 sizeof 类似，new 和 delete 也不是函数，它们都是 C++ 定义的关键字，通过特定的语法可以组成表达式。和 sizeof 不同的是，sizeof 在编译时候就可以确定其返回值，new 和 delete 背后的机制则比较复杂。
继续往下之前，请你想想你认为 new 应该要做些什么？也许你第一反应是，new 不就和 C 语言中的 malloc 函数一样嘛，就用来动态申请空间的。你答对了一半，看看下面语句：

string *ps = new string(“hello world”);
你就可以看出 new 和 malloc 还是有点不同的，malloc 申请完空间之后不会对内存进行必要的初始化，而 new 可以。所以 new expression 背后要做的事情不是你想象的那么简单。在我用实例来解释 new 背后的机制之前，你需要知道 operator new 和 operator delete 是什么玩意。

operator new 和 operator delete

这两个其实是 C++ 语言标准库的库函数，原型分别如下：

void *operator new(size_t); //allocate an object
void operator delete(void ); //free an object

void *operator new; //allocate an array
void operator delete[](void ); //free an array
后面两个你可以先不看，后面再介绍。前面两个均是 C++ 标准库函数，你可能会觉得这是函数吗？请不要怀疑，这就是函数！C++ Primer 一书上说这不是重载 new 和 delete 表达式（如 operator= 就是重载 = 操作符），因为 new 和 delete 是不允许重载的。但我还没搞清楚为什么要用 operator new 和 operator delete 来命名，比较费解。我们只要知道它们的意思就可以了，这两个函数和 C 语言中的 malloc 和 free 函数有点像了，都是用来申请和释放内存的，并且 operator new 申请内存之后不对内存进行初始化，直接返回申请内存的指针。

我们可以直接在我们的程序中使用这几个函数。

new 和 delete 背后机制

知道上面两个函数之后，我们用一个实例来解释 new 和 delete 背后的机制：

我们不用简单的 C++ 内置类型来举例，使用复杂一点的类类型，定义一个类 A：
```cpp
class A
{
public:
A(int v) : var(v)
{
fopen_s(&file, “test”, “r”);
}
~A()
{
fclose(file);
}
```
private:
int var;
FILE *file;
};
很简单，类 A 中有两个私有成员，有一个构造函数和一个析构函数，构造函数中初始化私有变量 var 以及打开一个文件，析构函数关闭打开的文件。

我们使用

```cpp
class *pA = new A(10);
```
来创建一个类的对象，返回其指针 pA。如下图所示 new 背后完成的工作：

简单总结一下：

首先需要调用上面提到的 operator new 标准库函数，传入的参数为 class A 的大小，这里为 8 个字节，至于为什么是 8 个字节，你可以看看《深入 C++ 对象模型》一书，这里不做多解释。这样函数返回的是分配内存的起始地址，这里假设是 0x007da290。
上面分配的内存是未初始化的，也是未类型化的，第二步就在这一块原始的内存上对类对象进行初始化，调用的是相应的构造函数，这里是调用 A:A(10); 这个函数，从图中也可以看到对这块申请的内存进行了初始化，var=10, file 指向打开的文件。
最后一步就是返回新分配并构造好的对象的指针，这里 pA 就指向 0x007da290 这块内存，pA 的类型为类 A 对象的指针。
所有这三步，你都可以通过反汇编找到相应的汇编代码，在这里我就不列出了。

好了，那么 delete 都干了什么呢？还是接着上面的例子，如果这时想释放掉申请的类的对象怎么办？当然我们可以使用下面的语句来完成：

```cpp
delete pA;
```
delete 所做的事情如下图所示：

delete 就做了两件事情：

调用 pA 指向对象的析构函数，对打开的文件进行关闭。
通过上面提到的标准库函数 operator delete 来释放该对象的内存，传入函数的参数为 pA 的值，也就是 0x007d290。
好了，解释完了 new 和 delete 背后所做的事情了，是不是觉得也很简单？不就多了一个构造函数和析构函数的调用嘛。

如何申请和释放一个数组？

我们经常要用到动态分配一个数组，也许是这样的：

string *psa = new string[10]; //array of 10 empty strings
int *pia = new int[10]; //array of 10 uninitialized ints
上面在申请一个数组时都用到了 new [] 这个表达式来完成，按照我们上面讲到的 new 和 delete 知识，第一个数组是 string 类型，分配了保存对象的内存空间之后，将调用 string 类型的默认构造函数依次初始化数组中每个元素；第二个是申请具有内置类型的数组，分配了存储 10 个 int 对象的内存空间，但并没有初始化。

如果我们想释放空间了，可以用下面两条语句：

```cpp
delete [] psa;
delete [] pia;
```
都用到 delete [] 表达式，注意这地方的 [] 一般情况下不能漏掉！我们也可以想象这两个语句分别干了什么：第一个对 10 个 string 对象分别调用析构函数，然后再释放掉为对象分配的所有内存空间；第二个因为是内置类型不存在析构函数，直接释放为 10 个 int 型分配的所有内存空间。

这里对于第一种情况就有一个问题了：我们如何知道 psa 指向对象的数组的大小？怎么知道调用几次析构函数？

这个问题直接导致我们需要在 new [] 一个对象数组时，需要保存数组的维度，C++ 的做法是在分配数组空间时多分配了 4 个字节的大小，专门保存数组的大小，在 delete [] 时就可以取出这个保存的数，就知道了需要调用析构函数多少次了。

还是用图来说明比较清楚，我们定义了一个类 A，但不具体描述类的内容，这个类中有显示的构造函数、析构函数等。那么 当我们调用

```cpp
class A *pAa = new A[3];
```
时需要做的事情如下：

从这个图中我们可以看到申请时在数组对象的上面还多分配了 4 个字节用来保存数组的大小，但是最终返回的是对象数组的指针，而不是所有分配空间的起始地址。

这样的话，释放就很简单了：

delete [] pAa;

这里要注意的两点是：

调用析构函数的次数是从数组对象指针前面的 4 个字节中取出；
传入 operator delete[] 函数的参数不是数组对象的指针 pAa，而是 pAa 的值减 4。
为什么 new/delete 、new []/delete[] 要配对使用？

其实说了这么多，还没到我写这篇文章的最原始意图。从上面解释的你应该懂了 new/delete、new[]/delete[] 的工作原理了，因为它们之间有差别，所以需要配对使用。但偏偏问题不是这么简单，这也是我遇到的问题，如下这段代码：

int *pia = new int[10];
delete []pia;
这肯定是没问题的，但如果把 delete []pia; 换成 delete pia; 的话，会出问题吗？

这就涉及到上面一节没提到的问题了。上面我提到了在 new [] 时多分配 4 个字节的缘由，因为析构时需要知道数组的大小，但如果不调用析构函数呢（如内置类型，这里的 int 数组）？我们在 new [] 时就没必要多分配那 4 个字节， delete [] 时直接到第二步释放为 int 数组分配的空间。如果这里使用 delete pia;那么将会调用 operator delete 函数，传入的参数是分配给数组的起始地址，所做的事情就是释放掉这块内存空间。不存在问题的。

这里说的使用 new [] 用 delete 来释放对象的提前是：对象的类型是内置类型或者是无自定义的析构函数的类类型！

我们看看如果是带有自定义析构函数的类类型，用 new [] 来创建类对象数组，而用 delete 来释放会发生什么？用上面的例子来说明：

class A *pAa = new class A[3];
delete pAa;
那么 delete pAa; 做了两件事：

调用一次 pAa 指向的对象的析构函数；
调用 operator delete(pAa); 释放内存。
显然，这里只对数组的第一个类对象调用了析构函数，后面的两个对象均没调用析构函数，如果类对象中申请了大量的内存需要在析构函数中释放，而你却在销毁数组对象时少调用了析构函数，这会造成内存泄漏。

上面的问题你如果说没关系的话，那么第二点就是致命的了！直接释放 pAa 指向的内存空间，这个总是会造成严重的段错误，程序必然会奔溃！因为分配的空间的起始地址是 pAa 指向的地方减去 4 个字节的地方。你应该传入参数设为那个地址！

同理，你可以分析如果使用 new 来分配，用 delete [] 来释放会出现什么问题？是不是总会导致程序错误？

总的来说，记住一点即可：new/delete、new[]/delete[] 要配套使用总是没错的！
————————————————
版权声明：本文为CSDN博主「__Lingyue__」的原创文章，遵循CC 4.0 BY-SA版权协议，转载请附上原文出处链接及本声明。
原文链接：https://blog.csdn.net/cFarmerReally/article/details/54585443