#!/bin/bash

# Import multiple packages into monorepo while preserving git history
# Usage: ./import-packages.sh

set -euo pipefail

# Array of packages: "name|git_url|branch"
PACKAGES=(
"root_stubs|git@github.com:acampove/root_stubs.git|master"
"zfit-stubs|git@github.com:acampove/zfit-stubs.git|master"
"vector_stubs|git@github.com:acampove/vector_stubs.git|master"
)

echo "Starting package import process..."

for package in "${PACKAGES[@]}"; do
    IFS='|' read -r name url branch <<< "$package"
    
    echo ""
    echo "=========================================="
    echo "Importing: $name"
    echo "=========================================="
    
    # Check if package directory already exists
    if [ -d "src/$name" ]; then
        echo "Warning: src/$name already exists. Skipping..."
        continue
    fi
    
    echo "Adding $name as subtree..."
    git subtree add --prefix="src/$name" "$url" "$branch"
    
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
