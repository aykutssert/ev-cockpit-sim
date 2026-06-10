# Agent Rules

Rules for any AI assistant working in this repository.

1. **Be direct.** No hedging, no padding, no "happy to help" filler. State the problem and the fix.
2. **Recommend the best approach, not the easiest one.** If the easy path produces a weaker result, say so explicitly and propose the better (even if harder) alternative.
3. **Do not avoid difficulty.** Hard but correct beats simple but shallow. If something is genuinely hard, say it's hard and do it anyway - don't quietly downgrade scope.
4. **Surface trade-offs and limitations honestly.** If an approach has a known weakness (e.g. a simplified battery model, an untested edge case, a faked download), document it instead of hiding it.
5. **No scope creep, no premature abstraction.** Build what the roadmap calls for, nothing more.
6. **Definition of done.** Writing code is not finished. The cycle is: build -> review -> fix issues -> review again. A task is only done when a follow-up review is clean - "I wrote it" is not "it's done". When done, check off the corresponding item in README's Roadmap in the same change - the roadmap is the single source of truth for progress, no separate task file.
7. **Verify currency before relying on it.** If unsure whether something is current/correct (Qt API, QML type, CMake option, tool behavior), do a web search before asserting it. Training data can be stale.
8. **Token efficiency.** Don't dump full file contents when a targeted read/grep suffices. Don't repeat code that's already visible in a diff. Avoid re-reading unchanged files. Keep explanations concise.
9. **Keep this file current.** When a convention here changes, update this file in the same change, not as a follow-up. Remove rules that no longer apply instead of letting them go stale.
10. **Scoped sub-module rules.** If a sub-directory (`bms/`, `app/`, `ota/`) develops its own conventions (build steps, QML structure, target-specific constraints), put them in an `AGENTS.md` inside that directory rather than growing this root file. Agents auto-discover nested `AGENTS.md` files.
11. **Verify the critical assumption first.** Before writing code for a new phase/task, identify the riskiest assumption it depends on (a tool/API works on this platform, a Qt module is available, a QML type behaves as expected) and check it first - small probe, web search, or test command. Don't build on top of an assumption that might collapse the whole approach. Example: before writing any UI, confirm Qt6 actually builds and launches a minimal QML window on this macOS via CMake.

## Design principles

This is a desktop simulation of an in-vehicle cockpit, not embedded firmware. Optimize for clarity and correctness of the simulation, not micro-performance.

- **Keep the simulation core Qt-free.** The BMS model is plain C++17, unit-testable without a GUI or event loop. Qt is the presentation layer, not the engine.
- **Deterministic simulation step.** The BMS advances via an explicit `step(dt)`; given the same inputs it produces the same outputs. No hidden wall-clock reads inside the model - the UI drives time.
- **One source of truth for state.** The C++ backend owns vehicle/battery state; QML reflects it through `Q_PROPERTY` + change signals. Don't duplicate state in QML.
- **Model real failure modes, not just the happy path.** Cell imbalance, overtemperature, failed/aborted OTA, checksum mismatch + rollback. A demo that only shows success is shallow.
- **No business logic in QML.** QML is layout, bindings, and animation. Decisions (fault thresholds, OTA state transitions) live in C++.

## Tooling

- Qt 6 (Homebrew `qt`), CMake (`find_package(Qt6 ...)`), C++17.
- Build: `cmake -S . -B build && cmake --build build -j`. Run the app bundle from `build/`.
- macOS host; the app is a native `.app`. No VM, no cross-compilation - this project runs directly on macOS.

## Commit & repo conventions

- All commits are authored and pushed by the repo owner. Do not add any AI/assistant attribution, co-author tags, or tool references to commit messages.
- All repo content (README, code comments, commit messages, docs) is in English only.
- No em-dash character in any repo content. Use a hyphen ("-") instead.

## Commit message format

- Subject: `<type>(<scope>): <imperative summary>`. Types: `feat`, `fix`, `refactor`, `perf`, `docs`, `test`, `chore`, `build`, `ci`. Imperative mood ("add", not "added"). No trailing period. Aim for <=50 chars, hard cap 72.
- Body only when needed: non-obvious *why*, breaking changes, migration notes. Skip if the subject is self-explanatory. Wrap at 72 chars, bullets with `-`.
- Never include: "this commit does X", AI attribution, emoji, restating the diff.
- Always include a body for: breaking changes, security fixes, data migrations, reverts.

## Code review format

When reviewing a diff/PR in this repo, output one line per finding:
`<file>:L<line>: <severity>: <problem>. <fix>.`

Severity: `bug` (broken behavior), `risk` (fragile - race, missing check, swallowed error), `nit` (style/naming, ignorable), `q` (genuine question).

Drop hedging ("perhaps", "consider"), restating the diff, and praise. Exception: security findings and architectural disagreements get a full explanation, not a one-liner.
