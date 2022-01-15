# Layout algorithm

```

LayoutInfo:
    Name: string
    Type: DataType
    Offset: number

StateLayout: LayoutInfo[]

Component
    IsDirty: boolean
    StateLayout: StateLayout
    InternalState: Any
    PrevInternalState: Any
    LayoutFunction: (component:Component)

void setComponentValue(component: Component layoutInfo: LayoutInfo, value: Any);
void layoutComponent(component: Component);
```
