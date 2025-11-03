#!/bin/bash

# Vital Synthesizer - GitHub Push Script
# This script will push your Vital synthesizer project to GitHub

echo "========================================="
echo "  Vital Synthesizer - GitHub Push"
echo "========================================="
echo ""

# Check if we're in a git repository
if [ ! -d ".git" ]; then
    echo "❌ Error: Not in a git repository!"
    echo "Please run this script from the vital_application directory"
    exit 1
fi

echo "✅ Git repository detected"
echo ""

# Set GitHub repository URL
GITHUB_REPO="https://github.com/12Matt3r/vital.git"

echo "📍 Target Repository: $GITHUB_REPO"
echo ""

# Rename branch to main if it's master
CURRENT_BRANCH=$(git branch --show-current)
if [ "$CURRENT_BRANCH" = "master" ]; then
    echo "🔄 Renaming branch 'master' to 'main'..."
    git branch -M main
    echo "✅ Branch renamed to 'main'"
else
    echo "✅ Current branch: $CURRENT_BRANCH"
fi
echo ""

# Add remote if it doesn't exist
if ! git remote | grep -q origin; then
    echo "🔗 Adding remote origin..."
    git remote add origin $GITHUB_REPO
    echo "✅ Remote added successfully"
else
    echo "✅ Remote 'origin' already exists"
    echo "   Current URL: $(git remote get-url origin)"
fi
echo ""

# Show current status
echo "📊 Current Status:"
git log --oneline -1
echo ""

# Ask for confirmation
echo "⚠️  IMPORTANT: This will push to GitHub"
echo "   Repository: $GITHUB_REPO"
echo "   Branch: $(git branch --show-current)"
echo ""
read -p "Do you want to continue? (y/N): " -n 1 -r
echo ""

if [[ ! $REPLY =~ ^[Yy]$ ]]; then
    echo "❌ Push cancelled"
    exit 0
fi

# Push to GitHub
echo ""
echo "🚀 Pushing to GitHub..."
echo ""

# Try to push
if git push -u origin $(git branch --show-current); then
    echo ""
    echo "========================================="
    echo "✅ SUCCESS! Project pushed to GitHub"
    echo "========================================="
    echo ""
    echo "🎉 Your Vital synthesizer is now on GitHub!"
    echo ""
    echo "📍 View it at:"
    echo "   https://github.com/12Matt3r/vital"
    echo ""
    echo "Next steps:"
    echo "1. Visit the repository on GitHub"
    echo "2. Update repository description and topics"
    echo "3. Enable GitHub Actions for CI/CD"
    echo "4. Share your amazing work!"
else
    echo ""
    echo "========================================="
    echo "❌ PUSH FAILED"
    echo "========================================="
    echo ""
    echo "Common reasons:"
    echo "1. Authentication failed - You may need to:"
    echo "   - Use GitHub CLI: gh auth login"
    echo "   - Or use Personal Access Token"
    echo "   - Or set up SSH keys"
    echo ""
    echo "2. Repository doesn't exist or you don't have access"
    echo "   - Create the repository on GitHub first"
    echo "   - Make sure you have push permissions"
    echo ""
    echo "3. Repository already has content"
    echo "   - Use: git push -u origin main --force"
    echo "   (Warning: This will overwrite existing content)"
    echo ""
    echo "For help, visit: https://docs.github.com/en/get-started"
    exit 1
fi
