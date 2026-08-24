#!/usr/bin/env bash
# Fetches the objdiff-cli binary (encounter/objdiff releases) into
# tools/objdiff/. Gitignored -- like tools/ido/ and tools/binutils/, this is
# a fetched third-party binary, never committed.
#
#   ./tools/setup_objdiff.sh          # latest release
#   OBJDIFF_VERSION=v3.8.0 ./tools/setup_objdiff.sh   # pin a version
#
# Only macOS arm64 is wired up here (this project's dev machine); add the
# x86_64/Linux asset names below if this ever needs to run elsewhere.
set -eu
cd "$(dirname "$0")/.."

VERSION="${OBJDIFF_VERSION:-latest}"
OUT=tools/objdiff
mkdir -p "$OUT"

case "$(uname -s)-$(uname -m)" in
  Darwin-arm64)
    ASSET="objdiff-cli-macos-arm64"
    ;;
  Darwin-x86_64)
    ASSET="objdiff-cli-macos-x86_64"
    ;;
  *)
    echo "$0: no known objdiff-cli asset for $(uname -s)-$(uname -m)." >&2
    echo "  See https://github.com/encounter/objdiff/releases for the full asset list." >&2
    exit 1
    ;;
esac

if [ "$VERSION" = "latest" ]; then
    URL="https://github.com/encounter/objdiff/releases/latest/download/$ASSET"
    resolved=$(curl -sI "$URL" | grep -i '^location:' | tail -1 | sed -E 's#.*/(v[0-9.]+)/.*#\1#' | tr -d '\r')
else
    URL="https://github.com/encounter/objdiff/releases/download/$VERSION/$ASSET"
    resolved="$VERSION"
fi

echo "Fetching $ASSET ($VERSION) ..."
curl -fL "$URL" -o "$OUT/objdiff-cli"
chmod +x "$OUT/objdiff-cli"

# Record the resolved version next to the binary (gitignored along with it;
# tools/objdiff_report.sh and tools/check_tools.sh print it via --version
# instead, this is just a quick human-readable note).
{
  echo "objdiff-cli"
  echo "asset:   $ASSET"
  echo "version: ${resolved:-$VERSION}"
  echo "fetched: $(date -u +%Y-%m-%dT%H:%M:%SZ)"
} > "$OUT/VERSION"

"$OUT/objdiff-cli" --version
echo "Installed to $OUT/objdiff-cli"
