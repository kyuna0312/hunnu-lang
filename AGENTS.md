# AGENTS.md - Hunnu Language Development Guidelines

This file provides guidance for agentic coding agents working on hunnu-lang.

## Project Overview

hunnu-lang is a programming language project. The codebase structure is being established.

---

## Build Commands

npm is used as the package manager.

### Main Commands

```bash
# Install dependencies
npm install

# Build the project
npm run build

# Run the compiler/interpreter
npm run start

# Run in development mode with hot reload
npm run dev
```

### Linting

```bash
# Run linter
npm run lint

# Fix linting issues automatically
npm run lint:fix
```

### Testing

```bash
# Run all tests
npm test

# Run tests in watch mode
npm run test:watch

# Run tests with coverage
npm run test:coverage

# Run a single test file (use -- flag with test runner)
npm test -- --testPathPattern="filename.test"
# or using vitest:
npx vitest run --test filename.test
```

---

## Code Style Guidelines

### General Principles

- Write clean, readable, idiomatic code
- Keep functions small and focused (single responsibility)
- Use meaningful variable and function names
- Avoid magic numbers - use constants
- Comment "why", not "what"

### Formatting

- Use 2 spaces for indentation (no tabs)
- Max line length: 100 characters
- Use trailing commas in multi-line objects/arrays
- Use single quotes for strings (double only for strings containing single quotes)
- Always use semicolons

Example:
```typescript
const config = {
  name: 'hunnu-lang',
  version: '0.1.0',
};
```

### Imports

- Use absolute imports when possible (from 'src/...')
- Group imports in this order:
  1. Node built-ins (path, fs, etc.)
  2. External libraries (third-party)
  3. Internal modules (from this project)

Example:
```typescript
import * as path from 'path';
import { Parser } from './parser';
import { Token, TokenType } from '../tokenizer';
```

### Naming Conventions

| Type | Convention | Example |
|------|-----------|---------|
| Variables | camelCase | `currentToken` |
| Constants | UPPER_SNAKE_CASE | `MAX_TOKEN_LENGTH` |
| Functions | camelCase | `parseExpression` |
| Classes | PascalCase | `Lexer` |
| Interfaces | PascalCase (prefix with I) | `INode` |
| Enums | PascalCase | `TokenType` |
| Files | kebab-case | `lexer.ts` |
| Tests | kebab-case.test | `lexer.test.ts` |

### Types

- Always use explicit types for function parameters and return values
- Prefer interfaces over types for object shapes
- Use `unknown` over `any` when type is truly unknown
- Never use `any` - use `unknown` or generic types
- Use utility types: `Partial<T>`, `Required<T>`, `Readonly<T>`

### Error Handling

- Use custom error classes extending `Error` or `Error`
- Include error codes and contextual info in errors
- Never silently swallow errors
- Use Result types for operations that can fail:

```typescript
type Result<T, E = Error> =
  | { ok: true; value: T }
  | { ok: false; error: E };
```

### Testing Guidelines

- Use descriptive test names: `should parse binary expression correctly`
- Follow AAA pattern: Arrange, Act, Assert
- Test both success and failure cases
- Use `describe` blocks for grouping related tests
- Mock external dependencies

---

## Architecture

### Directory Structure

```
src/
  tokenizer/     # Lexical analysis
  parser/        # Syntax analysis  
  ast/           # AST node definitions
  evaluator/    # Code generation/execution
  compiler/     # Compilation to target
  utils/        # Helper functions
tests/
  tokenizer/
  parser/
  ...
```

### Key Files

- `src/index.ts` - Entry point
- `src/cli.ts` - Command-line interface

---

## IDE Recommendations

- Use VS Code or Cursor
- Install recommended extensions (see .vscode/extensions.json)
- Enable "Format on Save"

---

## Common Issues

### Single Test Running

If tests fail to run with `--` syntax, try:

```bash
# For vitest
npx vitest run --testPathPattern "lexer"

# For jest  
npx jest --testPathPattern "lexer"
```

Check `package.json` for the actual test configuration.

---

## Configuration Files

- `package.json` - npm scripts and dependencies
- `tsconfig.json` - TypeScript configuration
- `jest.config.js` or `vitest.config.ts` - Test configuration
- `.eslintrc.json` - Linting rules
- `.prettierrc` - Code formatting

---

## Notes for Agents

- Always run lint/typecheck before committing
- Verify tests pass before submitting changes
- Keep PRs small and focused
- Add tests for new features
- Update this file when project setup changes