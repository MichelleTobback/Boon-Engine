#!/usr/bin/env bash
set -e

cd "$(dirname "$0")/.."

PROFILE="${1:-Linux-Debug}"
CONFIG="Debug"

case "$PROFILE" in
    *Release*) CONFIG="Release" ;;
esac

echo "========================================"
echo "Boon Generate Project"
echo "Profile: $PROFILE"
echo "Config : $CONFIG"
echo "========================================"

echo "========================================"
echo "Building BoonBuild"
echo "========================================"

cmake -S Boon/Tools/BoonBuild \
      -B out/build/BoonBuild-Debug \
      -G Ninja \
      -DCMAKE_BUILD_TYPE=Debug

cmake --build out/build/BoonBuild-Debug

echo "========================================"
echo "Running BoonBuild"
echo "========================================"

./bin/Debug/Tools/BoonBuild . --profile "$PROFILE"

echo "========================================"
echo "Configuring CMake"
echo "========================================"

cmake -S . \
      -B "out/build/$PROFILE" \
      -G Ninja \
      -DCMAKE_BUILD_TYPE="$CONFIG" \
      -DBOON_BUILD_PROFILE="$PROFILE"

echo "========================================"
echo "Done"
echo "========================================"