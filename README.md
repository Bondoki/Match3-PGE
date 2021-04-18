# Match3-PGE 
Simple clone of the common match3 genre using the [olc::PixelGameEngine](https://github.com/OneLoneCoder/olcPixelGameEngine) by [Javidx9](https://github.com/OneLoneCoder).
It's just for fun and educational purpose. Feel free to modify and use it :)
If you want to use the assets, feel free to use it. Acknowledgement is highly appreciated. 

![Match3-PGE_Screenshot_20210418](https://user-images.githubusercontent.com/16195016/115147815-2a7a6880-a05d-11eb-9505-4ae5e90ec12c.png)
 
## Installation
Just do for standard compilation on Linux (tested with gcc 8.2.1):

````sh
    # generates the application
    g++ -o Jewel mainJewels.cpp -lX11 -lGL -lpthread -lpng -lstdc++fs -std=c++17
    # run the application
    ./Jewel
````

## Usage
Simply use the mouse to select a gem and another to switch creating rows or coloumns.

Special gems:
- Bomb: 4 gems in consecutive order row/column
- Rainbow: 5 gems in consecutive order row/column

Effect:
- Bomb: destroy 3x3 vicinity of bomb
- Rainbow: destroy all gems of same color

Random Drop:
- Bomb: 1/64
- Rainbow: 1/4096

ESC : quit


## References and License
The program is licensed under [OLC-3](https://github.com/OneLoneCoder/olcPixelGameEngine/blob/master/LICENCE.md) and acknowledge gratefully the following libraries and refer to the appropriate licenses
* [olcPixelGameEngine](https://github.com/OneLoneCoder/olcPixelGameEngine) by [Javidx9](https://github.com/OneLoneCoder)
* [olcPGEX_AnimatedSprite](https://github.com/matt-hayward/olcPGEX_AnimatedSprite) by [matt-hayward](https://github.com/matt-hayward)

## Acknowledgement
Heavily inspired by the Match3 example in the [50K Live code party](https://www.youtube.com/watch?v=7y8Zg87rtjs) of [Javidx9](https://github.com/OneLoneCoder) giving the idea for this.<br>
For the original source with olcConsoleGameEngine please see: [50K Live code party source code](https://github.com/OneLoneCoder/videos/blob/master/OneLoneCoder_MatchingGems_50KSubSpecial.cpp).
