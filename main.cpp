#include <string>
#include <ctime>
#include <fstream>
#include <sstream>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_mixer.h>

#define TICK_INTERVAL 30
#define WIDTH 816
#define HEIGHT 624

static Uint32 next_time;

Uint32 time_left(void)
{
    Uint32 now;

    now = SDL_GetTicks();
    if(next_time <= now)
        return 0;
    else
        return next_time - now;
}

enum SLOT {
    SEVEN,
    CHERRY,
    BELL,
    BAR,
    ANY,
};

int bets[6] = {10, 20, 50, 100, 500, 1000};

SLOT winCombinations[][3] = {
    {SEVEN, SEVEN, SEVEN},
    {SEVEN, SEVEN, ANY},
    {ANY, SEVEN, SEVEN},
    {BAR, BAR, BAR},
    {BELL, BELL, BELL},
    {CHERRY, CHERRY, CHERRY},
};
int winRate[] = {
    10,
    5,
    5,
    7,
    3,
    2,
};


SDL_Color WHITE = {255, 255, 255};

SDL_Window *gWindow;
SDL_Renderer *gRenderer;

TTF_Font *gSans;

SDL_Texture *gBackgroundTexture;
SDL_Texture *gPrizesTexture;

SDL_Texture *gSlotMachineTexture;
SDL_Texture *gHandleOffTexture;
SDL_Texture *gHandleOnTexture;

SDL_Texture *gSlotOneTexture;
SDL_Texture *gSlotTwoTexture;
SDL_Texture *gSlotThreeTexture;
SDL_Texture *gSlotFourTexture;

SDL_Texture *gCashLabelTexture;
SDL_Texture *gCashTexture;

SDL_Texture *gBetLabelTexture;
SDL_Texture *gBetTexture;

SDL_Texture *gBetPlusTexture;
SDL_Texture *gBetPlusPressedTexture;

SDL_Texture *gBetMinusTexture;
SDL_Texture *gBetMinusPressedTexture;

SDL_Texture *gPrizeLabelTexture;

SDL_Texture *gAudioOnTexture;
SDL_Texture *gAudioOffTexture;

SLOT gSlots[3] = {SEVEN, SEVEN, SEVEN};

SDL_Texture* loadTexture(std::string path);
SDL_Texture *loadText(std::string text, SDL_Color color);
void save(int cash, bool audioOn);
void load(int &cash, bool &audioOn);

int main(int, char**)
{
    std::srand(std::time(nullptr));
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);

    int imgFlags = IMG_INIT_PNG;
    if( !( IMG_Init( imgFlags ) & imgFlags ) )
    {
        SDL_Log( "SDL_image could not initialize! SDL_image Error: %s\n", IMG_GetError() );
        exit(-1);
    }

    if( TTF_Init() == -1 )
    {
        SDL_Log( "SDL_ttf could not initialize! SDL_ttf Error: %s\n", TTF_GetError() );
        exit(-1);
    }

    if( Mix_OpenAudio( 44100, MIX_DEFAULT_FORMAT,2,2048) == -1 )
    {
        SDL_Log( "SDL_mixer could not initialize! SDL_mixer Error: %s\n", Mix_GetError() );
        exit(-1);
    }

    gWindow = SDL_CreateWindow("Slot Machine", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WIDTH, HEIGHT, SDL_WINDOW_SHOWN);
    gRenderer = SDL_CreateRenderer(gWindow, -1, SDL_RENDERER_ACCELERATED);


    bool audioOn = true;
    bool musicPlaying = false;
    bool playSoundPlaying = false;
    bool winSoundPlaying = false;
    bool jackpotSoundPlaying = false;
    Mix_Music *idleMusic = Mix_LoadMUS("assets/idle.mp3");
    Mix_Chunk *betSound = Mix_LoadWAV("assets/bet.wav");
    Mix_Chunk *jackpotSound = Mix_LoadWAV("assets/jackpot.wav");
    Mix_Chunk *playSound = Mix_LoadWAV("assets/play.wav");
    Mix_Chunk *startSound = Mix_LoadWAV("assets/start.wav");
    Mix_Chunk *winSound = Mix_LoadWAV("assets/win.wav");

    gSans = TTF_OpenFont("assets/OpenSans-Medium.ttf", 24);

    gBackgroundTexture = loadTexture("background.png");
    gPrizesTexture = loadTexture("prizes.png");

    gSlotMachineTexture = loadTexture("slot-machine1.png");
    gHandleOffTexture = loadTexture("slot-machine2.png");
    gHandleOnTexture = loadTexture("slot-machine3.png");

    gSlotOneTexture = loadTexture("slot-symbol1.png"); 
    gSlotTwoTexture = loadTexture("slot-symbol2.png");
    gSlotThreeTexture = loadTexture("slot-symbol3.png");
    gSlotFourTexture = loadTexture("slot-symbol4.png");

    gBetPlusTexture = loadTexture("bet_plus.png");
    gBetPlusPressedTexture = loadTexture("bet_plus_2.png");

    gBetMinusTexture = loadTexture("bet_minus.png");    
    gBetMinusPressedTexture = loadTexture("bet_minus_2.png");

    gCashLabelTexture = loadText("Cash", WHITE);
    gBetLabelTexture = loadText("Bet", WHITE);

    gAudioOnTexture = loadTexture("audioOn.png");
    gAudioOffTexture = loadTexture("audioOff.png");

    SDL_Rect backgroundPos = {0, 0, WIDTH, HEIGHT};
    SDL_Rect prizesPos = {0, 0, 125, 200};
    SDL_Rect slotMachinePos = {0, 0, 816, 624};
    SDL_Rect slotPos = {235, 303, 96, 96};
    SDL_Rect cashLabelPos = {225, 480, 24*4, 24};
    SDL_Rect cashPos = {225, 510, 18, 18};
    SDL_Rect prizeLabelPos = {207, 525, 18, 18};
    SDL_Rect betLabelPos = {400, 480, 24*3, 24};
    SDL_Rect betPos = {400, 510, 18, 18};

    SDL_Rect betPlusPos = {500, 495, 32, 32};
    SDL_Rect betMinusPos = {550, 495, 32, 32};
    SDL_Rect betPlusPressedPos = {500, 499, 32, 28};
    SDL_Rect betMinusPressedPos = {550, 499, 32, 28};

    SDL_Rect audioButtonPos = {766, 0, 50, 50};

    double playTime = 3200.0f;
    double playTimer = 0.0f;
    double slotOneStopTime = 1200.0f;
    double slotTwoStopTime = 2200.0f;
    double slotThreeStopTime = 3200.0f;

    double handleTime = 1000.0f;
    double handleTimer = 0.0f;

    double winTime = 1000.0f;
    double winTimer = 0.0f;

    double jackpotTime = 3000.0f;
    double jackpotTimer = 0.0f;

    double prizeTime = 5000.0f;
    double prizeTimer = 0.0f;

    double musicTimeout = 10000.0f;
    double musicTimoutTimer = musicTimeout;

    int cash = 1000;
    int bet = 0;
    int winCombination = 0;

    bool betPlusPressed = false;
    bool betMinusPressed = false;
    bool slotPlaying = false;
    bool handleOn = false;
    bool win = false;
    bool jackpotWin = false;
    bool running = true;
    SDL_Event event;

    load(cash, audioOn);

    while(running)
    {
        next_time = SDL_GetTicks() + TICK_INTERVAL;
        while(SDL_PollEvent(&event) != 0)
        {
            if(event.type == SDL_QUIT)
            {
                running = false;
            }
            else if(event.type == SDL_KEYDOWN && !slotPlaying && !win && !jackpotWin)
            {
                if(event.key.keysym.sym == SDLK_SPACE && cash >= bets[bet])
                {
                    Mix_PlayChannel( -1, startSound, 0 );
                    cash -= bets[bet];
                    playTimer = 0.0;
                    handleOn = true;
                    slotPlaying = true;
                    prizeTimer = prizeTime;
                }
            }
            else if(event.type == SDL_MOUSEBUTTONDOWN)
            {
                if(event.button.button == SDL_BUTTON_LEFT)
                {
                    if(event.button.x > audioButtonPos.x && event.button.y > audioButtonPos.y && event.button.x < audioButtonPos.x + audioButtonPos.w && event.button.y < audioButtonPos.y + audioButtonPos.h)
                    {
                        audioOn = !audioOn;
                        save(cash, audioOn);
                    }
                    if(!slotPlaying && !win && !jackpotWin)
                    {
                        if(event.button.x > 673 && event.button.y > 296 && event.button.x < 764 && event.button.y < 387)
                        {
                            if(cash >= bets[bet])
                            {
                                Mix_PlayChannel( -1, startSound, 0 );
                                cash -= bets[bet];
                                playTimer = 0.0;
                                handleOn = true;
                                slotPlaying = true;
                                prizeTimer = prizeTime;
                            }
                        }
                        else if(event.button.x > betPlusPos.x && event.button.y > betPlusPos.y && event.button.x < betPlusPos.x + betPlusPos.w && event.button.y < betPlusPos.y + betPlusPos.h)
                        {
                            betPlusPressed = true;
                        }
                        else if(event.button.x > betMinusPos.x && event.button.y > betMinusPos.y && event.button.x < betMinusPos.x + betMinusPos.w && event.button.y < betMinusPos.y + betMinusPos.h)
                        {
                            betMinusPressed = true;
                        }
                    }
                }
            }
            else if(event.type == SDL_MOUSEBUTTONUP)
            {
                if(betPlusPressed)
                {
                    betPlusPressed = false;
                    Mix_PlayChannel( -1, betSound, 0 );
                    if(bet < 5) bet++;
                }
                else if(betMinusPressed)
                {
                    betMinusPressed = false;
                    Mix_PlayChannel( -1, betSound, 0 );
                    if(bet > 0) bet--;
                }
            }
        }

        if(handleOn)
        {
            if(handleTimer >= handleTime)
            {
                handleOn = false;
                handleTimer = 0.0f;
            }
            else
            {
                handleTimer += TICK_INTERVAL;
            }
        }


        if(slotPlaying)
        {
            Mix_PauseMusic();
            if(!playSoundPlaying)
            {
                Mix_PlayChannel( -1, playSound, 0 );
                playSoundPlaying = true;
            }
            if(playTimer >= playTime) {
                slotPlaying = false;
                musicPlaying = false;
                playSoundPlaying = false;

                for(int i=0; i<15;i++)
                {
                    if((winCombinations[i][0] == gSlots[0] && winCombinations[i][1] == gSlots[1] && winCombinations[i][2] == gSlots[2]) ||
                        (winCombinations[i][0] == gSlots[0] && winCombinations[i][1] == gSlots[1] && winCombinations[i][2] == ANY) ||
                        (winCombinations[i][0] == ANY && winCombinations[i][1] == gSlots[1] && winCombinations[i][2] == gSlots[2]) ||
                        (winCombinations[i][0] == gSlots[0] && winCombinations[i][1] == ANY && winCombinations[i][2] == ANY) ||
                        (winCombinations[i][0] == ANY && winCombinations[i][1] == gSlots[1] && winCombinations[i][2] == ANY) ||
                        (winCombinations[i][0] == ANY && winCombinations[i][1] == ANY && winCombinations[i][2] == gSlots[2]))
                    {
                        if(gSlots[0] == SEVEN && gSlots[1] == SEVEN && gSlots[2] == SEVEN)
                        {
                            jackpotWin = true;
                        }
                        else {
                            win = true;
                        }
                        winCombination = i;
                        break;
                    }
                }

                save(cash, audioOn);
            }
            if(playTimer < slotOneStopTime) gSlots[0] = static_cast<SLOT>(rand() % 4);
            if(playTimer < slotTwoStopTime) gSlots[1] = static_cast<SLOT>(rand() % 4);
            if(playTimer < slotThreeStopTime) gSlots[2] = static_cast<SLOT>(rand() % 4);
            playTimer += TICK_INTERVAL;
        }
        else if(win)
        {
            if(!winSoundPlaying)
            {
                Mix_PlayChannel( -1, winSound, 0 );
                winSoundPlaying = true;
            }
            if(winTimer >= winTime)
            {
                winSoundPlaying = false;
                win = false;
                winTimer = 0.0f;
                cash += bets[bet] * winRate[winCombination];
                std::string prize = "+" + std::to_string(bets[bet] * winRate[winCombination]);
                gPrizeLabelTexture = loadText(prize, WHITE);
                prizeLabelPos.w = 18 * prize.size();
                prizeTimer = 0.0f;
                SDL_Log("Win %d\n", bets[bet] * winRate[winCombination]);
            }
            winTimer += TICK_INTERVAL;
        }
        else if(jackpotWin)
        {
            if(!jackpotSoundPlaying)
            {
                Mix_PlayChannel( -1, jackpotSound, 0 );
                jackpotSoundPlaying = true;
            }
            if(jackpotTimer >= jackpotTime)
            {
                jackpotSoundPlaying = false;
                jackpotWin = false;
                jackpotTimer = 0.0f;
                cash += bets[bet] * winRate[winCombination];
                std::string prize = "+" + std::to_string(bets[bet] * winRate[winCombination]);
                gPrizeLabelTexture = loadText(prize, WHITE);
                prizeLabelPos.w = 18 * prize.size();
                prizeTimer = 0.0f;
                SDL_Log("JACKPOT %d\n", bets[bet] * winRate[winCombination]);
            }
            jackpotTimer += TICK_INTERVAL;
        }
        else
        {
            if(!musicPlaying && !win && !jackpotWin)
            {
                if(musicTimoutTimer >= musicTimeout && audioOn)
                {
                    Mix_PlayMusic(idleMusic, -1);
                    musicPlaying = true;
                    musicTimoutTimer = 0.0f;
                }
                musicTimoutTimer += TICK_INTERVAL;
            }
        }

        if(prizeTimer < prizeTime) prizeTimer += TICK_INTERVAL;

        SDL_RenderClear(gRenderer);
        SDL_SetRenderDrawColor(gRenderer, 0x00, 0x00, 0x00, 0xFF);

        SDL_RenderCopy(gRenderer, gBackgroundTexture, NULL, &backgroundPos);
        SDL_RenderCopy(gRenderer, gPrizesTexture, NULL, &prizesPos);

        SDL_RenderCopy(gRenderer, gSlotMachineTexture, nullptr, &slotMachinePos);
        SDL_RenderCopy(gRenderer, handleOn ? gHandleOnTexture : gHandleOffTexture, nullptr, &slotMachinePos);

        slotPos.x = 235;
        for(int i=0; i < 3; i++)
        {
            switch (gSlots[i])
            {
                case SEVEN:
                    SDL_RenderCopy(gRenderer, gSlotOneTexture, nullptr, &slotPos);
                    break;
                case CHERRY:
                    SDL_RenderCopy(gRenderer, gSlotTwoTexture, nullptr, &slotPos);
                    break;
                case BELL:
                    SDL_RenderCopy(gRenderer, gSlotThreeTexture, nullptr, &slotPos);
                    break;
                case BAR:
                    SDL_RenderCopy(gRenderer, gSlotFourTexture, nullptr, &slotPos);
                    break;     
                case ANY:
                    break;       
            }
            slotPos.x += 130;
        }

        std::string cashStr = std::to_string(cash);
        std::string betStr = std::to_string(bets[bet]);
        gCashTexture = loadText(cashStr, WHITE);
        gBetTexture = loadText(betStr, WHITE);
        cashPos.w = 18 * cashStr.size();
        betPos.w = 18 * betStr.size();

        SDL_RenderCopy(gRenderer, gCashLabelTexture, NULL, &cashLabelPos);
        SDL_RenderCopy(gRenderer, gBetLabelTexture, NULL, &betLabelPos);
        SDL_RenderCopy(gRenderer, gCashTexture, NULL, &cashPos);
        SDL_RenderCopy(gRenderer, gBetTexture, NULL, &betPos);
        
        if(prizeTimer < prizeTime) SDL_RenderCopy(gRenderer, gPrizeLabelTexture, NULL, &prizeLabelPos);

        if(betPlusPressed)
        {
            SDL_RenderCopy(gRenderer, gBetPlusPressedTexture, NULL, &betPlusPressedPos);
        }
        else
        {
            SDL_RenderCopy(gRenderer, gBetPlusTexture, NULL, &betPlusPos);
        }

        if(betMinusPressed)
        {
            SDL_RenderCopy(gRenderer, gBetMinusPressedTexture, NULL, &betMinusPressedPos);
        }
        else
        {
            SDL_RenderCopy(gRenderer, gBetMinusTexture, NULL, &betMinusPos);
        }

        SDL_RenderCopy(gRenderer, audioOn ? gAudioOnTexture : gAudioOffTexture, NULL, &audioButtonPos);

        SDL_RenderPresent(gRenderer);
        SDL_DestroyTexture(gBetTexture);
        SDL_DestroyTexture(gCashTexture);

        Mix_MasterVolume(audioOn ? MIX_MAX_VOLUME : 0);
        Mix_VolumeMusic(audioOn ? MIX_MAX_VOLUME : 0);

        SDL_Delay(time_left());
        next_time += TICK_INTERVAL;
    }

    SDL_DestroyTexture(gBackgroundTexture);
    SDL_DestroyTexture(gPrizesTexture);

    SDL_DestroyTexture(gSlotMachineTexture);
    SDL_DestroyTexture(gHandleOffTexture);
    SDL_DestroyTexture(gHandleOnTexture);

    SDL_DestroyTexture(gSlotOneTexture);
    SDL_DestroyTexture(gSlotTwoTexture);
    SDL_DestroyTexture(gSlotThreeTexture);
    SDL_DestroyTexture(gSlotFourTexture);

    SDL_DestroyTexture(gCashLabelTexture);
    SDL_DestroyTexture(gCashTexture);

    SDL_DestroyTexture(gBetLabelTexture);
    SDL_DestroyTexture(gBetTexture);

    SDL_DestroyTexture(gBetPlusTexture);
    SDL_DestroyTexture(gBetPlusPressedTexture);

    SDL_DestroyTexture(gBetMinusTexture);
    SDL_DestroyTexture(gBetMinusPressedTexture);

    SDL_DestroyTexture(gPrizeLabelTexture);

    SDL_DestroyTexture(gAudioOnTexture);
    SDL_DestroyTexture(gAudioOffTexture);

    TTF_CloseFont(gSans);

    Mix_CloseAudio();
    Mix_FreeChunk(betSound);
    Mix_FreeChunk(winSound);
    Mix_FreeChunk(playSound);
    Mix_FreeChunk(jackpotSound);
    Mix_FreeChunk(startSound);
    Mix_FreeMusic(idleMusic);

    SDL_DestroyRenderer(gRenderer);
    SDL_DestroyWindow(gWindow);
    return 0;
}

SDL_Texture *loadTexture(std::string path)
{
    SDL_Texture *newTexture = nullptr;
    path = "assets/" + path;

    SDL_Surface *loadedSurface = IMG_Load(path.c_str());
    if(loadedSurface == nullptr)
    {
        SDL_Log("Unable to load image %s! SDL_image Error: %s\n", path.c_str(), IMG_GetError());
    }
    else
    {
        newTexture = SDL_CreateTextureFromSurface(gRenderer, loadedSurface);
        if( newTexture == nullptr )
        {
            SDL_Log( "Unable to create texture from %s! SDL Error: %s\n", path.c_str(), SDL_GetError() );
        }

        SDL_FreeSurface(loadedSurface);
    }

    return newTexture;
}

SDL_Texture *loadText(std::string text, SDL_Color color)
{
    SDL_Texture *newTexture = nullptr;

    SDL_Surface *loadedSurface = TTF_RenderText_Solid(gSans, text.c_str(), color);
    if(loadedSurface == nullptr)
    {
        SDL_Log("Unable to create text %s! SDL_ttf Error: %s\n", text.c_str(), TTF_GetError());
    }
    else
    {
        newTexture = SDL_CreateTextureFromSurface(gRenderer, loadedSurface);
        if( newTexture == nullptr )
        {
            SDL_Log( "Unable to create text from %s! SDL Error: %s\n", text.c_str(), SDL_GetError() );
        }

        SDL_FreeSurface(loadedSurface);
    }

    return newTexture;
}

void save(int cash, bool audioOn)
{
    std::ofstream dataFile("data");
    dataFile << std::to_string(cash) << "," << audioOn;
    dataFile.close();
}

void load(int &cash, bool &audioOn)
{
    std::string line;
    std::ifstream dataFile("data");
    std::string outputArray[2];
    std::getline(dataFile, line);
    dataFile.close();

    if(line.empty()) return;

    std::stringstream streamData(line);
    for(int i=0; i < 2; i++)
    {
        std::getline(streamData, line, ',');
        outputArray[i] = line;
    }

    cash = std::stoi(outputArray[0]);
    audioOn = outputArray[1] == "1" ? true : false;
}
