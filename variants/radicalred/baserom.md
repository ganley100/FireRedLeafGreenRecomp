# Base ROM — Radical Red

We ship **neither the ROM nor the Radical Red patch**. To build this variant:

1. Start from your own dump of **Pokémon FireRed USA v1.0 (rev0)**,
   SHA-1 `41cb23d8dccc8ebd7c649cd8fbb58eeace6e2fdc` — the same ROM the
   `firered` variant uses. Radical Red's patch is made for this revision.
2. Apply the official Radical Red `.ups` patch for the release you want
   (e.g. with Floating IPS / UPS tooling).
3. Save the result as `variants/radicalred/roms/radicalred.gba`
   (gitignored).
4. Record its hash:
   ```
   sha1sum variants/radicalred/roms/radicalred.gba
   crc32   variants/radicalred/roms/radicalred.gba
   ```
   Put the SHA-1 in `game.toml [identity] sha1` and
   `config/radicalred_usa.toml`, and configure CMake with
   `-DRADICALRED_ROM_SHA1=<sha1> -DRADICALRED_ROM_CRC32=0x<crc32>`.

The `RadicalRedRecomp` target is **not generated** until
`RADICALRED_ROM_SHA1` is set, so FireRed / LeafGreen builds are unaffected.

## Known-good patched ROMs

| Release | SHA-1 | Notes |
|---------|-------|-------|
| _none recorded yet_ | | Add a row once a patched ROM is verified against mGBA. |

Each Radical Red release is a different ROM: a new release is a new hash row
(and a re-run of `gba_recompile`), never an edit of an old one.
