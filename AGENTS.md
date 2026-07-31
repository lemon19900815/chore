# AGENTS.md

This is a personal learning repository ("杂货铺") - a collection of code samples, notes, and utilities. Not a deployable application.

## Structure

- `base/` - Technical notes and tutorials (C++, Linux, Docker, etc.)
- `py/` - Python utility scripts (ssh, sftp, mysql, excel conversion)
- `cpp-template/` - C++ template techniques (SFINAE, EBCO, CRTP)
- `qt/` - Qt framework examples and utilities
- `net-engine/` - Network engine based on redis2.8 (CMake build)
- `gof-sample/` - Design pattern examples (C#, C++)
- `algorithm/` - Sorting algorithms (insert, shell, merge, quick, heap, topK)
- `timer/` - Timer implementations (time-wheel, min-heap)
- `behavior-tree/` - C# behavior tree for Unity3D
- `reading-note/` - Book notes (DDIA, Linux kernel, etc.)
- `dl/`, `AI/` - Deep learning and AI notes

## Build

- **net-engine**: `cd net-engine/build && cmake .. && make` (Linux only; Windows MSVC not working)
- **qt/qt_study**: CMake-based Qt project
- Other C++ projects: Visual Studio `.sln` files in each project directory

## Notes

- No package managers (npm, pip, cargo) at root level
- Each subdirectory is independent - no unified build or test commands
- No AGENTS.md, CLAUDE.md, or similar instruction files previously existed

## Agent skills

### Issue tracker

Issues live in this repo's GitHub Issues (lemon19900815/chore), via the `gh` CLI. See `docs/agents/issue-tracker.md`.

### Triage labels

Default label vocabulary — the five canonical roles used verbatim. See `docs/agents/triage-labels.md`.

### Domain docs

Single-context: one `CONTEXT.md` + `docs/adr/` at the repo root. See `docs/agents/domain.md`.