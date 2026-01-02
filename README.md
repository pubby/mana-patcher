# Mana Patcher

This tool patches a `.nes` file into a `.uf2` file, which is what stores Mana mapper firmware.

## Drag & Drop Usage

You can drag a `.nes` file onto the executable to have it automatically convert.

**Note:** this requires `firmware.uf2` to be in the same folder.

## Command-line Usage

`
mana-patcher [in .nes] [in .uf2] [out .uf2]
`

- If `[in .uf2]` is not specified, it defaults to `firmware.uf2`.
- If `[out .uf2]` is not specified, it defaults to the `.nes` filename with `.uf2` appended.
