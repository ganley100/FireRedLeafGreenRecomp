#!/usr/bin/env bash
# recompile_variant.sh — regenerate variants/<variant>/generated from a ROM.
#
#   tools/recompile_variant.sh <variant> <rom.gba> [gba_recompile] [radicalred-sha1]
#
# Used by the Android CI workflow and usable by hand. Passes the same config
# layering tools/decomp/import-symbols.sh documents: the variant's game.toml
# (base), its generated symbol overlay, any reviewed seeds, and the imported
# function / data symbol TSVs. Radical Red has no decomp overlay and an empty
# [identity] sha1 in the committed game.toml (rule 3: never guessed), so its
# hash is supplied here and written to a gitignored game.local.toml beside it.
set -euo pipefail

variant=${1:?variant (firered|leafgreen|radicalred)}
rom=${2:?path to the ROM}
recompiler=${3:-gba_recompile}
rr_sha1=${4:-${RADICALRED_ROM_SHA1:-}}

repo=$(cd "$(dirname "$0")/.." && pwd)
dir="$repo/variants/$variant"
[[ -f "$dir/game.toml" ]] || { echo "unknown variant: $variant" >&2; exit 2; }

args=(--rom "$rom" --out "$dir/generated" --max-functions 65536)
case "$variant" in
  firered)
    args+=(--config "$dir/game.toml" --config "$dir/symbols/BPRE_symbols.toml")
    ;;
  leafgreen)
    args+=(--config "$dir/game.toml" --config "$dir/symbols/BPGE_symbols.toml"
           --config "$dir/symbols/BPGE_reviewed_seeds.toml")
    ;;
  radicalred)
    rr_sha1=$(echo "$rr_sha1" | tr 'A-F' 'a-f')
    [[ "$rr_sha1" =~ ^[0-9a-f]{40}$ ]] || {
      echo "radicalred needs its patched ROM's SHA-1 (arg 4 or RADICALRED_ROM_SHA1)" >&2; exit 2; }
    sed "s/^sha1 = \"\"$/sha1 = \"$rr_sha1\"/" "$dir/game.toml" > "$dir/game.local.toml"
    grep -q "^sha1 = \"$rr_sha1\"" "$dir/game.local.toml"
    args+=(--config "$dir/game.local.toml")
    ;;
  *) echo "unknown variant: $variant" >&2; exit 2 ;;
esac
[[ -f "$dir/symbols/imported_symbols.tsv" ]] &&
  args+=(--symbols "$dir/symbols/imported_symbols.tsv")
[[ -f "$dir/symbols/imported_data_symbols.tsv" ]] &&
  args+=(--data-symbols "$dir/symbols/imported_data_symbols.tsv")

# Generated shards from a previous run (or another ROM) must not linger.
find "$dir/generated" -maxdepth 1 \( -name '*.c' -o -name '*.cpp' -o -name '*.h' -o -name '*.json' \) -delete

echo "+ $recompiler ${args[*]}"
"$recompiler" "${args[@]}"
