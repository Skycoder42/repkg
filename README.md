# repkg
A tool to manage rebuilding of AUR packages based on their dependencies.

## Features
The general idea is: For an AUR package foo, that depends on bar, you always want to rebuild foo everytime bar is update. This tool makes this easily possible:

- Create rules for packages with other packages that will trigger a rebuild
	- system rules: System-wide rules, to work for any user
		- can be created by users and applications. This way packagers can provide the required dependencies, without actually depending on this package (i.e. opt depend)
	- user rules: created by the user, can override system rules. Only work when sudoing as that user
- Pacman hook to detect packages that need a rebuild after an update
	- works recursivly, i.e. if A dependes on B and B on C, C will trigger B and A
- Automatic detection of successful rebuilds
- Works with any pacman frontend that supports the default pacman syntax
	- pacaur and yaourt work out of the box
	- other frontends can be set via a command

## Installation
The tool is provides as AUR package: [repkg](https://aur.archlinux.org/packages/repkg)

This repository also has a sample repkg rule for itself.

## Usage
### Users
As a user, you can create rules by calling:
```
repkg create <package> [dependencies...]
```
`package` is the AUR package that should be rebuild, when one of the given `dependencies` is updated to a newer versions. You can also create rules directly by creating a rule file in `~/.config/repkg/rules`, with the package name beeing the filename (i.e. `package.rule`) and the content beeing the dependencies, space seperated.

When updating packages via pacman (or any frontend), rebuilds are automatically detected. You will see a message with all packages that need rebuilds at the end. You can also run 
```
repkg list detail
```
to show all packages that need rebuilds.

To actually rebuild them, simply run
```
repkg
```
This will start the frontend of your choice (e.g. pacaur, yaourt, ...) and rebuild all required packages. 

### Package Providers
Simply add a rule file to your PKGBUILD, and install it to `/etc/repkg/rules`. Assuming your package is name `my-pkg` and should be rebuild when `dep-a` or `dep-b` is updated, the file must be named `my-pkg.rule` and contain:
```
dep-a dep-b
```

Add repkg as (optional) dependency, and your good to go.