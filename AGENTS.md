# Repository Guidelines

## Project Structure & Module Organization
- `Aeon.uplugin` declares modules; each module sits under `Source/<Module>/Public` and `Source/<Module>/Private` so shared headers stay isolated from implementation-only details.
- Core gameplay helpers live in `Source/Aeon`, AI behaviors in `Source/AeonAI`, animation utilities in `Source/AeonAnimation`, and editor-only tooling in `Source/AeonEditor`.
- Blueprint samples and data assets belong in `SourceContent`, while `Content` is reserved for runtime assets that ship with the plugin. Generated binaries and build artifacts should stay out of version control.

## Build, Test, and Development Commands
- Use the Unreal Automation Tool to package the plugin:
  ```bash
  /path/to/UnrealEngine/Engine/Build/BatchFiles/RunUAT.sh BuildPlugin -Plugin="$PWD/Aeon.uplugin" -Package="$PWD/Dist/Aeon"
  ```
  Create the `Dist` folder before packaging.
- For iterative development, load the plugin inside a sandbox project and hot-reload with `UnrealEditor <Project>.uproject -game -log` so code and Blueprints recompile quickly.
- Run a targeted module build with `UnrealBuildTool AeonEditor Win64 Development -Project=<Project>.uproject` before opening pull requests to catch compilation regressions.

## Coding Style & Naming Conventions
- Follow Unreal Engine defaults: 4-space indentation, PascalCase types, camelCase locals, `FAeon*` for structs, `UAeon*` for UObject classes, and `EAeon*` for enums.
- Place new public headers under `Source/<Module>/Public/<Module>/` and implementation files under the matching `Private` path.
- Prefer UE logging macros with the `LogAeon` category; declare new categories in module `Private` headers when needed.

## Testing Guidelines
- Add low-level logic tests using Unreal Automation Specs under `Source/<Module>/Private/Tests/`. Mirror the module folder layout so intent stays obvious.
- Execute automation suites from the editor console with `Automation RunTests Aeon.*` and record failures in the pull request.
- When adding Blueprint utilities, provide a simple verification map in `SourceContent/Tests` that exercises the change.

## Commit & Pull Request Guidelines
- Write imperative, present-tense commit messages under 72 characters, mirroring existing history such as “Add accessor for AeonAbilitySystemComponent”.
- Squash noisy work-in-progress commits locally; each change should stand on its own.
- Open pull requests with a clear summary, reproduction or test notes, and screenshots or GIFs when changes impact in-editor UX.
