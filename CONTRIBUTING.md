# Contributing to CommLink

## Development Setup

```bash
# Clone repository
git clone <repository-url>
cd commlink

# Install dependencies
./scripts/install-deps.sh

# Build
./scripts/build.sh

# Run tests
cd build && ctest
```

## Code Style

- Follow C++17 standards
- Use clang-format for formatting: `./scripts/format.sh`
- Run static analysis before committing
- Write unit tests for new features

## Commit Messages

Follow conventional commits:
- `feat:` New feature
- `fix:` Bug fix
- `docs:` Documentation changes
- `test:` Test additions/changes
- `refactor:` Code refactoring
- `chore:` Maintenance tasks

## Pull Request Process

1. Create feature branch from `develop`
2. Write tests for new functionality
3. Ensure all tests pass
4. Update documentation
5. Submit PR with clear description
6. Address review comments

## Code Review Guidelines

- Code must pass CI checks
- Maintain test coverage above 80%
- Follow existing architecture patterns
- Document public APIs with Doxygen
