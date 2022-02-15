These files aren't used in the CI system, they are for users to make sure they can build and run the unit tests on their local machines _before_ uploading.  This is helpful because most people don't have the older compilers on their machine, and some distros don't keep them available in their package repositories (e.g. Fedora).

`Dockerfile` provides gcc v9.4 and v11.2, and clang v9 and v12; as well as CMake and Ninja.

To build the image, run:
```
docker build -t arg_router -f <Path to repo>/scripts/ci/docker/Dockerfile <Empty dir>
```
Run from an empty directory so nothing is brought into the build context.

You can run the compilers manually from within the container, but it's easier to use the provided scripts that load the container with the correct volume mappings and user settings, and then runs the associated program in it using the passed in arguments.