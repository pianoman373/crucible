#include <crucible/crucible.hpp>

int main(int argc, char *argv[]) {
    Window::create({1000, 800}, "test", false);

    Camera cam2d;
    cam2d.orthographic = true;

//    sf::Music music;
//    if (!music.openFromFile("resources/platformer_level03.ogg"))
//        return -1; // error
//    music.setVolume(50);
//    music.play();



    Texture run;
    run.load("resources/player-run.png", true);
    Texture idle;
    idle.load("resources/player-idle.png", true);
    Texture jump;
    jump.load("resources/player-jump.png", true);
    Texture ground;
    ground.load("resources/ground.png", true);
    Texture background;
    background.load("resources/background.png", true);
    Texture crate;
    crate.load("resources/crate.png", true);

    Renderer::init(false, 0, 1000, 800);
    Renderer::settings.fxaa = false;
    Renderer::settings.tonemap = false;
    Renderer::settings.vignette = false;
    Renderer::settings.bloom = false;
    Renderer::settings.ssao = false;

    float acc = 0.0f;
    int i = 0;

    vec2 playerPos;
    vec2 playerVelocity;
    float playerDirection = 1.0f;


    while (Window::isOpen()) {
        Window::begin();
        cam2d.dimensions = {10.0f, 10.0f/Window::getAspectRatio()};

        bool onGround = true;

        if (Input::isKeyDown(Input::KEY_A)) {
            playerPos.x -= 2.0f * Window::deltaTime();
            playerDirection = -1.0f;
        }
        if (Input::isKeyDown(Input::KEY_D)) {
            playerPos.x += 2.0f * Window::deltaTime();
            playerDirection = 1.0f;
        }
        if (playerPos.y < 0.01f) {
            playerVelocity.y = 0.0f;
        }
        else {
            playerVelocity.y -= 9.8f * Window::deltaTime();
            onGround = false;
        }
        if (Input::isKeyDown(Input::KEY_SPACE) && playerPos.y < 0.01f) {
            playerVelocity.y = 5.0f;
        }

        playerPos = playerPos + (playerVelocity * Window::deltaTime());

        acc += Window::deltaTime();

        if (acc > 0.1f) {
            acc = 0.0f;
            i++;
        }

        if (onGround) {
            if (Input::isKeyDown(Input::KEY_A) || Input::isKeyDown(Input::KEY_D)) {
            Renderer::renderSprite(run, playerPos, vec2(playerDirection, 1.0f),
                                   vec4((float) i / 6.0f, 0.0f, 1.0f / 6.0f, 1.0f));
        } else {
            Renderer::renderSprite(idle, playerPos, vec2(playerDirection, 1.0f),
                                   vec4((float) i / 4.0f, 0.0f, 1.0f / 4.0f, 1.0f));
        }
    }
        else {
            if (playerVelocity.y > 0.0f)
                Renderer::renderSprite(jump, playerPos, vec2(playerDirection, 1.0f),
                                   vec4(0.0f / 2.0f, 0.0f, 1.0f / 2.0f, 1.0f));
            else
                Renderer::renderSprite(jump, playerPos, vec2(playerDirection, 1.0f),
                                       vec4(1.0f / 2.0f, 0.0f, 1.0f / 2.0f, 1.0f));
        }

        Renderer::renderSprite(ground, vec2(0.0f, -2.5f), vec2(16.0f, 4.0f), vec4(0.0f, 0.0f, 16.0f, 1.0f));


        Renderer::renderSprite(crate, vec2(4.0f, 0.0f), vec2(1.0f, 1.0f), vec4(0.0f, 0.0f, 1.0f, 1.0f));
        Renderer::renderSprite(crate, vec2(-3.0f, 0.0f), vec2(1.0f, 1.0f), vec4(0.0f, 0.0f, 1.0f, 1.0f));
        Renderer::renderSprite(crate, vec2(-4.0f, 0.0f), vec2(1.0f, 1.0f), vec4(0.0f, 0.0f, 1.0f, 1.0f));
        Renderer::renderSprite(crate, vec2(-3.6f, 1.0f), vec2(1.0f, 1.0f), vec4(0.0f, 0.0f, 1.0f, 1.0f));


        Renderer::renderSprite(background, vec2(0.0f, 0.0f), vec2(14.0f, cam2d.dimensions.y), vec4(0.0f, 0.0f, 1.0f, 1.0f));
        Renderer::flush(cam2d);

        Window::end();
    }
}
