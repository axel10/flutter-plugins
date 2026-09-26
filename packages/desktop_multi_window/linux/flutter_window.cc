#include "flutter_window.h"

#include <iostream>

FlutterWindow::FlutterWindow(const std::string& id,
                             const std::string& argument,
                             GtkWidget* window)
    : id_(id), window_argument_(argument), window_(window) {}

FlutterWindow::~FlutterWindow() = default;

void FlutterWindow::SetChannel(FlMethodChannel* channel) {
  channel_ = channel;
}

void FlutterWindow::NotifyWindowEvent(const gchar* event, FlValue* data) {
  if (channel_) {
    fl_method_channel_invoke_method(channel_, event, data, nullptr, nullptr, nullptr);
  }
}

void FlutterWindow::Show() {
  if (window_) {
    gtk_widget_show(GTK_WIDGET(window_));
  }
}

void FlutterWindow::Hide() {
  if (window_) {
    gtk_widget_hide(GTK_WIDGET(window_));
  }
}

void FlutterWindow::SetDarkMode(bool is_dark) {
  GtkSettings* settings = window_ != nullptr ? gtk_widget_get_settings(window_)
                                             : gtk_settings_get_default();
  if (settings != nullptr) {
    g_object_set(settings, "gtk-application-prefer-dark-theme",
                 is_dark ? TRUE : FALSE, nullptr);
    if (!is_dark) {
      g_autofree gchar* theme_name = nullptr;
      g_object_get(settings, "gtk-theme-name", &theme_name, nullptr);
      if (theme_name != nullptr && g_str_has_suffix(theme_name, "-dark")) {
        g_autofree gchar* light_theme_name =
            g_strndup(theme_name, strlen(theme_name) - 5);
        g_object_set(settings, "gtk-theme-name", light_theme_name, nullptr);
      }
    }
  }
}

void FlutterWindow::SetAlwaysOnTop(bool is_always_on_top) {
  if (window_ != nullptr) {
    gtk_window_set_keep_above(GTK_WINDOW(window_),
                              is_always_on_top ? TRUE : FALSE);
  }
}

void FlutterWindow::HandleWindowMethod(const gchar* method,
                                       FlValue* arguments,
                                       FlMethodCall* method_call) {
  g_autoptr(FlMethodResponse) response = nullptr;

  if (strcmp(method, "window_show") == 0) {
    Show();
    response = FL_METHOD_RESPONSE(fl_method_success_response_new(nullptr));
  } else if (strcmp(method, "window_hide") == 0) {
    Hide();
    response = FL_METHOD_RESPONSE(fl_method_success_response_new(nullptr));
  } else if (strcmp(method, "window_set_dark_mode") == 0) {
    if (arguments != nullptr && fl_value_get_type(arguments) == FL_VALUE_TYPE_MAP) {
      FlValue* dark_val = fl_value_lookup_string(arguments, "darkMode");
      if (dark_val != nullptr && fl_value_get_type(dark_val) == FL_VALUE_TYPE_BOOL) {
        SetDarkMode(fl_value_get_bool(dark_val));
      }
    }
    response = FL_METHOD_RESPONSE(fl_method_success_response_new(nullptr));
  } else if (strcmp(method, "window_set_always_on_top") == 0) {
    if (arguments != nullptr && fl_value_get_type(arguments) == FL_VALUE_TYPE_MAP) {
      FlValue* top_val = fl_value_lookup_string(arguments, "alwaysOnTop");
      if (top_val != nullptr && fl_value_get_type(top_val) == FL_VALUE_TYPE_BOOL) {
        SetAlwaysOnTop(fl_value_get_bool(top_val));
      }
    }
    response = FL_METHOD_RESPONSE(fl_method_success_response_new(nullptr));
  } else {
    g_autofree gchar* error_msg = g_strdup_printf("unknown method: %s", method);
    response = FL_METHOD_RESPONSE(
        fl_method_error_response_new("-1", error_msg, nullptr));
  }

  fl_method_call_respond(method_call, response, nullptr);
}
