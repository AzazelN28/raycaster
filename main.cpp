#include <iostream>
#include <string>
#include <cmath>
#include <SDL.h>

struct vec2_t {
  float x;
  float y;

  vec2_t() : x(0), y(0) {}
  vec2_t(float x, float y) : x(x), y(y) {}

  void copy(const vec2_t &vec) {
    this->x = vec.x;
    this->y = vec.y;
  }

  void advance(const float direction, const float step) {
    this->advanceX(direction, step);
    this->advanceY(direction, step);
  }

  void advanceX(const float direction, const float step) {
    this->x += cos(direction) * step;
  }

  void advanceY(const float direction, const float step) {
    this->y += sin(direction) * step;
  }
};

struct level_t
{
  char *data;

  unsigned char width;
  unsigned char height;

  level_t() : width(5), height(5) {}
  level_t(unsigned char width, unsigned char height)
  {
    this->width = width;
    this->height = height;
    this->data = new char[width * height];
  }

  ~level_t()
  {
    delete this->data;
  }

  char get(const float x, const float y)
  {
    const int tx = static_cast<int>(floor(x));
    const int ty = static_cast<int>(floor(y));
    const int offset = ty * this->width + tx;
    return this->data[offset];
  }
};

struct transform_t {
  vec2_t position;
  float direction;

  transform_t() : direction(0) {}

  void copy(transform_t transform) {
    this->position.copy(transform.position);
    this->direction = transform.direction;
  }

  void strafe(const float step) {
    this->position.advance(this->direction + M_PI * 0.5, step);
  }

  void advance(const float step) {
    this->position.advance(this->direction, step);
  }

  void advanceX(const float step) {
    this->position.advanceX(this->direction, step);
  }

  void advanceY(const float step) {
    this->position.advanceY(this->direction, step);
  }
};

struct camera_t {
  transform_t transform;
  float field_of_view;

  camera_t() : field_of_view(M_PI * 0.125) {}

  void advance(const float step) {
    this->transform.advance(step);
  }

  void advanceX(const float step) {
    this->transform.advanceX(step);
  }

  void advanceY(const float step) {
    this->transform.advanceY(step);
  }

  void strafe(const float step) {
    this->transform.strafe(step);
  }

  bool collides(level_t &level) {
    if (this->transform.position.x <= 0
     || this->transform.position.x > level.width - 1
     || this->transform.position.y <= 0
     || this->transform.position.y > level.height - 1) {
      return true;
    }
    return level.get(this->transform.position.x, this->transform.position.y) > 0;
  }
};

struct ray_t {
  transform_t transform;
  float distance;
  float distanceX;
  float distanceY;
  float stepX;
  float stepY;

  void copy_from_camera(camera_t &camera, float direction) {
    this->transform.copy(camera.transform);
    this->transform.direction = camera.transform.direction + direction;
    this->stepX = cos(this->transform.direction);
    this->stepY = sin(this->transform.direction);
    this->distance = 0;
    this->distanceX = 0;
    this->distanceY = 0;
  }

  void advance(const float step) {
    this->transform.advance(step);
    this->distance += step;
    this->distanceX += step * this->stepX;
    this->distanceY += step * this->stepY;
  }

  void advanceX(const float step) {
    this->transform.advanceX(step);
    this->distanceX += step * this->stepX;
  }

  void advanceY(const float step) {
    this->transform.advanceY(step);
    this->distanceY += step * this->stepY;
  }

  void calculateDistance() {
    this->distance = sqrt(this->distanceX * this->distanceX + this->distanceY * this->distanceY);
  }

  bool collides(level_t &level) {
    if (this->transform.position.x <= 0
     || this->transform.position.x > level.width - 1
     || this->transform.position.y <= 0
     || this->transform.position.y > level.height - 1) {
      return true;
    }
    return level.get(this->transform.position.x, this->transform.position.y) > 0;
  }
};

int main() {

  const int width = 320;
  const int height = 200;

  bool correct_perspective = true;

	SDL_Window *window;
	SDL_Renderer *renderer;
  SDL_Event event;
  SDL_Rect ceiling_rect;
  ceiling_rect.x = 0;
  ceiling_rect.y = 0;
  ceiling_rect.w = 320;
  ceiling_rect.h = 100;

  SDL_Rect floor_rect;
  floor_rect.x = 0;
  floor_rect.y = 100;
  floor_rect.w = 320;
  floor_rect.h = 100;

  level_t level(5,5);
  level.data[0] = 1;
  level.data[1] = 1;
  level.data[2] = 1;
  level.data[3] = 1;
  level.data[4] = 1;

  level.data[5] = 1;
  level.data[6] = 0;
  level.data[7] = 0;
  level.data[8] = 0;
  level.data[9] = 1;

  level.data[10] = 1;
  level.data[11] = 0;
  level.data[12] = 1;
  level.data[13] = 0;
  level.data[14] = 1;

  level.data[15] = 1;
  level.data[16] = 0;
  level.data[17] = 0;
  level.data[18] = 0;
  level.data[19] = 1;

  level.data[20] = 1;
  level.data[21] = 1;
  level.data[22] = 1;
  level.data[23] = 1;
  level.data[24] = 1;

  camera_t camera;
  camera.transform.position.x = 1.5;
  camera.transform.position.y = 1.5;

  ray_t ray;

  if (SDL_Init(SDL_INIT_VIDEO) < 0) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't initialize SDL: %s", SDL_GetError());
    return 3;
  }

  if (SDL_CreateWindowAndRenderer(width, height, 0, &window, &renderer)) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't create window and renderer: %s", SDL_GetError());
    return 3;
  }

  while (1) {
    SDL_PollEvent(&event);
    const Uint8 *state = SDL_GetKeyboardState(NULL);
    if (event.type == SDL_QUIT) {
      break;
    }

    if (state[SDL_SCANCODE_LEFT] ) {
      camera.transform.direction -= 0.01;
    } else if (state[SDL_SCANCODE_RIGHT]) {
      camera.transform.direction += 0.01;
    }

    if (state[SDL_SCANCODE_UP] || state[SDL_SCANCODE_W]) {
      camera.advance(0.01);
    } else if (state[SDL_SCANCODE_DOWN] || state[SDL_SCANCODE_S]) {
      camera.advance(-0.01);
    }

    if (state[SDL_SCANCODE_A]) {
      camera.strafe(-0.01);
    } else if (state[SDL_SCANCODE_D]) {
      camera.strafe(0.01);
    }

    SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0x00);
    SDL_RenderClear(renderer);

    SDL_SetRenderDrawColor(renderer, 0x33, 0x33, 0x33, 0x00);
    SDL_RenderFillRect(renderer, &ceiling_rect);

    SDL_SetRenderDrawColor(renderer, 0x66, 0x66, 0x66, 0x00);
    SDL_RenderFillRect(renderer, &floor_rect);

    for (int x = 0; x < width; x++) {
      float p = static_cast<float>(x) / static_cast<float>(width);
      float a = (p - 0.5) * 2.0 * camera.field_of_view;

      bool is_bright = false;
      ray.copy_from_camera(camera, a);
      while (!ray.collides(level)) {
        ray.advanceX(0.01);
        if (ray.collides(level)) {
          is_bright = true;
          break;
        }
        ray.advanceY(0.01);
        if (ray.collides(level)) {
          break;
        }
      }
      ray.calculateDistance();

      if (ray.distance > 0) {

        // TODO: Esta no es exactamente la fórmula necesaria para calcular la distancia correcta.
        float z = (correct_perspective ? 1 / (ray.distance * cos(a)) : 1 / ray.distance);

        int y_horizon = height >> 1;

        int wall_height = static_cast<int>(round(z * y_horizon));

        int wall_start = y_horizon - wall_height;
        int wall_end = y_horizon + wall_height;

        for (int y = wall_start; y < wall_end; y++) {
          if (is_bright) {
            SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0xFF, 0x00);
          } else {
            SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x66, 0x00);
          }
          SDL_RenderDrawPoint(renderer, x, y);
        }
      }
    }

    SDL_RenderPresent(renderer);
  }

  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);

  SDL_Quit();

  return 0;
}
