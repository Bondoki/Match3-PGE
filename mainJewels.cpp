#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"

#define OLC_PGEX_ANIMSPR
#include "olcPGEX_AnimatedSprite.h"


/*
 * Jewels - Basic implementation of Match 3 Gameplay
 * 
 * Version: 0.3
 * Created on 3. April 2021
 * Last modified on: 18. April 2021
 * Author: Bondoki (Ron Dockhorn)
 * 
 * Heavily inspired by the Match3 example in the 50K Live code party of Javidx9 ( https://www.youtube.com/watch?v=7y8Zg87rtjs ) gave the idea for this.
 * For the original source with olcConsoleGameEngine please see: https://github.com/OneLoneCoder/videos/blob/master/OneLoneCoder_MatchingGems_50KSubSpecial.cpp
 * 
 * It's just for fun and educational purpose. Feel free to modify and use it :)
 * If you want to use the assets, feel free to use it. Acknowledgement is highly appreciated. 
 * 
 * Compile on Linux (tested with gcc 8.2.1):
 * g++ -o Jewel mainJewels.cpp -lX11 -lGL -lpthread -lpng -lstdc++fs -std=c++17
 * 
 * Special gems:
 * Bomb: 4 gems in consecutive order row/column
 * Rainbow: 5 gems in consecutive order row/column
 * 
 * Effect:
 * Bomb: destroy 3x3 vicinity of bomb
 * Rainbow: destroy all gems of same color
 * 
 * Random Drop:
 * Bomb: 1/64
 * Rainbow: 1/4096
 * 
 * ToDo:
 * + Score counter
 * + Animation of swap
 * + Cursor sprite
 * + check for simultaneous 3 row and 3 column
 * 
 * License: This piece of code is licensed to OLC-3 according to (see below)
 * https://github.com/OneLoneCoder/olcPixelGameEngine/blob/master/LICENCE.md
 * also attributing Javidx9 for the unique and easy-to-use olc::PixelGameEngine and the underlying OLC-3 license see
 * https://github.com/OneLoneCoder/olcPixelGameEngine/wiki/Licencing
 * For more information about Javidx9 and OneLoneCoder please see https://github.com/OneLoneCoder
 * Also big thanks to Matt Hayward, 0xnicholasc, and Moros1138 for the olcPGEX_AnimatedSprite extension.
 * Please see https://github.com/matt-hayward/olcPGEX_AnimatedSprite and the underlying OLC-3 license.
 * 
 * License (OLC-3)
	~~~~~~~~~~~~~~~
	Copyright 2018 - 2021 OneLoneCoder.com
	Redistribution and use in source and binary forms, with or without modification,
	are permitted provided that the following conditions are met:
	1. Redistributions or derivations of source code must retain the above copyright
	notice, this list of conditions and the following disclaimer.
	2. Redistributions or derivative works in binary form must reproduce the above
	copyright notice. This list of conditions and the following	disclaimer must be
	reproduced in the documentation and/or other materials provided with the distribution.
	3. Neither the name of the copyright holder nor the names of its contributors may
	be used to endorse or promote products derived from this software without specific
	prior written permission.
	THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS	"AS IS" AND ANY
	EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
	OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
	SHALL THE COPYRIGHT	HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
	INCIDENTAL,	SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
	TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
	BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
	CONTRACT, STRICT LIABILITY, OR TORT	(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
	ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
	SUCH DAMAGE.
 */

#include <stack>          // std::stack

#define BOARD_X 8
#define BOARD_Y 8

#define TILESIZE_X 52
#define TILESIZE_Y 52

#define SCREENSIZE_X 640
#define SCREENSIZE_Y 480

#define NUMBERS_X 33
#define NUMBERS_Y 52
 
class JuwelsGame : public olc::PixelGameEngine
{
public:
  JuwelsGame()
  {
    sAppName = "Match 3 Jewels";
  }
  
  struct sGem
  {
    uint8_t color; // 0 NONE; 1 BLUE; 2 GREEN; 3 ORANGE; 4 PURPLE; 5 RED; WHITE; YELLOW
    uint8_t animation_mode; // 0 NONE; 1 ROTATING; 2 TILTING
    bool bExist;
    bool bRemove;
    olc::AnimatedSprite sprite; 
    
    enum GEMTYPE
    {
      GEM, 
      BOMB,
      RAINBOW,
    } type;
  };
  
  enum STATES
  {
    STATE_USER, 
    STATE_SWAP,
    STATE_CHECK,
    STATE_CHECK_BOMB,
    STATE_ERASE, 
    STATE_COMPRESS,
    STATE_NEWGEMS,
  } nState, nNextState;
  
  // define sprite in your PGE program for drawing gems/bombs/rainbow
  std::vector<olc::AnimatedSprite> m_GemSprite;
  
  // define sprite in your PGE program for explosion
  std::vector<olc::AnimatedSprite> m_FragmentSprite;
  
  // Playfield
  sGem m_GemsPlayfield[BOARD_X][BOARD_Y];
  
  float fDelayTime = 0.0f;
  
  int nTotalGems = 0;
  
  olc::vi2d nCursor = {0,0};
  olc::vi2d nSwap = {0,0};
  
  bool bSwapFail = false;
  bool bGemsToRemove = false;
  
  bool bIsfirstClickMouse = true;
  
  struct sFragment
  {
    float x; 
    float y; 
    float vx; 
    float vy;
    uint8_t color;
    olc::AnimatedSprite sprite; 
  };
  
  
  std::list<sFragment> fragments;
  
  olc::Renderable gfxTiles;
  
  olc::Renderable gfxNumbers;
  
  uint32_t score;
  int score_combo = 0;
  int score_removed_tiles = 0;
  std::stack<uint32_t> stack_score;
  
private:
  
  void getScoreInDigits()
    {
        stack_score=std::stack<uint32_t>(); //empty stack

        uint32_t number = score;

        while (number > 0) {
            stack_score.push( number % 10 );
            number = number / 10;
        }
    }
  
public:
  bool OnUserCreate() override
  {
    // Called once at the start, so create things here
    
    score=0; getScoreInDigits(); //max: 4294967295

    // configure the sprite:
    olc::Renderable* spritesheet = new olc::Renderable();
    spritesheet->Load("./assets/Gem_Bomb_Rainbow.png");
    
    gfxTiles.Load("./assets/Board.png");
    
    gfxNumbers.Load("./assets/Numbers.png");
    
    for (int j = 0; j < 7; j++)
    {
      olc::AnimatedSprite sprite;
      sprite.mode = olc::AnimatedSprite::SPRITE_MODE::SINGLE; // set sprite to use a single spritesheet
      sprite.spriteSheet = spritesheet; // define image to use for the spritesheet
      sprite.SetSpriteSize({TILESIZE_X, TILESIZE_Y}); // define size of each sprite with an olc::vi2d
      sprite.SetSpriteScale(1.0f); // define scale of sprite; 1.0f is original size. Must be above 0 and defaults to 1.0f
      sprite.type = olc::AnimatedSprite::SPRITE_TYPE::DECAL;
      
      // define states - state name and vector of olc::vi2d to define the top-left position of each frame in the spritesheet
      std::vector<olc::vi2d> pos;
      
      
      sprite.AddState("gem idle", 0.1f, olc::AnimatedSprite::PLAY_MODE::LOOP, {olc::vi2d({0,   TILESIZE_Y*(3*j)}),});
      
      for(int i=0; i < 39; i++)
        pos.push_back( olc::vi2d({i*TILESIZE_X,   (3*j)*TILESIZE_Y}));
      
      sprite.AddState("gem rotating", 0.1f, olc::AnimatedSprite::PLAY_MODE::LOOP, pos);
      
      
      
      sprite.AddState("bomb idle", 0.1f, olc::AnimatedSprite::PLAY_MODE::LOOP, {olc::vi2d({0,   TILESIZE_Y*(3*j)+TILESIZE_Y}),});
      
      pos.clear();
      for(int i=0; i < 39; i++)
        pos.push_back( olc::vi2d({i*TILESIZE_X,   (3*j)*TILESIZE_Y+TILESIZE_Y}));
      
      sprite.AddState("bomb rotating", 0.1f, olc::AnimatedSprite::PLAY_MODE::LOOP, pos);
      
      sprite.AddState("rainbow idle", 0.1f, olc::AnimatedSprite::PLAY_MODE::LOOP, {olc::vi2d({0,   TILESIZE_Y*(3*j)+2*TILESIZE_Y}),});
      
      pos.clear();
      for(int i=0; i < 39; i++)
        pos.push_back( olc::vi2d({i*TILESIZE_X,   (3*j)*TILESIZE_Y+2*TILESIZE_Y}));
      
      sprite.AddState("rainbow rotating", 0.1f, olc::AnimatedSprite::PLAY_MODE::LOOP, pos);
      
      
      // set initial state
      sprite.SetState("gem idle");
      
      m_GemSprite.push_back(sprite);
    }
    
    for (int j = 0; j < 7; j++)
    {
      olc::AnimatedSprite sprite;
      sprite.mode = olc::AnimatedSprite::SPRITE_MODE::SINGLE; // set sprite to use a single spritesheet
      sprite.spriteSheet = spritesheet; // define image to use for the spritesheet
      sprite.SetSpriteSize({TILESIZE_X, TILESIZE_Y}); // define size of each sprite with an olc::vi2d
      sprite.SetSpriteScale(0.25f); // define scale of sprite; 1.0f is original size. Must be above 0 and defaults to 1.0f
      sprite.type = olc::AnimatedSprite::SPRITE_TYPE::DECAL;
      
      // define states - state name and vector of olc::vi2d to define the top-left position of each frame in the spritesheet
      std::vector<olc::vi2d> pos;
      
      for(int i=0; i < 39; i++)
        pos.push_back( olc::vi2d({i*TILESIZE_X,   (3*j)*TILESIZE_Y+0}));
      
      
      sprite.AddState("rotating", 0.02f, olc::AnimatedSprite::PLAY_MODE::LOOP, pos);
      
      // set initial state
      sprite.SetState("rotating");
      
      m_FragmentSprite.push_back(sprite);
    }
    
    
    // fill the playfield
    for (int x = 0; x < BOARD_X; x++)
    {
      for (int y = 0; y < BOARD_Y; y++)
      {
        m_GemsPlayfield[x][y].color = rand() % 7 + 1;
        m_GemsPlayfield[x][y].animation_mode = 0;
        m_GemsPlayfield[x][y].sprite = m_GemSprite[m_GemsPlayfield[x][y].color-1];
        m_GemsPlayfield[x][y].bExist = false;
        m_GemsPlayfield[x][y].bRemove = false;
        m_GemsPlayfield[x][y].type = sGem::GEMTYPE::GEM;
      }
    }
    
    return true;
  }
  
  bool OnUserUpdate(float fElapsedTime) override
  {
    if (fDelayTime > 0.0f)
    {
      fDelayTime -= fElapsedTime;
    }
    else
    {
      
      auto boom = [&](int x, int y, int size, uint8_t color)
      {
        auto random_float = [&](float min, float max)
        {
          return ((float)rand() / (float)RAND_MAX) * (max - min) + min;
        };
        
        
        for (int i = 0; i < size; i++)
        {
          float a = random_float(0, 2.0f * 3.14159f);
          sFragment f = { (float)x, (float)y, cosf(a) * random_float(100.0f, 300.0f), sinf(a) * random_float(100.0f, 300.0f), color, m_FragmentSprite[color-1] };
          fragments.push_back(f);
        }
        
      };
      
      bool bBombToRemove = false;
      
      bool bRainbowToRemove = false;
      
      
      int score_removed_tiles = 0;
      
      // Gameplay
      switch (nState)
      {
        case STATE_USER:
          if (nTotalGems < 64)
            nNextState = STATE_COMPRESS;
          else
          {
            // Get Mouse in world
            if (GetMouse(0).bPressed)
            {
              //  start new combo series
              score_combo = 0;
      
              
              olc::vi2d vMouse = { GetMouseX(), GetMouseY() };
              
              
              // Work out active cell
              float offset_X = 0.5f*(SCREENSIZE_X-BOARD_X*TILESIZE_X);
              
              olc::vi2d vCell = { (vMouse.x-int(offset_X)) / TILESIZE_X, vMouse.y / TILESIZE_Y };
              
              if(bIsfirstClickMouse == true)
              {
                if(vCell.x >= 0 && vCell.x < BOARD_X && vCell.y >= 0 && vCell.y <BOARD_Y)
                {
                  m_GemsPlayfield[vCell.x][vCell.y].animation_mode = 1;//rand() % 2 +1;//(m_GemsPlayfield[vCell.x][vCell.y].animation_mode+1) %3;
                  
                  
                  if (m_GemsPlayfield[vCell.x][vCell.y].type ==  sGem::GEMTYPE::GEM)
                    m_GemsPlayfield[vCell.x][vCell.y].sprite.SetState("gem rotating");
                  else if (m_GemsPlayfield[vCell.x][vCell.y].type ==  sGem::GEMTYPE::BOMB)
                    m_GemsPlayfield[vCell.x][vCell.y].sprite.SetState("bomb rotating");
                  else if (m_GemsPlayfield[vCell.x][vCell.y].type ==  sGem::GEMTYPE::RAINBOW)
                    m_GemsPlayfield[vCell.x][vCell.y].sprite.SetState("rainbow rotating");
                  
                  nCursor.x = vCell.x;
                  nCursor.y = vCell.y;
                  
                  bIsfirstClickMouse = false;
                }
              }
              else
              {
                nSwap.x = nCursor.x;
                nSwap.y = nCursor.y;
                
                if(vCell.x >= 0 && vCell.x < BOARD_X && vCell.y >= 0 && vCell.y <BOARD_Y)
                {
                  // second tile in vicinity
                  if((nCursor-vCell).mag2() == 1 )
                  {
                    nSwap.x = vCell.x;
                    nSwap.y = vCell.y;
                    
                    m_GemsPlayfield[nCursor.x][nCursor.y].animation_mode = 0;//(m_GemsPlayfield[vCell.x][vCell.y].animation_mode+1) %3;
                    
                    if (m_GemsPlayfield[nCursor.x][nCursor.y].type ==  sGem::GEMTYPE::GEM)
                      m_GemsPlayfield[nCursor.x][nCursor.y].sprite.SetState("gem idle");
                    else if (m_GemsPlayfield[nCursor.x][nCursor.y].type ==  sGem::GEMTYPE::BOMB)
                      m_GemsPlayfield[nCursor.x][nCursor.y].sprite.SetState("bomb idle");
                    else if (m_GemsPlayfield[nCursor.x][nCursor.y].type ==  sGem::GEMTYPE::RAINBOW)
                      m_GemsPlayfield[nCursor.x][nCursor.y].sprite.SetState("rainbow idle");
                    
                    
                    bIsfirstClickMouse = true;
                    nNextState = STATE_SWAP;
                  }
                  else
                  { 
                    m_GemsPlayfield[nCursor.x][nCursor.y].animation_mode = 0;//(m_GemsPlayfield[vCell.x][vCell.y].animation_mode+1) %3;
                    
                    if (m_GemsPlayfield[nCursor.x][nCursor.y].type ==  sGem::GEMTYPE::GEM)
                      m_GemsPlayfield[nCursor.x][nCursor.y].sprite.SetState("gem idle");
                    else if (m_GemsPlayfield[nCursor.x][nCursor.y].type ==  sGem::GEMTYPE::BOMB)
                      m_GemsPlayfield[nCursor.x][nCursor.y].sprite.SetState("bomb idle");
                    else if (m_GemsPlayfield[nCursor.x][nCursor.y].type ==  sGem::GEMTYPE::RAINBOW)
                      m_GemsPlayfield[nCursor.x][nCursor.y].sprite.SetState("rainbow idle");
                    
                    
                    // newly selected 
                    nCursor.x = vCell.x;
                    nCursor.y = vCell.y;
                    
                    
                    
                    m_GemsPlayfield[vCell.x][vCell.y].animation_mode = 1;//rand() % 2 +1;//(m_GemsPlayfield[vCell.x][vCell.y].animation_mode+1) %3;
                    
                    if (m_GemsPlayfield[vCell.x][vCell.y].type ==  sGem::GEMTYPE::GEM)
                      m_GemsPlayfield[vCell.x][vCell.y].sprite.SetState("gem rotating");
                    else if (m_GemsPlayfield[vCell.x][vCell.y].type ==  sGem::GEMTYPE::BOMB)
                      m_GemsPlayfield[vCell.x][vCell.y].sprite.SetState("bomb rotating");
                    else if (m_GemsPlayfield[vCell.x][vCell.y].type ==  sGem::GEMTYPE::RAINBOW)
                      m_GemsPlayfield[vCell.x][vCell.y].sprite.SetState("rainbow rotating");
                    
                    
                    bIsfirstClickMouse = false;
                  }
                }
                
                
                
              }
              
              
              
            }
            
          }
          
          break;
          case STATE_SWAP:
            bSwapFail = true;
            std::swap(m_GemsPlayfield[nCursor.x][nCursor.y], m_GemsPlayfield[nSwap.x][nSwap.y]);
            fDelayTime = 0.5f;
            
            nNextState = STATE_CHECK;
            break;
          case STATE_CHECK:
            
            //bGemsToRemove = false;
            
            bBombToRemove = false;
            
            score_combo++;
            score_removed_tiles = 0;
            
            
            for (int x = 0; x < BOARD_X; x++)
            {
              for (int y = 0; y < BOARD_Y; y++)
              {
                if (!m_GemsPlayfield[x][y].bRemove)
                {
                  bool bPlaceBomb = false;
                  
                  bool bPlaceRainbow = false;
                  
                  // Check Horizontally
                  int nChain = 1;
                  while (((nChain + x) < BOARD_X) && (m_GemsPlayfield[x][y].color == m_GemsPlayfield[x + nChain][y].color) ) nChain++;
                  if (nChain >= 3)
                  {
                    if (nChain == 4) bPlaceBomb = true;
                    
                    if (nChain >= 5) bPlaceRainbow = true;
                    
                    while (nChain > 0)
                    {
                      m_GemsPlayfield[x + nChain - 1][y].bRemove = true;
                      
                      score_removed_tiles++;
                      
                      if (m_GemsPlayfield[x + nChain - 1][y].type == sGem::GEMTYPE::BOMB)
                      {                       
                        m_GemsPlayfield[x + nChain - 1][y].bRemove = true;
                        
                        bBombToRemove = true;
                      }
                      
                      if (m_GemsPlayfield[x + nChain - 1][y].type == sGem::GEMTYPE::RAINBOW)
                      {
                        
                        m_GemsPlayfield[x + nChain - 1][y].bRemove = true;
                        
                        bRainbowToRemove = true;
                      }
                      
                      
                      nChain--;
                      bSwapFail = false;
                      bGemsToRemove = true;
                    }
                  }
                  
                  
                  // Check Vertically
                  nChain = 1;
                  //while ( ((nChain + y) < 8) && (m_GemsPlayfield[x][y].color == m_GemsPlayfield[x][y + nChain].color) && (!m_GemsPlayfield[x][y+ nChain].bRemove) ) nChain++;
                  while ( ((nChain + y) < BOARD_Y) && (m_GemsPlayfield[x][y].color == m_GemsPlayfield[x][y + nChain].color)  ) nChain++;
                  
                  if (nChain >= 3)
                  {
                    if (nChain == 4) bPlaceBomb = true;
                    
                    if (nChain >= 5) bPlaceRainbow = true;
                    
                    
                    while (nChain > 0)
                    {
                      m_GemsPlayfield[x][y + nChain - 1].bRemove = true;
                      
                      score_removed_tiles++;
                      
                      
                      if (m_GemsPlayfield[x][y + nChain - 1].type == sGem::GEMTYPE::BOMB)
                      {             
                        m_GemsPlayfield[x][y + nChain - 1].bRemove = true;
                        
                        bBombToRemove = true;
                      }
                      
                      if (m_GemsPlayfield[x][y + nChain - 1].type == sGem::GEMTYPE::RAINBOW)
                      {
                        m_GemsPlayfield[x][y + nChain - 1].bRemove = true;
                        
                        bRainbowToRemove = true;
                      }
                      
                      nChain--;
                      bSwapFail = false;
                      bGemsToRemove = true;
                    }
                  }
                  
                  if (bPlaceBomb)
                  {
                    m_GemsPlayfield[x][y].bRemove = false;
                    m_GemsPlayfield[x][y].type = sGem::GEMTYPE::BOMB;
                    
                    m_GemsPlayfield[x][y].sprite.SetState("bomb idle");
                    
                  }
                  
                  if (bPlaceRainbow)
                  {
                    m_GemsPlayfield[x][y].bRemove = false;
                    m_GemsPlayfield[x][y].type = sGem::GEMTYPE::RAINBOW;
                    
                    m_GemsPlayfield[x][y].sprite.SetState("rainbow idle");
                    
                  }
                  
                }
                
              }
            }
            
            
            
            if(bBombToRemove == true)
            {
              for (int x = 0; x < BOARD_X; x++)
              {
                for (int y = 0; y < BOARD_Y; y++)
                {
                  if (m_GemsPlayfield[x][y].type == sGem::GEMTYPE::BOMB && m_GemsPlayfield[x][y].bRemove)
                  {
                    for (int i = -1; i < 2; i++)
                    {
                      for (int j = -1; j < 2; j++)
                      {
                        int m = std::min(std::max(i + x, 0), BOARD_X-1);
                        int n = std::min(std::max(j + y, 0), BOARD_Y-1);
                        m_GemsPlayfield[m][n].bRemove = true;
                        
                        score_removed_tiles++;
                      }
                    }
                  }
                }
              }
              
              nNextState = STATE_CHECK_BOMB;
            }
            
            if(bRainbowToRemove == true)
            {
              for (int x = 0; x < BOARD_X; x++)
              {
                for (int y = 0; y < BOARD_Y; y++)
                {
                  if (m_GemsPlayfield[x][y].type == sGem::GEMTYPE::RAINBOW && m_GemsPlayfield[x][y].bRemove)
                  {
                    for (int k = 0; k < BOARD_X; k++)
                    {
                      for (int l = 0; l < BOARD_Y; l++)
                      {
                        if(m_GemsPlayfield[x][y].color == m_GemsPlayfield[k][l].color)
                        { 
                          m_GemsPlayfield[k][l].bRemove = true;
                          score_removed_tiles++;
                        }
                      }
                    }
                    
                    
                  }
                }
              }
              
              nNextState = STATE_CHECK_BOMB;
            }
            
            if((bBombToRemove != true) && (bRainbowToRemove != true))
            {
              nNextState = STATE_ERASE;
            }
            
            if (bGemsToRemove)
              fDelayTime = 0.75f;
            
            score += std::pow(score_combo,2)*score_removed_tiles*10;
            
            
            break;
            
            case STATE_CHECK_BOMB:
              
              bBombToRemove = false;
              
              score_removed_tiles = 0;
              
              // bomb exploded and remove vicinity
              // maybe another bomb will explode or a rainbow will be triggered
              
              for (int x = 0; x < BOARD_X; x++)
              {
                for (int y = 0; y < BOARD_Y; y++)
                {
                  if (m_GemsPlayfield[x][y].type == sGem::GEMTYPE::BOMB && m_GemsPlayfield[x][y].bRemove)
                  {
                    for (int i = -1; i < 2; i++)
                    {
                      for (int j = -1; j < 2; j++)
                      {
                        int m = std::min(std::max(i + x, 0), BOARD_X-1);
                        int n = std::min(std::max(j + y, 0), BOARD_Y-1);
                        m_GemsPlayfield[m][n].bRemove = true;
                        score_removed_tiles++;
                        
                        if (m_GemsPlayfield[m][n].type == sGem::GEMTYPE::RAINBOW && m_GemsPlayfield[m][n].bRemove)
                        {
                          for (int k = 0; k < BOARD_X; k++)
                          {
                            for (int l = 0; l < BOARD_Y; l++)
                            {
                              if(m_GemsPlayfield[m][n].color == m_GemsPlayfield[k][l].color)
                              {
                                m_GemsPlayfield[k][l].bRemove = true;
                                score_removed_tiles++;
                              }
                            }
                          }
                          
                          m_GemsPlayfield[m][n].type = sGem::GEMTYPE::GEM;
                        }
                        
                      }
                    }
                    m_GemsPlayfield[x][y].type = sGem::GEMTYPE::GEM;
                    bBombToRemove = true;
                  }
                }
              }
              
              //maybe this triggers another bomb
              if(bBombToRemove)
                nNextState = STATE_CHECK_BOMB;
              else
                nNextState = STATE_ERASE;
              
              fDelayTime = 0.75f;
              
              score += std::pow(score_combo,2)*score_removed_tiles*10;
            
              
              break;
              
              case STATE_ERASE:
                if (!bGemsToRemove)
                {
                  if (bSwapFail)
                  {
                    std::swap(m_GemsPlayfield[nCursor.x][nCursor.y], m_GemsPlayfield[nSwap.x][nSwap.y]);
                  }
                  
                  nNextState = STATE_USER;
                }
                else
                {
                  for (int x = 0; x < BOARD_X; x++)
                  {
                    for (int y = 0; y < BOARD_Y; y++)
                    {
                      if (m_GemsPlayfield[x][y].bRemove)
                      {
                        m_GemsPlayfield[x][y].bExist = false;
                        m_GemsPlayfield[x][y].type == sGem::GEMTYPE::GEM;
                        float offset_X = 0.5f*(SCREENSIZE_X-BOARD_X*TILESIZE_X);
                        boom(offset_X + x * TILESIZE_X + TILESIZE_X/2, y * TILESIZE_Y + TILESIZE_Y/2, 15, m_GemsPlayfield[x][y].color);
                        nTotalGems--;
                      }
                    }
                  }
                  
                  bGemsToRemove = false;
                  
                  if(bBombToRemove)
                    nNextState = STATE_CHECK_BOMB;
                  else
                    nNextState = STATE_COMPRESS;
                }
                break;
              case STATE_COMPRESS:
                
                for (int y = BOARD_Y-2; y >= 0; y--)
                {
                  for (int x = 0; x < BOARD_X; x++)
                  {
                    if (m_GemsPlayfield[x][y].bExist && !m_GemsPlayfield[x][y + 1].bExist)
                      std::swap(m_GemsPlayfield[x][y], m_GemsPlayfield[x][y + 1]);
                  }
                }
                
                fDelayTime = 0.1f;
                nNextState = STATE_NEWGEMS;
                break;
                
              case STATE_NEWGEMS:
                for (int x = 0; x < BOARD_X; x++)
                {
                  if (!m_GemsPlayfield[x][0].bExist)
                  {
                    
                    m_GemsPlayfield[x][0].color = rand() % 7 + 1;
                    m_GemsPlayfield[x][0].animation_mode = 1;
                    m_GemsPlayfield[x][0].sprite = m_GemSprite[m_GemsPlayfield[x][0].color-1];
                    m_GemsPlayfield[x][0].bExist = true;
                    m_GemsPlayfield[x][0].bRemove = false;
                    m_GemsPlayfield[x][0].type = rand() % 64 + 1 > 1 ? sGem::GEMTYPE::GEM : rand() % 64 + 1 > 1 ? sGem::GEMTYPE::BOMB : sGem::GEMTYPE::RAINBOW;
                    
                    if (m_GemsPlayfield[x][0].type ==  sGem::GEMTYPE::GEM)
                      m_GemsPlayfield[x][0].sprite.SetState("gem idle");
                    else if (m_GemsPlayfield[x][0].type ==  sGem::GEMTYPE::BOMB)
                      m_GemsPlayfield[x][0].sprite.SetState("bomb idle");
                    else if (m_GemsPlayfield[x][0].type ==  sGem::GEMTYPE::RAINBOW)
                      m_GemsPlayfield[x][0].sprite.SetState("rainbow idle");
                    
                    
                    nTotalGems++;
                  }
                }
                
                if (nTotalGems < 64)
                {
                  fDelayTime = 0.1f;
                  nNextState = STATE_COMPRESS;
                }
                else
                  nNextState = STATE_CHECK;
                
                
                break;
      }
      
      
      nState = nNextState;
      
    } // End Gameplay
    
    
    Clear(olc::VERY_DARK_BLUE);
    
    //0.5f*(SCREENSIZE_X-BOARD_X*TILESIZE_X);
    
    float offset_X = 0.5f*(SCREENSIZE_X-BOARD_X*TILESIZE_X);
    
    DrawDecal({offset_X, 0.0f}, gfxTiles.Decal());
    
    //DrawDecal({0.0f, 0.0f}, gfxTiles.Decal());
    
    
    
    // draw the playfield
    for (int x = 0; x < BOARD_X; x++)
    {
      for (int y = 0; y < BOARD_Y; y++)
      {
        if (m_GemsPlayfield[x][y].bExist)
        {  
          float offset_X = 0.5f*(SCREENSIZE_X-BOARD_X*TILESIZE_X);
          m_GemsPlayfield[x][y].sprite.Draw(fElapsedTime, {offset_X + x*float(TILESIZE_X), y*float(TILESIZE_Y)}, olc::Sprite::Flip::NONE, m_GemsPlayfield[x][y].bRemove ? olc::DARK_GREY : olc::WHITE); // draws the sprite at location x, y and animates it
        }
      }
    }
    
    
    // Draw score
    
    float offset_numbers_X = 0.5f*(SCREENSIZE_X-10*NUMBERS_X);
    
    
    getScoreInDigits();
    int countIdx = 0;
    int stackSize = stack_score.size();
    while (!stack_score.empty()) {

            uint32_t numberOnStack = stack_score.top();
            stack_score.pop();
            DrawPartialDecal({SCREENSIZE_X-offset_numbers_X+(-1*stackSize+countIdx)*NUMBERS_X, 8.0f*TILESIZE_Y}, gfxNumbers.Decal(), {numberOnStack*float(NUMBERS_X), 0.0f}, {NUMBERS_X,NUMBERS_Y} );
            countIdx++;
        }

        // adding 0 to unused score fields
        for (int i = 0; i <= 9-countIdx; i++)
        {
          DrawPartialDecal({offset_numbers_X+i*NUMBERS_X, 8.0f*TILESIZE_Y}, gfxNumbers.Decal(), {0*NUMBERS_X, 0}, {NUMBERS_X,NUMBERS_Y} );
            
           // reg.batch.draw(texNumbers[0], 60+i*texNumbers[0].getRegionWidth(),reg.windowHeight-texNumbers[0].getRegionHeight()-20 );
        }
    //DrawDecal({offset_X, 8.0f*TILESIZE_Y}, gfxNumbers.Decal());
    
    DrawStringDecal({0,0},std::to_string(score_combo));
    
    // Draw fragments if applicable
    
    for (auto &f : fragments)
    {
      f.sprite.Draw(fElapsedTime, {f.x, f.y});
      f.x += f.vx * fElapsedTime;
      f.y += f.vy * fElapsedTime;
    }
    
    // erase - remove_if idom
    fragments.erase(
      std::remove_if(fragments.begin(), fragments.end(),
                     [&](const sFragment &f) { return f.x < 0 || f.x > ScreenWidth() || f.y < 0 || f.y > ScreenHeight(); }),
                    fragments.end());
    
    
    // Graceful exit if user is in full screen mode
    return !GetKey(olc::Key::ESCAPE).bPressed;
  }
};


int main()
{
  JuwelsGame demo;
  if (demo.Construct(SCREENSIZE_X, SCREENSIZE_Y, 1, 1))
    demo.Start();
  
  return 0;
}
