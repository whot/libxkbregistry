libxkbregistry -  look up available XKB components
==================================================

libxkbregistry is a library to query the XKB database for available XKB
layouts, models, etc. It is effectively a library wrapper around the
`evdev.xml` file which applications used to access directly.


Configuration
-------------

The goal of libxkbregistry is that user-defined rules files are loaded
correctly from `$XDG_CONFIG_HOME/xkb/rules`. For this to work, the XML to
load (`evdev.xml`) must be a valid XML. Below is an example:

```xml
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE xkbConfigRegistry SYSTEM "xkb.dtd">
<xkbConfigRegistry version="1.1" xmlns:xi="http://www.w3.org/2001/XInclude">
  <modelList>
    <model>
      <configItem>
        <name>foobar</name>
        <description>Definitely foobar</description>
        <vendor>bar</vendor>
      </configItem>
    </model>
    <xi:include href="xkb://rules/evdev.xml" xpointer="xpointer(xkbConfigRegistry/modelList/*)" />
 </modelList>
  <xi:include href="xkb://rules/evdev.xml" xpointer="xpointer(xkbConfigRegistry/layoutList)" />
  <xi:include href="xkb://rules/evdev.xml" xpointer="xpointer(xkbConfigRegistry/optionList)" />
</xkbConfigRegistry>
```

Note the `<xi:include ... />` tag, we want the rules file to include the
system files for the complete list of elements. But the current DTD doesn't
allow for us to just include it, so we have to rebuild the same XML files.

The `xml://` prefix expands to the system XKB root directory. Other prefixes
available are `home://` and of course `file://` or just `/`.

In the example above, the `modelList` contains one user-specific model and
includes all remaning models from the system directory.
The `layoutList` and `optionList` are includes as-is.

License
-------

libxkbregistry is licensed under the MIT license.
