# Contributing

Thank you for improving Geometric Modeling.

## Development workflow

1. Create a focused branch from `main`.
2. Keep geometry algorithms in `mesh_core`; UI code belongs in `main.cpp`.
3. Add or update a regression test for every topology change.
4. Run the complete local validation:

   ```bash
   cmake -S TP1/MeshViewerCMake -B build -DCMAKE_BUILD_TYPE=Release
   cmake --build build --config Release
   ctest --test-dir build -C Release --output-on-failure
   ```

5. Open a pull request describing the algorithm, invariants, and visible result.

Commit messages should be short, imperative, and scoped when useful, for
example `fix(mesh): preserve twins after edge collapse`.
