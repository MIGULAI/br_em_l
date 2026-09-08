Import("env")

env.AddPostAction(
    "$BUILD_DIR/${PROGNAME}.bin",
    env.VerboseAction(" ".join([
        "$PYTHONEXE", "$OBJCOPY", "--chip", "esp32s3",
        "merge_bin", "-o", "$BUILD_DIR/merged.bin",
        "--flash_mode", "dio", "--flash_freq", "80m", "--flash_size", "8MB",
        "0x0", "$BUILD_DIR/bootloader.bin",
        "0x8000", "$BUILD_DIR/partitions.bin",
        "0x10000", "$BUILD_DIR/${PROGNAME}.bin",
    ]), "Merging flash image")
)