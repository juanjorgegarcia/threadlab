#include "helpers.c"

struct cp_args {
  char *input_path;
  char *output_path;
  pthread_mutex_t *m;
};

void *cp_th(void *_arg) {
  struct cp_args *args = _arg;

  cp(args->input_path, args->output_path);

  return NULL;
}

int main(int argc, char *argv[]) {
  printf("%s\n", argv[1]);
  char *input_path = argv[1];
  char *output_path = argv[2];

  pthread_mutex_t m;
  pthread_mutex_init(&m, NULL);
  struct cp_args args = {
      input_path,
      output_path, &m};
  pthread_t cp_thread;
  pthread_create(&cp_thread, NULL, cp_th, &args);

  int screenWidth = 800;
  int screenHeight = 450;
  InitWindow(screenWidth, screenHeight, "raylib [core] example - input mouse wheel");
  int boxPositionY = screenHeight / 2;
  SetTargetFPS(60);
  // Main game loop
  char *header_total_progress = malloc(sizeof(char) * 1024);
  char *header_file = malloc(sizeof(char) * 1024);

  int button_x = 100;
  int button_y = 300;
  int button_h = 50;
  int button_w = 120;
  int padding = 15;

  count_files(input_path);

  while (!WindowShouldClose()) { // Detect window close button or ESC key {
    BeginDrawing();
    ClearBackground(RAYWHITE);

    sprintf(header_total_progress, "Copying file %2d of %2d...", current_file, number_of_files);
    DrawText(header_total_progress, 90, boxPositionY - 122, 20, GRAY);

    DrawRectangle(100, boxPositionY - 100, total_progress * 3, 50, MAROON);

    DrawText(header_file, 90, boxPositionY - 22, 20, GRAY);

    sprintf(header_file, "Progress of file %2d: %0.2f%%", current_file, progress_of_file);

    DrawRectangle(100, boxPositionY, progress_of_file * 3, 50, MAROON);

    DrawRectangle(button_x, button_y, button_w, button_h, GRAY);
    DrawText("Cancelar", button_x + padding, button_y + padding, 20, WHITE);
    if (err_output_file) {
      DrawText("ERRO: Copiar o pasta para um arquivo é invalido", button_x + button_w + 50, button_y + button_h + 50, 20, MAROON);
    }

    // DrawText(header, 90, boxPositionY - 20, 20, GRAY);

    // se nao tiver funcionando o botao cancelar tirar a multiplicacao por 2

    int mouseX = 2 * GetMouseX(); // Returns mouse position X
    int mouseY = 2 * GetMouseY(); //

    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
      printf("MouseX:%d--- MouseY:%d\n", mouseX, mouseY);
      printf("buttonX:%d--- ButtonYY:%d\n", button_x, button_y);
    }
    if ((mouseX > button_x) && (mouseX < (button_x + button_w))) {
      if ((mouseY > button_y) && (mouseY < (button_y + button_h))) {
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
          cancel_copy(output_path);

          EndDrawing();

          break;
        }
      }
    }
    // DrawText("Texto teste!", 10, 10, 20, GRAY);
    EndDrawing();
    //----------------------------------------------------------------------------------
  }
  free(header_file);
  free(header_total_progress);

  pthread_join(cp_thread, NULL);
  CloseWindow();

  return 0;
}
