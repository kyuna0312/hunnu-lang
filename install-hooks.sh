#!/bin/bash
# Install git hooks - Run this to enable automatic linting on commit
# Usage: ./install-hooks.sh

set -e

HOOKS_DIR=".git/hooks"
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"

echo "Installing Hunnu git hooks..."

# Install pre-commit hook
if [ -f "$SCRIPT_DIR/.git/hooks/pre-commit" ]; then
    cp "$SCRIPT_DIR/.git/hooks/pre-commit" "$HOOKS_DIR/pre-commit"
    chmod +x "$HOOKS_DIR/pre-commit"
    echo "✓ Installed pre-commit hook"
fi

echo ""
echo "Hooks installed! They will run automatically before each commit."
echo "To remove: rm .git/hooks/pre-commit"