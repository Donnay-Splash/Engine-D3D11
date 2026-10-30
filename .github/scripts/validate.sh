#!/usr/bin/env bash
# Native macOS: build locally. Elsewhere: push a temp agent-validate/* branch and stream the macOS CI run.
set -euo pipefail

cd "$(git rev-parse --show-toplevel)"

if [[ "$(uname)" == "Darwin" ]]; then
  cmake -B build && cmake --build build
  exit $?
fi

command -v gh >/dev/null || { echo "gh CLI is required" >&2; exit 1; }

branch="agent-validate/$(date -u +%Y%m%d-%H%M%S)-$$"

# Snapshot the working tree (incl. uncommitted/untracked files) into a commit
# using a temporary index, so the user's branch, index and HEAD stay untouched.
tmpIndex="$(mktemp -u)"
trap 'rm -f "$tmpIndex"' EXIT
export GIT_INDEX_FILE="$tmpIndex"
git read-tree HEAD
git add -A
tree="$(git write-tree)"
unset GIT_INDEX_FILE
commit="$(git commit-tree "$tree" -p HEAD -m "agent-validate: snapshot for remote Metal validation")"

git push origin "$commit:refs/heads/$branch"
echo "Pushed $branch ($commit)"

cleanup() { git push origin --delete "$branch" >/dev/null 2>&1 || true; }
trap 'cleanup; rm -f "$tmpIndex"' EXIT

runId=""
for _ in $(seq 1 30); do
  runId="$(gh run list --branch "$branch" --limit 1 \
    --json databaseId --jq '.[0].databaseId // empty')"
  [[ -n "$runId" ]] && break
  sleep 3
done
[[ -n "$runId" ]] || { echo "No workflow run found for $branch" >&2; exit 1; }

if gh run watch "$runId" --exit-status; then
  echo "Remote macOS validation PASSED"
else
  echo "Remote macOS validation FAILED; failing logs:" >&2
  gh run view "$runId" --log-failed >&2 || true
  exit 1
fi
