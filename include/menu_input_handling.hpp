#ifndef MENU_INPUT_HANDLING_H
#define MENU_INPUT_HANDLING_H

#include "Menu_browser.h"

enum class Menu_action {browsed, selected, selected_shift, space, esc};

namespace Menu_input_handling
{

Menu_action get_action(Menu_browser& browser);

} //Menu_input_handling

#endif