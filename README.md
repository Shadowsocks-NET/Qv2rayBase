# Qv2ray Base Library (libQv2rayBase)

The baselib of Qv2ray, with minimal dependencies but maximal feature set, modified from Shadowsocks-NET/Qv2ray.

GUI developers could link against this library to enjoy the functionalities of:

- Connection Management
- Group Management
  - Subscription Management
- Plugins
  - Kernel Plugins
  - EventHandler Plugins
  - Connection Management Plugins (Add, Remove, Update, Query)
  - Subscription Provider Plugins

## Super Easy Usage

### As a system package

```cmake
find_package(Qv2rayBase)
# Then
target_link_libraries(YourApp PRIVATE Qv2ray::Qv2rayBase)
```

### As a submodule

```cmake
add_subdirectory(path/to/Qv2rayBase)
# Then
target_link_libraries(YourApp PRIVATE Qv2ray::Qv2rayBase)
```

## License

GPLv3
