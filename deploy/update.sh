#!/bin/sh
set -eu
cd "$HOME/durak"
export DURAK_REGISTRY=ghcr.io/kxigor
compose() {
  /usr/libexec/docker/cli-plugins/docker-compose -f compose.yaml -f deploy/compose.isolated.yaml "$@"
}
healthy() {
  curl --fail --silent --max-time 5 http://127.0.0.1:8189/api/health >/dev/null
}
previous=$(cat .deployed 2>/dev/null || true)
if [ -n "$previous" ] && ! healthy; then
  DURAK_TAG="$previous" compose up -d --no-build --pull never --wait --wait-timeout 75
fi
latest=$(curl --fail --silent --show-error --max-time 20 \
  'https://api.github.com/repos/kxigor/Durak/actions/workflows/web.yml/runs?branch=main&event=push&status=success&per_page=1' |
  jq -er '.workflow_runs[0].head_sha | select(type == "string" and test("^[0-9a-f]{40}$"))')
[ "${#latest}" -eq 40 ] || exit 1
if [ "$latest" = "$previous" ] && healthy; then exit 0; fi
export DURAK_TAG="$latest"
compose pull backend frontend
if compose up -d --no-build --wait --wait-timeout 75 &&
   healthy; then
  printf '%s\n' "$latest" >.deployed.new
  mv .deployed.new .deployed
else
  if [ -n "$previous" ]; then
    DURAK_TAG="$previous" compose up -d --no-build --wait --wait-timeout 75
  fi
  exit 1
fi
