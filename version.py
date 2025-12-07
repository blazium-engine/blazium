short_name = "blazium"
name = "Blazium Engine"
major = 4
minor = 5
patch = 0
status = "stable"
module_config = ""
website = "https://blazium.app"
docs = "latest"
external_major = 0
external_minor = 8
external_patch = 0
external_status = "dev"
external_sha = "876b290332ec6f2e6d173d08162a02aa7e6ca46d"
mirror_list = "https://blazium.app/api/mirrorlist/"
version_url = "https://blazium.app/api/versions-" + external_status + ".json"
# XOR obfuscation key for pack files
# Set to None or empty string to disable functionality.
# To generate functional key, which has to be exactly
# 1024 bytes, run the following and set the output as
# a string value inside blazium_xor_key variable:
# $ tr -dc 'A-Za-z0-9' </dev/urandom | head -c 1024
blazium_xor_key = None
