/*
 * program.c
 */

#define TUI_IMPLEMENT
#include "tui.h"

#define DEBUG_IMPLEMENT
#include "debug.h"

/*
 *
 */
bool input_window_event(tui_window_t* window, int key)
{
  info_print("input_window_event: %d", key);

  if (tui_input_event(window->data, key))
  {
    return true;
  }

  return false;
}

/*
 * Main function
 */
int main(int argc, char* argv[])
{
  debug_file_open("debug.log");

  if(tui_init() != 0)
  {
    info_print("Failed to initialize TUI");

    debug_file_close();

    return 1;
  }

  info_print("Initialized TUI");

  tui_t* tui = tui_create((tui_config_t)
  {
    .color = (tui_color_t)
    {
      .bg = TUI_COLOR_BLACK,
      .fg = TUI_COLOR_WHITE
    }
  });

  if (!tui)
  {
    info_print("Failed to create TUI");

    tui_quit();

    debug_file_close();

    return 2;
  }

  info_print("Created TUI");


  tui_window_parent_t* parent = tui_window_parent_create(tui, (tui_window_parent_config_t)
  {
    .name = "parent",
    .rect = TUI_RECT_NONE,
    .color = (tui_color_t)
    {
      .bg = TUI_COLOR_GREEN,
      .fg = TUI_COLOR_MAGENTA
    },
    .border = (tui_border_t)
    {
      .is_active = true,
      .color = TUI_COLOR_NONE
    },
    .is_inflated = true,
    .has_padding = true,
    .pos = TUI_POS_CENTER,
  });

  tui_window_parent_t* banner = tui_window_parent_create(tui, (tui_window_parent_config_t)
  {
    .name = "banner",
    .rect = (tui_rect_t)
    {
      .w = TUI_PARENT_SIZE,
      .h = 8,
      .y = -12
    },
    .color = (tui_color_t)
    {
      .bg = TUI_COLOR_GREEN,
      .fg = TUI_COLOR_MAGENTA
    },
    .border = (tui_border_t)
    {
      .is_active = true,
      .color = TUI_COLOR_NONE
    },
    .is_inflated = false,
    .has_padding = true,
    .pos = TUI_POS_CENTER,
    .align = TUI_ALIGN_CENTER
  });


  tui_window_text_t* input_window = tui_parent_child_text_create(banner, (tui_window_text_config_t)
  {
    .string = "This is some text",
    .rect = TUI_RECT_NONE,
    .color = (tui_color_t)
    {
      .bg = TUI_COLOR_CYAN,
      .fg = TUI_COLOR_BLACK
    },
    .pos = TUI_POS_END,
    .align = TUI_ALIGN_CENTER,
    .event = &input_window_event
  });

  tui_input_t* input = tui_input_create(100, input_window);

  input_window->head.data = input;


  tui_window_parent_t* footer = tui_window_parent_create(tui, (tui_window_parent_config_t)
  {
    .name = "footer",
    .rect = (tui_rect_t)
    {
      .w = 0,
      .h = 1,
      .y = -1
    },
    .color = (tui_color_t)
    {
      .bg = TUI_COLOR_GREEN,
      .fg = TUI_COLOR_MAGENTA
    },
    .border = (tui_border_t)
    {
      .is_active = false
    },
    .is_inflated = false,
    .has_padding = true,
    .pos = TUI_POS_CENTER,
    .align = TUI_ALIGN_BETWEEN,
  });

  char* numbers[] =
  {
    "one",
    "two",
    "three",
    "four",
    "five",
    "six",
    "seven",
    "eight",
    "nine",
  };

  for (size_t index = 0; index < 9; index++)
  {
    tui_parent_child_text_create(footer, (tui_window_text_config_t)
    {
      .string = numbers[index],
      .rect = TUI_RECT_NONE,
      .color = (tui_color_t)
      {
        .bg = TUI_COLOR_WHITE,
        .fg = TUI_COLOR_BLACK,
      },
      .align = TUI_POS_CENTER,
      .pos = TUI_POS_CENTER,
    });
  }




  tui_window_parent_t* box = tui_parent_child_parent_create(parent, (tui_window_parent_config_t)
  {
    .name = "box",
    .rect = TUI_RECT_NONE,
    .color = (tui_color_t)
    {
      .fg = TUI_COLOR_RED
    },
    .border = (tui_border_t)
    {
      .is_active = false
    },
    .is_vertical = true,
    .is_inflated = true,
    .has_padding = false,
    .pos = TUI_POS_END,
    .align = TUI_ALIGN_BETWEEN
  });

  char* lines[] =
  {
    "[+] Apple",
    "[+] Pear\nnewline",
    "[+] Banana"
  };

  for (size_t index = 0; index < 3; index++)
  {
    tui_parent_child_text_create(box, (tui_window_text_config_t)
    {
      .string = lines[index],
      .rect = TUI_RECT_NONE,
      .color = (tui_color_t)
      {
        .bg = TUI_COLOR_BLUE
      },
      .align = TUI_POS_CENTER,
      .pos = TUI_POS_CENTER,
    });
  }

  tui_window_parent_t* box2 = tui_parent_child_parent_create(parent, (tui_window_parent_config_t)
  {
    .name = "box2",
    .rect = TUI_RECT_NONE,
    .color = (tui_color_t)
    {
      .bg = TUI_COLOR_WHITE,
      .fg = TUI_COLOR_RED
    },
    .border = (tui_border_t)
    {
      .is_active = true
    },
    .is_vertical = true,
    .has_padding = true,
    .pos = TUI_POS_END,
  });

  tui_parent_child_text_create(box2, (tui_window_text_config_t)
  {
    .name = "box2-title",
    .string = "BOX2",
    .rect = (tui_rect_t)
    {
      .x = 1,
      .y = 0,
      .w = TUI_PARENT_SIZE - 2,
      .h = 1
    },
    .color = (tui_color_t)
    {
      .bg = TUI_COLOR_MAGENTA
    }
  });

  char* lines2[] =
  {
    "[+] Keyboard",
    "[+] Mouse",
    "[+] Computer\nnewline",
    "[+] Case",
    "[+] Voltage",
    "[+] Fan"
  };

  for (size_t index = 0; index < 6; index++)
  {
    tui_parent_child_text_create(box2, (tui_window_text_config_t)
    {
      .string = lines2[index],
      .rect = TUI_RECT_NONE,
      .color = (tui_color_t)
      {
        .bg = TUI_COLOR_BLUE
      },
      .align = TUI_POS_START
    });
  }


  tui->window = (tui_window_t*) input_window;

  tui->is_running = true;

  tui_render(tui);

  int key;

  while (tui->is_running && (key = wgetch(stdscr)))
  {
    if (key == KEY_CTRLS)
    {
      tui->is_running = false;
      break;
    }

    tui_event(tui, key);

    tui_render(tui);
  }


  tui_input_delete(&input);

  tui_delete(&tui);

  info_print("Deleted TUI");


  tui_quit();

  info_print("Quitted TUI");

  debug_file_close();

  return 0;
}
