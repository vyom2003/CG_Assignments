# JETPACK JOYRIDE

## Instructions to build

1. <b>INITAL SETUP</b> (do this once)

    - Setup FreeType to render text 
        - Linux/macOS (recommended OSes)
            - Download the latest stable release(2.10.1) from [the official website](https://freetype.org/download.html)
            - Run the following
                1.  `tar -xvf freetype-2.10.1.tar.gz`
                2.  `cd freetype-2.10.1`
                3.  `./configure --prefix=/usr/local/freetype/2_10_1 --enable-freetype-config` : Prefix defines where freetype is installed
                4.  `make; make install`
2. `mkdir build; cd build`
3. `cmake ..; make`
## Instructions to Play the game 
1. The Player uses the jetpack on Pressing SPACEBAR key.
2. On completion of a level press the right arrow key to enter the next level.
3. In case you lose press Enter to restart.
