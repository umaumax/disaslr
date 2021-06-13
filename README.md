# disaslr

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
