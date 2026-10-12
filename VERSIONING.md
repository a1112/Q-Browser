# Version management

## Authorities and release units

Existing version values are retained. A value in a manifest is development/package metadata, not evidence that a release was published. Each unit is selected explicitly; matching numbers do not couple unrelated units.

- `tools-migrator`: `0.1.0` from `tools/migrator/package.json`; 1 precisely bound mirror fields.
- `tools-mock-api`: `0.1.0` from `tools/mock-api/package.json`; 1 precisely bound mirror fields.
- `tools`: `0.1.0` from `tools/package.json`; 2 precisely bound mirror fields.
- `native`: `0.1.0` from `CMakeLists.txt`; 0 precisely bound mirror fields.

## Safe local workflow

Requires Python 3.11+ (standard library only). From the repository root:

```sh
python scripts/version_manager.py show
python scripts/version_manager.py check
python scripts/test_versioning.py
python tests/test_version_manager.py
```

Preview one unit first; `bump`/`set` without `--apply` never write files:

```sh
python scripts/version_manager.py bump patch --unit tools-migrator
python scripts/version_manager.py bump minor --unit tools-migrator
```

For an intentional update, append `--expect CURRENT_VERSION --apply` to the reviewed command. Use `set NEW_VERSION --unit UNIT` for an explicit version or `bump prerelease --preid rc --unit UNIT` where the native format supports prereleases. `set` to the current version is a no-op. The helper refuses missing, ambiguous, mismatched, unsafe, or stale sources before writing. It does not invoke package managers, lifecycle hooks, Git, tags, publishing, deployment, signing or network access.

Only allowlisted own-version string tokens change. Dependency/SDK versions, lock resolution/integrity data, generated outputs and schemas are never rewritten. A mismatch should be investigated against the canonical authority before a reviewed correction; automatic guessing is intentionally refused. Review the complete diff, update the Unreleased changelog, run unit-specific native tests/builds and installer validation, then separately follow the project release process. Native installer/platform restrictions still apply; a metadata check is not a full application build.

## Preserved boundaries

CMake is the native authority. tools, migrator and mock-api are independent Node units. Signed qapkg manifest versions, update acceptance fixtures, schema versions and test packages are not the application version; build-release.ps1 is retained unchanged.

## Adoption evidence and validation

Adoption reviewed the current default `codex/q-browser-mvp` at `f8e6d54df1c6c795e24686303fa050e7dfa0e25b` and used `chore/vite-security-2026-10-11` at `45c618f37a46084f5465a12efb6177d61672b5e8` as the parent. Repair bases include the current default; earlier refs remain untouched.

GitHub tags/releases and the existing central R-Box-Releases catalog were inspected on 2026-10-12. No verified central application release mapping was found for this repository; no assumption about unpublished or external releases is made. Existing source versions are authoritative for this change. No version bump or release is part of adoption.

Offline tests cover check, CLI dry-run, no-op, mismatch refusal, stale expected versions, unit isolation and exact unrelated-byte preservation. Native/full-application builds are separate and were not run as part of this metadata-only adoption. The new CI workflow is read-only (`contents: read`) and cannot publish a release.
