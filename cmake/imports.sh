
# Beatuy!SpaceStage — © 2026 Sheroll s.r.o® All rights reserved.
# Sheroll Non-Commercial License 1.0: see LICENSE in the project root.

set -euo pipefail

dll=$1
objdump=${LLVM_OBJDUMP:-$(command -v llvm-objdump || command -v llvm-objdump-21 || true)}
if [[ -z $objdump ]]; then
    printf 'imports guard: llvm-objdump was not found; add it to PATH or set LLVM_OBJDUMP\n' >&2
    exit 1
fi
status=0
while read -r _ _ name; do
    case ${name,,} in
        kernel32.dll | user32.dll | vcruntime140.dll | api-ms-win-crt-*.dll) ;;
        *)
            printf 'imports guard: %s imports %s\n' "${dll##*/}" "$name" >&2
            status=1
            ;;
    esac
done < <("$objdump" -p "$dll" | grep 'DLL Name:')
exit $status

