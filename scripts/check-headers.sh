#!/usr/bin/env bash
# KiloNode - Developed by M6VPN (M6VPN@tuta.com)
# kilonode/scripts/check-headers.sh

set -eu

status=0

check_file()
{
	file="$1"
	header_path="kilonode/${file}"

	case "$file" in
		*.c|*.h)
			header_one="/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */"
			header_two="/* ${header_path} */"
			;;
		*.sh|*.cmake|CMakeLists.txt|*.conf)
			header_one="# KiloNode - Developed by M6VPN (M6VPN@tuta.com)"
			header_two="# ${header_path}"
			;;
		*)
			return
			;;
	esac

	if [ "${file##*.}" = "sh" ]; then
		line_one="$(sed -n '2p' "$file")"
		line_two="$(sed -n '3p' "$file")"
	else
		line_one="$(sed -n '1p' "$file")"
		line_two="$(sed -n '2p' "$file")"
	fi

	if [ "$line_one" != "$header_one" ] || [ "$line_two" != "$header_two" ]; then
		echo "missing header: $file"
		status=1
	fi
}

while IFS= read -r file; do
	check_file "$file"
done <<EOF
$(find CMakeLists.txt cmake docs/examples include scripts src tests -type f \
	\( -name 'CMakeLists.txt' -o -name '*.cmake' -o -name '*.conf' -o \
	-name '*.c' -o -name '*.h' -o -name '*.sh' \) | sort)
EOF

exit "$status"
