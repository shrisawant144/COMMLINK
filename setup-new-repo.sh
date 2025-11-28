#!/bin/bash

echo "╔══════════════════════════════════════════════════════════════════╗"
echo "║         Create New GitHub Repository for CommLink               ║"
echo "╚══════════════════════════════════════════════════════════════════╝"
echo ""

# Get GitHub username
read -p "Enter your GitHub username: " GITHUB_USER

# Repository name
REPO_NAME="CommLink"

echo ""
echo "Creating repository: $GITHUB_USER/$REPO_NAME"
echo ""

# Instructions for manual creation
cat << EOF
═══════════════════════════════════════════════════════════════════

MANUAL STEPS TO CREATE REPOSITORY:

1. Go to: https://github.com/new

2. Repository settings:
   - Repository name: CommLink
   - Description: Professional network communication tool with multi-format support
   - Visibility: Public (or Private)
   - ✓ Add README (uncheck - we have one)
   - ✓ Add .gitignore (uncheck - we have one)
   - ✓ Choose a license (uncheck - we have MIT)

3. Click "Create repository"

4. After creation, run these commands:

   git remote add origin https://github.com/$GITHUB_USER/CommLink.git
   git branch -M main
   git push -u origin main

   OR with SSH:

   git remote add origin git@github.com:$GITHUB_USER/CommLink.git
   git branch -M main
   git push -u origin main

═══════════════════════════════════════════════════════════════════

ALTERNATIVE: Using GitHub CLI (if installed)

1. Install GitHub CLI:
   curl -fsSL https://cli.github.com/packages/githubcli-archive-keyring.gpg | sudo dd of=/usr/share/keyrings/githubcli-archive-keyring.gpg
   echo "deb [arch=\$(dpkg --print-architecture) signed-by=/usr/share/keyrings/githubcli-archive-keyring.gpg] https://cli.github.com/packages stable main" | sudo tee /etc/apt/sources.list.d/github-cli.list > /dev/null
   sudo apt update
   sudo apt install gh

2. Authenticate:
   gh auth login

3. Create and push:
   gh repo create CommLink --public --source=. --remote=origin --push

═══════════════════════════════════════════════════════════════════

Press Enter to continue...
EOF

read

# Provide quick commands
echo ""
echo "Quick setup commands (after creating repo on GitHub):"
echo ""
echo "# For HTTPS:"
echo "git remote add origin https://github.com/$GITHUB_USER/CommLink.git"
echo "git push -u origin main"
echo ""
echo "# For SSH:"
echo "git remote add origin git@github.com:$GITHUB_USER/CommLink.git"
echo "git push -u origin main"
echo ""
