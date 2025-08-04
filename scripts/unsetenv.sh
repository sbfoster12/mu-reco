#!/bin/bash

# Get absolute path to the root of the repo (assuming this script is in scripts/)
export MU_RECO_PATH="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
export MU_RECO_CMAKE_PREFIX_PATH="$MU_RECO_PATH/install"

# Function to remove a path from an environment variable
remove_from_path() {
    local var_name=$1
    local path_to_remove=$2
    local current_value=$(eval echo \$$var_name)
    
    # Remove the path and clean up any double colons
    local new_value=$(echo "$current_value" | awk -v RS=: -v ORS=: -v rm="$path_to_remove" '$0 != rm' | sed 's/:$//')

    # Set the new value
    eval export $var_name="$new_value"
    
    # If the variable is now empty, unset it
    if [ -z "$(eval echo \$$var_name)" ]; then
        unset $var_name
    fi
}

# Remove paths that were added by setenv.sh
remove_from_path "ROOT_INCLUDE_PATH" "$MU_RECO_PATH/install/include"
remove_from_path "LD_LIBRARY_PATH" "$MU_RECO_PATH/install/lib"
remove_from_path "DYLD_LIBRARY_PATH" "$MU_RECO_PATH/install/lib"
remove_from_path "CMAKE_PREFIX_PATH" "$MU_RECO_CMAKE_PREFIX_PATH"

# Remove nlohmann_json paths if they were added
NLOHMANN_PATHS=$(pkg-config --cflags nlohmann_json 2>/dev/null | sed 's/-I//g')
if [ ! -z "$NLOHMANN_PATHS" ]; then
    for path in $NLOHMANN_PATHS; do
        remove_from_path "ROOT_INCLUDE_PATH" "$path"
    done
fi

echo "Environment paths removed for $MU_RECO_PATH"

# Finally unset the MU_RECO_PATH variable itself
unset MU_RECO_PATH
unset MU_RECO_CMAKE_PREFIX_PATH