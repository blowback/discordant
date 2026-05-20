idf_component_register(
    SRCS "csrc/pf_cglue.c" "csrc/pf_clib.c" "csrc/pf_core.c"
         "csrc/pf_inner.c" "csrc/pf_io.c"   "csrc/pf_io_none.c"
         "csrc/pf_mem.c"   "csrc/pf_save.c" "csrc/pf_text.c"
         "csrc/pf_words.c" "csrc/pfcompil.c"
         "pfcustom.c"
         "pf_io_esp32s3.c"
    INCLUDE_DIRS "csrc" "."
    REQUIRES esp_driver_usb_serial_jtag esp_timer
)

target_compile_options(${COMPONENT_LIB} PRIVATE
    -Wno-cast-function-type
)

target_compile_definitions(${COMPONENT_LIB} PRIVATE
    PF_STATIC_DIC
)
