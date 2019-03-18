

# Contributing to vmaccel - VMware Interface for Accelerator APIs

The vmaccel project team welcomes contributions from the community. Before you start working with vmaccel, please read our [Developer Certificate of Origin](https://cla.vmware.com/dco). All contributions to this repository must be signed as described on that page. Your signature certifies that you wrote the patch or have the right to pass it on as an open-source patch.

## Getting Started

First off, please read the [README.md](README.md) "Try it out", "Prerequisites", and "Build & Run" sections for information on how to build the project. Familiarize yourself with the accelerator you want to contribute to, a guideline for the design can be found in the "Design Overview..." section of [README.md](README.md).

## Contribution Flow

This is a rough outline of what a contributor's workflow looks like:

- Create a topic branch from where you want to base your work
- Make commits of logical units
- Make sure your commit messages are in the proper format (see below)
- Push your changes to a topic branch in your fork of the repository
- Submit a pull request

Example:

``` shell
git remote add upstream https://github.com/vmware/vmaccel.git
git checkout -b my-new-feature master
git commit -a
git push origin my-new-feature
```

### Staying In Sync With Upstream

When your branch gets out of sync with the vmware/master branch, use the following to update:

``` shell
git checkout my-new-feature
git fetch -a
git pull --rebase upstream master
git push --force-with-lease origin my-new-feature
```

### Updating pull requests

If your PR fails to pass CI or needs changes based on code review, you'll most likely want to squash these changes into
existing commits.

If your pull request contains a single commit or your changes are related to the most recent commit, you can simply
amend the commit.

``` shell
git add .
git commit --amend
git push --force-with-lease origin my-new-feature
```

If you need to squash changes into an earlier commit, you can use:

``` shell
git add .
git commit --fixup <commit>
git rebase -i --autosquash master
git push --force-with-lease origin my-new-feature
```

Be sure to add a comment to the PR indicating your new changes are ready to review, as GitHub does not generate a
notification when you git push.

### Code Style

Coding standards are specified by .clang-format. To apply the coding
standards, run the following makefile directive:

``` shell
  $ build/make pre-checkin
```

OR clang-format as follows:

``` shell
  $ build/external/spirv-llvm/bin/clang-format <temp source> > <target file>
```

clang-format is generated with the spirv-llvm build target that is built
from the project's top level build from [README.md](README.md) "Build & Run: Step 2".

### Formatting Commit Messages

We follow the conventions on [How to Write a Git Commit Message](http://chris.beams.io/posts/git-commit/).

Be sure to include any related GitHub issue references in the commit message.  See
[GFM syntax](https://guides.github.com/features/mastering-markdown/#GitHub-flavored-markdown) for referencing issues
and commits.

## Reporting Bugs and Creating Issues

When opening a new issue, try to roughly follow the commit message format conventions above.

## Repository Structure

### Source Structure
* accelerators - A collection of accelerator types, abstracted by their protocol defined in accelerators/.../specs/*.x
* backends - A collection of backends supporting the accelerator types
* common - Headers and source for common functionality of the project
* external - External projects referenced by the project
* frontends - Frontends for the accelerator types
* scripts - A collection of scripts used by the project
* test - A collection of unit tests for functionality exposed by the project

### Build Structure
* build/bin - Executables for each component
* build/external - External project build targets
* build/lib - Libraries for each component
* build/inc - Headers used for the libraries of each component
* build/specs - Spec files for use with the libraries and headers
* build/gen - Auto-generated files for the protocol specifications
* build/test - Compiled unit tests for the framework

