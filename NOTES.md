# Notes
- fix flickering screen
- remove specific window_ _create functions and make it more generic.
- fix tui_tab_ functions: only is_interact windows can be tabbed to
- it looks like: chart is not resizing when window is resizing, only one refresh later
- 'f' for fullscreen in chart window
- input search window (under list) stock symbol show in chart
- 's' for search bar
- list item is marked when viewing stock in chart

## Maybe
- Remove rect argument and use child->_rect instead
- move is_inflated from parent to specific children
  and split up to x and y inflation
- utilize that tui, menu and parent has same fields (create generic functions)

## Future
- fix tui_tab_forward and tui_tab_backward
- tidy up stock.h header library
- create new menu with stock in table with HLOC values
