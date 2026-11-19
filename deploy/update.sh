#!/bin/sh
set -eu
cd "$HOME/durak"
export DURAK_REGISTRY=ghcr.io/kxigor
latest=$(curl --fail --silent --show-error --max-time 20 \
  'https://api.github.com/repos/kxigor/Durak/actions/workflows/web.yml/runs?branch=main&event=push&status=success&per_page=1' |
  jq -er '.workflow_runs[0].head_sha | select(type == "string" and test("^[0-9a-f]{40}$"))')
[ "${#latest}" -eq 40 ] || exit 1
previous=$(cat .deployed 2>/dev/null || true)
[ "$latest" != "$previous" ] || exit 0
export DURAK_TAG="$latest"
docker compose pull
if docker compose up -d --no-build --force-recreate --wait --wait-timeout 75 &&
   curl --fail --silent --max-time 5 http://127.0.0.1:8189/api/health >/dev/null; then
  printf '%s\n' "$latest" >.deployed.new
  mv .deployed.new .deployed
else
  if [ -n "$previous" ]; then
    DURAK_TAG="$previous" docker compose up -d --no-build --force-recreate --wait --wait-timeout 75
  fi
  exit 1
fi
