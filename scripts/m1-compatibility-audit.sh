#!/usr/bin/env bash
# KiloNode - Developed by M6VPN (M6VPN@tuta.com)
# kilonode/scripts/m1-compatibility-audit.sh

set -eu

status=0
compat="./build/kilonode-compat"

run_script()
{
	script="$1"
	if [ ! -x "$script" ]; then
		printf 'FAIL missing %s\n' "$script"
		status=1
		return
	fi
	if ! "$script"; then
		status=1
	fi
}

run_compat()
{
	if [ ! -x "$compat" ]; then
		printf 'FAIL missing %s; run cmake --build build\n' "$compat"
		status=1
		return
	fi
	if ! "$compat" "$@"; then
		status=1
	fi
}

printf 'M1-COMPAT clean_room=true linbpq_run=false gpl_source=false\n'
run_script ./scripts/compat-validate-observation-pack.sh
run_script ./scripts/compat-check-node-plan.sh
run_script ./scripts/compat-replay-fixtures.sh
run_script ./scripts/compat-validate-captures.sh
run_compat check-pack tests/fixtures/compat/linbpq-node/manifest.pack
run_compat check-requirements tests/fixtures/compat/linbpq-node/requirements.plan
run_compat check-command-profiles tests/fixtures/compat/linbpq-node/command-profiles.plan

if [ "$status" -eq 0 ]; then
	printf 'OK m1-compatibility-audit fixtures=true observations=true linbpq_run=false\n'
fi

exit "$status"
