#include "bird.h"
void Bird::init()
{
    currentRenderingTexture = mid;
    animationFrames = 0;
    score = 0;

    velocity = 0.0f;
    acceleration = 0.0f;

    pos.x = WIDTH / 3 - 32;
    pos.y = HEIGHT / 2;
    pos.w = BIRD_WIDTH;
    pos.h = BIRD_HEIGHT;
}

Bird::Bird(SDL_Texture *up, SDL_Texture *mid, SDL_Texture *down, SDL_Renderer *renderer) : up(up), mid(mid), down(down), renderer(renderer)
{
    point = Mix_LoadWAV("sound/point.wav");
    init();
}

Bird::~Bird()
{
    if (point) {
        Mix_FreeChunk(point);
        point = nullptr;
    }
}

void Bird::update(bool jump, float elapsedTime)
{      
    elapsedTime *= 5;
    if(jump)
    {
        acceleration = 0.0f;
        velocity = -GRAVITY;
    }
    else
    {
        acceleration += GRAVITY * elapsedTime;  // gia tốc = trọng lực x thời gian
    }

    if(acceleration >= GRAVITY)
        acceleration = GRAVITY; // ngăn cho Bird ko rơi quá nhanh

    velocity += acceleration * elapsedTime; // vận tốc tăng dần theo gia tốc và thời gian
    pos.y += velocity * elapsedTime; // vị trí y của Bird được cập nhật theo vận tốc và thời gian. Khi vận tốc là số âm, đối tượng Bird sẽ rơi xuống dưới đất
}

bool Bird::collisionDetector(Pipe* pipe) {
    bool collided = false;
    if (pos.x + pos.w > pipe->top_dst.x && pos.x < pipe->top_dst.x + PIPE_WIDTH) {
        if (pos.y < pipe->top_dst.y + pipe->top_dst.h || pos.y + pos.h > pipe->bottom_dst.y) {
            collided = true;
        }
    }
    else if (pos.x > pipe->top_dst.x + PIPE_WIDTH) {
        if (pipe->top_dst.x + PIPE_WIDTH < pos.x && !pipe->passed) {
            score++;
            Mix_PlayChannel(-1, point, 0); // phát ra âm thanh tính điểm
            pipe->passed = true;
        }
    }

    if (pos.y + pos.h > HEIGHT - GROUND_HEIGHT || pos.y < 0) { // Bird chạm trần hoặc sàn thì sẽ bị đánh dấu
        collided = true;
    }

    return collided;
}



void Bird::render()
{
    animation();    

    if(velocity == 0)
        SDL_RenderCopy(renderer, mid, NULL, &pos);
    else if(velocity < 50)
        SDL_RenderCopyEx(renderer, currentRenderingTexture, NULL, &pos, -30.0, NULL, SDL_FLIP_NONE);
    else if(velocity >= 50 && velocity < 200)
        SDL_RenderCopyEx(renderer, currentRenderingTexture, NULL, &pos, 30.0, NULL, SDL_FLIP_NONE);
    else if(velocity >= 200)
        SDL_RenderCopyEx(renderer, mid, NULL, &pos, 90.0, NULL, SDL_FLIP_NONE);

}

void Bird::animation()
{
    animationFrames++; // theo dõi số lượng frame đã được hiển thị kể từ lần cuối cùng khi hình ảnh của chim được thay đổi

    if(animationFrames == 5)
        currentRenderingTexture = down;
    else if(animationFrames == 15)
        currentRenderingTexture = mid;
    else if(animationFrames == 20) // khi animationFrames đạt giá trị 20, hàm sẽ đặt lại animationFrames về 0 để chu trình chuyển đổi hình ảnh của chim có thể tiếp tục
    {
        animationFrames = 0;
        currentRenderingTexture = up;
    }
}
 