
```
layout.child0_r = layout.container_r + 10
```

```
COPY -1
LOAD_STRING "container_r"
LOAD_TABLE
LOAD_INT 10
ADD
COPY -1
LOAD_TABLE
LOAD_STRING "child0_r"
STORE_TABLE
```