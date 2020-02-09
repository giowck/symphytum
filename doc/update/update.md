# Symphytum Update

Quick links:
* [What's New?](#what-is-new)
* [How to Update](#how-to-update)

## New Version: 2.6
A new software version is available.  
Updating Symphytum is safe, all your data and files won't be deleted or altered.

Please **create a backup (File->Backup) before upgrading** to be on the safe side.
  
## What is New

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
- MEGA sync driver: fix after upstream API change (MEGAcmd output)
- Duplicated collections no longer produce empty CSV and PDF files (#121, #122)
- Duplicated collections no longer contain inconsistent records with null ids (#122)
- Critical bug where deleting a field results in "invalid_column" (#125)
- Don't update last modification date time if no data was changed in table view (#115)
- Save and restore section order correctly in table view (#119)


The complete changelog can be found [here](https://github.com/giowck/symphytum/blob/master/CHANGELOG.md).

### MEGA Cloud
If you use the MEGA cloud service integration, please make sure to update your installed MEGAcmd version to 1.1.0, which is done automatically with the Windows installer if MEGAcmd checkbox is selected during install and for the snap package for Linux, other operating systems must update manually.
You can download MEGAcmd 1.1.0 from [https://mega.nz/cmd](https://mega.nz/cmd).

### Compatibility
Version 2.6 **upgrades the internal database format** to a new version (v4), making it incompatible with older versions. Your database schema will be automatically upgraded during the first start. Older software versions using the cloud sync will show a warning about incompatible database versions recommending to upgrade the software version to resolve the issue.

## How to Update
First, it is highly recommended to create a backup before proceeding (**File->Backup**).  
Symphytum can be updated to the latest version manually by downloading and repeating the installation procedure.

**Note: Close Symphytum before starting the installation**

Please visit the [download section](https://github.com/giowck/symphytum#download) on the main page and download the appropriate executable for your operating system. The software will detect and upgrade automatically the internal database structure, if required, on first launch.

**Windows portable**: to update your windows portable folder, just copy the `portable_data` folder over to the new portable folder (once extracted).
