#!/bin/bash

# Import multiple packages into monorepo while preserving git history
# Usage: ./import-packages.sh

set -euo pipefail

# Array of packages: "name|git_url|branch"
PACKAGES=(
"ecal_calibration|ssh://git@gitlab.cern.ch:7999/rx_run3/ecal_calibration.git|master"
"ap_utilities|ssh://git@gitlab.cern.ch:7999/rx_run3/ap_utilities.git|master"
"dmu|ssh://git@gitlab.cern.ch:7999/rx_run3/dmu.git|master"
"rx_data|ssh://git@gitlab.cern.ch:7999/rx_run3/rx_data.git|master"
"post_ap|ssh://git@gitlab.cern.ch:7999/rx_run3/post_ap.git|master"
"rx_common|ssh://git@gitlab.cern.ch:7999/rx_run3/rx_common.git|master"
"rx_calibration|ssh://git@gitlab.cern.ch:7999/rx_run3/rx_calibration.git|master"
"rx_efficiencies|ssh://git@gitlab.cern.ch:7999/rx_run3/rx_efficiencies.git|master"
"rx_classifier|ssh://git@gitlab.cern.ch:7999/rx_run3/rx_classifier.git|master"
"fitter|ssh://git@gitlab.cern.ch:7999/rx_run3/fitter.git|master"
"rx_misid|ssh://git@gitlab.cern.ch:7999/rx_run3/rx_misid.git|master"
"rx_pid|ssh://git@gitlab.cern.ch:7999/rx_run3/rx_pid.git|master"
"rx_plots|ssh://git@gitlab.cern.ch:7999/rx_run3/rx_plots.git|master"
"rx_q2|ssh://git@gitlab.cern.ch:7999/rx_run3/rx_q2.git|master"
"rx_selection|ssh://git@gitlab.cern.ch:7999/rx_run3/rx_selection.git|master"
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
