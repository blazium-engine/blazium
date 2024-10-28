#!/usr/bin/env sh

set -euo pipefail
IFS=$'\n\t'

# Check currently installed and latest available Vulkan SDK versions.
if [ -d "$HOME/VulkanSDK" ]; then
	if command -v jq 2>&1 >/dev/null; then
		curl -L "https://sdk.lunarg.com/sdk/download/latest/mac/config.json" -o /tmp/vulkan-sdk.json

		new_ver=`jq -r '.version' /tmp/vulkan-sdk.json`
		new_ver=`echo "$new_ver" | awk -F. '{ printf("%d%02d%04d%02d\n", $1,$2,$3,$4); }';`

		rm -f /tmp/vulkan-sdk.json

		for f in $HOME/VulkanSDK/*; do
			if [ -d "$f" ]; then
				f=`echo "${f##*/}" | awk -F. '{ printf("%d%02d%04d%02d\n", $1,$2,$3,$4); }';`
				if [ $f -ge $new_ver ]; then
					echo 'Latest or newer Vulkan SDK is already installed. Skipping installation.'
					exit 0
				fi
			fi
		done
	fi
fi

# Download and install the Vulkan SDK.
curl -L "https://sdk.lunarg.com/sdk/download/latest/mac/vulkan-sdk.dmg" -o /tmp/vulkan-sdk.dmg
hdiutil attach /tmp/vulkan-sdk.dmg -mountpoint /Volumes/vulkan-sdk
/Volumes/vulkan-sdk/InstallVulkan.app/Contents/MacOS/InstallVulkan \
    --accept-licenses --default-answer --confirm-command install

cnt=5
until hdiutil detach -force /Volumes/vulkan-sdk
do
   [[ cnt -eq "0" ]] && break
   sleep 1
   ((cnt--))
done

rm -f /tmp/vulkan-sdk.dmg

echo 'Vulkan SDK installed successfully! You can now build Godot by running "scons".'
