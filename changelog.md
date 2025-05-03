## v1.9.0 && v1.9.1
* Bump to Geode `v4.4.0`
* Add support for `SimpleAxisLayout`
* Add missing `AxisLayoutOptions` attributes
* Add AxisGap for `AxisLayout`
* Add `Auto Grow Axis` to `AxisLayout` attributes
* Add `Ignore Invisible Children` checkbox for layouts
* Add the ability to remove layouts and layout options
* Add display for texture name / paths and respective copy buttons
* Add copy class name button
* Add `Tag` attribute
* Add `Color` and `Vector` attributes for `CCLayerGradient`
* Add `CCMenuItemSpriteExtra` attributes
* Add toggleable cascade color and opacity
* Add `Flip X` and `Flip Y` for `CCSprite`
* Add custom theme colors
* Reorganize and categorize some related attributes
* Fix flashing Geode Team logo
* Fix some settings not saving
* Fix fullscreen toggling sometimes breaking the UI
* Fix resizing the window causing the GD view to break
* Add back custom resolutions and window information
* Hook Geode's `getMousePos` to be relative to the GD view while DevTools is open

Thanks to <cj>Alphalaneous</c> for doing <co>most of the changes in this update!</c>

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