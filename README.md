# Match3-PGE 
Simple clone of the common match3 genre using the [olcPixelGameEngine](https://github.com/OneLoneCoder/olcPixelGameEngine) by [Javidx9](https://github.com/OneLoneCoder).
It's just for fun and educational purpose. Feel free to modify and use it :)
If you want to use the assets, feel free to use it. Acknowledgement is highly appreciated. 

![Screenshot_20210428_231624](https://user-images.githubusercontent.com/16195016/116474118-617c2580-a878-11eb-81db-7e4ff7144937.png)

## Installation
Just do for standard compilation on Linux (tested with gcc 8.2.1):

````sh
    # generates the application
    g++ -o Jewel mainJewels.cpp -lX11 -lGL -lpthread -lpng -lstdc++fs -std=c++17
    # run the application
    ./Jewel
````

## Usage
Simply use the mouse to select a gem and another for switching gems. Try to create rows or coloumns with at least 3 similar gems.

Special gems:
- Bomb: 4 gems in consecutive order row/column
- Rainbow: 5 gems in consecutive order row/column
- Star (AnyColor gem): 7 different gems in consecutive order row/column
- Color Heart (AnyColor bomb): 3 gems + star in consecutive order row/column
- Color Rainbow: 4 gems + star/color heart in consecutive order row/column or 3+2stars

Effect:
- Bomb: destroy 3x3 vicinity of bomb
- Rainbow: destroy all gems of same color
- Star: has every color for replacement
- Color Heart: bomb with every color
- Color Rainbow: destroys all gems as it has all colors

Random Drop:
- Bomb: 1/64
- Rainbow: 1/4096

Color Drop:
- AnyColor: 1/64
- Other: 63/64

ESC : quit


## References and License
The program is licensed under [OLC-3](https://github.com/OneLoneCoder/olcPixelGameEngine/blob/master/LICENCE.md) and acknowledge gratefully the following libraries and refer to the appropriate licenses
* [olcPixelGameEngine](https://github.com/OneLoneCoder/olcPixelGameEngine) by [Javidx9](https://github.com/OneLoneCoder)
* [olcPGEX_AnimatedSprite](https://github.com/matt-hayward/olcPGEX_AnimatedSprite) by [matt-hayward](https://github.com/matt-hayward)

## Acknowledgement
Heavily inspired by the Match3 example in the [50K Live code party](https://www.youtube.com/watch?v=7y8Zg87rtjs) of [Javidx9](https://github.com/OneLoneCoder) giving the idea for this.<br>
For the original source with olcConsoleGameEngine please see: [50K Live code party source code](https://github.com/OneLoneCoder/videos/blob/master/OneLoneCoder_MatchingGems_50KSubSpecial.cpp).
