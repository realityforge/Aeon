# Repository Guidelines

## Project Structure & Module Organization
- `Aeon.uplugin` declares modules; each module sits under `Source/<Module>/Public` and `Source/<Module>/Private` so shared headers stay isolated from implementation-only details.
- Core gameplay helpers live in `Source/Aeon`, AI behaviors in `Source/AeonAI`, animation utilities in `Source/AeonAnimation`, and editor-only tooling in `Source/AeonEditor`.
- Raw files (such as `.csv` files) from which Unreal assets are imported belong in `SourceContent`, while `Content` is reserved for runtime assets that ship with the plugin.
- Generated binaries and build artifacts should stay out of version control.

## Tooling & Engine Version
- Target Unreal Engine 5.6 for both development and verification; earlier engine releases are unsupported.

## Coding Style & Naming Conventions
- Follow Unreal Engine defaults: 4-space indentation, PascalCase types, camelCase locals, `FAeon*` for structs, `UAeon*` for UObject classes, and `EAeon*` for enums.
- Place new public headers under `Source/<Module>/Public/<Module>/` and implementation files under the matching `Private` path.
- Prefer UE logging macros with the `LogAeon` category; declare new categories in module `Private` headers when needed.

## Testing Guidelines
- Automation coverage is aspirational. Capture edge cases in unit-style specs once a testing harness lands under `Source/<Module>/Private/Tests/`.
- Until formal suites exist, document manual reproduction steps or sample maps in the pull request so reviewers can exercise the change.

## Commit & Pull Request Guidelines
- Write imperative, present-tense commit messages under 72 characters, mirroring existing history such as “Add accessor for AeonAbilitySystemComponent”.
- Squash noisy work-in-progress commits locally; each change should stand on its own.
- Open pull requests with a clear summary, reproduction or test notes, and screenshots or GIFs when changes impact in-editor UX.
