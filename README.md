# repkg
A tool to manage rebuilding of AUR packages based on their dependencies.

## Features
The general idea is: For an AUR package foo, that depends on bar, you always want to rebuild foo everytime bar is update. This tool makes this easily possible:

- Create rules for packages with other packages that will trigger a rebuild
	- System rules: System-wide rules, to work for any user
		- can be created by users and applications. This way packagers can provide the required dependencies, without actually depending on this package (i.e. opt depend)
	- User rules: created by the user, can override system rules. Only work when sudoing as that user
- Pacman hook to detect packages that need a rebuild after an update
	- works recursivly, i.e. if A dependes on B and B on C, C will trigger B and A
- Automatic detection of successful rebuilds
- Works with any pacman frontend that supports the default pacman syntax
	- yay, trizen, pacaur and yaourt work out of the box
	- other frontends can be set via a command
	- allows "waved" rebuilds for frontends that are unable to correctly order packages for the rebuild

## Installation
The tool is provided as AUR package: [repkg](https://aur.archlinux.org/packages/repkg)

This repository also has a sample repkg rule for itself.

## Usage
### Users
As a user, you can create rules by calling:
```
repkg create <package> [dependencies...]
```
`package` is the AUR package that should be rebuild, when one of the given `dependencies` is updated to a newer versions. You can also create rules directly by creating a rule file in `~/.config/repkg/rules`, with the package name beeing the filename (i.e. `package.rule`) and the content beeing the dependencies, space seperated.

For system admins, when running this command as root, the rules are instead written to `/etc/repkg/rules`. For repkg prior to version `1.3.0` this will overwrite the rules created by installed packages. But since `1.3.0` packages should place their rules in `/etc/repkg/rules/system` to prevent such conflicts.

When updating packages via pacman (or any frontend), rebuilds are automatically detected. You will see a message with all packages that need rebuilds at the end. You can also run
```
repkg list detail
```
to show all packages that need rebuilds.

To actually rebuild them, simply run
```
repkg
```
This will start the frontend of your choice (e.g. yay, trizen, pacaur, yaourt, ...) and rebuild all required packages. 

### Package Providers
Simply add a rule file to your PKGBUILD, and install it to `/etc/repkg/rules/system` (or `/etc/repkg/rules` if you want to be compatible with versions of repkg before `1.3.0`). Assuming your package is name `my-pkg` and should be rebuild when `dep-a` or `dep-b` is updated, the file must be named `my-pkg.rule` and contain:
```
dep-a dep-b
```

Add repkg as (optional) dependency, and your good to go.

#### Version Filtering
If you want the package to only be updated if the change is significant enough (i.e. a major version update), you can do so by adding a version filter expression to the dependency. These special filters tell repkg to only compare parts of the version numbers, not the whole number. The general syntax for that is:
```
<package>[=<filter>]
```
Leaving out the filter means that the package is always rebuilt if the version string changes in any way. Possible filter expressions are:

- `0`: Only update if the epoche changes. E.g. from `1:2.3.5` to `2:1.0.0`.
- `v`: Only update if the epoche or the version number itself changes. This *exlcudes* the package revision and possible suffixes. E.g. from `1.2.3` to `1.2.4`
- `1..*` (any other positive number besides 0): Same as `v`, but limit the version segments to check. E.g. if specifying `1`, Only major version updates (`1.2.3` to `2.0.0`) trigger a rebuild. With `2` this also includes minor versions etc.
- `s`: Only update if the epoche or the version number itself changes. This *includes* the package revision and possible suffixes. E.g. from `1.2.3-alpha` to `1.2.3`
- `r`: Always update, even if only the package revision changes. E.g. `1.2.3-1` to `1.2.3-2`
- `:<offset>[:<length>]`: Do a normal string based comaprison, but only compare a substring of the version number, starting at `offset` and `length` characters long (both must be 0 or positive integers). E.g. `:2:4` on `1.2345.6` will reduce the string to `2345` before comparing.
- `:<offset>[:<length>]::<filter>`: Same as before, but instead of a string compare, use another filter. Can be any of the above except the two range limiters. E.g. using the filter `:1::v` on `v1.2.3` will reduce the string to `1.2.3` and then do a normal version compare. Without the previous removal of the `v`, a version-based compare would not work for this example.
