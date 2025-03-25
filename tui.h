/*
 * tui.h - terminal user interface library
 *
 * Written by Hampus Fridholm
 *
 * Last updated: 2025-03-19
 *
 * This library depends on debug.h
 */

#ifndef TUI_H
#define TUI_H

#include <ncurses.h>
#include <stdbool.h>
#include <stdint.h>

/*
 * Definitions of keys
 */

#define KEY_CTRLC  3
#define KEY_CTRLZ 26
#define KEY_ESC   27
#define KEY_CTRLS 19
#define KEY_CTRLH  8
#define KEY_CTRLD  4
#define KEY_ENTR  10
#define KEY_TAB    9

/*
 * Declarations of tui structs
 */

typedef struct tui_t tui_t;

typedef struct tui_menu_t tui_menu_t;

typedef struct tui_window_t tui_window_t;

/*
 * Definitions of event function signatures
 */

typedef bool (*tui_window_event_t)(tui_window_t* window, int key);

typedef bool (*tui_menu_event_t)(tui_menu_t* menu, int key);

typedef bool (*tui_event_t)(tui_t* tui, int key);

/*
 * Normal rect
 */
typedef struct tui_rect_t
{
  int  w;
  int  h;
  int  x;
  int  y;
  bool is_none; // Hidden flag to represent NONE rect
} tui_rect_t;

const tui_rect_t TUI_RECT_NONE = { .is_none = true };

/*
 * Foreground and background color struct
 */
typedef struct tui_color_t
{
  short fg;
  short bg;
} tui_color_t;

/*
 * Definitions of colors
 *
 * The rest of the colors come defined in ncurses.h
 */
#define COLOR_NONE -1

const tui_color_t TUI_COLOR_NONE = { COLOR_NONE, COLOR_NONE };

/*
 * Border
 */
typedef struct tui_border_t
{
  tui_color_t color;
  bool        is_dashed;
} tui_border_t;

/*
 * Declarations of window types
 */

typedef struct tui_window_parent_t tui_window_parent_t;

typedef struct tui_window_text_t   tui_window_text_t;

/*
 * Window struct
 */
typedef struct tui_window_t
{
  bool                 is_text;
  char*                name;
  bool                 is_visable;
  tui_rect_t           rect;
  tui_rect_t           _rect; // Temp calculated rect
  WINDOW*              window;
  tui_color_t          color;
  tui_window_event_t   event;
  tui_window_parent_t* parent;
  tui_t*               tui;
  void*                data; // User attached data
} tui_window_t;

/*
 * Input data struct, that can be attached to window
 *
 * If text window is NULL:
 * - the input is not visable
 * - arrow keys don't work
 */
typedef struct tui_input_t
{
  char*              buffer;
  size_t             buffer_size;
  size_t             buffer_len;

  tui_window_text_t* window;
  char*              string; // Visable string
} tui_input_t;

/*
 * List data struct, that can be attached to window
 */
typedef struct tui_list_t
{
  tui_window_t* windows;
  size_t        window_count;
  size_t        window_index;
} tui_list_t;

/*
 * Position
 */
typedef enum tui_pos_t
{
  TUI_POS_START,
  TUI_POS_CENTER,
  TUI_POS_END
} tui_pos_t;

/*
 * Alignment
 */
typedef enum tui_align_t
{
  TUI_ALIGN_START,
  TUI_ALIGN_CENTER,
  TUI_ALIGN_END,
  TUI_ALIGN_BETWEEN,
  TUI_ALIGN_AROUND,
  TUI_ALIGN_EVENLY
} tui_align_t;

/*
 * Text window struct
 */
typedef struct tui_window_text_t
{
  tui_window_t head;
  char*        string;
  char*        text;
  tui_pos_t    pos;
  tui_align_t  align;
} tui_window_text_t;

/*
 * Parent window struct
 */
typedef struct tui_window_parent_t
{
  tui_window_t   head;
  tui_window_t** children;
  size_t         child_count;
  bool           is_vertical;
  tui_border_t*  border;
  bool           has_padding;
  tui_pos_t      pos;
  tui_align_t    align;
} tui_window_parent_t;

/*
 * Menu struct
 */
typedef struct tui_menu_t
{
  char*            name;
  tui_window_t**   windows;
  size_t           window_count;
  tui_menu_event_t event;
  tui_t*           tui;
} tui_menu_t;

/*
 * Tui struct
 */
typedef struct tui_t
{
  int            w;
  int            h;
  tui_menu_t**   menus;
  size_t         menu_count;
  tui_window_t** windows;
  size_t         window_count;
  tui_menu_t*    menu;
  tui_window_t*  window;
  tui_color_t    color; // Active color
  tui_event_t    event;
  bool           is_running;
} tui_t;

#endif // TUI_H

#ifdef TUI_IMPLEMENT

#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

#include "debug.h"

/*
 * Get ncurses color index from tui color
 */
static inline short tui_ncurses_color_get(tui_color_t color)
{
  return (color.fg + 1) * 9 + (color.bg + 1);
}

/*
 * Inherit color from last color in case of transparency
 *
 * If foreground or background is NONE, last color is used
 */
static inline tui_color_t tui_color_inherit(tui_color_t last_color, tui_color_t color)
{
  if (color.fg == COLOR_NONE)
  {
    color.fg = last_color.fg;
  }

  if (color.bg == COLOR_NONE)
  {
    color.bg = last_color.bg;
  }

  return color;
}

/*
 * Turn on foreground and background color
 */
void tui_color_on(tui_t* tui, tui_color_t color)
{
  color = tui_color_inherit(tui->color, color);

  attron(COLOR_PAIR(tui_ncurses_color_get(color)));

  tui->color = color;
}

/*
 * Turn off foreground and background color
 */
void tui_color_off(tui_t* tui, tui_color_t color)
{
  color = tui_color_inherit(tui->color, color);

  attroff(COLOR_PAIR(tui_ncurses_color_get(color)));

  tui->color = TUI_COLOR_NONE;
}

/*
 * Fill window with it's background and foreground color
 */
void tui_window_fill(tui_window_t* window)
{
  tui_t* tui = window->tui;

  tui_color_t color = tui_color_inherit(tui->color, window->color);

  attron(COLOR_PAIR(tui_ncurses_color_get(color)));

  tui->color = color;
}

/*
 * Draw window border with it's background and foreground color
 */
void tui_border_draw(tui_window_parent_t* window)
{
  tui_border_t* border = window->border;

  if (!border) return;

  tui_window_t head = window->head;

  tui_color_on(head.tui, border->color);

  if (border->is_dashed)
  {
    box(head.window, 0, 0);
  }
  else
  {
    box(head.window, 0, 0);
  }

  tui_color_off(head.tui, border->color);
}

/*
 * Initialize tui colors
 *
 * ncurses color and index differ by 1
 */
void tui_colors_init(void)
{
  for (short fg_index = 0; fg_index < 9; fg_index++)
  {
    for (short bg_index = 0; bg_index < 9; bg_index++)
    {
      size_t index = fg_index * 9 + bg_index;

      short fg = (fg_index - 1);
      short bg = (bg_index - 1);

      init_pair(index, fg, bg);
    }
  }
}

/*
 * Initialize tui (ncurses)
 */
int tui_init(void)
{
  initscr();
  noecho();
  raw();
  keypad(stdscr, TRUE);

  if (start_color() == ERR || !has_colors())
  {
    endwin();

    return 1;
  }

  use_default_colors();

  tui_colors_init();

  clear();
  refresh();

  return 0;
}

/*
 * Quit tui (ncurses)
 */
void tui_quit(void)
{
  clear();

  refresh();

  endwin();
}

/*
 * Create ncurses WINDOW* for tui_window_t
 */
static inline WINDOW* tui_ncurses_window_create(int w, int h, int x, int y)
{
  WINDOW* window = newwin(h, w, y, x);

  if (!window)
  {
    return NULL;
  }

  keypad(window, TRUE);

  return window;
}

/*
 * Resize ncurses WINDOW*
 */
static inline void tui_ncurses_window_resize(WINDOW* window, int w, int h, int x, int y)
{
  wresize(window, h, w);

  mvwin(window, y, x);
}

/*
 * Delete ncurses WINDOW*
 */
static inline void tui_ncurses_window_free(WINDOW** window)
{
  if (!window || !(*window)) return;

  wclear(*window);

  wrefresh(*window);

  delwin(*window);

  *window = NULL;
}

/*
 * Create tui struct
 */
tui_t* tui_create(tui_event_t event)
{
  tui_t* tui = malloc(sizeof(tui_t));

  if (!tui)
  {
    return NULL;
  }

  memset(tui, 0, sizeof(tui_t));

  *tui = (tui_t)
  {
    .w     = getmaxx(stdscr),
    .h     = getmaxy(stdscr),
    .event = event
  };

  return tui;
}

static inline void tui_windows_free(tui_window_t*** windows, size_t* count);

/*
 * Free parent window struct
 */
static inline void tui_window_parent_free(tui_window_parent_t** window)
{
  tui_windows_free(&(*window)->children, &(*window)->child_count);

  tui_ncurses_window_free(&(*window)->head.window);

  free(*window);

  *window = NULL;
}

/*
 * Free text window struct
 */
static inline void tui_window_text_free(tui_window_text_t** window)
{
  tui_ncurses_window_free(&(*window)->head.window);

  free((*window)->text);

  free(*window);

  *window = NULL;
}

/*
 * Free window struct
 */
static inline void tui_window_free(tui_window_t** window)
{
  if (!window || !(*window)) return;

  if ((*window)->is_text)
  {
    tui_window_text_free((tui_window_text_t**) window);
  }
  else
  {
    tui_window_parent_free((tui_window_parent_t**) window);
  }
}

/*
 * Free windows
 */
static inline void tui_windows_free(tui_window_t*** windows, size_t* count)
{
  for (size_t index = 0; index < *count; index++)
  {
    tui_window_free(&(*windows)[index]);
  }

  free(*windows);

  *windows = NULL;
  *count = 0;
}

/*
 * Free menu struct
 */
static inline void tui_menu_free(tui_menu_t** menu)
{
  if (!menu || !(*menu)) return;

  tui_windows_free(&(*menu)->windows, &(*menu)->window_count);

  free(*menu);

  *menu = NULL;
}

/*
 * Delete tui struct
 */
void tui_delete(tui_t** tui)
{
  if (!tui || !(*tui)) return;

  for (size_t index = 0; index < (*tui)->menu_count; index++)
  {
    tui_menu_free(&(*tui)->menus[index]);
  }

  free((*tui)->menus);

  tui_windows_free(&(*tui)->windows, &(*tui)->window_count);

  free(*tui);

  *tui = NULL;
}

/*
 * Trigger tui events
 */
void tui_event(tui_t* tui, int key)
{

}

/*
 * Get the height of wrapped text given the width
 */
static inline int tui_text_h_get(char* text, int max_w)
{
  size_t length = strlen(text);

  int h = 1;

  int line_w = 0;
  int space_index = 0;

  int last_space_index = space_index;

  for (size_t index = 0; index < length; index++)
  {
    char letter = text[index];

    if (letter == ' ')
    {
      space_index = index;
    }

    if (letter == '\n')
    {
      line_w = 0;

      h++;
    }
    else if (line_w >= max_w)
    {
      line_w = 0;

      h++;

      // Current word cannot be wrapped
      if (space_index == last_space_index)
      {
        info_print("Cannot be wrapped: max_w: %d\n", max_w);
        return -1;
      }

      index = space_index;

      last_space_index = space_index;
    }
    else
    {
      line_w++;
    }
  }

  return h;
}

/*
 * Get the width of wrapped text given the height
 */
static inline int tui_text_w_get(char* text, int h)
{
  int left  = 1;
  int right = strlen(text);

  int min_w = right;

  // Try every value between left and right, inclusive left == right
  while (left <= right)
  {
    int mid = (left + right) / 2;

    int curr_h = tui_text_h_get(text, mid);

    // If width was too small to wrap, increase width
    if (curr_h == -1)
    {
      left = mid + 1;
    }
    // If height got to large, increase width
    else if (curr_h > h)
    {
      left = mid + 1;
    }
    else // If the height is smaller than max height, store current best width
    {
      min_w = mid;
      right = mid - 1;
    }
  }

  return min_w;
}

/*
 * Get widths of lines in text, regarding max height
 */
static inline void tui_text_ws_get(int* ws, char* text, int h)
{
  int max_w = tui_text_w_get(text, h);

  size_t length = strlen(text);

  int line_index = 0;
  int line_w = 0;

  int space_index = 0;

  for (size_t index = 0; (index < length) && (line_index < h); index++)
  {
    char letter = text[index];

    if (letter == ' ')
    {
      space_index = index;
    }

    if (letter == ' ' && line_w == 0)
    {
      line_w = 0;
    }
    else if (letter == '\n')
    {
      ws[line_index++] = line_w;

      line_w = 0;
    }
    else if (line_w >= max_w)
    {
      // full line width - last partial word
      ws[line_index++] = line_w - (index - space_index);

      line_w = 0;

      index = space_index;
    }
    else
    {
      line_w++;
    }

    // Store the width of last line
    if (index + 1 == length)
    {
      ws[line_index] = line_w;
    }
  }
}

/*
 * Render text in rect in window
 *
 * xpos determines if the text is aligned to the left, centered or right
 */
static inline void tui_text_render(tui_window_text_t* window)
{
  tui_window_t head = window->head;

  tui_rect_t rect = head.rect;

  int h = tui_text_h_get(window->text, rect.w);

  int ws[h];

  tui_text_ws_get(ws, window->text, h);

  int line_index = 0;
  int line_w = 0;

  int y = 0;

  size_t length = strlen(window->string);

  for (size_t index = 0; index < length; index++)
  {
    char letter = window->string[index];

    if (letter == '\033')
    {
      while (index < length && window->string[index] != 'm') index++;
    }
    if (letter == ' ' && line_w == 0)
    {
      line_w = 0;
    }
    else if (line_w >= ws[line_index])
    {
      line_index++;

      line_w = 0;

      y++;
    }
    else
    {
      int x_shift = (rect.w - ws[line_index]) / 2;

      int y_shift = window->pos * (rect.h - h) / 2;

      wmove(head.window, rect.y + y_shift + y, rect.x + x_shift + line_w);

      waddch(head.window, letter);

      line_w++;
    }
  }
}

/*
 * Extract just the text from string
 *
 * ANSI escape characters will be left out
 */
static inline char* tui_text_extract(char* string)
{
  char* text = strdup(string);

  size_t length = strlen(string);

  memset(text, '\0', sizeof(char) * (length + 1));

  size_t text_len = 0;

  for (size_t index = 0; index < length; index++)
  {
    char letter = string[index];

    if (letter == '\033')
    {
      while (index < length && string[index] != 'm') index++;
    }
    else
    {
      text[text_len++] = letter;
    }
  }

  text[text_len] = '\0';

  return text;
}

/*
 * Render text window
 */
static inline void tui_window_text_render(tui_window_text_t* window)
{
  tui_window_t head = window->head;

  curs_set(0);

  werase(head.window);

  // Draw background
  tui_window_fill((tui_window_t*) window);

  // Draw text
  if (window->text)
  {
    free(window->text);
  }

  window->text = tui_text_extract(window->string);

  tui_text_render(window);

  wrefresh(head.window);
}

static inline void tui_window_render(tui_window_t* window);

/*
 * Render parent window with all it's children
 */
static inline void tui_window_parent_render(tui_window_parent_t* window)
{
  tui_window_t head = window->head;

  curs_set(0);

  werase(head.window);

  // Draw background
  tui_window_fill((tui_window_t*) window);

  // Draw border
  tui_border_draw(window);

  // Render children
  for (size_t index = 0; index < window->child_count; index++)
  {
    tui_window_render(window->children[index]);
  }

  wrefresh(head.window);
}

/*
 * Render window
 */
static inline void tui_window_render(tui_window_t* window)
{
  if (window->is_text)
  {
    tui_window_text_render((tui_window_text_t*) window);
  }
  else
  {
    tui_window_parent_render((tui_window_parent_t*) window);
  }
}

/*
 * Render windows
 */
static inline void tui_windows_render(tui_window_t** windows, size_t count)
{
  for (size_t index = count; index-- > 0;)
  {
    tui_window_render(windows[index]);
  }
}

/*
 * Render tui - active menu and all windows
 */
void tui_render(tui_t* tui)
{
  curs_set(0);

  tui_windows_render(tui->windows, tui->window_count);

  tui_menu_t* menu = tui->menu;

  if (menu)
  {
    tui_windows_render(menu->windows, menu->window_count);
  }

  refresh();
}

/*
 * Create tui_border_t* object
 */
tui_border_t* _tui_border_create(tui_color_t color, bool is_dashed)
{
  tui_border_t* border = malloc(sizeof(tui_border_t));

  if (!border)
  {
    return NULL;
  }

  *border = (tui_border_t)
  {
    .color     = color,
    .is_dashed = is_dashed
  };

  return border;
}

/*
 * Just create tui_window_parent_t* object
 */
static inline tui_window_parent_t* _tui_window_parent_create(tui_t* tui, char* name, tui_window_event_t event, tui_rect_t rect, tui_color_t color, bool is_visable, tui_border_t* border, bool has_padding, tui_pos_t pos, tui_align_t align, bool is_vertical)
{
  tui_window_parent_t* window = malloc(sizeof(tui_window_parent_t));

  if (!window)
  {
    return NULL;
  }

  memset(window, 0, sizeof(tui_window_parent_t));

  tui_window_t head = (tui_window_t)
  {
    .is_text    = false,
    .name       = name,
    .rect       = rect,
    .is_visable = is_visable,
    .color      = color,
    .event      = event,
    .tui        = tui
  };

  *window = (tui_window_parent_t)
  {
    .head        = head,
    .has_padding = has_padding,
    .border      = border,
    .pos         = pos,
    .align       = align,
    .is_vertical = is_vertical
  };

  return window;
}

/*
 *
 */
// tui_window_parent_t* tui_window_create(tui_t* tui, )

#endif // TUI_IMPLEMENT
