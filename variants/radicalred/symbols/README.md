# symbols/ — Radical Red

Intentionally empty of imported symbols.

`pret/pokefirered` describes **vanilla** FireRed. Radical Red is built on
CFRU + Dynamic Pokémon Expansion, which:

- overwrite the start of many vanilla routines with hook branches, and
- add thousands of new functions in previously-free ROM space.

Re-using `../../firered/symbols/*.tsv` as seeds would therefore name (and
force function starts at) addresses whose bytes no longer match. Until a
reviewed Radical Red seed list exists, the recompile is driven only by the
entry point plus the recompiler's discovery loop, and misses are bridged and
healed at runtime.

If you later produce seeds (e.g. from CFRU's own build symbol output for
the exact release you patched), put them here as `imported_symbols.tsv` and
add `seeds = "symbols/imported_symbols.tsv"` to `../game.toml [recompiler]`.
