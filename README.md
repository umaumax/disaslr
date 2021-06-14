# disaslr

disable ASLR(Address Space Layout Randomization) for Max OS X

You can use this tool at Ubuntu too, but it is easy to use `setarch $(uname -m) -R ./show-main-addr`.

## how to build
``` bash
g++ -std=c++11 disaslr.cpp -o disaslr

g++ -std=c++11 show-main-addr.cpp -o show-main-addr
```

## how to run
``` bash
gobjdump -dF ./show-main-addr | grep '<_main> (File Offset:'

./disaslr show-main-addr
```

* main function's address is same under ASLR but variables address(stack address) change at Ubuntu16.04.
* main function's address and variables address(stack address) change at Mac OS X.
