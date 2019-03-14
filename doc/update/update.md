# Symphytum Update

Quick links:
* [What's New?](#what-is-new)
* [How to Update](#how-to-update)

## New Version: 2.5
A new software version is available.  
Updating Symphytum is safe, all your data and files won't be deleted or altered.

Please **create a backup (File->Backup) before upgrading** to be on the safe side.
  
## What is New

This is a minor quick fix release to address a compatibility issue for the MEGA sync driver, after a change at the MEGA API.

### Bug Fixes
- MEGA sync driver: hotfix API compatibility change, issue #86


The complete changelog can be found [here](https://github.com/giowck/symphytum/blob/master/CHANGELOG.md).

### MEGA Cloud
*(since version 2.4)*

If you use the MEGA cloud service integration, please make sure to update your installed MEGAcmd version to 1.0.0, which is done automatically with the Windows installer if MEGAcmd checkbox is selected during install and for the snap package for Linux, other operating systems must update manually.
You can download MEGAcmd 1.0.0 from [https://mega.nz/cmd](https://mega.nz/cmd).

### Compatibility
Version 2.5 **doesn't change the internal database format** so it remains compatible with 2.4 clients.

## How to Update
First, it is highly recommended to create a backup before proceeding (**File->Backup**).  
Symphytum can be updated to the latest version manually by downloading and repeating the installation procedure.

**Note: Close Symphytum before starting the installation**

Please visit the [download section](https://github.com/giowck/symphytum#download) on the main page and download the appropriate executable for your operating system. The software will detect and upgrade automatically the internal database structure, if required, on first launch.

**Windows portable**: to update your windows portable folder, just copy the `portable_data` folder over to the new portable folder (once extracted).
