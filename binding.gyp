{
  "targets": [
    {
      "target_name": "kqueue",
      "sources": [ "kqueue.cpp" ],
      "include_dirs": [
        "<!(node -p \"require('node-addon-api').include\")"
      ],
      "dependencies": [
        "<!(node -p \"require('node-addon-api').targets\"):node_addon_api_except_all"
      ],
      "defines": [
        "NODE_ADDON_API_ENABLE_EXCEPTION"
      ],
      "cflags_cc": [
        "-std=c++20",
        "-fexceptions"
      ],
      "conditions": [
        ["OS=='mac'", {
          "cflags+": [ "-fvisibility=hidden" ],
          "xcode_settings": {
            "GCC_SYMBOLS_PRIVATE_EXTERN": "YES"
          }
        }]
      ]
    }
  ]
}
