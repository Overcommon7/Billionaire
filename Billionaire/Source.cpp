#include "pch.h"
#include "Player.h" 
#include "Button.h"
#include "doublyLinkedlist.h"
#include "IO.h"

typedef enum GameScreen { VIEWPLAYERSTORES = 0, JAIL, SHOP, AREYOUSURE, SETTINGS, NEWTIER} GameScreen;
extern bool exitGame = false, noStores = false, newTier = false;;
bool first = true, inJail = false;
long JailTime = std::stol(GetLine(2, "time"));
extern long long timeElapsed = GetElapsedTime();
extern DoublyLinkedList<Store> Stores{};
extern float playerThreatForJailThread = std::stof(GetLine(3, "PlayerStats"));
extern int numOfStores = 0;

void Capture(Image& screenshot, Texture2D& SS, const char* name = "screenshot.png")
{
    TakeScreenshot(name);
    if (!first) UnloadTexture(SS);
    first = false;
    screenshot = LoadImage(name);
    SS = LoadTextureFromImage(screenshot);
    UnloadImage(screenshot);
}

static Image LoadAssests(string* file)
{
    Image load = LoadImage(file->c_str());
    file->erase(file->begin(), file->begin() + file->find('/') + 1);
    file->erase(file->begin() + file->find('.'), file->end());
    return load;
}

void JailCalculator()
{
    float TimesSuspected = std::stof(GetLine(8, "PlayerStats"));
    while (!exitGame)
    {
        if (!inJail)
        {
            if (Random() < playerThreatForJailThread) ++TimesSuspected;
            else TimesSuspected = 0;
        }
        else --JailTime;
        //cout << "Suspicion: " << TimesSuspected / 8 << "\n";
        if (TimesSuspected > 7 && !inJail)
        {
            inJail = true;
            JailTime = 1200;
            TimesSuspected = 0;
        }
        if (JailTime <= 0) inJail = false;
        delay();
    }
    SaveLine(8, std::to_string(TimesSuspected), "PlayerStats");
    SaveLine(2, std::to_string(JailTime), "time");
}


//void DrawUI(const Player& p, const Texture2D& BG, const Font& font)
//{
//    DrawTexture(BG, 0, 0, WHITE);
//    DrawTextEx(font, std::to_string(p.netWorth).c_str(), {66.f, 32.f}, 35, 3, WHITE);
//    DrawTextEx(font, std::to_string(p.money).c_str(), {207.f, 143.f}, 20, 3, WHITE);
//    DrawTextEx(font, std::to_string(p.threat).c_str(), {440.f, 149.f}, 12, 3, WHITE);
//    DrawTextEx(font, std::to_string(p.threat).c_str(), {45.f, 149.f}, 12, 3, WHITE);
//}

//------------------------------------------------------------------------------------------
// Main entry point
//------------------------------------------------------------------------------------------
int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 700, middleWidth = 350;
    const int screenHeight = 975, middleHeight = 487;
    //mitigates time errors when opening game
    while (timeElapsed > 100000000 || timeElapsed < 0)
    {
        timeElapsed = GetElapsedTime();
    }

    InitWindow(screenWidth, screenHeight, "Billionaire Replica");

    if (JailTime > 0) JailTime -= timeElapsed;
   
    

    GameScreen currentScreen = VIEWPLAYERSTORES;
    GameScreen previousScreen = SHOP;
    if (JailTime > 0)
    {
        inJail = true;
        currentScreen = JAIL;
    }
    else JailTime = 0;

    // TODO: Initialize all required variables and load all required data here!
    thread JailThread(JailCalculator);
    int framesCounter = 0;
    float textY = middleHeight;
    long long result = -1;
    Player player;
    unordered_map<string, Texture2D> assets;
    vector<string> filePaths;
    vector<future<Image>> futures;
    short AssestsSize = GetNumberOfLines("GameAssests");
    int storeClicked = 0, playerStoreIndex = 0;
    Image screenshot, tempImage;
    Texture2D SS, storeEmblem;
    string title = "";
    Font defaultFont = GetFontDefault();
    Rectangle shopButton = { 618.f, 367.f, 67.f, 57.f };
    bool isSelling = true, canUpgrade = false;
    vector<int> p = { 1,4,6,7 };
    
   
#pragma region LoadAssests
    ifstream inFile("GameAssests.txt");
    string line = "";
    while (std::getline(inFile, line))
        filePaths.push_back(line);

    inFile.close();
    static Image (*func_ptr)(string*) = LoadAssests;

    for (auto& file : filePaths)
    {
        futures.push_back(std::async(std::launch::async, func_ptr, &file));
    }
    
    LoadStores(&Stores, numOfStores, player.tier);
    cout << Stores.size();
    for (short i = 0; i < futures.size(); i++)
    {
        futures[i].wait();
        assets.insert({ filePaths[i], LoadTextureFromImage(futures[i].get())});
    }
    futures.clear();
    futures.shrink_to_fit();
#pragma endregion            
#pragma region LoadStoreAssests

    Button collect(20, 855, "Viewing/Collect.png");
    Button Yes(215, 500, "StoreObjects/YES.png");
    Button No(325, 500, "StoreObjects/NO.png");
    Button Sell(13, 690, "Viewing/Sell.png");
    Button Upgrade(77, 690, "Viewing/Upgrade.png");
        
#pragma endregion
   
    cout << timeElapsed << '\n';
    SetTargetFPS(60);               // Set desired framerate (frames-per-second)
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------

        for (auto& it : player.stores)
        {
            if (it.buildTime > 0) --it.buildTime;        
            else it.currentcapacity += (it.earningPerSec * 0.0166666666666667);

            if (it.currentcapacity > it.maxHold) it.currentcapacity = it.maxHold;
        }

        if (inJail) currentScreen = JAIL;
        if (newTier)
        {
            std::remove("screenshot1.png");
            currentScreen = NEWTIER;
            Capture(screenshot, SS, "screenshot1.png");
            newTier = false;
        }

        switch (currentScreen)
        {
        case VIEWPLAYERSTORES:
        {
            if ((IsGestureDetected(GESTURE_SWIPE_LEFT) || IsKeyPressed(KEY_RIGHT)) && playerStoreIndex < player.stores.size() - 1)  ++playerStoreIndex;         
            else if ((IsGestureDetected(GESTURE_SWIPE_RIGHT) || IsKeyPressed(KEY_LEFT)) && playerStoreIndex > 0)  --playerStoreIndex;
            else if (IsGestureDetected(GESTURE_TAP))
            {
                player.UpdatePosition();
                if (noStores && CheckCollisionRecs(player.pointer, { 171.f, 460.f, 350.f, 350.f }))
                {
                    currentScreen = SHOP;
                    break;
                }
                else if (CheckCollisionRecs(player.pointer, collect.collider))
                {
                    player.UpdateMoney(player.stores[playerStoreIndex].currentcapacity);
                    player.stores[playerStoreIndex].currentcapacity = 0;
                }
                else if (CheckCollisionRecs(player.pointer, shopButton))  currentScreen = SHOP;
                else if (player.stores[playerStoreIndex].buildTime > 0)
                {
                    if (CheckCollisionRecs(player.pointer, { 160.f, 490.f, static_cast<float>(assets["Construction"].width), static_cast<float>(assets["Construction"].height) }))
                        player.stores[playerStoreIndex].buildTime -= player.clicker;
                }
                else if (CheckCollisionRecs(player.pointer, Sell.collider)
                    && player.stores[playerStoreIndex].threatLevel >= 0)
                {
                    Capture(screenshot, SS);
                    title = player.stores[playerStoreIndex].name;
                    GetStoreSkinFromName(title, Stores, tempImage);
                    AddSpace(title);
                    std::transform(title.begin(), title.end(), title.begin(), std::toupper);
                    title.insert(0, "SELL ");
                    ImageCrop(&tempImage, { 30.f, 0.f, 160.f, 120.f });
                    storeEmblem = LoadTextureFromImage(tempImage);
                    UnloadImage(tempImage);
                    isSelling = true;
                    currentScreen = AREYOUSURE;
                    previousScreen = VIEWPLAYERSTORES;
                }
                else if (CheckCollisionRecs(player.pointer, Upgrade.collider))
                {
                    Capture(screenshot, SS);
                    title = player.stores[playerStoreIndex].name;
                    GetStoreSkinFromName(title, Stores, tempImage);
                    AddSpace(title);
                    std::transform(title.begin(), title.end(), title.begin(), std::toupper);
                    title.insert(0, "UPGRADE ");
                    ImageCrop(&tempImage, { 30.f, 0.f, 160.f, 120.f });
                    storeEmblem = LoadTextureFromImage(tempImage);
                    UnloadImage(tempImage);
                    isSelling = false;
                    currentScreen = AREYOUSURE;
                    previousScreen = VIEWPLAYERSTORES;
                }
                else if (CheckCollisionRecs(player.pointer, {650.f,925.f, 50.f, 50.f }))
                    currentScreen = SETTINGS;        
            }

        } break;
        case JAIL:
        {
            if (IsGestureDetected(GESTURE_TAP))
            {
                player.UpdateClicker();
                if (CheckCollisionRecs(player.pointer, { 102.f, 802.f, 500.f, 70.f }))
                {
                    if (player.crystals >= 35)
                    {
                        currentScreen = VIEWPLAYERSTORES;
                        player.crystals -= 35;
                        inJail = false;
                        JailTime = 0;
                        break;
                    }
                }
            }
            
            if (!inJail || JailTime < 1)
            {
                currentScreen = VIEWPLAYERSTORES;
                inJail = false;
                JailTime = 0;
            }
            
        } break;
        case SHOP:
        {
            if (IsGestureDetected(GESTURE_TAP))
            {
                player.UpdatePosition();
                if (player.pointer.y > 265)
                {
                    for (short i = 0; i < numOfStores; i++)
                    {
                        if (CheckCollisionRecs(Stores[i].collider, player.pointer))
                        {
                            storeClicked = i;
                            Capture(screenshot, SS);
                            title = Stores[i].name;
                            AddSpace(title);
                            std::transform(title.begin(), title.end(), title.begin(), std::toupper);
                            title.insert(0, "BUILD ");
                            tempImage = LoadImageFromTexture(Stores[i].skin);
                            ImageCrop(&tempImage, { 30.f, 0.f, 160.f, 120.f });
                            storeEmblem = LoadTextureFromImage(tempImage);
                            UnloadImage(tempImage);
                            currentScreen = AREYOUSURE;
                            previousScreen = SHOP;
                            break;
                        }
                    }
                }
                else if (CheckCollisionRecs(player.pointer, { 631.f, 202.f, 700.f, 53.f }))
                {
                    currentScreen = VIEWPLAYERSTORES;
                    break;
                }

            }

            //Move store buttins with scrollwheel
            float scroll = GetMouseWheelMove();
            if (scroll != 0)
            {
                int oneOrTwo = 1;
                if (numOfStores % 2 != 0) oneOrTwo = 2;
                scroll *= 45;

                if (!((Stores[0].posY > 266 && scroll > 0) || (Stores[numOfStores - oneOrTwo].posY < 267 && scroll < 0)))
                {
                    for (short i = 0; i < numOfStores; i++)
                    {
                        Stores[i].UpdatePosition(0, (int)scroll);
                    }
                }  
            }
            
        } break;
        case AREYOUSURE:
        {
            if (IsGestureDetected(GESTURE_TAP))
            {
                player.UpdatePosition();
                if (CheckCollisionRecs(player.pointer, Yes.collider))
                {
                    if (previousScreen == SHOP)
                    {
                        result = player.AddStore(Stores[storeClicked]);
                        playerStoreIndex = player.stores.size() - 1;
                    }
                    else if (previousScreen == VIEWPLAYERSTORES)
                    {
                        if (isSelling)
                        {
                            player.RemoveStore(playerStoreIndex);
                            isSelling = false;
                            if (playerStoreIndex != 0) playerStoreIndex = player.stores.size() - 1;
                        }
                        else canUpgrade = player.UpgadeStore(playerStoreIndex);
                    }
                    currentScreen = VIEWPLAYERSTORES;
                }
                else if (CheckCollisionRecs(player.pointer, No.collider))
                {
                    std::remove("screenshot.png");
                    if (previousScreen == SHOP) currentScreen = SHOP;
                    else currentScreen = VIEWPLAYERSTORES;
                }
            }
        //Make sure the Are you Sure menu doesnt last 4ever
            if (!canUpgrade) result = -1;
            if (result > -1)
            {
                ++framesCounter;
                if ((result == 0) && framesCounter > 30)
                {
                    framesCounter = 0;
                    result = -1;
                    canUpgrade = true;                  
                    UnloadTexture(SS);
                    UnloadTexture(storeEmblem);
                    std::remove("screenshot.png");
                }
                else if (result > 0 && framesCounter > 60)
                {
                    framesCounter = 0;
                    result = -1;
                    currentScreen = SHOP;
                    UnloadTexture(SS);
                    UnloadTexture(storeEmblem);
                    std::remove("screenshot.png");
                }
            }
            else if (!canUpgrade > framesCounter > 30)
            {
                UnloadTexture(SS);
                UnloadTexture(storeEmblem);
                canUpgrade = true;
                std::remove("screenshot.png");
            }
        } break;
        case SETTINGS:
        {
            if (IsGestureDetected(GESTURE_TAP)) currentScreen = VIEWPLAYERSTORES;
        } break;
        case NEWTIER:
        {
            if (IsGestureDetected(GESTURE_TAP)) currentScreen = VIEWPLAYERSTORES;
        } break;
        default: break;
        }
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

        switch (currentScreen)
        {
        case VIEWPLAYERSTORES:
        {
            // TODO: Draw LOGO screen here!
            ClearBackground(RAYWHITE);
            if (player.stores[playerStoreIndex].buildTime < 1) DrawTexture(player.stores[playerStoreIndex].skin, 171, 460, WHITE);
            DrawTexture(assets["BG"], 0, 202, WHITE);           
            if (player.stores[playerStoreIndex].name != "NULL")
            {
                PlayerStore tempPS = player.stores[playerStoreIndex];
                if (tempPS.buildTime < 1)
                {
                    Color t_color = RED;                 
                    DrawTextEx(defaultFont, ("Earning Rate: $" + std::to_string(static_cast<long long>(tempPS.earningPerSec * 60)) + "/min").c_str(), { 155, 765 }, 15, 3, GREEN);
                    DrawTextEx(defaultFont, ("Max Capacity: $" + std::to_string(tempPS.maxHold)).c_str(), { 155, 785 }, 15, 3, BLACK);
                    if (tempPS.threatLevel == 0) t_color = DARKGRAY;
                    else if (tempPS.threatLevel < 0) t_color = GREEN;
                    string threat = std::to_string(tempPS.threatLevel);
                    threat.erase(threat.begin() + (threat.find('.') + 3), threat.end());
                    DrawTextEx(defaultFont, ("Threat      : " + threat + "%").c_str(), { 155, 810 }, 15, 3, t_color);
                    if (tempPS.threatLevel >= 0)
                    {
                        DrawTexture(Sell.skin, Sell.posX, Sell.posY, WHITE);
                        DrawTexture(collect.skin, collect.posX, collect.posY, WHITE);
                        DrawTextEx(defaultFont, std::to_string(static_cast<long>(tempPS.currentcapacity)).c_str(), { 130, 875 }, 60, 10, WHITE);
                    }
                    DrawTexture(Upgrade.skin, Upgrade.posX, Upgrade.posY, WHITE);
                    DrawTexture(assets["LevelBanner"], 20, 765, WHITE);
                    DrawTextEx(defaultFont, std::to_string(tempPS.level).c_str(), { 42, 794 }, 29, 3.5f, WHITE);
                    DrawTextEx(defaultFont, ("TIER " + std::to_string(tempPS.tier)).c_str(), { 27, 773 }, 10, 1, DARKGREEN);
                }
                else
                {
                    string time = ConvertToTime(tempPS.buildTime);
                    Vector2 XY = MeasureTextEx(defaultFont, time.c_str(), 25, 3);
                    float xPos = middleWidth - (XY.x / 2);
                    DrawTexture(assets["Construction"], 160, 490, WHITE);
                    DrawTextEx(defaultFont, "Under Connstruction", { 75, 775 }, 25, 3, ORANGE);
                    DrawRectangle(xPos - 30, 280, XY.x + 60, XY.y + 20, GREEN);
                    DrawTextEx(defaultFont, time.c_str(), { xPos, 300 }, 25, 3, WHITE);
                }
                string nameWithSpace = tempPS.name;
                AddSpace(nameWithSpace);
                DrawTextEx(defaultFont, nameWithSpace.c_str(), { 155, 725 }, 25, 3, SKYBLUE);
                tempPS.~PlayerStore();
            }
            

        } break;
        case JAIL:
        {
            DrawTexture(assets["JailBG"], 0, 202, WHITE);        
            string time = ConvertToTime(JailTime, false);
            DrawTextEx(defaultFont, time.c_str(), { 240.f, 940.f }, 20, 3, RED);

        } break;
        case SHOP:
        {
            ClearBackground({ 21, 168, 33, 255 });
            for (short i = 0; i < numOfStores; i++)
            {
               DrawTexture(Stores[i].skin, Stores[i].posX, Stores[i].posY, WHITE);             
            }
            DrawTexture(assets["Banner"], 0, 202, WHITE);
        } break;
        case AREYOUSURE:
        {
            // TODO: Draw ENDING screen here!
            ClearBackground({ 21, 168, 33, 255 });
            DrawTexture(SS, 0, 0, WHITE);
            DrawTexture(assets["AreYouSure"], 200, 305, WHITE);
            DrawTexture(storeEmblem, 225, 355, WHITE);
            DrawTexture(Yes.skin, Yes.posX, Yes.posY, WHITE);
            DrawTexture(No.skin, No.posX, No.posY, WHITE);
            DrawTextEx(defaultFont, title.c_str(), { 210.f, 320.f }, 10, 3, BLACK);           
            if (result == 1) DrawTextEx(defaultFont, "You Already Own That Store!", { 75.f, 800.f }, 30, 3, RED);           
            else if (result > 1)
            {
                string print = ("You need: " + std::to_string(result) + "$ more.");
                DrawTextEx(defaultFont, print.c_str(), {static_cast<float>((MeasureText(print.c_str(), 30) - (middleWidth / 2))), 800.f}, 30.f, 3, RED);
            }
            if (result == 1) DrawTextEx(defaultFont, "You Already Own That Store!", { 75.f, 800.f }, 30, 3, RED);
            if (previousScreen == VIEWPLAYERSTORES)
            {
                if (isSelling) 
                    DrawTextEx(defaultFont, ("Sell Amount: " + std::to_string(player.stores[playerStoreIndex].sellPrice) + "$").c_str(), {215.f, 475.f}, 10, 3, GREEN);
                else
                {
                    if (player.stores[playerStoreIndex].threatLevel >= 0)
                    {
                        DrawTextEx(defaultFont, ("Earning Rate: " + std::to_string(static_cast<long long>(player.stores[playerStoreIndex].earningPerSec * (1 + static_cast<double>((static_cast<double>(player.stores[playerStoreIndex].level) / 10))) * 60)) + "$/min").c_str(), { 215.f, 467.f }, 10, 3, GREEN);
                        DrawTextEx(defaultFont, ("Max Capacity: " + std::to_string(static_cast<long long>(player.stores[playerStoreIndex].maxHold * (1 + static_cast<double>((static_cast<double>(player.stores[playerStoreIndex].level) / 10))))) + "$").c_str(), {215.f, 477.f}, 10, 3, BLACK);
                    }
                    else
                    {
                        string threat = std::to_string(player.stores[playerStoreIndex].threatLevel);
                        threat.erase(threat.begin() + (threat.find('.') + 3), threat.end());
                        DrawTextEx(defaultFont, ("Threat: " + threat + "%").c_str(), { 215.f, 477.f }, 10, 3, GREEN);
                    }
                    DrawTextEx(defaultFont, ("Price: " + std::to_string(static_cast<long long>(player.stores[playerStoreIndex].upgradePrice * (1 + static_cast<double>((static_cast<double>(player.stores[playerStoreIndex].level) / 10)))) * player.stores[playerStoreIndex].level) + "$").c_str(), {215.f, 487.f}, 10, 3, RED);
                }
            }
        } break;
        case SETTINGS:
        {
            DrawTexture(assets["Instructions"], 0, 0, WHITE);
        } break;
        case NEWTIER:
        {
            DrawTexture(SS, 0, 0, WHITE);
            DrawTexture(assets["Unlocked"], 250, 375, WHITE);
            DrawTextEx(defaultFont, to_string(player.tier).c_str(), { 340.f, 500.f }, 60, 1, GREEN);
        } break;
        default: 
            break;
        }

        if (currentScreen != SETTINGS && currentScreen != NEWTIER)
        {
            if (player.threat > 0) DrawTexture(assets["StatsBad"], 0, 0, WHITE);
            else DrawTexture(assets["StatsGood"], 0, 0, WHITE);
            DrawTextEx(defaultFont, std::to_string(player.netWorth).c_str(), { 66.f, 46.f }, 35, 3, WHITE);
            DrawTextEx(defaultFont, std::to_string(player.money).c_str(), { 207.f, 161.f }, 20, 3, WHITE);
            string threat = std::to_string(player.threat);
            threat.erase(threat.begin() + (threat.find('.') + 3), threat.end());
            DrawTextEx(defaultFont, threat.c_str(), { 440.f, 163.f }, 12, 3, WHITE);
            DrawTextEx(defaultFont, std::to_string(player.crystals).c_str(), { 45.f, 163.f }, 12, 3, WHITE);
            //To Fade the UI
            if (currentScreen == SHOP || currentScreen == AREYOUSURE) DrawRectangle(0, 0, 700, 202, { 176, 176, 176, 176 });
        }
        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization & Saving
    //--------------------------------------------------------------------------------------

    //UnloadTexture(texture);
    exitGame = true;
    std::remove("screenshot.png");
    std::remove("screenshot1.png");
    UnloadFont(defaultFont);
    if (!noStores)
    {
        for (short i = 0; i < player.stores.size(); i++)
        {
            SaveLine(i + 1, player.stores[i].ConvertDataToString(), "PlayerStores");
        }
    }

    JailThread.join();

    SaveLine(1, std::to_string(time(0)), "time");
    //Save PlayerStats
    SaveMultipleLines({ 1,2,3,4,5,6,7,9,10 }, 
        { std::to_string(player.money), std::to_string(player.netWorth), std::to_string(player.threat), std::to_string(player.crystals), 
        std::to_string(player.clicker), std::to_string(player.crystals), std::to_string(player.tier), std::to_string(player.timesClicked), 
        std::to_string(player.lifeTimeEarning)}, 
        "PlayerStats");
       
    DeleteSecondInstanceOfWord("PlayerStores");
    for (auto& it : assets)
    {
        UnloadTexture(it.second);        
    }
    assets.clear();
    player.~Player();
    
    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}
