# Symphytum Update

Quick links
* [What's New?](#what-is-new)
* [How to Update](#how-to-update)

## New Version: 2.4
A new software version is available.  
Updating Symphytum is safe, all your data and files won't be deleted or altered.

Please **create a backup (File->Backup) before upgrading** to be on the safe side.
  
## What is New

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


The complete changelog can be found [here](https://github.com/giowck/symphytum/blob/master/CHANGELOG.md).

### MEGA Cloud
If you use the MEGA cloud service integration, please make sure to update your installed MEGAcmd version to 1.0.0, which is done automatically with the Windows installer if MEGAcmd checkbox is selected during install and for the snap package for Linux, other operating systems must update manually.
You can download MEGAcmd 1.0.0 from [https://mega.nz/cmd](https://mega.nz/cmd).

### Compatibility
Version 2.4 **upgrades the internal database format** to a new version (v3), making it incompatible with older versions. Your database schema will be automatically upgraded during the first start. Older software versions using the cloud sync will show a warning about incompatible database versions recommending to upgrade the software version to resolve the issue.

## How to Update
Symphytum can be updated to the latest version manually by downloading and repeating the installation procedure.

**Note: Close Symphytum before starting the installation**

Please visit the [download section](https://github.com/giowck/symphytum#download) on the main page and download the appropriate executable for your operating system. The software will detect and upgrade automatically the internal database structure, if required, on first launch.

**Windows portable**: to update your windows portable folder, just copy the `portable_data` folder over to the new portable folder (once extracted).
