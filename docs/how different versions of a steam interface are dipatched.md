# How different versions/implementations of 1 interface are maintained and dispatched
This is done by ~~ab~~using the inheritance of C++ and how virtual functions tables (`vftables`) are implemented by compilers.  

The point of this article is explaining this somewhat cryptic code in [steam_client.cpp](../dll/steam_client.cpp)
```c++
if (strcmp(pchVersion, "SteamNetworking001") == 0) {
    return (ISteamNetworking *)(void *)(ISteamNetworking001 *)steam_networking_temp;
} else if (strcmp(pchVersion, "SteamNetworking002") == 0) {
    return (ISteamNetworking *)(void *)(ISteamNetworking002 *)steam_networking_temp;
}
```

And this other cryptic code in [flat.cpp](../dll/flat.cpp)
```c++
STEAMAPI_API uint32 SteamAPI_ISteamUtils_GetSecondsSinceAppActive( ISteamUtils* self )
{
    long long client_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_utils);
    long long server_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_gameserver_utils);
    auto ptr = get_steam_client()->steam_gameserver_utils;
    if (client_vftable_distance >= 0 && (server_vftable_distance < 0 || client_vftable_distance < server_vftable_distance)) {
        ptr = get_steam_client()->steam_utils;
    }

    return (ptr)->GetSecondsSinceAppActive();
}
```

Let's say you have 2 versions of an interface `INetworking_1` and `INetworking_2`, both provide 3 functions, but the newer one has subtle differences and maybe provides a new 4th function.  
During runtime the game will ask for `"net_1"` or `"net_2"` to get a pointer to some class implementing the corresponding interface, how should we do this?  

One way is to implement each interface separately in a standalone class
```c++
class NetImpl_1 { ... }
```
```c++
class NetImpl_2 { ... }
```
And if both of them have some common functionality, we can compose a 3rd class implementing the common stuff
```c++
// concrete class implementing the common functionality
class NetCommon { ... }

// back in these 2
class NetImpl_1 : public INetworking_1 {
private:
    class NetCommon *common;
}

class NetImpl_2 : public INetworking_2 {
private:
    class NetCommon *common;
}
```
Then return a pointer to either `NetImpl_1` or `NetImpl_2` whenever the game asks for `"net_1"` or `"net_2"` respectively.  
The obvious downside is the boilerplate code that has to be written for common functions:
```c++
// common provided function in each interface
class INetworking_1 {
public:
    virtual void common_func() = 0;
    ...
}

class INetworking_2 {
public:
    virtual void common_func() = 0;
    ...
}

class NetImpl_1 : public INetworking_1 {
private:
    class NetCommon *common;
public:
    void common_func() {
        common->do_whatever();
    }
}

class NetImpl_2 : public INetworking_2 {
private:
    class NetCommon *common;
public:
    void common_func() {
        common->do_whatever();
    }
}
```
Both versions of the interface offer/provide `common_func()`, hence both implementations has to implement it, so the function *body* must be written twice.

Another way, and this is how it's done for now, is benefitting from the multiple inheritance feature, in this case we only need to write the common function body once, also we need only one concrete implementation!
```c++
// common provided function in each interface
class INetworking_1 {
public:
    virtual void common_func() = 0;
    ...
}

class INetworking_2 {
public:
    virtual void common_func() = 0;
    ...
}

// notice we no longer need 2 classes with a common composite between them!
class NetImpl_all :
public INetworking_1,
public INetworking_2
{
public:
    void common_func() {
        /* do_whatever directly here! */
    }
}
```
That looks much better, isn't it?  
Well, yes it looks much better, especially if the functionality is the same anyway. But how the memory layout would look like when the 2 interfaces are different
```c++
class INetworking_1 {
public:
    virtual void common_func() = 0;
    virtual int func_aa() = 0;
    virtual float func_bb() = 0;
}

class INetworking_2 {
public:
    virtual void common_func() = 0;
    virtual int func_aa() = 0;
    // new!
    virtual const char* func_new() = 0;
    virtual float func_bb() = 0;
    ...
}
```
In C++ this actually makes a difference!, if the game asked for `"net_1"` expecting a pointer to an implementation for `INetworking_1`, and later it tried to call `func_bb()`, it will access the **3rd** member function.  
Imagine if we returned a wrong pointer to an implementation for `INetworking_2`, the **3rd** member function will be `func_new()`, by the way this is why failing to generate old interfaces will eventually cause a crash.  


# Exploring a simple layout
To understand this deeper, we'll create a console app, using `Visual Studio` on `Windows` OS, and the compilation type is `x64`.  
First let's check the layout of some structs, common includes:
```c++
#include <cstdint> // uint64_t
#include <iostream> // std::cout
```
Then simple structs to explore
```c++
struct My_Struct_1 {
    uint64_t x1 = 1111; // 8 bytes
    uint64_t y1 = 1111111; // 8 bytes
};

struct My_Struct_2 {
    uint64_t x2 = 2222; // 8 bytes
    uint64_t y2 = 22222222; // 8 bytes

    virtual void MyFn_2()
    {
        std::cout << "I'm My_Struct_2::MyFn_2()" << std::endl;
    }
};

struct My_Struct_3 {
    uint64_t x3 = 3333;
    uint64_t y3 = 33333333;

    virtual void MyFn_3()
    {
        std::cout << "I'm My_Struct_3::MyFn_3()" << std::endl;
    }

    virtual void MyOtherFn_3()
    {
        std::cout << "I'm My_Struct_3::MyOtherFn_3()" << std::endl;
    }
};
```
Let's print the size of each struct
```c++
int main(int argc, char* argv[])
{
    // create
    My_Struct_1 ss1;
    My_Struct_2 ss2;
    My_Struct_3 ss3;

    // check size
    std::cout << "My_Struct_1 size " << sizeof(ss1) << std::endl;
    std::cout << "My_Struct_2 size " << sizeof(ss2) << std::endl;
    std::cout << "My_Struct_3 size " << sizeof(ss3) << std::endl;
    std::cout << "=========" << std::endl;
    return 0;
}
```
We know for sure `My_Struct_1` will be 8+8=16 bytes, but let's look at the n others
```shell
My_Struct_1 size 16
My_Struct_2 size 24
My_Struct_3 size 24
=========
```
This looks quite unexpected, both `My_Struct_2` and `My_Struct_3` have an extra 8 bytes, why is that?  


# `reinterpret_cast<...>` aka "trust me bro I know what I'm doing"
We need to look at the memory layout of each struct to understand what's going on and why we have extra 8 bytes.  
We would like to take the address of each variable and print each 8 bytes alone, but we'll take a tangent since this won't work
```c++
uint64_t* ptr_ss = &ss1;
// error C2440: 'initializing': cannot convert from 'My_Struct_1 *' to 'uint64_t *'
// Types pointed to are unrelated; conversion requires reinterpret_cast, C-style cast or parenthesized function-style cast
```
The error and it's solution are obvious, let's do a simple `C-style cast`, but in C++ this actually performs multiple casts (during compilation) until the first one succeeds, check the order of casting here: https://en.cppreference.com/w/cpp/language/explicit_cast  
To explicitly tell the compiler *"hey do it as I want, I know what I'm doing"* there are 2 common options:
* `reinterpret_cast<uint64_t*>(&ss1)` which does what we want because the compiler will just pretend that it's a `uint64_t*` without a second thought or any checking, this is the usual way
* This hacky way: `uint64_t* ptr_ss = (uint64_t*)(void*)&ss1` this exploits the fact the casting from any pointer to `void*` will always succeed (not really but mostly will), and casting from `void*` back to any other pointer type will always be accepted (again not really but pretend it always succeeds).

We'll use the second way since it's used a lot in the code anyway to get used to it.


# Printing the layout of the structs
The idea is to get the address of each object, *pretend* each will be multiple of 8, and print each 8-bytes alone.  
Given that we knew from the above code that
```shell
My_Struct_1 size = 16 bytes (2 * 8)
My_Struct_2 size = 24 bytes (3 * 8)
My_Struct_3 size = 24 bytes (3 * 8)
```
We can just loop over
```c++
uint64_t* layout_1 = (uint64_t*)(void*)&ss1;
uint64_t* layout_2 = (uint64_t*)(void*)&ss2;
uint64_t* layout_3 = (uint64_t*)(void*)&ss3;

// print the layout of My_Struct_1
for (int i = 0; i < 2; ++i) {
    std::cout << "layout My_Struct_1 [" << i << "] = " << layout_1[i] << std::endl;
}
std::cout << "=========" << std::endl;

// print the layout of My_Struct_2
for (int i = 0; i < 3; ++i) {
    std::cout << "layout My_Struct_2 [" << i << "] = " << layout_2[i] << std::endl;
}
std::cout << "=========" << std::endl;

// print the layout of My_Struct_3
for (int i = 0; i < 3; ++i) {
    std::cout << "layout My_Struct_3 [" << i << "] = " << layout_3[i] << std::endl;
}
std::cout << "=========" << std::endl;
```
This is the output
```shell
layout My_Struct_1 [0] = 1111
layout My_Struct_1 [1] = 1111111
=========
layout My_Struct_2 [0] = 140695735040816
layout My_Struct_2 [1] = 2222
layout My_Struct_2 [2] = 22222222
=========
layout My_Struct_3 [0] = 140695735040872
layout My_Struct_3 [1] = 3333
layout My_Struct_3 [2] = 33333333
=========
```
Ok, this looks mostly fine, we can see the numbers for each variable in its own struct, but what is the first value of the layout from `My_Struct_2` and `My_Struct_3`?  

Because both structs have `virtual` functions, the compiler has added a new member for us, which holds the address of a compiler-generated array/list/table, and each value in this list is simply the address of the virtual function!  
This table is called the virtual functions table `vftable`, here's the layout in poor ASCII art
```
layout of <My_Struct_3>
|-----------------|
| 140695735040872 | // [0] memory address of the vftable
|-----------------|
| 3333            | // [1] our guy!
|-----------------|
| 33333333        | // [2] also our guy!
|-----------------|
```
```
layout of <140695735040872> (aka vftable)
|-----------------|
| &MyFn_3         | // [0] address of first virtual function
|-----------------|
| &MyOtherFn_3    | // [1] address of second virtual function
|-----------------|
```

Let's proove that, we'll grab the address of each virtual function, print it, and even invoke it.  
The 1st element of the struct layout was a pointer to the table, so let's take that 1st element and grab the address of that `vftable`
```c++
uint64_t* vtable_3 = (uint64_t*)layout_3[0]; // casting numbers to pointers is fine when you know what you're doing
```

Next, since we know we have 2 virtual functions, we'll print their addresses
```c++
for (int i = 0; i < 2; ++i) {
    std::cout << "vtable3 [" << i << "] = " << vtable_3[i] << std::endl;
}
```
Here's the output
```shell
vtable3 [0] = 140695734809281
vtable3 [1] = 140695734811996
```

Ok but how can we be sure that these are the actual functions addresses? Let's call them.  
In C++ class/struct member functions are simply regular C-functions with a *hidden* first argument, which is a pointer to the object, aka `this`
```c++
// a function which takes a ponter and returns void
typedef void (*vtable_member_t)(void* _this_obj);
```

We'll take each element in the vftable (address of `MyFn_3()` and address of `MyOtherFn_3()`) and call them
```c++
vtable_member_t vt_mem_item0 = (vtable_member_t)vtable_3[0];
std::cout << "calling original vtable_3[0]:" << std::endl;
vt_mem_item0(&ss3);
std::cout << "=========" << std::endl;

vtable_member_t vt_mem_item1 = (vtable_member_t)vtable_3[1];
std::cout << "calling original vtable_3[1]:" << std::endl;
vt_mem_item1(&ss3);
std::cout << "=========" << std::endl;
```

And here's the output!
```
calling original vtable_3[0]:
I'm My_Struct_3::MyFn_3()
=========
calling original vtable_3[1]:
I'm My_Struct_3::MyOtherFn_3()
=========
```

Nice, this prooves that structs/classes with `virtual` functions will have a *bonus hidden* member at the top, holding the address of a compiler-generated table called `vftable` (virtual functions table).  
If we grab the 1st element in that table, it will be the address of the 1st virtual function, the 2nd element in that table will be the address of the 2nd virtual function, etc...


# Inheritance from multiple interfaces
This is how all steam interfaces are implemented, it's always 1 class implementing all interfaces and providing all the implementations.  
But remember, the order of the virtual functions in source code impacts the order of the `vftable` members, go ahead and swap the functions in the source code
```c++
struct My_Struct_3 {
    uint64_t x3 = 3333;
    uint64_t y3 = 33333333;

    // this is now at the top
    virtual void MyOtherFn_3()
    {
        std::cout << "I'm My_Struct_3::MyOtherFn_3()" << std::endl;
    }
    
    // and this is placed at the bottom
    virtual void MyFn_3()
    {
        std::cout << "I'm My_Struct_3::MyFn_3()" << std::endl;
    }

};
```

Then, run the code which invokes the functions from the `vftable`, here's the output
```
calling original vtable_3[0]:
I'm My_Struct_3::MyOtherFn_3()
=========
calling original vtable_3[1]:
I'm My_Struct_3::MyFn_3()
=========
```

Also remember, games will get a pointer to that class based on the interface version they send to us, and will invoke the corresponding member function according to its order in the header `.h` file.  
So, what if 2 interfaces share the same set of functions, but the order is different? And what if a newer interface version removed or added a member function?  

Actually the answer is the same for all of the scenarios, let's first define a new struct which inherits from the previous ones
```c++
struct My_Struct_final :
    public My_Struct_3,
    public My_Struct_2,
    public My_Struct_1
{
    uint64_t x_final = 9999;
    uint64_t y_final = 99999999;

    virtual void MyOtherFn_final()
    {
        std::cout << "I'm My_Struct_final::MyOtherFn_final()" << std::endl;
    }
};
```

Let's also do all the previous steps we did on this new struct, create an object
```c++
// create
My_Struct_final ss_final;
```

Print the size
```c++
std::cout << "My_Struct_final size " << sizeof(ss_final) << std::endl;
std::cout << "=========" << std::endl;
```

Output
```
My_Struct_final size 80
=========
```

That is 80 bytes / 8 bytes (each element) = 10 elements, so let's print the layout
```c++
uint64_t* layout_final = (uint64_t*)(void*)&ss_final;

for (int i = 0; i < 10; ++i) {
    std::cout << "layout My_Struct_final [" << i << "] = " << layout_final[i] << std::endl;
}
std::cout << "=========" << std::endl;
```

And the output
```
layout My_Struct_final [0] = 140695735040976
layout My_Struct_final [1] = 3333
layout My_Struct_final [2] = 33333333
layout My_Struct_final [3] = 140695735041008
layout My_Struct_final [4] = 2222
layout My_Struct_final [5] = 22222222
layout My_Struct_final [6] = 1111
layout My_Struct_final [7] = 1111111
layout My_Struct_final [8] = 9999
layout My_Struct_final [9] = 99999999
=========
```

Let's analyze what we have from bottom to top, the last 2 elements are `9999` and `99999999` which are the members of the struct itself, not inherited.  
Next we have `1111` and `1111111` which are inherted from `My_Struct_1`.  
Next we have a trio { `140695735041008`, `2222`, and `22222222` }, and another similar trio { `140695735040976`, `3333`, and `33333333` }.  

It looks like the compiler just combined toghether all the layouts from above and put them in some sequential order!  
To verify this, let's *assume* that the large number above `3333` (`140695735040976`) is the address of the `vftable` for `My_Struct_3`, and because our new struct `My_Struct_final` didn't actually override anything, so the addresses of the virtual functions should be the same, let's test that.  

Get the first element from the layout
```c++
uint64_t* vtable_3_inheritance = (uint64_t*)layout_final[0]; // casting numbers to pointers is fine when you know what you're doing
```

Pretend that value is the address of the `vftable` for the inherited `My_Struct_3`, so grab the 1st item in the vftable (address of 1st virtual function)
```c++
vtable_member_t vt_mem_inheritance = (vtable_member_t)vtable_3_inheritance[0];
```

Invoke the first element in the `vftable`
```c++
std::cout << "calling vtable_3[0] from inheritance:" << std::endl;
vt_mem_inheritance(&ss_final);
std::cout << "=========" << std::endl;
```

Output!
```
calling vtable_3[0] from inheritance:
I'm My_Struct_3::MyFn_3()
=========
```

Ok that actually worked, it turns out our assumption was correct, so the compiler indeed combined all layouts into one, but the order in weird.  
And notice how the compiler created **a new vftable for each layout inherited**, so we don't have to worry about different versions of an interface, because the compiler will create a new vftable even if all versions share a set of similar functions. In our example we could even pretend `My_Struct_1`, `My_Struct_2`, and `My_Struct_3` are different versions of some arbitrary interface from the point of view of `My_Struct_final`, even though they are completely unrelated, the compiler doesn't care anyway.  

The Microsoft compiler vaguely implements the following layout from bottom to top:
```
2. layout of non-trivial classes/structs (with any virtual functions), in the order they're defined in source code
1. layout of trivial classes/structs without any virtual functions, in the order they're defined in source code
```

Go ahead and swap the order and print the layout again
```c++
struct My_Struct_final :
    public My_Struct_2, // now at the top
    public My_Struct_3,
    public My_Struct_1
{
    ...
};
```

Print it (notice how the layout containing `2222` is now above the one containing `3333`)
```
layout My_Struct_final [0] = 140699306621904
layout My_Struct_final [1] = 2222
layout My_Struct_final [2] = 22222222
layout My_Struct_final [3] = 140699306621936
layout My_Struct_final [4] = 3333
layout My_Struct_final [5] = 33333333
layout My_Struct_final [6] = 1111
layout My_Struct_final [7] = 1111111
layout My_Struct_final [8] = 9999
layout My_Struct_final [9] = 99999999
=========
```


# Casting pointers to classes with inheritance (answering the 1st riddle)
Casting an object of type `My_Struct_final*` to `My_Struct_1*` will make the compiler simply offset the pointer to point at the layout at the very bottom, let's proove it
```c++
My_Struct_1* ptr_adjusted_by_compiler = (My_Struct_1*)&ss_final; // notice how we're just casting, nothing crazy
uint64_t* layout_ptr = (uint64_t*)(void*)ptr_adjusted_by_compiler;
for (int i = 0; i < 2; ++i) {
    std::cout << "layout ptr [" << i << "] = " << layout_ptr[i] << std::endl;
}
std::cout << "=========" << std::endl;
```

Output
```
layout ptr [0] = 1111
layout ptr [1] = 1111111
=========
```

Casting an object of type `My_Struct_final*` to `My_Struct_1*` will simply offset the pointer to get the corresponding layout.  
etc...  

So, the key point here is that **even C-style casts aren't for free!** they definitely have an impact when the pointer target is some class/struct with inheritance.  
The above code is similar to this C++ style casts
```c++
My_Struct_1* ptr_adjusted_by_compiler = static_cast<My_Struct_1*>(&ss_final); // don't use "reinterpret_cast", we want the compiler to do its magic
uint64_t* layout_ptr = reinterpret_cast<uint64_t*>(ptr_adjusted_by_compiler); // trust me bro I know what I'm doing!
for (int i = 0; i < 2; ++i) {
    std::cout << "layout ptr [" << i << "] = " << layout_ptr[i] << std::endl;
}
std::cout << "=========" << std::endl;
```

Output (same)
```
layout ptr [0] = 1111
layout ptr [1] = 1111111
=========
```

Now you have a full picture what this evil line is doing
```c++
if (strcmp(pchVersion, "SteamNetworking001") == 0) {
    return (ISteamNetworking *)(void *)(ISteamNetworking001 *)steam_networking_temp;
}
```

If the game is asking for `v001` of the `networking` interface, from right to left these are the steps
1. We first cast the pointer of the implementation class `steam_networking_temp` to `ISteamNetworking001 *` via a `C-Style` cast, this allows the compiler to do its **magic** and adjust/offset the pointer to the relevant/correct part of the huge layout
2. Cast that to `void *` so the compiler will lose the type information and will not do any more adjustments
3. Cast the result back to `ISteamNetworking *`, but this won't do any magic, the compiler already lost type info

That whole sentence could be replaced with
```c++
return reinterpret_cast<ISteamNetworking *>(static_cast<ISteamNetworking001 *>(steam_networking_temp));
```

Whether this is better or worse is up to you, both are ugly and took an article to explain!

But can't we just do this
```c++
return (ISteamNetworking *)(ISteamNetworking001 *)steam_networking_temp;
```

Remember C-Style cast in C++ isn't deterministic, casting a pointer of derived class to its parent will adjust the pointer again!, let's prove it
```c++
My_Struct_final* what_ptr_is_this = (My_Struct_final*)(My_Struct_1*)&ss_final; // hmmm
uint64_t* what_layout_is_this = reinterpret_cast<uint64_t*>(what_ptr_is_this); // trust me bro I know what I'm doing!
for (int i = 0; i < 3; ++i) {
    std::cout << "what_layout_is_this [" << i << "] = " << what_layout_is_this[i] << std::endl;
}
std::cout << "=========" << std::endl;
```

Output
```
what_layout_is_this [0] = 140700555742240
what_layout_is_this [1] = 3333
what_layout_is_this [2] = 33333333
=========
```

We got back the layout of the original/big class, not the small one, so we can't really ommit the `(void *)` cast in the middle, let's do it again with the `(void *)` cast
```c++
My_Struct_final* what_ptr_is_this = (My_Struct_final*)(void*)(My_Struct_1*)&ss_final; // all we did was literally add (void*)
uint64_t* what_layout_is_this = reinterpret_cast<uint64_t*>(what_ptr_is_this); // trust me bro I know what I'm doing!
for (int i = 0; i < 3; ++i) {
    std::cout << "what_layout_is_this [" << i << "] = " << what_layout_is_this[i] << std::endl;
}
std::cout << "=========" << std::endl;
```

Output
```
what_layout_is_this [0] = 1111
what_layout_is_this [1] = 1111111
what_layout_is_this [2] = 9999
=========
```

Pure evil in my opinion


# Detecting the base of the layout from a given address (answering the 2nd riddle)
In this code
```c++
STEAMAPI_API uint32 SteamAPI_ISteamUtils_GetSecondsSinceAppActive( ISteamUtils* self )
{
    long long client_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_utils);
    long long server_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_gameserver_utils);
    auto ptr = get_steam_client()->steam_gameserver_utils;
    if (client_vftable_distance >= 0 && (server_vftable_distance < 0 || client_vftable_distance < server_vftable_distance)) {
        ptr = get_steam_client()->steam_utils;
    }

    return (ptr)->GetSecondsSinceAppActive();
}
```

At the first line
```c++
long long client_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_utils);
```

1. We get the address of `steam_utils` and use a `C-Style` cast to convert it to `(char *)`  
   If we tried
   ```c++
   static_cast<char*>(get_steam_client()->steam_utils)
   ```
   We'll get a compiler error `invalid type conversion`, but remember in C++, a C-Style cast will force the compiler to try different other casts: https://en.cppreference.com/w/cpp/language/explicit_cast  
   The one that will work here is the `reinterpret_cast` (aka *"don't do any magic, I'm aware of what I'm doing"*)
2. Subtract `given address - base layout address`
3. Do the same for the `gameserver` instance (`steam_gameserver_utils`) layout

We end up with 2 results from the subtraction
* If the given address is somewhere within the layout of the client instance (`steam_utils`),  
  then the distnce `client_vftable_distance` will be a positive value  
  and the other distance `server_vftable_distance` will be a negative value
* If the given address is somewhere within the layout of the gameserver instance (`steam_gameserver_utils`),  
  then the distnce `server_vftable_distance` will be a positive value  
  and the other distance `client_vftable_distance` will be a negative value

This is why we have that condition
```c++
if (client_vftable_distance >= 0 ... ) {

}
```

But what if we're unlucky and we got 2 memory blocks under each other like this:
```
|------------------------|
|   ..................   |
|   ..................   |
|   ..................   |
|       steam_utils      | // client instance layout
|   ..................   |
|   ..................   |
|   ..................   |
|------------------------|
|   ..................   |
|   ..................   |
|   ..................   |
| steam_gameserver_utils | // gameserver instance layout
|   ..................   |
|   ..................   |
|   ..................   |
|------------------------|
```

In that case the distance `client_vftable_distance` will be positive if the given pointer was somewhere inside the layout of the gameserver instance, and for that reason we have the other condition
```c++
if ( ... && (server_vftable_distance < 0 || client_vftable_distance < server_vftable_distance)) {

}
```

If `server_vftable_distance < 0` then the answer is obvious, but in the unlucky case `client_vftable_distance < server_vftable_distance` will be our savior.  
