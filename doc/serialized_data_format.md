# Serialized Data Format

A serialized chunk starts with the chunk header

```
header = 'DATA'
length = u64
```

where length is the size of the serialized content
not including the header and length

first the version for the data chunk is listed
```
version = u16
```
The chunk then lists type definitions for the
serialized data starting with type imports
```
type_import_count = u32
    *repeated type_import_count times
    module_name = ostr
    export_name = ostr

type_definition_count = u32
    *repeated type_definition_count
    export_name = ostr (empty string if not exported)
    type = u16
    flags = u16
    
    switch (type) {
        PrimitiveDataType:
        StringDataType:
            (empty)
        DataTypeWeakPointer:
        DataTypePointer:
            subType = strong type ref
        DataTypeFixedArray:
            elementCount = u32
            subType = strong type ref
        DataTypeDynamicArray:
            subType = strong type ref
        DataTypeObject:
            byteSize = u32
            subTypeCount = u32
                *repeat subTypeCount
                name = ostr
                type = strong type ref
                offset = u32
    }
```

next comes the list of imported serialized data

```
import_count = u32
    *repeated import_count 
    module_name = ostr
    export_name = ostr

serialized_object:
    type = strong type ref

    switch (type->type) {
        DataTypeNull:
            (empty)
        DataTypeInt8:
        DataTypeUInt8:
        DataTypeInt16:
        DataTypeUInt16:
        DataTypeInt32:
        DataTypeUInt32:
        DataTypeInt64:
        DataTypeUInt64:
        DataTypeFloat32:
        DataTypeFloat64:
            value = sizeof(type)
        DataTypeWeakPointer:
            pointer_to_head = strong ref
            offset = u32
        DataTypePointer:
            pointer = strong ref
        DataTypeString:
            value = ostr
        DataTypeFixedArray:
            *repeated type->elementCount
            element = serialized_object
        DataTypeDynamicArray:
            count = u32
            capacity = u32
                *repeated count
                element = serialized_object
        DataTypeObject:
            *foreach type->subTypes
            element = serialized_object
    }


definition_count = u32
    *repeated definition_count 
    export_name = ostr (empty string if not exported)

```