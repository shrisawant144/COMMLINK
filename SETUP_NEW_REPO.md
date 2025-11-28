# Setup New GitHub Repository

## Current Status

✅ Folder renamed: `/home/miko/CommLink`
✅ Old remote removed
✅ Ready for new repository

## Option 1: Manual Creation (Easiest)

### Step 1: Create Repository on GitHub

1. Go to: https://github.com/new
2. Settings:
   - **Name**: `CommLink`
   - **Description**: `Professional network communication tool with multi-format support`
   - **Visibility**: Public
   - **DO NOT** initialize with README, .gitignore, or license (we have them)
3. Click **Create repository**

### Step 2: Connect and Push

```bash
cd /home/miko/CommLink

# Using HTTPS
git remote add origin https://github.com/YOUR_USERNAME/CommLink.git
git push -u origin main

# OR using SSH (recommended)
git remote add origin git@github.com:YOUR_USERNAME/CommLink.git
git push -u origin main
```

## Option 2: Using GitHub CLI

```bash
# Install GitHub CLI
curl -fsSL https://cli.github.com/packages/githubcli-archive-keyring.gpg | sudo dd of=/usr/share/keyrings/githubcli-archive-keyring.gpg
echo "deb [arch=$(dpkg --print-architecture) signed-by=/usr/share/keyrings/githubcli-archive-keyring.gpg] https://cli.github.com/packages stable main" | sudo tee /etc/apt/sources.list.d/github-cli.list > /dev/null
sudo apt update && sudo apt install gh

# Authenticate
gh auth login

# Create and push in one command
cd /home/miko/CommLink
gh repo create CommLink --public --source=. --remote=origin --push --description "Professional network communication tool with multi-format support"
```

## Option 3: Using cURL API

```bash
# Set your GitHub token
export GITHUB_TOKEN="your_personal_access_token"
export GITHUB_USER="your_username"

# Create repository
curl -X POST -H "Authorization: token $GITHUB_TOKEN" \
  -H "Accept: application/vnd.github.v3+json" \
  https://api.github.com/user/repos \
  -d '{
    "name": "CommLink",
    "description": "Professional network communication tool with multi-format support",
    "private": false
  }'

# Add remote and push
cd /home/miko/CommLink
git remote add origin https://github.com/$GITHUB_USER/CommLink.git
git push -u origin main
```

## Verify Setup

After pushing, verify:

```bash
git remote -v
git branch -a
git log --oneline -5
```

## Add CI/CD Workflow (Optional)

After first push, add the workflow file:

```bash
git add .github/workflows/ci.yml
git commit -m "ci: add GitHub Actions workflow"
git push origin main
```

**Note**: Requires Personal Access Token with `workflow` scope.

## Repository Features to Enable

After creation, go to repository settings and enable:

1. **Issues** - For bug tracking
2. **Projects** - For project management
3. **Wiki** - For additional documentation
4. **Discussions** - For community engagement
5. **Branch protection** - Protect main branch

## Next Steps

1. Add repository topics: `qt`, `cpp`, `networking`, `tcp-udp`, `json`, `xml`
2. Add repository description
3. Set up branch protection rules
4. Configure GitHub Pages (optional)
5. Add collaborators (if needed)
