{
  "targets": [
    {
      "target_name": "A3",
      "cflags_cc": [
        "-std=c++17",
        "-fexceptions"
      ],
      "xcode_settings": {
        "OTHER_CFLAGS": [
          "-std=c++17",
          "-fexceptions",
          "-mmacosx-version-min=11.0"
        ],
      },
      "sources": [
        "src/xml/Parser.cpp",
        "src/A3.cpp"
      ],
      "include_dirs": [
        "<!@(node -p \"require('node-addon-api').include\")",
        "include",
        "/usr/include",
        "/usr/local/include"
      ],
      "libraries": [ ],
      'conditions': [
        [ 'OS=="linux"', { 'libraries': [ '/usr/local/lib/libpugixml.a', '/usr/local/lib/libglog.so' ] } ],
        [ 'OS=="mac"',   { 'libraries': [ '/usr/local/lib/libpugixml.a' ] } ],
        [ 'OS=="win"',   { 'libraries': [ 'os_not_supported.windows' ] } ],
      ],
      'defines': [
        'NAPI_DISABLE_CPP_EXCEPTIONS'
      ],
    }
  ]
}