#include "raylib.h"

struct AnimData {
    Rectangle rec;
    Vector2 pos;
    int frame;
    float updateTime;
    float runningTime;
};

bool isOnGround(AnimData data, int windowHeight) {
    return data.pos.y >= windowHeight - data.rec.height;
};

AnimData updateAnimData(AnimData data, float deltaTime, int maxFrame){
  // Update running time
  data.runningTime += deltaTime;
  if (data.runningTime >= data.updateTime) {
    data.runningTime = 0.0;
    // Update animation frame
    data.rec.x = data.frame * data.rec.width;
    data.frame++;
    if (data.frame > maxFrame) {
        data.frame = 0;
    }
  }
  return data;
}


int main() {

  // Start with a default size for InitWindow — browser will override it
  InitWindow(800, 600, "Dapper_Dasher"); //512 // 380
  
  InitAudioDevice();      // Initialize audio device
  
  Sound loseSound = LoadSound("sounds/lose.wav");
  Music mainMusic = LoadMusicStream("sounds/main.wav");
  if (IsMusicValid(mainMusic)) {
    PlayMusicStream(mainMusic);
    SetMusicVolume(mainMusic, 0.5f);
  }
  Sound runningSound = LoadSound("sounds/running.wav");
  Sound gruntSound = LoadSound("sounds/grunt.wav");
  Sound winSound = LoadSound("sounds/win.wav");
  Sound jumpSound = LoadSound("sounds/jump.wav");
  Sound impactSound = LoadSound("sounds/impact.wav");
  
  // Now get actual browser canvas dimensions
  int screenWidth = GetScreenWidth();
  int screenHeight = GetScreenHeight();

  // Window dimensions
  int windowDimensions[2] = { screenWidth, screenHeight };


  // Accelaration due to gravity (pixel/s)
  const int gravity{1'000};

  // Nebula texture
  Texture2D nebula = LoadTexture("textures/12_nebula_spritesheet.png");
  AnimData nebulae[10]{};
  int index = 0;
  for (auto& neb: nebulae) {
    neb = {
        {0.0, 0.0, static_cast<float>(nebula.width / 8), static_cast<float>(nebula.height / 8)},
        {static_cast<float>(windowDimensions[0] + index * 300), static_cast<float>(windowDimensions[1] - nebula.height / 8)},
        0,
        1.0 / 16.0,
        0.0,
    };
    ++index;
  }

  float finishLine{nebulae[(sizeof(nebulae) / sizeof(nebulae[0])) - 1].pos.x + 800};

  // Nebula X velocity pixels/second
  int nebVel{-200};


  // Scarfy texture
  Texture2D scarfy = LoadTexture("textures/scarfy.png");
  AnimData scarfyData;
  scarfyData.rec.width = scarfy.width / 6;
  scarfyData.rec.height = scarfy.height;
  scarfyData.rec.x = 0;
  scarfyData.rec.y = 0;
  scarfyData.pos.x = windowDimensions[0] / 2 - scarfyData.rec.width / 2;
  scarfyData.pos.y = windowDimensions[1] - scarfyData.rec.height;
  scarfyData.frame = 0;
  scarfyData.updateTime = 1.0 / 12.0;
  scarfyData.runningTime = 0.0;

  // Rectangle in air status
  bool isInAir{}; // default false
  // Jump velocity pixels/s
  const int jumpVel{-600};

  int velocity{0};

  Texture2D background = LoadTexture("textures/far-buildings.png");
  float bgX{};
  Texture2D midground = LoadTexture("textures/back-buildings.png");
  float mgX{};
  Texture2D foreground = LoadTexture("textures/foreground.png");
  float fgX{};

  bool collision{};
  int collision_count = 0;

  SetTargetFPS(60);
  while(!WindowShouldClose()) {
    UpdateMusicStream(mainMusic);
    // Delta time
    const float dT{GetFrameTime()};

    // Start Drawing
    BeginDrawing();
    ClearBackground(WHITE);

    float bgScale = static_cast<float>(GetScreenHeight()) / background.height;
    float bgWidthScaled = background.width * bgScale;

    float mgScale = static_cast<float>(GetScreenHeight()) / midground.height;
    float mgWidthScaled = midground.width * mgScale;

    float fgScale = static_cast<float>(GetScreenHeight()) / foreground.height;
    float fgWidthScaled = foreground.width * fgScale;

    // Scroll background
    bgX -= 20 * dT;
    if (bgX <= -bgWidthScaled) {
      bgX = 0.0f;
    }

    // Scroll midground
    mgX -= 40 * dT;
    if (mgX <= -mgWidthScaled) {
      mgX = 0.0f;
    }

    // Scroll foreground
    fgX -= 80 * dT;
    if (fgX <= -fgWidthScaled) {
      fgX = 0.0f;
    }


    // Draw the background
    DrawTextureEx(background, { bgX, 0 }, 0.0f, bgScale, WHITE);
    DrawTextureEx(background, { bgX + bgWidthScaled, 0 }, 0.0f, bgScale, WHITE);

    // Draw the midground
    DrawTextureEx(midground, { mgX, 0 }, 0.0f, mgScale, WHITE);
    DrawTextureEx(midground, { mgX + mgWidthScaled, 0 }, 0.0f, mgScale, WHITE);


    // Draw the foreground
    DrawTextureEx(foreground, { fgX, 0 }, 0.0f, fgScale, WHITE);
    DrawTextureEx(foreground, { fgX + fgWidthScaled, 0 }, 0.0f, fgScale, WHITE);

    // Ground check
    if (isInAir) PlaySound(runningSound);

    if (isOnGround(scarfyData, windowDimensions[1])) {
      // rectangle is on the ground
      velocity = 0;
      isInAir = false;
    } else {
      // apply gravity
      velocity += gravity * dT;
      isInAir = true;
    }

    // Jump check
    if (IsKeyPressed(KEY_SPACE) && !isInAir) {
      PlaySound(jumpSound);
      velocity += jumpVel;
    }

    // Update nebulae position
    for (auto& neb: nebulae) {
      neb.pos.x += nebVel * dT;
    }

    // Update finishline
    finishLine += nebVel * dT;

    // update scarfy position
    scarfyData.pos.y += velocity * dT;

    if (!isInAir) {
      scarfyData = updateAnimData(scarfyData, dT, 5);
    }

    // update nebulea running time and animation frame
    for (auto& neb: nebulae) {
        neb = updateAnimData(neb, dT, 7);
    }

    for (AnimData& nebula: nebulae) {
        float pad{40};
        Rectangle nebRec{
            nebula.pos.x + pad,
            nebula.pos.y + pad,
            nebula.rec.width - 2 * pad,
            nebula.rec.height - 2 * pad
        };
        Rectangle scarfyRec{
            scarfyData.pos.x,
            scarfyData.pos.y,
            scarfyData.rec.width,
            scarfyData.rec.height
        };
        if (CheckCollisionRecs(nebRec, scarfyRec)){
            collision = true;
            collision_count += 1;
            if (collision_count <= 1) {
              PlaySound(gruntSound);
              PlaySound(impactSound);
              PlaySound(loseSound);
            }
        }
    }


    if (collision || scarfyData.pos.x > finishLine) {
      static bool winSoundPlayed = false;
      bool playerWon = !collision && scarfyData.pos.x > finishLine;
      // Define message and font size
      const char* message = collision ? "Game Over" : "You Win!";
      int fontSize = 40;

      if (playerWon && !winSoundPlayed) {
        PlaySound(winSound);
        winSoundPlayed = true;
      }

      // Measure the text dimensions
      int textWidth = MeasureText(message, fontSize);
      int textHeight = fontSize; // Approximation: fontSize is usually the height

      // Center the text
      int textX = windowDimensions[0] / 2 - textWidth / 2;
      int textY = windowDimensions[1] / 2 - textHeight / 2;

      // Draw the text
      DrawText(message, textX, textY, fontSize, collision ? RED : GREEN);

      // Define restart button size and position below the text
      float btnWidth = 140;
      float btnHeight = 50;
      float btnX = windowDimensions[0] / 2 - btnWidth / 2;
      float btnY = textY + textHeight + 20;

      // Draw restart button
      Rectangle restartBtn = { btnX, btnY, btnWidth, btnHeight };
      DrawRectangleRec(restartBtn, GRAY);

      // Center the "Restart" text inside the button
      const char* btnText = "Restart";
      int btnFontSize = 20;
      int btnTextWidth = MeasureText(btnText, btnFontSize);
      int btnTextX = btnX + (btnWidth - btnTextWidth) / 2;
      int btnTextY = btnY + (btnHeight - btnFontSize) / 2;

      DrawText(btnText, btnTextX, btnTextY, btnFontSize, WHITE);


      // Check for button click
      if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        Vector2 mousePos = GetMousePosition();
        if (CheckCollisionPointRec(mousePos, restartBtn)) {
          // Reset all game variables
          collision = false;
          collision_count = 0;
          winSoundPlayed = false;
          velocity = 0;
          scarfyData.pos = {windowDimensions[0] / 2 - scarfyData.rec.width / 2, windowDimensions[1] - scarfyData.rec.height};
          scarfyData.frame = 0;
          scarfyData.runningTime = 0.0f;
          finishLine = nebulae[(sizeof(nebulae) / sizeof(nebulae[0])) - 1].pos.x + 800;

          // Reset nebulae positions
          index = 0;
          for (auto& neb : nebulae) {
            neb.pos.x = static_cast<float>(windowDimensions[0] + index * 300);
            neb.runningTime = 0.0f;
            ++index;
          }
        }
      }
    } else {
      // Draw nebulae
      for (auto& neb: nebulae) {
        DrawTextureRec(nebula, neb.rec, neb.pos, WHITE);
      }
      // Draw scarfy
      DrawTextureRec(scarfy, scarfyData.rec, scarfyData.pos, WHITE);
    }

    // Stop drawind
    EndDrawing();
  }

  UnloadSound(winSound);
  UnloadSound(loseSound);
  UnloadSound(gruntSound);
  UnloadSound(jumpSound);
  UnloadMusicStream(mainMusic);
  UnloadSound(runningSound);
  UnloadSound(impactSound);

  UnloadTexture(scarfy);
  UnloadTexture(nebula);
  UnloadTexture(background);
  UnloadTexture(midground);
  UnloadTexture(foreground);
  CloseWindow();
}
