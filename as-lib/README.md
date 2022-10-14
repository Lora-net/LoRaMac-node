# LoRaMac-node 'as-lib'

**WIP**

This directory is provided to allow LoRaMac-node to be built as a static library for inclusion in other projects.

## CMake Options

- LORAMAC_AS_LIB:BOOL Whether to build the project as a static library (when ON), or to build the example applications
- LORAMAC_SUFFIX:STRING Defaults to empty, but can be set to any string to allow for multiple static libraries to
  be build (for example with different region support)
- LORAMAC_SECURE_ELEMENT Name of the secure element, defaults to SOFT_SE
- LORAMAC_SECURE_ELEMENT_PRE_PROVISIONED Whether the secure element is pre-provisioned (default ON)
- LORAMAC_SOFT_SE_AES_DEC_PREKEYED Whether the SOFT SE secure elements AES support is DEC Prekeyed (default ON)
- LORAMAC_RADIO Name of the radio driver, defaults to sx1272
- LORAMAC_USE_RADIO_DEBUG Enable Radio Debug GPIO's (default OFF)

## TODO

- FetchContent should not override GIT_SUBMODULES or lr1110-driver and atecc608a-tnglora-se/cryptoauthlib won't be loaded
- AES_DEC_PREKEYED - why did I define this in the project, it may not be needed