# The MoleRat RFID / Arduino daemon for OSX

Uses CMAKE and the like. It can run as normal or as a daemon process. 

I built this using clang on osx, but a few small changes should make this work with Linux as well.

It uses the mysql connector library - I include the MacOSX one so if you want to build under Linux you need to swap that out. I statically compile it in as it's very small.

The general idea is we can get a lot more control over our serial devices this way, and take up a smaller footprint. With a bit more control we can keep memory and other problems to a minimum, with no reliance on Java or similar.

To build it, create a directory inside this one called 'build'

    cd build
    cmake ..
    make

To do the daemon bit copy the plist file to '~/Library/LaunchAgents'

    launchctl load -w systems.rat.rfid.01.plist
    launchctl start systems.rat.rfid.01

To remove it:
    
    launchctl stop systems.rat.rfid.01
    launchctl unload -w systems.rat.rfid.01.plist

The program can be run from the command line as a test and has several command line switches. To see the options run

    ./molerats_rfid -h
