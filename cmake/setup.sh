
# Beatuy!SpaceStage — © 2026 Sheroll s.r.o® All rights reserved.
# Sheroll Non-Commercial License 1.0: see LICENSE in the project root.

set -euo pipefail
LC_ALL=C
shopt -u patsub_replacement 2>/dev/null || true

SCRIPT=$(realpath -- "${BASH_SOURCE[0]}")
ROOT=${SCRIPT%/*/*}
DEPS=$(realpath -ms -- "${CIV_DEPS:-$ROOT/deps}")
DETOURS_REPO=https://github.com/microsoft/Detours.git
DETOURS_TAG=v4.0.1
DETOURS_COMMIT=e4bfd6b03e50de46b47abfbd1e46b384f0c5f833
XWIN_VERSION=0.10.0
declare -A XWIN_SHA256=(
    [x86_64]=d870eb4b2f390878af6da1ccd3cf321d22fcb72720984853b4be732ae597fc88
    [aarch64]=6d56d28537a86f37aa3d041318898f25ee3100c6b6ec332ad873c28faf37be23
)
MSVC_CRT_VERSION=14.44.17.14
WINDOWS_SDK_VERSION=10.0.26100

LICENSE_MESSAGE='the MSVC CRT and Windows SDK are under the Microsoft license and are not redistributed.
Re-run with --accept-msvc-license to download them with xwin (https://github.com/Jake-Shadle/xwin),
or pass --msvc-sdk PATH to reuse an existing xwin "splat" tree.'

ACCEPT_LICENSE=
MSVC_SDK=
XWIN=
XWIN_CACHE=
DOWNLOAD_DIR=

die() {
    printf 'setup: %s\n' "$*" >&2
    exit 1
}

run() {
    printf '+ %s\n' "$*"
    "$@"
}

usage() {
    cat <<TEXT
Fetches and prepares everything the Linux cross-build needs into deps/ (or \$CIV_DEPS):
Microsoft Detours v4.0.1 and the MSVC CRT 14.44 with the Windows SDK 10.0.26100 for x86 (Microsoft license)
from xwin 0.10.0.

Usage: bash cmake/setup.sh [options]
  --accept-msvc-license  download the MSVC CRT and Windows SDK with xwin
  --xwin PATH            the xwin executable (default: xwin $XWIN_VERSION from PATH, else downloaded into deps/.xwin)
  --xwin-cache PATH      keep the xwin download cache there (default: deps/.xwin-cache, removed after success)
  --msvc-sdk PATH        reuse an existing xwin splat tree instead of downloading
  -h, --help             show this text
TEXT
}

need_value() {
    (( $# > 1 )) || die "argument $1: expected one argument"
}

parse_args() {
    while (( $# )); do
        if [[ $1 == --*=* ]]; then
            set -- "${1%%=*}" "${1#*=}" "${@:2}"
        fi
        case $1 in
            --accept-msvc-license) ACCEPT_LICENSE=1; shift ;;
            --msvc-sdk) need_value "$@"; MSVC_SDK=$2; shift 2 ;;
            --xwin) need_value "$@"; XWIN=$2; shift 2 ;;
            --xwin-cache) need_value "$@"; XWIN_CACHE=$2; shift 2 ;;
            -h | --help) usage; exit 0 ;;
            *) die "unrecognized argument: $1 (see --help)" ;;
        esac
    done
}

sha256() {
    local sum
    sum=$(sha256sum < "$1")
    printf '%s' "${sum%% *}"
}

fetch() {
    local name=$1 repo=$2 tag=$3 commit=$4 target=$DEPS/$1 head eol
    if [[ -d $target/.git ]]; then
        head=$(git -C "$target" rev-parse HEAD 2>/dev/null) || head=
        eol=$(git -C "$target" ls-files --eol 2>/dev/null) || eol=w/crlf
        if [[ $head == "$commit" && $eol != *w/crlf* ]] && git -C "$target" diff --quiet HEAD -- 2>/dev/null; then
            printf '%s %s is already in %s\n' "$name" "$tag" "$target"
            return
        fi
    fi
    rm -rf -- "$target"
    run git -c advice.detachedHead=false -c core.autocrlf=false -c core.eol=lf clone --depth 1 --branch "$tag" "$repo" "$target" ||
        die "cannot clone $repo $tag"
    head=$(git -C "$target" rev-parse HEAD)
    [[ $head == "$commit" ]] || die "$name $tag resolved to $head, expected $commit"
}

msvc_complete() {
    [[ -d $1/crt/include && -d $1/sdk/lib/um/x86 ]]
}

msvc_ready() {
    local target=$DEPS/msvc-sdk versions
    msvc_complete "$target" || return 1
    if [[ -L $target ]]; then
        return 0
    fi
    [[ -f $target/.civ-versions ]] || return 1
    versions=$(<"$target/.civ-versions")
    [[ $versions == "$MSVC_CRT_VERSION $WINDOWS_SDK_VERSION" ]]
}

check_msvc_tree() {
    MSVC_SDK=$(realpath -m -- "$MSVC_SDK")
    msvc_complete "$MSVC_SDK" || die "$MSVC_SDK is not an xwin-style tree (crt/include, sdk/lib/um/x86)"
}

check_deps_folder() {
    if [[ -e $DEPS && ! -e $DEPS/.setup-stamp && -n $(ls -A -- "$DEPS" 2>/dev/null) ]]; then
        die "$DEPS is not empty and was not prepared by this script; CIV_DEPS must be a folder used only for these dependencies"
    fi
}

link_msvc_sdk() {
    local target=$1
    if [[ $(realpath -m -- "$target") == "$MSVC_SDK" ]]; then
        printf 'MSVC CRT/SDK is already in %s\n' "$target"
    elif [[ -d $target && ! -L $target ]]; then
        printf 'setup: warning: %s is a folder, not a link; it is kept and %s is not linked\n' "$target" "$MSVC_SDK" >&2
    else
        rm -f -- "$target"
        ln -sT -- "$MSVC_SDK" "$target"
        printf 'MSVC CRT/SDK: %s -> %s\n' "$target" "$MSVC_SDK"
    fi
}

xwin_version() {
    local version words
    version=$("$1" --version 2>/dev/null) || return 1
    read -r -d '' -a words <<< "$version" || true
    printf '%s' "${words[*]: -1}"
}

download_xwin() {
    local arch name url sha dir=$DEPS/.xwin
    arch=$(uname -m)
    sha=${XWIN_SHA256[$arch]:-}
    [[ -n $sha ]] || die "no pinned xwin $XWIN_VERSION build for $arch; pass --xwin PATH"
    name=xwin-$XWIN_VERSION-$arch-unknown-linux-musl
    XWIN=$dir/$name/xwin
    if [[ -x $XWIN ]]; then
        return
    fi
    url=https://github.com/Jake-Shadle/xwin/releases/download/$XWIN_VERSION/$name.tar.gz
    printf '+ download %s\n' "$url"
    mkdir -p -- "$dir"
    DOWNLOAD_DIR=$(mktemp -d "$dir/.download.XXXXXX")
    trap 'rm -rf -- "$DOWNLOAD_DIR"' EXIT
    curl -fsSL --connect-timeout 120 -o "$DOWNLOAD_DIR/xwin.tar.gz" "$url" || die "cannot download $url"
    [[ $(sha256 "$DOWNLOAD_DIR/xwin.tar.gz") == "$sha" ]] || die "$name.tar.gz does not match the expected SHA-256"
    tar -xzf "$DOWNLOAD_DIR/xwin.tar.gz" -C "$DOWNLOAD_DIR" "$name/xwin" || die "cannot unpack $name.tar.gz"
    rm -rf -- "${dir:?}/$name"
    mv -- "$DOWNLOAD_DIR/$name" "$dir/$name"
    rm -rf -- "$DOWNLOAD_DIR"
}

resolve_xwin() {
    local found
    if [[ -n $XWIN ]]; then
        found=$(xwin_version "$XWIN") || die "$XWIN --version failed"
        if [[ $found != "$XWIN_VERSION" ]]; then
            printf 'setup: warning: tested with xwin %s, found %s\n' "$XWIN_VERSION" "${found:-an unknown version}" >&2
        fi
    elif found=$(command -v xwin) && [[ $(xwin_version "$found") == "$XWIN_VERSION" ]]; then
        XWIN=$found
    else
        download_xwin
    fi
}

run_xwin() {
    local target=$1 partial=$DEPS/.msvc-sdk.partial cache=$DEPS/.xwin-cache
    resolve_xwin
    if [[ -n $XWIN_CACHE ]]; then
        cache=$(realpath -m -- "$XWIN_CACHE")
    fi
    rm -rf -- "$partial"
    run "$XWIN" --accept-license --arch x86 --manifest-version 17 --crt-version "$MSVC_CRT_VERSION" \
        --sdk-version "$WINDOWS_SDK_VERSION" --cache-dir "$cache" splat --output "$partial" || die 'xwin failed'
    msvc_complete "$partial" || die "xwin left no crt/include and sdk/lib/um/x86 in $partial"
    printf '%s %s\n' "$MSVC_CRT_VERSION" "$WINDOWS_SDK_VERSION" > "$partial/.civ-versions"
    if [[ -L $target || -f $target ]]; then
        rm -f -- "$target"
    else
        rm -rf -- "$target"
    fi
    mv -T -- "$partial" "$target"
}

prepare_msvc_sdk() {
    local target=$DEPS/msvc-sdk
    if [[ -n $MSVC_SDK ]]; then
        link_msvc_sdk "$target"
    elif msvc_ready; then
        printf 'MSVC CRT/SDK is already in %s\n' "$target"
    else
        run_xwin "$target"
    fi
    remove_leftovers
}

remove_leftovers() {
    local path resolved
    for path in "$DEPS/.msvc-sdk.partial" "$DEPS/.xwin-cache"; do
        resolved=$(realpath -m -- "$path")
        if [[ -n $MSVC_SDK && $MSVC_SDK/ == "$resolved"/* ]]; then
            continue
        fi
        if [[ -n $XWIN_CACHE && $(realpath -m -- "$XWIN_CACHE")/ == "$resolved"/* ]]; then
            continue
        fi
        rm -rf -- "$path"
    done
}

main() {
    parse_args "$@"
    if [[ -n $MSVC_SDK ]]; then
        check_msvc_tree
    elif [[ -z $ACCEPT_LICENSE ]] && ! msvc_ready; then
        die "$LICENSE_MESSAGE"
    fi
    check_deps_folder
    mkdir -p -- "$DEPS"
    printf 'incomplete\n' > "$DEPS/.setup-stamp"
    fetch Detours "$DETOURS_REPO" "$DETOURS_TAG" "$DETOURS_COMMIT"
    prepare_msvc_sdk
    printf '%s\n' "$(sha256 "$SCRIPT")" > "$DEPS/.setup-stamp"
    printf 'Dependencies are ready in %s\n' "$DEPS"
}

main "$@"

