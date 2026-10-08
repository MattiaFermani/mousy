#!/usr/bin/env bash
set -e

# ==============================================================================
# Mousy Automated Release Tool
# Follows Semantic Versioning 2.0.0: MAJOR.MINOR.PATCH
# Usage:
#   ./scripts/release.sh [patch|minor|major] [optional description]
# ==============================================================================

DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$DIR"

TYPE="${1:-patch}"
CUSTOM_DESC="${2:-Automated release update}"

# Read current version
if [ -f "VERSION" ]; then
    CURRENT=$(cat VERSION | tr -d '[:space:]')
else
    CURRENT="1.0.0"
fi

IFS='.' read -r MAJOR MINOR PATCH <<< "$CURRENT"

case "$TYPE" in
    major|MAJOR)
        MAJOR=$((MAJOR + 1))
        MINOR=0
        PATCH=0
        SEMVER_REASON="MAJOR: Incompatible architectural change or fundamental engine rewrite."
        ;;
    minor|MINOR)
        MINOR=$((MINOR + 1))
        PATCH=0
        SEMVER_REASON="MINOR: Subversion feature addition (new studio tabs, lighting engines, hardware tools)."
        ;;
    patch|PATCH)
        PATCH=$((PATCH + 1))
        SEMVER_REASON="PATCH: Revision maintenance, calibration adjustments, UI polish, or bug fixes."
        ;;
    *)
        echo "Error: Unknown release type '$TYPE'. Use 'major', 'minor', or 'patch'."
        exit 1
        ;;
esac

NEW_VERSION="${MAJOR}.${MINOR}.${PATCH}"
TAG="v${NEW_VERSION}"

echo "=========================================================="
echo "  Bumping version: ${CURRENT}  ==>  ${NEW_VERSION} (${TAG})"
echo "  Reason: ${SEMVER_REASON}"
echo "=========================================================="

# 1. Update VERSION file
echo "$NEW_VERSION" > VERSION

# 2. Re-run CMake to generate updated Version.h
cmake -B build -S .
cmake --build build

# 3. Commit version bump
git add -A
git commit -m "chore(release): bump version to ${TAG} [${TYPE}]

SemVer Meaning:
${SEMVER_REASON}

Summary:
${CUSTOM_DESC}" || true

# 4. Create annotated Git tag
git tag -a "${TAG}" -m "Mousy ${TAG}

Semantic Versioning Breakdown:
• MAJOR (${MAJOR}): Core architecture
• MINOR (${MINOR}): Subversion / Feature set
• PATCH (${PATCH}): Maintenance & bug fixes

${CUSTOM_DESC}"

# 5. Push to GitHub
echo "Pushing commits and tags to GitHub..."
git push origin master
git push origin "${TAG}"

# 6. Package release binary
PKG_DIR="mousy-${TAG}-linux-x86_64"
mkdir -p "${PKG_DIR}"
cp build/Mousy "${PKG_DIR}/"
cp README.md "${PKG_DIR}/"
cp build.sh "${PKG_DIR}/"
cat << 'EOF' > "${PKG_DIR}/run.sh"
#!/bin/bash
DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
export LD_LIBRARY_PATH="/usr/lib/x86_64-linux-gnu:$LD_LIBRARY_PATH"
exec "$DIR/Mousy" "$@"
EOF
chmod +x "${PKG_DIR}/run.sh" "${PKG_DIR}/Mousy"
tar -czvf "${PKG_DIR}.tar.gz" "${PKG_DIR}"
sha256sum "${PKG_DIR}.tar.gz" > "${PKG_DIR}.tar.gz.sha256"

# 7. Create GitHub Release via gh CLI if authenticated
if command -v gh &> /dev/null && gh auth status &> /dev/null; then
    echo "Creating GitHub Release on https://github.com/MattiaFermani/mousy/releases ..."
    RELEASE_NOTES="## 🖱 Mousy ${TAG}

### 📐 Semantic Versioning Meaning
- **MAJOR (${MAJOR})**: Core engine & architectural foundation.
- **MINOR (${MINOR})**: Subversion additions: Interactive Button Mapper, RGB Lighting Studio, Live Heatmap, Pro Aim Trainer, Macro Studio.
- **PATCH (${PATCH})**: Revision maintenance, precision calibration, and UI polish.

**Category**: \`${TYPE}\` — ${SEMVER_REASON}

---

### 📝 Release Details
${CUSTOM_DESC}

---

### 📦 Installation
Download and extract \`${PKG_DIR}.tar.gz\`:
\`\`\`bash
tar -xzvf ${PKG_DIR}.tar.gz
cd ${PKG_DIR}
./run.sh
\`\`\`
"
    gh release create "${TAG}" \
        "${PKG_DIR}.tar.gz" \
        "${PKG_DIR}.tar.gz.sha256" \
        --title "Mousy ${TAG}" \
        --notes "$RELEASE_NOTES"
    echo "✅ Release ${TAG} published successfully!"
fi

# Clean up local package folder
rm -rf "${PKG_DIR}"

echo "=========================================================="
echo "  Release ${TAG} complete!"
echo "=========================================================="
