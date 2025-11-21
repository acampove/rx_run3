#!/bin/bash

# Import multiple packages into monorepo while preserving git history
# Usage: ./import-packages.sh

set -euo pipefail

# Array of packages: "name|git_url|branch"
PACKAGES=(
    "ap_utilities|ssh://git@gitlab.cern.ch:7999/rx_run3/ap_utilities.git|master"
    "dmu|ssh://git@gitlab.cern.ch:7999/rx_run3/dmu.git|master"
)

echo "Starting package import process..."

for package in "${PACKAGES[@]}"; do
    IFS='|' read -r name url branch <<< "$package"
    
    echo ""
    echo "=========================================="
    echo "Importing: $name"
    echo "=========================================="
    
    # Check if package directory already exists
    if [ -d "packages/$name" ]; then
        echo "Warning: packages/$name already exists. Skipping..."
        continue
    fi
    
    echo "Adding $name as subtree..."
    git subtree add --prefix="packages/$name" "$url" "$branch"
    
    echo "✓ Successfully imported $name"
done

echo ""
echo "=========================================="
echo "Import complete!"
echo "=========================================="
echo ""
echo "Next steps:"
echo "1. Review the imported packages"
echo "2. Update each package's pyproject.toml if needed"
echo "3. Run: uv sync"
echo "4. Test imports between packages"
