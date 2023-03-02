{
  "target_defaults": {
    "default_configuration": "Release",
    "configurations": {
      "Release": {
        "cflags": [ "-O3" ]
      },
      "Debug": {
        "defines": [ "DEBUG", "_DEBUG" ],
        "cflags": [ "-g", "-O0" ]
      }
    }
  },
  "targets": [{
    "target_name": "libencode",
    "type": "static_library",
    "sources": [
      "src/urlencode-data.cc",
      "src/urlencode.cc"
    ],
    "include_dirs": [
      "include"
    ],
    "direct_dependent_settings": {
      "include_dirs": [
        "include"
      ]
    }
  }, {
    "target_name": "example",
    "type": "executable",
    "sources": [
      "example/main.cc"
    ],
    "dependencies": [
      "libencode"
    ]
  }]
}
