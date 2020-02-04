CHANGELOG

Version 2.6
===========

### New Features
- New sync driver: generic folder sync for any folder based sync service, like Nextcloud and OwnCloud (#126)
- New safe edit mode where destructive actions are disabled and only record editing is allowed (#82)
- Allow empty rows and columns by default in form view. New setting to enable auto pruning of unused space (#120).
- Automatic column width based on contents, as a new option in the settings (#116)
- Improve performance of scrolling by allowing to hide images in table view (#107)
- Italian translation (thanks to Pierfrancesco Passerini)

### Improvements
- Set current date in CalenderWidget popup when date is not set (#91)
- Collections can be renamed als by clicking on the context menu
- CSV exports now include the original file name of images (#83)
- Ask for user permission before checking for updates
- Minor fixes in German translation files
- Improve slightly the styling of view mode buttons in tool bar

### Bug Fixes
- Duplicated collections no longer produce empty CSV and PDF files (#121, #122)
- Duplicated collections no longer contain inconsistent records with null ids (#122)
- Critical bug where deleting a field results in "invalid_column" (#125)
- Don't update last modification date time if no data was changed in table view (#115)
- Save and restore section order correctly in table view (#119)


Version 2.5
===========

### Bug Fixes
- MEGA sync driver: hotfix API compatibility change, issue #86


Version 2.4
===========

### New Features
- Ability to select font style for form view in settings, issue #70 
- Toolbar button to lock the form view to prevent unwanted field movements, issue #62 
- Reorder collections in the collections list by context menu (right mouse button click), issue #69 

### Improvements
- French translation added, thanks to Yann Yvinec
- Show info on Windows to restart after background color change, issue #66
- Allow empty dates as default value for new records, issue #56
- Allow minimum date as low as 100.01.01 and display empty values on form view for unset dates
- Add original directory import path for file type fields, issue #57
- Show an info dialog after a successful software upgrade
- Add wiki link to help menu
- Add error checking and backup handling during database version upgrades
- MEGA sync driver upgrade to MEGAcmd 1.0.0 and 2FA support, issues #75 and #76 
- New donation links and actions

### Bug Fixes
- Files are now correctly included in the backup file when using Qt 5.11, bug #60 
- Highlight correct characters after editing text containing search results, bug #64
- Reload views when undo redo commands are executed, fixes #68


Version 2.3
===========

### New Features
- Duplicate collections, with or without contents
- MEGA cloud sync support
- New deployment methods: Windows portable, AppImage, Snap

### Improvements
- Updated Qt to 5.10.1 (User interface library) for better operating system compatibility

### Bug Fixes
- Fix decimal point handling for numeric field type depending on system locale, fixes #46
- Handle unicode user names in dropbox sync client, fixes #31


Version 2.2
===========

### New Features
- New preference setting for font size in form view
- Highlight search results in form view, closes #34
- New setting to allow more than one line in table view for each row
- New setting to cache images in table view, fixes #37
- Allow custom database directory in settings, closes #38

### Improvements
- Show taskbar progress on windows during sync
- Update dropbox python sdk, fixes #35

### Bug Fixes
- Enable high DPI support on windows and linux, fixes #18
- Disable completely editing, including layout changes, when read only sync session is active, fixes #36
- Fix font issue where characters overlap when printing on windows
- Fix missing svg dependency windows setup script
- Add missing dependencies and langauges to windows installer, fixes #39
- Fix dependencies in PKGBUILD
- Windows, fix wrong text color in table view of selected rows (white on light blue)
- Allow removing items in combo box fields, fixes #42
- Escape single and double quotes characters for combobox fields, should also prevent SQL injections, fixes #40
- Hide progress dialog on optimize database size when finished, fixes #32
- Select correct collection when a collection is added or deleted, fixes #43
- Escape double quotes in field names correctly, fixes #29


Version 2.1
===========

### New Features
- Ability to hide the collection sidebar from the view menu
- Polish traslation added (thanks to MCbx)

### Improvements
- Dropbox API migration to v2 (v1 will stop working by 2017-06-28)
- Updated Qt to 5.9 (User interface library) for better compatibility with Windows 10 and macOS Sierra

### Bug Fixes
- Fixed crash when adding a new collection
- Collection list is not updated correctly when a new collection is added or deleted


Version 2.0
===========

### New Features
- #11 #12 #16 New fields for web links and email addresses

### Improvements
- Migration from Qt4 to Qt5 (User interface library)
- Updated Dropbox certificates (Old ones are valid until August 2016)
- Implemented database version checking and upgrading on format change
- Updated about dialog to include github project link
- Compiler warning fixes

### Bug Fixes
- #19 Unable to restore from backup on windows caused by improper sample image file permissions


Version 1.2
===========

### New Features
- Symphytum is now open source
- Proprietary artwork replaced with permissive icons and images
- German translation added

### Improvements
- Source code translation and localization capabilities for future interface translation work

### Bug Fixes
- #1 OS X 10.6 error, python .pyc files have bad magic number
- #7 Display correct text string of progress data in table view


Version 1.1
===========

### New Features
- New appearance option for dark toolbar (Ambiance theme) on Linux

### Improvements
- Last used record is now saved and restored when changing between collections
- Performance of form view has been noticeably improved
- Selection of a field in the form view can now be cleared by pressing the escape key
- The clean database menu action "Menu->Tools->Database->Free unused space" now includes the removal
  of obsolete files, which are no longer linked to any specific record

### Bug Fixes
- Fields with identical names are now correctly identified and handled by form view
- CSV exporting generates no longer an invalid column/content structure when records contain multiple new lines
- Form view is no longer disabled when adding a new record after a search operation with no results
- Form view scrolls now correctly the viewport to ensures visibility of the last selected field in table view mode
- Field name is now correctly loaded when editing an existing field of creation date type and last modification date type
- Incorrect records are no longer restored after adding, removing or modifying a field, during an active search filter
- Alarms are now set to fire at midnight (time 00:00) if new records have date fields without the time format spec
- Alarms are no longer incorrectly modified when enabling reminders for date fields with existing records
- Alarms are now correctly enabled/disabled when a field modification is triggered by undo/redo
- Last used collection is now correctly selected after a cloud sync session, where collections have been deleted
- Image fields are now scaling correctly the size of the content image, after changing from table view to form view
- Minor UI bugfixes


Version 1.0
===========

- Initial release
