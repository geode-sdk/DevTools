## v1.9.0
* Bump to Geode `v4.4.0`
* Add support for `SimpleAxisLayout`
* Fix some bugs with `AxisLayout` options
* Hook Geode's `getMousePos` to be relative to the GD view while DevTools is open
* Add display for texture name / paths
* Fix fullscreen toggling sometimes breaking the UI
* Add `Auto Grow Axis` to `AxisLayout` attributes
* Add `Ignore Invisible Children` checkbox for layouts
* Add `CCMenuItemSpriteExtra` attributes
* Add toggleable cascade color and opacity
* Add `Flip X` and `Flip Y` for `CCSprite`

Thanks to <cj>Alphalanous</c> for doing <co>most of the changes in this update!</c>

## v1.8.0
* Support for 2.206
* Bump to Geode `v4.0.0-beta.1`
* Memory viewer overhaul (#46) - thanks @hiimjustin000
* Fix a crash related to UTF-8 directory names (3edd503)

## v1.7.1
* Added support for Intel Mac
* Disabled GD hooks that were still enabled on Windows

## v1.7.0

* Support for 2.206
* Bump to Geode `v3.0.0-beta.1`
* Fixes for 64-bit
* Make the time format for memory dumps better
* Remove logs for GL extension string
* Actually save the settings used

## v1.6.0

* Adds Node IDs into the Attributes menu, along with a button to copy them
* Adds a way to create and modify an AnchorLayout
* Removes the Show Mod Index option in preparation for the new index