#!/usr/bin/env bash
# Clone and build coddog (ethteck, https://github.com/ethteck/coddog), a Rust
# tool that finds matching code between binaries by opcode-sequence hashing.
# See docs/skeleton-scan.md's "coddog" section for status, findings, and how
# it compares to tools/skeleton_scan.py.
#
# Builds outside this repo: ~/Desktop/dev/coddog. Never modifies Mickey's
# tree or any reference decomp's tree.
#
#   tools/setup_coddog.sh
#
# Requires: cargo (this project's setup does not install one; the working
# invocation during development was /opt/homebrew/bin/cargo 1.97).
set -euo pipefail

CODDOG_DIR="${CODDOG_DIR:-$HOME/Desktop/dev/coddog}"
CARGO="${CARGO:-cargo}"

if [ -d "$CODDOG_DIR/.git" ]; then
  echo "coddog already cloned at $CODDOG_DIR -- skipping clone" >&2
else
  git clone https://github.com/ethteck/coddog "$CODDOG_DIR"
fi

# Only the CLI binary is needed for compare-raw/match/etc.; the api/db crates
# pull in Postgres (sqlx) and are not required. SQLX_OFFLINE keeps sqlx's
# compile-time query checking from trying to reach a live database -- the
# repo ships a `.sqlx` query cache, so this works without installing Postgres.
SQLX_OFFLINE=true "$CARGO" build --release --manifest-path "$CODDOG_DIR/Cargo.toml" -p coddog-cli

echo "built: $CODDOG_DIR/target/release/coddog"
echo "see docs/skeleton-scan.md for decomp.yaml setup and usage against Mickey's ROM"
