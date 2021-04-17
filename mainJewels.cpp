#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"

#define OLC_PGEX_ANIMSPR
#include "olcPGEX_AnimatedSprite.h"

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
    //bool bBomb;
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
  
  // define sprite in your PGE program
  std::vector<olc::AnimatedSprite> m_GemSprite;
  
  // define sprite in your PGE program
  std::vector<olc::AnimatedSprite> m_BombSprite;
  
  // define sprite in your PGE program
  std::vector<olc::AnimatedSprite> m_FragmentSprite;
  
  
  // Playfield
  sGem m_GemsPlayfield[8][8];
  
  float fDelayTime = 0.0f;
  
  int nTotalGems = 0;
  
  olc::vi2d nCursor = {0,0};
  olc::vi2d nSwap = {0,0};
  //int nCursorX = 0, nCursorY = 0;
  //int nSwapX = 0, nSwapY = 0;

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
  
public:
  bool OnUserCreate() override
  {
    // Called once at the start, so create things here
    
    // configure the sprite:
    olc::Renderable* spritesheet = new olc::Renderable();
    spritesheet->Load("./assets/Gem_Bomb_Rainbow.png");
    
    gfxTiles.Load("./assets/Board.png");
    
    for (int j = 0; j < 7; j++)
    {
      olc::AnimatedSprite sprite;
      sprite.mode = olc::AnimatedSprite::SPRITE_MODE::SINGLE; // set sprite to use a single spritesheet
      sprite.spriteSheet = spritesheet; // define image to use for the spritesheet
      sprite.SetSpriteSize({52, 52}); // define size of each sprite with an olc::vi2d
      sprite.SetSpriteScale(1.0f); // define scale of sprite; 1.0f is original size. Must be above 0 and defaults to 1.0f
      sprite.type = olc::AnimatedSprite::SPRITE_TYPE::DECAL;
      
      // define states - state name and vector of olc::vi2d to define the top-left position of each frame in the spritesheet
      std::vector<olc::vi2d> pos;
      
      
      sprite.AddState("gem idle", 0.1f, olc::AnimatedSprite::PLAY_MODE::LOOP, {olc::vi2d({0,   52*(3*j)}),});
      
      for(int i=0; i < 39; i++)
        pos.push_back( olc::vi2d({i*52,   (3*j)*52}));
      
      sprite.AddState("gem rotating", 0.1f, olc::AnimatedSprite::PLAY_MODE::LOOP, pos);
      
      
      
      sprite.AddState("bomb idle", 0.1f, olc::AnimatedSprite::PLAY_MODE::LOOP, {olc::vi2d({0,   52*(3*j)+52}),});
      
      pos.clear();
      for(int i=0; i < 39; i++)
        pos.push_back( olc::vi2d({i*52,   (3*j)*52+52}));
      
      sprite.AddState("bomb rotating", 0.1f, olc::AnimatedSprite::PLAY_MODE::LOOP, pos);
      
      sprite.AddState("rainbow idle", 0.1f, olc::AnimatedSprite::PLAY_MODE::LOOP, {olc::vi2d({0,   52*(3*j)+104}),});
      
      pos.clear();
      for(int i=0; i < 39; i++)
        pos.push_back( olc::vi2d({i*52,   (3*j)*52+104}));
      
      sprite.AddState("rainbow rotating", 0.1f, olc::AnimatedSprite::PLAY_MODE::LOOP, pos);
      
      
      // set initial state
      sprite.SetState("gem idle");
      
      m_GemSprite.push_back(sprite);
    }
    
    // configure the sprite:
    olc::Renderable* spritesheetBomb = new olc::Renderable();
    spritesheetBomb->Load("./assets/Gem_Bomb.png");
    
    for (int j = 0; j < 7; j++)
    {
      olc::AnimatedSprite sprite;
      sprite.mode = olc::AnimatedSprite::SPRITE_MODE::SINGLE; // set sprite to use a single spritesheet
      sprite.spriteSheet = spritesheetBomb; // define image to use for the spritesheet
      sprite.SetSpriteSize({52, 52}); // define size of each sprite with an olc::vi2d
      sprite.SetSpriteScale(1.0f); // define scale of sprite; 1.0f is original size. Must be above 0 and defaults to 1.0f
      sprite.type = olc::AnimatedSprite::SPRITE_TYPE::DECAL;
      
      // define states - state name and vector of olc::vi2d to define the top-left position of each frame in the spritesheet
      std::vector<olc::vi2d> pos;
      
      
      sprite.AddState("idle", 0.1f, olc::AnimatedSprite::PLAY_MODE::LOOP, {olc::vi2d({0,   52*(2*j)+52}),});
      
      for(int i=0; i < 39; i++)
        pos.push_back( olc::vi2d({i*52,   (2*j)*52+52}));
      
      sprite.AddState("rotating", 0.1f, olc::AnimatedSprite::PLAY_MODE::LOOP, pos);
      
      // set initial state
      sprite.SetState("idle");
      
      m_BombSprite.push_back(sprite);
    }
    
    for (int j = 0; j < 7; j++)
    {
      olc::AnimatedSprite sprite;
      sprite.mode = olc::AnimatedSprite::SPRITE_MODE::SINGLE; // set sprite to use a single spritesheet
      sprite.spriteSheet = spritesheet; // define image to use for the spritesheet
      sprite.SetSpriteSize({52, 52}); // define size of each sprite with an olc::vi2d
      sprite.SetSpriteScale(0.25f); // define scale of sprite; 1.0f is original size. Must be above 0 and defaults to 1.0f
      sprite.type = olc::AnimatedSprite::SPRITE_TYPE::DECAL;
      
      // define states - state name and vector of olc::vi2d to define the top-left position of each frame in the spritesheet
      std::vector<olc::vi2d> pos;
      
      for(int i=0; i < 39; i++)
        pos.push_back( olc::vi2d({i*52,   (3*j)*52+0}));
      
      
      sprite.AddState("rotating", 0.02f, olc::AnimatedSprite::PLAY_MODE::LOOP, pos);
      
      // set initial state
      sprite.SetState("rotating");
      
      m_FragmentSprite.push_back(sprite);
    }
    
    
    // fill the playfield
    for (int x = 0; x < 8; x++)
    {
      for (int y = 0; y < 8; y++)
      {
        m_GemsPlayfield[x][y].color = rand() % 7 + 1;
        m_GemsPlayfield[x][y].animation_mode = 0;
        m_GemsPlayfield[x][y].sprite = m_GemSprite[m_GemsPlayfield[x][y].color-1];
        m_GemsPlayfield[x][y].bExist = false;
        m_GemsPlayfield[x][y].bRemove = false;
        //m_GemsPlayfield[x][y].bBomb = false;
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
              olc::vi2d vMouse = { GetMouseX(), GetMouseY() };
              
              // Work out active cell
              olc::vi2d vCell = { vMouse.x / 52, vMouse.y / 52 };
              
              if(bIsfirstClickMouse == true)
              {
                if(vCell.x >= 0 && vCell.x < 8 && vCell.y >= 0 && vCell.y <8)
                {
                  m_GemsPlayfield[vCell.x][vCell.y].animation_mode = 1;//rand() % 2 +1;//(m_GemsPlayfield[vCell.x][vCell.y].animation_mode+1) %3;
                  
                  //if(m_GemsPlayfield[vCell.x][vCell.y].animation_mode==0)
                  //  m_GemsPlayfield[vCell.x][vCell.y].sprite.SetState("idle");
                  
                  if (m_GemsPlayfield[vCell.x][vCell.y].type ==  sGem::GEMTYPE::GEM)
                     m_GemsPlayfield[vCell.x][vCell.y].sprite.SetState("gem rotating");
                  else if (m_GemsPlayfield[vCell.x][vCell.y].type ==  sGem::GEMTYPE::BOMB)
                    m_GemsPlayfield[vCell.x][vCell.y].sprite.SetState("bomb rotating");
                  else if (m_GemsPlayfield[vCell.x][vCell.y].type ==  sGem::GEMTYPE::RAINBOW)
                    m_GemsPlayfield[vCell.x][vCell.y].sprite.SetState("rainbow rotating");
                  
//                   if(m_GemsPlayfield[vCell.x][vCell.y].animation_mode==1)
//                     m_GemsPlayfield[vCell.x][vCell.y].sprite.SetState("rotating");
//                   
//                   if(m_GemsPlayfield[vCell.x][vCell.y].animation_mode==2)
//                     m_GemsPlayfield[vCell.x][vCell.y].sprite.SetState("tilting");
//                 
                nCursor.x = vCell.x;
                nCursor.y = vCell.y;
                
                bIsfirstClickMouse = false;
                }
              }
              else
              {
                nSwap.x = nCursor.x;
                nSwap.y = nCursor.y;
                
                if(vCell.x >= 0 && vCell.x < 8 && vCell.y >= 0 && vCell.y <8)
                {
                  // second tile in vicinity
                  if((nCursor-vCell).mag2() == 1 )
                  {
                    nSwap.x = vCell.x;
                    nSwap.y = vCell.y;
                    
                    m_GemsPlayfield[nCursor.x][nCursor.y].animation_mode = 0;//(m_GemsPlayfield[vCell.x][vCell.y].animation_mode+1) %3;
                    //m_GemsPlayfield[nCursor.x][nCursor.y].sprite.SetState("idle");
                    
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
                    //m_GemsPlayfield[nCursor.x][nCursor.y].sprite.SetState("idle");
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
                  
                  //if(m_GemsPlayfield[vCell.x][vCell.y].animation_mode==0)
                  //  m_GemsPlayfield[vCell.x][vCell.y].sprite.SetState("idle");
                  
                    if (m_GemsPlayfield[vCell.x][vCell.y].type ==  sGem::GEMTYPE::GEM)
                      m_GemsPlayfield[vCell.x][vCell.y].sprite.SetState("gem rotating");
                    else if (m_GemsPlayfield[vCell.x][vCell.y].type ==  sGem::GEMTYPE::BOMB)
                      m_GemsPlayfield[vCell.x][vCell.y].sprite.SetState("bomb rotating");
                    else if (m_GemsPlayfield[vCell.x][vCell.y].type ==  sGem::GEMTYPE::RAINBOW)
                      m_GemsPlayfield[vCell.x][vCell.y].sprite.SetState("rainbow rotating");
                    
                    //               
                    
                  //if(m_GemsPlayfield[vCell.x][vCell.y].animation_mode==1)
                    //m_GemsPlayfield[vCell.x][vCell.y].sprite.SetState("rotating");
                  
                  //if(m_GemsPlayfield[vCell.x][vCell.y].animation_mode==2)
                   // m_GemsPlayfield[vCell.x][vCell.y].sprite.SetState("tilting");
                    
                    bIsfirstClickMouse = false;
                  }
                }
                
                /*if (GetKey(VK_LEFT).bPressed && nCursorX > 0) nSwapX = nCursorX - 1;
                 i f (G*etKey(VK_RIGHT).bPressed && nCursorX < 7) nSwapX = nCursorX + 1;
                 if (GetKey(VK_UP).bPressed && nCursorY > 0) nSwapY = nCursorY - 1;
                 if (GetKey(VK_DOWN).bPressed && nCursorY < 7) nSwapY = nCursorY + 1;
                 if (nSwapX != nCursorX || nSwapY != nCursorY) nNextState = STATE_SWAP;
                 */
                
              }
              
              
              
            }
            /*if (!GetKey(VK_SPACE).bHeld)
            {
              if (GetKey(VK_LEFT).bPressed) nCursorX--;
              if (GetKey(VK_RIGHT).bPressed) nCursorX++;
              if (GetKey(VK_UP).bPressed) nCursorY--;
              if (GetKey(VK_DOWN).bPressed) nCursorY++;
              if (nCursorX < 0) nCursorX = 0;
              if (nCursorX > 7) nCursorX = 7;
              if (nCursorY < 0) nCursorY = 0;
              if (nCursorY > 7) nCursorY = 7;
            }
            else
            {
              nSwapX = nCursorX;
              nSwapY = nCursorY;
              if (GetKey(VK_LEFT).bPressed && nCursorX > 0) nSwapX = nCursorX - 1;
              if (GetKey(VK_RIGHT).bPressed && nCursorX < 7) nSwapX = nCursorX + 1;
              if (GetKey(VK_UP).bPressed && nCursorY > 0) nSwapY = nCursorY - 1;
              if (GetKey(VK_DOWN).bPressed && nCursorY < 7) nSwapY = nCursorY + 1;
              if (nSwapX != nCursorX || nSwapY != nCursorY) nNextState = STATE_SWAP;
          }*/
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
            
            
            for (int x = 0; x < 8; x++)
            {
              for (int y = 0; y < 8; y++)
              {
                if (!m_GemsPlayfield[x][y].bRemove)
                {
                  bool bPlaceBomb = false;
                  
                  bool bPlaceRainbow = false;
                  
                  // Check Horizontally
                  int nChain = 1;
                  while (((nChain + x) < 8) && (m_GemsPlayfield[x][y].color == m_GemsPlayfield[x + nChain][y].color) ) nChain++;
                  if (nChain >= 3)
                  {
                    if (nChain == 4) bPlaceBomb = true;
                    
                    if (nChain >= 5) bPlaceRainbow = true;
                    
                    while (nChain > 0)
                    {
                      m_GemsPlayfield[x + nChain - 1][y].bRemove = true;
                      
                      if (m_GemsPlayfield[x + nChain - 1][y].type == sGem::GEMTYPE::BOMB)
                      {
//                         for (int i = -1; i < 2; i++)
//                         {
//                           for (int j = -1; j < 2; j++)
//                           {
//                             int m = std::min(std::max(i + (x + nChain - 1), 0), 7);
//                             int n = std::min(std::max(j + y, 0), 7);
//                             m_GemsPlayfield[m][n].bRemove = true;
//                           }
//                         }
                        
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
                  while ( ((nChain + y) < 8) && (m_GemsPlayfield[x][y].color == m_GemsPlayfield[x][y + nChain].color)  ) nChain++;
                  
                  if (nChain >= 3)
                  {
                    if (nChain == 4) bPlaceBomb = true;
                    
                    if (nChain >= 5) bPlaceRainbow = true;
                    
                    
                    while (nChain > 0)
                    {
                      m_GemsPlayfield[x][y + nChain - 1].bRemove = true;
                      
                      
                      if (m_GemsPlayfield[x][y + nChain - 1].type == sGem::GEMTYPE::BOMB)
                      {
//                         for (int i = -1; i < 2; i++)
//                         {
//                           for (int j = -1; j < 2; j++)
//                           {
//                             int m = std::min(std::max(i + x, 0), 7);
//                             int n = std::min(std::max(j + (y + nChain - 1), 0), 7);
//                             m_GemsPlayfield[m][n].bRemove = true;
//                           }
//                         }
                        
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
                    //m_GemsPlayfield[x][y].bBomb = true;
                    m_GemsPlayfield[x][y].bRemove = false;
                    m_GemsPlayfield[x][y].type = sGem::GEMTYPE::BOMB;
                    
                    m_GemsPlayfield[x][y].sprite.SetState("bomb rotating");
               
                  }
                  
                  if (bPlaceRainbow)
                  {
                    //m_GemsPlayfield[x][y].bBomb = true;
                    m_GemsPlayfield[x][y].bRemove = false;
                    m_GemsPlayfield[x][y].type = sGem::GEMTYPE::RAINBOW;
                    
                    m_GemsPlayfield[x][y].sprite.SetState("rainbow rotating");
               
                  }
                  
                }
                
              }
            }
            
            
            
            if(bBombToRemove == true)
            {
              for (int x = 0; x < 8; x++)
              {
                for (int y = 0; y < 8; y++)
                {
                  if (m_GemsPlayfield[x][y].type == sGem::GEMTYPE::BOMB && m_GemsPlayfield[x][y].bRemove)
                  {
                    for (int i = -1; i < 2; i++)
                    {
                      for (int j = -1; j < 2; j++)
                      {
                        int m = std::min(std::max(i + x, 0), 7);
                        int n = std::min(std::max(j + y, 0), 7);
                        m_GemsPlayfield[m][n].bRemove = true;
                      }
                    }
                  }
                }
              }
              
              nNextState = STATE_CHECK_BOMB;
            }
            
            if(bRainbowToRemove == true)
            {
              for (int x = 0; x < 8; x++)
              {
                for (int y = 0; y < 8; y++)
                {
                  if (m_GemsPlayfield[x][y].type == sGem::GEMTYPE::RAINBOW && m_GemsPlayfield[x][y].bRemove)
                  {
                    for (int k = 0; k < 8; k++)
                    {
                      for (int l = 0; l < 8; l++)
                      {
                        if(m_GemsPlayfield[x][y].color == m_GemsPlayfield[k][l].color)
                          m_GemsPlayfield[k][l].bRemove = true;
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
              fDelayTime = 1.75f;
            
          
          
          
          break;
          
          case STATE_CHECK_BOMB:
            
            bBombToRemove = false;
            
            // bomb exploded and remove vicinity
            // maybe another bomb will explode or a rainbow will be triggered
            
            for (int x = 0; x < 8; x++)
            {
              for (int y = 0; y < 8; y++)
              {
                if (m_GemsPlayfield[x][y].type == sGem::GEMTYPE::BOMB && m_GemsPlayfield[x][y].bRemove)
                {
                  for (int i = -1; i < 2; i++)
                  {
                    for (int j = -1; j < 2; j++)
                    {
                      int m = std::min(std::max(i + x, 0), 7);
                      int n = std::min(std::max(j + y, 0), 7);
                      m_GemsPlayfield[m][n].bRemove = true;
                      
                      if (m_GemsPlayfield[m][n].type == sGem::GEMTYPE::RAINBOW && m_GemsPlayfield[m][n].bRemove)
                      {
                        for (int k = 0; k < 8; k++)
                        {
                          for (int l = 0; l < 8; l++)
                          {
                            if(m_GemsPlayfield[m][n].color == m_GemsPlayfield[k][l].color)
                              m_GemsPlayfield[k][l].bRemove = true;
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
            
             fDelayTime = 1.75f;
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
              for (int x = 0; x < 8; x++)
              {
                for (int y = 0; y < 8; y++)
                {
                  if (m_GemsPlayfield[x][y].bRemove)
                  {
                    m_GemsPlayfield[x][y].bExist = false;
                    m_GemsPlayfield[x][y].type == sGem::GEMTYPE::GEM;
                    boom(x * 52 + 26, y * 52 + 26, 15, m_GemsPlayfield[x][y].color);
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
            
            for (int y = 6; y >= 0; y--)
            {
              for (int x = 0; x < 8; x++)
              {
                if (m_GemsPlayfield[x][y].bExist && !m_GemsPlayfield[x][y + 1].bExist)
                  std::swap(m_GemsPlayfield[x][y], m_GemsPlayfield[x][y + 1]);
              }
            }
            
            fDelayTime = 0.1f;
            nNextState = STATE_NEWGEMS;
            break;
            
          case STATE_NEWGEMS:
            for (int x = 0; x < 8; x++)
            {
              if (!m_GemsPlayfield[x][0].bExist)
              {
                
                m_GemsPlayfield[x][0].color = rand() % 5 + 1;
                m_GemsPlayfield[x][0].animation_mode = 1;
                m_GemsPlayfield[x][0].sprite = m_GemSprite[m_GemsPlayfield[x][0].color-1];
                m_GemsPlayfield[x][0].bExist = true;
                m_GemsPlayfield[x][0].bRemove = false;
                //m_GemsPlayfield[x][0].bBomb = rand() % 64 + 1 <= 1 ? true : false;
                m_GemsPlayfield[x][0].type = rand() % 64 + 1 > 1 ? sGem::GEMTYPE::GEM : rand() % 64 + 1 > 16 ? sGem::GEMTYPE::BOMB : sGem::GEMTYPE::RAINBOW;
                
//                 if (m_GemsPlayfield[x][0].bBomb)
//                   m_GemsPlayfield[x][0].sprite.SetState("bomb idle");
//                 else
//                   m_GemsPlayfield[x][0].sprite.SetState("gem idle");
//                 
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
    
    DrawDecal({0.0f, 0.0f}, gfxTiles.Decal());
    
    // draw the playfield
    for (int x = 0; x < 8; x++)
    {
      for (int y = 0; y < 8; y++)
      {
        if (m_GemsPlayfield[x][y].bExist)
        {  //if(m_GemsPlayfield[x][y].bBomb == true)
            //  m_BombSprite[m_GemsPlayfield[x][y].color-1].Draw(fElapsedTime, {x*52.0f, y*52.0f}, olc::Sprite::Flip::NONE, m_GemsPlayfield[x][y].bRemove ? olc::VERY_DARK_GREY : olc::WHITE);
            //else
              m_GemsPlayfield[x][y].sprite.Draw(fElapsedTime, {x*52.0f, y*52.0f}, olc::Sprite::Flip::NONE, m_GemsPlayfield[x][y].bRemove ? olc::DARK_GREY : olc::WHITE); // draws the sprite at location x, y and animates it
        }
        /*if(m_GemsPlayfield[x][y].animation_mode == 0)
        {
          m_GemSprite[m_GemsPlayfield[x][y].color-1].SetState("idle");
          m_GemSprite[m_GemsPlayfield[x][y].color-1].Draw(fElapsedTime, {x*52.0f, y*52.0f}); // draws the sprite at location x:20, y:20 and animates it
        }
        if(m_GemsPlayfield[x][y].animation_mode == 1)
        {
          m_GemSprite[m_GemsPlayfield[x][y].color-1].SetState("rotating");
          m_GemSprite[m_GemsPlayfield[x][y].color-1].Draw(fElapsedTime, {x*52.0f, y*52.0f}); // draws the sprite at location x:20, y:20 and animates it
        }
        if(m_GemsPlayfield[x][y].animation_mode == 2)
        {
          m_GemSprite[m_GemsPlayfield[x][y].color-1].SetState("tilting");
          m_GemSprite[m_GemsPlayfield[x][y].color-1].Draw(fElapsedTime, {x*52.0f, y*52.0f}); // draws the sprite at location x:20, y:20 and animates it
        
        }
        */
        
      }
    }
    
    //std::cout << fragments.size() << " ";
    
    
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
    
    
//     std::remove_if(fragments.begin(), fragments.end(), 
//                    [&](const sFragment &f)
//                    {
//                      //return f.x < 0 || f.x > ScreenWidth() || f.y < 0 || f.y > ScreenHeight();
//                      return f.x < 0 || f.x > 640 || f.y < 0 || f.y > 480;
//                    });
//     


    
//     Get Mouse in world
//     if (GetMouse(0).bPressed)
//     {
//       olc::vi2d vMouse = { GetMouseX(), GetMouseY() };
//       
//       Work out active cell
//       olc::vi2d vCell = { vMouse.x / 52, vMouse.y / 52 };
//       
//       if(vCell.x >= 0 && vCell.x < 8 && vCell.y >= 0 && vCell.y <8)
//       {
//         m_GemsPlayfield[vCell.x][vCell.y].animation_mode = (m_GemsPlayfield[vCell.x][vCell.y].animation_mode+1) %3;
//         
//         if(m_GemsPlayfield[vCell.x][vCell.y].animation_mode==0)
//           m_GemsPlayfield[vCell.x][vCell.y].sprite.SetState("idle");
//         
//         if(m_GemsPlayfield[vCell.x][vCell.y].animation_mode==1)
//           m_GemsPlayfield[vCell.x][vCell.y].sprite.SetState("rotating");
//         
//         if(m_GemsPlayfield[vCell.x][vCell.y].animation_mode==2)
//           m_GemsPlayfield[vCell.x][vCell.y].sprite.SetState("tilting");
//       }
//       
//     }
    
    //
      
    
                
    
    if(GetKey(olc::Key::S).bPressed)
    {
      
      !m_GemSprite[0].GetState().compare("idle") ? m_GemSprite[0].SetState("rotating") : !m_GemSprite[0].GetState().compare("rotating") ? m_GemSprite[0].SetState("tilting") : m_GemSprite[0].SetState("idle");
      
    }
    // Graceful exit if user is in full screen mode
    return !GetKey(olc::Key::ESCAPE).bPressed;
  }
};


int main()
{
  JuwelsGame demo;
  if (demo.Construct(640, 480, 1, 1))
    demo.Start();
  
  return 0;
}
