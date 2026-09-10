#include "game.h"

Game::Game(const char *title, int width, int height)
{
    window = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, SDL_WINDOW_SHOWN);
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        printf("SDL_mixer could not initialize! SDL_mixer Error: %s\n", Mix_GetError());
    }
    die = Mix_LoadWAV("sound/die.wav");
    hit = Mix_LoadWAV("sound/hit.wav");
    swooshing = Mix_LoadWAV("sound/swooshing.wav");
    wing = Mix_LoadWAV("sound/wing.wav");

    if (die == NULL || hit == NULL || swooshing == NULL || wing == NULL) {
        printf("Failed to load sounds! SDL_mixer Error: %s\n", Mix_GetError());
    }

    loadTextures();

    bird = new Bird(tex_playerDown, tex_playerMid, tex_playerUp, renderer);

    isRunning = true;
    gameover = false;

    ground1 = -WIDTH / 2;
    ground2 = WIDTH   / 2;

    srand(time(NULL));
}

void Game::init()
{

    bird->init();

    while(!pipes.empty()) {
        delete pipes.front();
        pipes.pop_front();
    }

    Pipe* firstPipe = new Pipe(WIDTH * 2 + PIPE_DISTANCE, rand() % 301 + 150);
    Pipe* secondPipe = new Pipe(firstPipe->bottom_dst.x + PIPE_DISTANCE, rand() % 301 + 150);
    Pipe* thirdPipe = new Pipe(secondPipe->bottom_dst.x + PIPE_DISTANCE, rand() % 301 + 150);

    pipes.push_back(firstPipe);
    pipes.push_back(secondPipe);
    pipes.push_back(thirdPipe);

    gameStarted = false;
    gameover = false;

    render();
}



void Game::Start()
{    
    // khởi tạo
    init();
    std::ifstream file("highscore.txt"); // nhận vào file txt lưu điểm highscore
    if (file.is_open()) {
        file >> highScore;
        file.close();
    }
    SDL_RenderClear(renderer);
    //background
    if((bird->score >= 10 && bird->score <= 20) || (bird->score >= 30 && bird->score <= 40) || (bird->score >= 50 && bird->score <= 60)){
        SDL_RenderCopy(renderer, tex_backgroundN, NULL, NULL);
    } else {
        SDL_RenderCopy(renderer, tex_backgroundD, NULL, NULL);
    }

    //pipes
    for(auto pipe : pipes)
    {
        pipe->render(renderer, tex_pipe);
    }

    //ground
    SDL_Rect dstrect4 =  (SDL_Rect) {ground1, HEIGHT - GROUND_HEIGHT, WIDTH, GROUND_HEIGHT};
    SDL_RenderCopy(renderer, tex_ground, NULL, &dstrect4);
    SDL_Rect dstrect5 =  (SDL_Rect) {ground2, HEIGHT - GROUND_HEIGHT, WIDTH, GROUND_HEIGHT};
    SDL_RenderCopy(renderer, tex_ground, NULL, &dstrect5);

    bird->render();

    SDL_Rect dstrect1 = (SDL_Rect) {(WIDTH - 200) / 2, HEIGHT / 3, 200, 50};
    SDL_RenderCopy(renderer, tex_getreddy, NULL, &dstrect1);

    
    SDL_Rect textRect = {WIDTH / 3 - 50 , HEIGHT / 3 + 50, 300, 50};
    SDL_RenderCopy(renderer, tex_guild, NULL, &textRect);

    SDL_RenderPresent(renderer);

    auto t1 = std::chrono::system_clock::now();
    auto t2 = t1;
    // vòng lặp chính 
    while(isRunning)
    {
    t1 = t2;
    t2 = std::chrono::system_clock::now();
    
    std::chrono::duration<float> dt = t2 - t1;

    bool jump = false;
    while(SDL_PollEvent(&event))
    {  
        if(event.type == SDL_QUIT)
            isRunning = false;

        if((event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_SPACE) || (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT))
        {
            jump = gameStarted = true;
            if((event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_SPACE) || (event.button.button == SDL_BUTTON_LEFT))
            {
                Mix_PlayChannel(-1, wing, 0);
                Mix_PlayChannel(-1, swooshing, 0);
            }
        }
        if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT && event.button.x >= 10 && event.button.x <= 45 && event.button.y >= 10 && event.button.y <= 45)
            pausedButtonClicked = true;
    }

    if(frameDelay > dt.count() * 1000) // giữ cho game chạy với tốc độ frame rate ổn định
        SDL_Delay(frameDelay - dt.count() * 1000);
    
    if(gameStarted && !gamePaused)
    {
        update(jump, dt.count(), gameover);
        if(gameover)
            gameOver();
    }

    if(pausedButtonClicked == true ){
        gamePaused = !gamePaused; // Chuyển trạng thái gamePaused
        pausedButtonClicked = false;
    }

    if(gameStarted && !gameover) {
        render();
    }
}

    Close();
}

void Game::update(bool jump, float elapsedTime, bool &gameover)
{
    bird->update(jump, elapsedTime);

    for (auto it = pipes.begin(); it != pipes.end();)
    {
        auto p = *it;
        int pipeSpeed = PIPE_V; // tăng tốc độ cho Bird theo điểm số
    if (bird->score >= 11 && bird->score <= 30) {
        pipeSpeed = PIPE_V + 1;
    } else if (bird->score >= 31 && bird->score <= 50) {
        pipeSpeed = PIPE_V + 2;
    } else if (bird->score >= 51 && bird->score <= 70) {
        pipeSpeed = PIPE_V + 3;
    } else if (bird->score >= 71 && bird->score <= 90) {
        pipeSpeed = PIPE_V + 4;
    } else if (bird->score > 90) {
        pipeSpeed = PIPE_V + 5;
    }
        // Pipe movement now scales with elapsedTime to decouple from FPS drops
        p->bottom_dst.x -= pipeSpeed * (elapsedTime * 60.0f); // 60 is the target FPS
        p->top_dst.x = p->bottom_dst.x;

        if (p->bottom_dst.x + p->bottom_dst.w  < 0)
        {
            it = pipes.erase(it);
            delete p;
            auto* new_pipe = new Pipe(pipes.back()->bottom_dst.x + PIPE_DISTANCE, rand() % 301 + 150);
            pipes.push_back(new_pipe);
        }
        else
        {
            ++it;
        }   
        if (bird->collisionDetector(p)){
            Mix_PlayChannel(-1, hit, 0);
            gameover = true;
        }
                
    }
       

    ground1 -= PIPE_V * (elapsedTime * 60.0f);
    ground2 -= PIPE_V * (elapsedTime * 60.0f);

    if (ground1 + WIDTH < 0)
        ground1 = WIDTH - 10;
    if (ground2 + WIDTH < 0)
        ground2 = WIDTH - 10;
}


void Game::gameOver()
{
    SDL_RenderClear(renderer);
    
    //background
    if((bird->score >= 10 && bird->score <= 20) || (bird->score >= 30 && bird->score <= 40) || (bird->score >= 50 && bird->score <= 60)){
        SDL_RenderCopy(renderer, tex_backgroundN, NULL, NULL);
    } else {
        SDL_RenderCopy(renderer, tex_backgroundD, NULL, NULL);
    }

    //pipes
    for(auto pipe : pipes)
    {
        pipe->render(renderer, tex_pipe);
    }

    //ground
    SDL_Rect dstrect4 =  (SDL_Rect) {ground1, HEIGHT - GROUND_HEIGHT, WIDTH, GROUND_HEIGHT};
    SDL_RenderCopy(renderer, tex_ground, NULL, &dstrect4);
    SDL_Rect dstrect5 =  (SDL_Rect) {ground2, HEIGHT - GROUND_HEIGHT, WIDTH, GROUND_HEIGHT};
    SDL_RenderCopy(renderer, tex_ground, NULL, &dstrect5);

    bird->render();
    Mix_PlayChannel(-1, die, 0); // Phát âm thanh die


    if (bird->score > highScore) {
        highScore = bird->score;

        // Lưu high score vào file
        std::ofstream file("highscore.txt");
        if (file.is_open()) {
            file << highScore;
            file.close();
        }
    }
    gameStarted = false;
    SDL_Rect dstrect = (SDL_Rect) {(WIDTH - 200) / 2, HEIGHT / 3, 200, 50};
    SDL_RenderCopy(renderer, tex_gameover, NULL, &dstrect);
    SDL_Rect dstrect2 = (SDL_Rect) {(WIDTH - 300) / 2, HEIGHT / 2 - 25, 289, 131};
    SDL_RenderCopy(renderer, tex_score, NULL, &dstrect2);

    //render score
    if(bird->score > 9)
    {
    SDL_Rect dstrectb = (SDL_Rect) {WIDTH / 2 + 70, 330, 20, 26};
    int tens_digit = bird->score / 10 % 10;
    int ones_digit = bird->score % 10;
    SDL_RenderCopy(renderer, tex_numbers[tens_digit], NULL, &dstrectb);
    dstrectb.x += dstrectb.w;
    SDL_RenderCopy(renderer, tex_numbers[ones_digit], NULL, &dstrectb);
    }
    else
    {
    SDL_Rect dstrectb = (SDL_Rect) {WIDTH / 2 + 70, 330, 20, 26};
    SDL_RenderCopy(renderer, tex_numbers[bird->score], NULL, &dstrectb);
    }

    //render highscore
    if(highScore > 9)
    {
    SDL_Rect dstrecta = (SDL_Rect) {WIDTH / 2 + 70, 390, 20, 26};
    int tens_digit = highScore / 10 % 10;
    int ones_digit = highScore % 10;
    SDL_RenderCopy(renderer, tex_numbers[tens_digit], NULL, &dstrecta);
    dstrecta.x += dstrecta.w;
    SDL_RenderCopy(renderer, tex_numbers[ones_digit], NULL, &dstrecta);
    }
    else
    {
    SDL_Rect dstrecta = (SDL_Rect) {WIDTH / 2 + 70, 390, 20, 26};
    SDL_RenderCopy(renderer, tex_numbers[highScore], NULL, &dstrecta);
    }

    //reder huy chuong
    if(bird->score >= 0 && bird->score <= 20 )
    {
        SDL_Rect rectcu = {WIDTH / 2 - 125, 340, 60, 60};
        SDL_RenderCopy(renderer, tex_dong, NULL, &rectcu);
    } else if (bird->score > 20 && bird->score <= 40)
    {
        SDL_Rect rectsliver = {WIDTH / 2 - 125, 340, 60, 60};
        SDL_RenderCopy(renderer, tex_bac, NULL, &rectsliver);
    } else {
        SDL_Rect rectgold = {WIDTH / 2 - 125, 340, 60, 60};
        SDL_RenderCopy(renderer, tex_vang, NULL, &rectgold);
    }
    
    SDL_RenderPresent(renderer);

    SDL_Delay(1000);

    bool wait = true, playagain = false;
    while(wait)
    {
        while(SDL_PollEvent(&event))
        {   
            if(event.type == SDL_QUIT)
                wait = false;

            if((event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_SPACE) || (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT))
                wait = false, playagain = true;
        }
    }

    if(playagain)
    {
        SDL_Delay(500);
        gameover = false;
        gameStarted = true;
        init();
    } else 
    {
        isRunning = false;
    }
}


void Game::render()
{   
    SDL_RenderClear(renderer);
    
    // background
    if((bird->score >= 10 && bird->score <= 20) || (bird->score >= 30 && bird->score <= 40) || (bird->score >= 50 && bird->score <= 60)){
        SDL_RenderCopy(renderer, tex_backgroundN, NULL, NULL);
    } else {
        SDL_RenderCopy(renderer, tex_backgroundD, NULL, NULL);
    }

    // pipes
    for(auto pipe : pipes)
    {
        pipe->render(renderer, tex_pipe);
    }

    // score
    if(bird->score > 9)
    {
    SDL_Rect dstrect = (SDL_Rect) {WIDTH / 2, 100, 40, 52};
    int tens_digit = bird->score / 10 % 10;
    int ones_digit = bird->score % 10;
    SDL_RenderCopy(renderer, tex_numbers[tens_digit], NULL, &dstrect);
    dstrect.x += dstrect.w;
    SDL_RenderCopy(renderer, tex_numbers[ones_digit], NULL, &dstrect);
    }
    else
    {
    SDL_Rect dstrect = (SDL_Rect) {WIDTH / 2, 100, 40, 52};
    SDL_RenderCopy(renderer, tex_numbers[bird->score], NULL, &dstrect);
    }

    // ground
    SDL_Rect dstrect4 =  (SDL_Rect) {ground1, HEIGHT - GROUND_HEIGHT, WIDTH, GROUND_HEIGHT};
    SDL_RenderCopy(renderer, tex_ground, NULL, &dstrect4);
    SDL_Rect dstrect5 =  (SDL_Rect) {ground2, HEIGHT - GROUND_HEIGHT, WIDTH, GROUND_HEIGHT};
    SDL_RenderCopy(renderer, tex_ground, NULL, &dstrect5);

    // player
    bird->render();

    // luân phiên reder hai nút pause và resume
    if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT && event.button.x >= 10 && event.button.x <= 45 && event.button.y >= 10 && event.button.y <= 45)
            pausedButtonClicked = true;
    if(pausedButtonClicked == true ){
        gamePaused = !gamePaused; // Chuyển trạng thái gamePaused
        pausedButtonClicked = false;}
    if(gamePaused){
        SDL_Rect textRect2 = {WIDTH / 3 , HEIGHT / 3 , 150, 40};
        SDL_RenderCopy(renderer, tex_dung, NULL, &textRect2);
        SDL_Rect bothRect = {10, 10, 35, 35};
        SDL_RenderCopy(renderer, tex_resume, NULL, &bothRect);
    } else {
        SDL_Rect bothRect = {10, 10, 35, 35};
        SDL_RenderCopy(renderer, tex_pause, NULL, &bothRect);
    }
    

    SDL_RenderPresent(renderer);
}

void Game::loadTextures()
{
    tex_backgroundD = IMG_LoadTexture(renderer, "image/background-day.png");
    tex_backgroundN = IMG_LoadTexture(renderer, "image/background-night.png");
    tex_pipe = IMG_LoadTexture(renderer, "image/pipe.png");
    tex_playerMid = IMG_LoadTexture(renderer, "image/chimmid.png");
    tex_playerUp = IMG_LoadTexture(renderer, "image/chimup.png");
    tex_playerDown = IMG_LoadTexture(renderer, "image/chimdown.png");
    tex_ground = IMG_LoadTexture(renderer, "image/base.png");
    tex_gameover = IMG_LoadTexture(renderer, "image/gameover.png");
    tex_getreddy = IMG_LoadTexture(renderer, "image/getreddy.png");
    tex_score = IMG_LoadTexture(renderer, "image/score.jpg");
    tex_pause = IMG_LoadTexture(renderer, "image/pause.jpg");
    tex_resume = IMG_LoadTexture(renderer, "image/resume.jpg");
    tex_vang = IMG_LoadTexture(renderer, "image/vangmedal.png");
    tex_bac = IMG_LoadTexture(renderer, "image/bacmedal.png");
    tex_dong = IMG_LoadTexture(renderer, "image/dongmedal.png");

    for(int i = 0; i < 10; i++)
    {
        std::string path = "image/" + std::to_string(i) + ".png";
        tex_numbers[i] = IMG_LoadTexture(renderer, path.c_str());
    }

    font = TTF_OpenFont("Arial.ttf", 36);
    SDL_Color textColor = { 255, 255, 255 }; // màu trắng
    SDL_Surface* surface1 = TTF_RenderText_Solid(font, "SPACE or CLICK to START !!!", textColor);
    tex_guild = SDL_CreateTextureFromSurface(renderer, surface1);
    SDL_FreeSurface(surface1);
    SDL_Surface* surface2 = TTF_RenderText_Solid(font, "PAUSED", textColor);
    tex_dung = SDL_CreateTextureFromSurface(renderer, surface2);
    SDL_FreeSurface(surface2);
    TTF_CloseFont(font);
}

void Game::Close() {

    SDL_DestroyTexture(tex_backgroundD);
    SDL_DestroyTexture(tex_backgroundN);
    SDL_DestroyTexture(tex_ground);
    SDL_DestroyTexture(tex_playerUp);
    SDL_DestroyTexture(tex_playerMid);
    SDL_DestroyTexture(tex_playerDown);
    SDL_DestroyTexture(tex_pipe);
    for(int i = 0; i < 10; i++) {
        SDL_DestroyTexture(tex_numbers[i]);
    }
    SDL_DestroyTexture(tex_gameover);
    SDL_DestroyTexture(tex_getreddy);
    SDL_DestroyTexture(tex_guild);
    SDL_DestroyTexture(tex_score);
    SDL_DestroyTexture(tex_pause);
    SDL_DestroyTexture(tex_resume);
    SDL_DestroyTexture(tex_vang);
    SDL_DestroyTexture(tex_bac);
    SDL_DestroyTexture(tex_dong);
    SDL_DestroyTexture(tex_dung);

    Mix_FreeChunk(die);
    Mix_FreeChunk(hit);
    Mix_FreeChunk(swooshing);
    Mix_FreeChunk(wing);

    delete bird;

    while(!pipes.empty()) {
        delete pipes.front();
        pipes.pop_front();
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    Mix_CloseAudio();
    SDL_Quit();
}
