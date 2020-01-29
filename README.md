[![Snap Status](https://build.snapcraft.io/badge/joshirio/symphytum-build.snapcraft.io.svg)](https://build.snapcraft.io/user/joshirio/symphytum-build.snapcraft.io)

![logo](https://raw.githubusercontent.com/giowck/symphytum/master/stuff/logo/symphytum_64.png "Symphytum")
# Symphytum
Symphytum is a personal database software for everyone who desires to manage and organize data in an easy and intuitive way, without having to study complex database languages and software user interfaces.        

**Table of Contents**
* [Introduction](#introduction)
    * [Features](#features)
    * [How it Looks](#how-it-looks)
    * [Wiki and User Guide](#wiki-and-user-guide)
* [Download](#download)
    * [Windows](#windows)
        * [Installer](#installer)
        * [Portable ZIP Archive](#portable-zip-archive)
    * [macOS](#macos)
    * [Linux](#linux)
        * [Ubuntu Based](#ubuntu-based)
        * [Arch Linux Based](#arch-linux-based)
        * [AppImage](#appimage)
        * [Snap](#snap)
* [Build from Source](#build-from-source)
* [Contribute](#contribute)
* [License](#license)
* [Donate](#donate)

## Introduction
Symphytum is a free and open-source personal database software written in C++ and Qt for Windows, macOS and Linux. Design and edit simple databases in a visual and intuitive way, without any need to study complex database languages. Symphytum is directed at users who just want to organize data in custom designed collections without giving up advantages of database engines like fast loading speed, large data set handling, fast searching, sorting and more.

Manage all kind of data ranging from contacts, inventory, any type of collection, customers and so on. There are limitations though, Symphytum is not able to handle relational data and automatic field calculations yet. Also the import from CSV files is very limited, all imported data sets are handled just as text fields, future improvements to the import dialog, to specify each field type, are planned though.

Symphytum is able to synchronise your data through different cloud services like Dropbox or MEGA. It can detect and handle sync conflicts in case multiple user are using the same database via a supported cloud service.

Technically Symphytum is powered by the SQLite database engine, which is the leading embedded database solution, used in many mobile apps and modern computer programs, like web browsers, media players and email clients.
SQLite is tiny, efficient and very fast. It can handle huge amount of data while being highly resistant to data corruption. 

### Features
* **Fields Are Not Just Text**. Design your input forms with support for different data types: text, numeric, date, progress, image, file list, checkbox, combobox, etc.
* **Two Views On The Same Data**. Use the form view for structured data input and representation, use the table-like view for searching, sorting and comparing.
* **Dynamic Layout Engine**. Rearrange dynamically your database layout by drag and drop in form view.
* **Integrated Cloud Sync**. Using Symphytum across multiple computers is a joy. Your data is always automatically synchronised everywhere. Drivers for cloud services such as Dropbox and MEGA are included.
* **Sync Conflict Management**. Symphytum manages synchronisation conflicts for you. While only one session with write access is allowed at the same time, other computers may access the database in read-only mode during an open session.
* **Date Reminder**. Date fields keep you informed on tasks, appointments or birthdays, if requested. All Reminders, once triggered, are listed in one place.
* **Fast Search**. Search while typing with highlighted results in a table view.
* **Backup and Export**. Backup your data with a simple backup wizard and export your data to CSV.
* **Multilingual**. User interface available in English, German, Polish, French and Italian. More info at the [project Wiki](https://github.com/giowck/symphytum/wiki/Help-Translate-Symphytum).


### How it Looks
Some screenshots showing the form view, table view, dynamic layout engine and the field addition dialog.

![form_view_img](https://raw.githubusercontent.com/giowck/symphytum/master/stuff/screenshots/mainwindow.png "Form view")
![table_view_img](https://raw.githubusercontent.com/giowck/symphytum/master/stuff/screenshots/tablieview.png "Table view")
![dynamic_layout_img](https://raw.githubusercontent.com/giowck/symphytum/master/stuff/screenshots/dynamic_layout.gif "Dynamic layout engine")

![add_field_img](https://raw.githubusercontent.com/giowck/symphytum/master/stuff/screenshots/addfield.png "Add field")

### Wiki and User Guide
Please visit the [project wiki](https://github.com/giowck/symphytum/wiki) for additional information and an user guide.

## Download
Please see the appropriate download section for your operating system below.
General releases, source archives and other info can be found on the [releases](https://github.com/giowck/symphytum/releases) page. Thank you for downloading Symphytum, please consider a small [donation](https://github.com/giowck/symphytum/blob/master/doc/donate.md) if you like it.

### Windows
For Windows 7, 8 and 10 32 or 64 bit

#### Installer
Download the Windows installer [symphytum-2.5-setup.exe](https://github.com/giowck/symphytum/releases/download/v2.5/symphytum-2.5-setup.exe)

#### Portable ZIP Archive
A portable ZIP for Windows is just a ZIP archive that, once extracted, can be moved and launched on any machine. The personal data is contained inside the folder alongside the main executable (symphytum.exe).

Download [Symphytum-windows-portable.zip](https://github.com/giowck/symphytum/releases/download/v2.5/Symphytum-windows-portable.zip)

---

### macOS
For macOS 10.11 (El Capitan) and later, 64bit

Download [symphytum-2.5.dmg](https://github.com/giowck/symphytum/releases/download/v2.5/symphytum-2.5.dmg)

---

### Linux
For GNU/Linux, 64 bit. The AppImage should run on most linux machines, choose that if unsure.

#### Ubuntu Based
Ubuntu 18.04 and other derivatives such as Linux Mint, elementaryOS and other.

Download [symphytum-2.5-x86_64.deb](https://github.com/giowck/symphytum/releases/download/v2.5/symphytum-2.5-x86_64.deb)

#### Arch Linux Based
Arch Linux and derivatives like Manjaro can install Symphytum from the Arch User Repository (AUR).

[Symphytum AUR package](https://aur.archlinux.org/packages/symphytum/)

#### AppImage
An [AppImage](https://appimage.org/) is a self containing executable which should run on most common modern Linux distributions. For more info on how to make the downloaded image executable, please visit [this page](https://discourse.appimage.org/t/how-to-make-an-appimage-executable/80). To improve the system integration of the AppImage, please visit the [AppImage Wiki](https://github.com/AppImage/AppImageKit/wiki).

Download [Symphytum-x86_64.AppImage](https://github.com/giowck/symphytum/releases/download/v2.5/Symphytum-x86_64.AppImage)

#### Snap
A [Snap package](https://snapcraft.io/) is a new self containing distribution format, supposed to work on most Linux distributions (Ubuntu, Debian, Arch Linux, Fedora, etc). The technology is still young with some limitations.

[Symphytum on the Snap Store](https://snapcraft.io/symphytum). The snap file can also be downloaded manually from the [releases](https://github.com/giowck/symphytum/releases) page. Note that snaps installed from the Snap Store are automatically updated, except the .snap file when downloaded and installed manually with `sudo snap install symphytum_amd64.snap --dangerous`.

[![Get it from the Snap Store](https://snapcraft.io/static/images/badges/en/snap-store-black.svg)](https://snapcraft.io/symphytum)

## Build from Source
Unpack source archive   
```
cd symphytum
qmake -config release
make
```
See [doc/deployment/](https://github.com/giowck/symphytum/tree/master/doc/deployment) and [stuff/installers/](https://github.com/giowck/symphytum/tree/master/stuff/installers) for further information on dependencies and deployment. More detailed instructions will be published to the [project wiki](https://github.com/giowck/symphytum/wiki).

## Contribute
Report an issue, bug or feature proposal at the [project's issue tracker](https://github.com/giowck/symphytum/issues). For additional ways to contribute such as writing code, translating Symphytum and more please visit the [project wiki](https://github.com/giowck/symphytum/wiki).

## License
Symphytum is licensed under the BSD 2-Clause License, see [LICENSE](https://github.com/giowck/symphytum/blob/master/LICENSE). 
You can use Symphytum for free and for any purprose.

## Donate
If you find Symphytum useful. please consider [donating](https://github.com/giowck/symphytum/blob/master/doc/donate.md) to support this project, thanks.

Copyright (c) 2014-2020 Symphytum Developers  
Copyright (c) 2012-2014 GIOWISYS Software UG
