void setWorkingDisplay();
void set_message_display(String title, String text);
void setPopupMessage(String title, String text);
void set_display_number_being_input(int new_value);
void activate_number_input_display();
void set_number_input_display(String in_number_input_prompt, int in_number_being_input);
void setStringSelection(String newValue);
void activeStringSelectionDisplay();
void setStringSelectionDisplay(String inStringSelectionPrompt, String inCurrentStringSelection);
void setMenuDisplay(String menu_text, uint64_t pos_in_menu);
void move_selector_up();
void move_selector_down();
int getSelectedItem();
bool set_selected_item(int item);
void setClearDisplayPage();

void setup_lcd();

void loop_lcd();
