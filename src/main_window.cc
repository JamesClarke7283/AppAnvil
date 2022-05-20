#include "main_window.h"

#include <tuple>

MainWindow::MainWindow()
    : prof_control{new ProfilesControllerInstance()},
      proc_control{new ProcessesControllerInstance()},
      logs_control{new LogsControllerInstance()},
      file_chooser_control{new FileChooserControllerInstance()},
      about{new About()},
      console{new ConsoleThreadInstance(prof_control, proc_control, logs_control)}
{
  // Add tabs to the stack pane
  m_tab_stack.add(*(prof_control->get_tab()), "prof", "Profiles");
  m_tab_stack.add(*(proc_control->get_tab()), "proc", "Processes");
  m_tab_stack.add(*(logs_control->get_tab()), "logs", "Logs");
  m_tab_stack.add(*(file_chooser_control->get_tab()), "file_chooser", "Load Profile");

  // Attach the stack to the stack switcher
  m_switcher.set_stack(m_tab_stack);

  // Connect the stackswitcher to the 'on_switch' method
  auto focus = sigc::mem_fun(*this, &MainWindow::on_switch);
  m_switcher.signal_event().connect(focus, true);
  on_switch(NULL);

  // Connect the profile tab to the `send_status_change` method
  auto change_fun = sigc::mem_fun(*this, &MainWindow::send_status_change);
  prof_control->get_tab()->set_status_change_signal_handler(change_fun);

  // Configure settings related to the 'About' button
  m_about_button.set_image_from_icon_name("dialog-question");

  auto about_togggle_fun = sigc::mem_fun(*this, &MainWindow::on_about_toggle);
  m_about_button.signal_toggled().connect(about_togggle_fun, true);

  // Configure settings related to 'Search' button
  m_search_button.set_image_from_icon_name("edit-find-symbolic");

  auto search_togggle_fun = sigc::mem_fun(*this, &MainWindow::on_search_toggle);
  m_search_button.signal_toggled().connect(search_togggle_fun, true);

  // Add the main page and the about page to the top stack
  // This stack controls whether the 'About' page is visible, or the main application 
  m_top_stack.add(m_tab_stack, "main_page");
  m_top_stack.add(*about, "about_page");

  // Set some default properties for titlebar
  m_headerbar.set_custom_title(m_switcher);
  m_headerbar.pack_end(m_about_button);
  m_headerbar.pack_end(m_search_button);

  m_headerbar.set_title("AppAnvil");
  m_headerbar.set_subtitle("GUI for AppArmor");
  m_headerbar.set_hexpand(true);
  m_headerbar.set_show_close_button(true);

  // Set the icon
  auto builder         = Gtk::Builder::create_from_resource("/resources/icon.glade");
  Gtk::Image *icon_ptr = nullptr;
  builder->get_widget<Gtk::Image>("icon", icon_ptr);
  this->set_icon(icon_ptr->get_pixbuf());

  // Set some default settings for the window
  this->set_default_size(DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT);
  this->add_events(Gdk::EventMask::ENTER_NOTIFY_MASK);

  // Add and show all children
  this->set_titlebar(m_headerbar);
  this->add(m_top_stack);
  this->show_all();

  // Hide all tabs with a searchbar
  on_search_toggle();
}

void MainWindow::send_status_change(const std::string &profile, const std::string &old_status, const std::string &new_status)
{
  console->send_change_profile_status_message(profile, old_status, new_status);
}

void MainWindow::on_about_toggle()
{
  bool is_active = m_about_button.get_active();

  if(is_active){
    m_switcher.hide();
    m_top_stack.set_visible_child("about_page");
    m_about_button.set_label("Return to application");
    m_about_button.set_always_show_image(false);
  }
  else {
    m_switcher.show();
    m_top_stack.set_visible_child("main_page");
    m_about_button.set_label("");
    m_about_button.set_image_from_icon_name("dialog-question");
    m_about_button.set_always_show_image(true);
  }

  handle_search_button_visiblity();
}

void MainWindow::on_search_toggle()
{
  bool is_active = m_search_button.get_active();

  if(is_active){
    prof_control->get_tab()->show_searchbar();
    proc_control->get_tab()->show_searchbar();
    logs_control->get_tab()->show_searchbar();
    about->show_searchbar();
  }
  else {
    prof_control->get_tab()->hide_searchbar();
    proc_control->get_tab()->hide_searchbar();
    logs_control->get_tab()->hide_searchbar();
    about->hide_searchbar();
  }
}

bool MainWindow::on_switch(GdkEvent *event)
{
  std::ignore = event;
  
  // Make sure to clear the label on the load-profiles tab 
  file_chooser_control->clearLabel();

  std::string visible_child = m_tab_stack.get_visible_child_name();
  if(visible_child == "prof") {
    console->send_refresh_message(PROFILE);
  } else if(visible_child == "proc") {
    console->send_refresh_message(PROCESS);
  } else if(visible_child == "logs") {
    console->send_refresh_message(LOGS);
  }

  handle_search_button_visiblity();
  return false;
}

void MainWindow::handle_search_button_visiblity()
{
  bool about_is_active = m_about_button.get_active();
  std::string visible_child = m_tab_stack.get_visible_child_name();

  if(visible_child == "file_chooser" && !about_is_active) {
    m_search_button.hide();
  } else {
    m_search_button.show();
  }
}
