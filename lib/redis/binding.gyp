{
  "targets": [
    {
      "target_name": "A3",
      "cflags_cc": [
        "-std=c++17",
        "-fexceptions"
      ],
      "sources": [
        "src/lib/Worker.cpp",
        "src/lib/A3.cpp"
      ],
      "include_dirs": [
        "<!@(node -p \"require('node-addon-api').include\")",
        "include",
        "/usr/local/include"
      ],
      "libraries": [
        "/usr/local/lib/libredis++.so",
        "/usr/local/lib/libglog.so",
        "/usr/local/lib/libgtest.a"
      ],
      'defines': [
        'NAPI_DISABLE_CPP_EXCEPTIONS'
      ],
    }
  ]
}