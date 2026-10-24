/*
 * menu.cpp:  Drop-down and popup menus as GtkPopoverMenu, from a GMenu built of
 *   the core's items: one action per item (stateful when checked, disabled when
 *   grayed), sections between separators.
 */

#include "core/menu.h"

#include "gtk/gtk_backend.h"

namespace w32::native {

namespace {

struct Open
{
   Top *top;
   GtkWidget *popover;
   GSimpleActionGroup *actions;
   void (*done)(UINT, void *);
   void *data;
   UINT chosen = 0;
};

// "&File" -> "_File"; "&&" -> "&"; "_" -> "__".
std::string Label(const std::string &ansi)
{
   std::string out;
   for (size_t i = 0; i < ansi.size(); i++)
   {
      char c = ansi[i];
      if (c == '&')
      {
         if (i + 1 < ansi.size() && ansi[i + 1] == '&')
            out += '&', i++;
         else
            out += '_';
      }
      else if (c == '_')
         out += "__";
      else
         out += c;
   }
   return AnsiToUtf8(out.c_str());
}

// "Ctrl+Shift+X" -> "<Control><Shift>x", or empty if GTK can't parse it.
std::string Accel(const std::string &text)
{
   std::string out, key;
   size_t start = 0;
   while (start <= text.size())
   {
      size_t plus = text.find('+', start);
      std::string part = text.substr(start, plus == std::string::npos ? std::string::npos : plus - start);
      if (plus == std::string::npos)
      {
         key = part;
         break;
      }
      if (part == "Ctrl" || part == "Strg")
         out += "<Control>";
      else if (part == "Alt")
         out += "<Alt>";
      else if (part == "Shift" || part == "Umschalt")
         out += "<Shift>";
      else
         return "";
      start = plus + 1;
   }
   static const struct
   {
      const char *win, *gtk;
   } names[] = {{"Del", "Delete"}, {"Entf", "Delete"}, {"Ins", "Insert"}, {"Enter", "Return"}, {"Esc", "Escape"},
                {"Space", "space"}, {"Tab", "Tab"}, {"Home", "Home"}, {"End", "End"}};
   if (key.size() == 1)
      key = (char) g_ascii_tolower(key[0]);
   else
      for (const auto &n : names)
         if (g_ascii_strcasecmp(key.c_str(), n.win) == 0)
            key = n.gtk;
   out += key;
   guint keyval;
   GdkModifierType mods;
   if (!gtk_accelerator_parse(out.c_str(), &keyval, &mods))
      return "";
   return out;
}

void OnActivate(GSimpleAction *, GVariant *, gpointer data)
{
   auto *o = (Open *) g_object_get_data(G_OBJECT(data), "w32-open");
   if (o)
      o->chosen = GPOINTER_TO_UINT(g_object_get_data(G_OBJECT(data), "w32-id"));
}

GMenuModel *Build(Open *o, const std::vector<MenuEntry> &items, int *counter)
{
   GMenu *root = g_menu_new();
   GMenu *section = g_menu_new();
   for (const MenuEntry &e : items)
   {
      if (e.separator)
      {
         if (g_menu_model_get_n_items(G_MENU_MODEL(section)) > 0)
         {
            g_menu_append_section(root, nullptr, G_MENU_MODEL(section));
            g_object_unref(section);
            section = g_menu_new();
         }
         continue;
      }
      GMenuItem *item;
      if (e.has_sub)
      {
         GMenuModel *sub = Build(o, e.sub, counter);
         item = g_menu_item_new_submenu(Label(e.text).c_str(), sub);
         g_object_unref(sub);
      }
      else
      {
         std::string name = "i" + std::to_string((*counter)++);
         GSimpleAction *a = e.checked ? g_simple_action_new_stateful(name.c_str(), nullptr, g_variant_new_boolean(TRUE))
                                      : g_simple_action_new(name.c_str(), nullptr);
         g_simple_action_set_enabled(a, !e.grayed);
         g_object_set_data(G_OBJECT(a), "w32-open", o);
         g_object_set_data(G_OBJECT(a), "w32-id", GUINT_TO_POINTER(e.id));
         g_signal_connect(a, "activate", G_CALLBACK(OnActivate), a);
         g_action_map_add_action(G_ACTION_MAP(o->actions), G_ACTION(a));
         g_object_unref(a);
         item = g_menu_item_new(Label(e.text).c_str(), ("w32menu." + name).c_str());
         if (!e.accel.empty())
            if (std::string accel = Accel(e.accel); !accel.empty())
               g_menu_item_set_attribute(item, "accel", "s", accel.c_str());
      }
      g_menu_append_item(section, item);
      g_object_unref(item);
   }
   if (g_menu_model_get_n_items(G_MENU_MODEL(section)) > 0)
      g_menu_append_section(root, nullptr, G_MENU_MODEL(section));
   g_object_unref(section);
   return G_MENU_MODEL(root);
}

gboolean Finish(gpointer data)
{
   auto *o = (Open *) data;
   gtk_widget_insert_action_group(o->top->overlay, "w32menu", nullptr);
   gtk_widget_unparent(o->popover);
   g_object_unref(o->actions);
   auto done = o->done;
   void *d = o->data;
   UINT id = o->chosen;
   delete o;
   done(id, d);
   return G_SOURCE_REMOVE;
}

void OnClosed(GtkPopover *, gpointer data)
{
   // The item's action runs after the popover closes: report on the next turn.
   g_idle_add(Finish, data);
}

}  // namespace

void TrackMenu(Top *top, const std::vector<MenuEntry> &items, const RECT &anchor, void (*done)(UINT id, void *data),
               void *data)
{
   auto *o = new Open{top, nullptr, g_simple_action_group_new(), done, data};
   int counter = 0;
   GMenuModel *model = Build(o, items, &counter);
   gtk_widget_insert_action_group(top->overlay, "w32menu", G_ACTION_GROUP(o->actions));
   o->popover = gtk_popover_menu_new_from_model_full(model, GTK_POPOVER_MENU_NESTED);   // cascades, as Windows
   g_object_unref(model);
   gtk_widget_set_parent(o->popover, top->overlay);
   GdkRectangle r = {(int) anchor.left, (int) anchor.top, (int) (anchor.right - anchor.left),
                     (int) (anchor.bottom - anchor.top)};
   gtk_popover_set_pointing_to(GTK_POPOVER(o->popover), &r);
   gtk_popover_set_has_arrow(GTK_POPOVER(o->popover), FALSE);
   gtk_popover_set_position(GTK_POPOVER(o->popover), GTK_POS_BOTTOM);
   gtk_widget_set_halign(o->popover, GTK_ALIGN_START);
   g_signal_connect(o->popover, "closed", G_CALLBACK(OnClosed), o);
   gtk_popover_popup(GTK_POPOVER(o->popover));
}

}  // namespace w32::native
