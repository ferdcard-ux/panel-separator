#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <gtk/gtk.h>
#include <libxfce4panel/libxfce4panel.h>
#include <libxfce4util/libxfce4util.h>

typedef struct {
    GtkWidget *drawing_area;
    gint width;
    gboolean transparent;
    gboolean auto_expand;
    GdkRGBA color;
    gint plugin_id;
    XfcePanelPlugin *plugin;
} SeparatorData;

#define SEP_DATA_KEY "custom-separator-data"
#define CONF_PATH "/.config/xfce4-panel-custom-separator.conf"

static SeparatorData *
get_data (XfcePanelPlugin *plugin)
{
    return g_object_get_data (G_OBJECT (plugin), SEP_DATA_KEY);
}

static gchar *
get_conf_path (void)
{
    return g_strconcat (g_get_home_dir (), CONF_PATH, NULL);
}

static gint
read_plugin_id_from_cmdline (void)
{
    gchar *cmdline = NULL;
    gsize length = 0;
    gint plugin_id = -1;

    if (g_file_get_contents ("/proc/self/cmdline", &cmdline, &length, NULL) && length > 0) {
        GPtrArray *args = g_ptr_array_new ();
        gchar *p = cmdline;
        gchar *end = cmdline + length;

        while (p < end) {
            g_ptr_array_add (args, g_strdup (p));
            p += strlen (p) + 1;
        }

        if (args->len >= 3) {
            plugin_id = atoi ((gchar *) g_ptr_array_index (args, 2));
        }

        g_ptr_array_free (args, TRUE);
    }
    g_free (cmdline);

    return plugin_id;
}

static void
load_config (SeparatorData *sep)
{
    gchar *path = get_conf_path ();
    gchar *contents = NULL;
    gchar section[32];

    sep->width = 6;
    sep->transparent = TRUE;
    sep->auto_expand = FALSE;
    gdk_rgba_parse (&sep->color, "#800080");

    g_snprintf (section, sizeof (section), "[%d]", sep->plugin_id);

    if (g_file_get_contents (path, &contents, NULL, NULL)) {
        gchar **lines = g_strsplit (contents, "\n", -1);
        gboolean in_section = FALSE;
        gint i;

        for (i = 0; lines[i]; i++) {
            gchar *line = g_strstrip (lines[i]);

            if (line[0] == '[') {
                in_section = (g_strcmp0 (line, section) == 0);
                continue;
            }

            if (!in_section)
                continue;

            if (g_str_has_prefix (line, "width="))
                sep->width = CLAMP (atoi (line + 6), 1, 5000);
            else if (g_str_has_prefix (line, "transparent="))
                sep->transparent = g_str_has_suffix (line, "=true");
            else if (g_str_has_prefix (line, "auto_expand="))
                sep->auto_expand = g_str_has_suffix (line, "=true");
            else if (g_str_has_prefix (line, "color="))
                gdk_rgba_parse (&sep->color, line + 6);
        }
        g_strfreev (lines);
    }
    g_free (contents);
    g_free (path);
}

static void
save_config (SeparatorData *sep)
{
    gchar *path = get_conf_path ();
    gchar *contents = NULL;
    gchar *new_contents = NULL;
    gchar section[32];
    gchar color_str[64];
    gchar *entry;
    gint i;

    g_snprintf (section, sizeof (section), "[%d]", sep->plugin_id);

    g_snprintf (color_str, sizeof (color_str), "#%02x%02x%02x",
                (gint)(sep->color.red * 255),
                (gint)(sep->color.green * 255),
                (gint)(sep->color.blue * 255));

    entry = g_strdup_printf ("%s\nwidth=%d\ntransparent=%s\nauto_expand=%s\ncolor=%s\n",
                             section,
                             sep->width,
                             sep->transparent ? "true" : "false",
                             sep->auto_expand ? "true" : "false",
                             color_str);

    g_file_get_contents (path, &contents, NULL, NULL);

    if (contents) {
        gchar **lines = g_strsplit (contents, "\n", -1);
        gboolean in_section = FALSE;
        gint start_idx = -1;
        gint end_idx = -1;

        for (i = 0; lines[i]; i++) {
            gchar *l = g_strstrip (lines[i]);
            if (l[0] == '[') {
                if (g_strcmp0 (l, section) == 0) {
                    in_section = TRUE;
                    start_idx = i;
                } else if (in_section) {
                    end_idx = i;
                    break;
                }
            }
        }

        if (in_section && end_idx == -1)
            end_idx = i;

        if (start_idx >= 0) {
            GString *rebuild = g_string_new (NULL);
            for (i = 0; i < start_idx; i++) {
                g_string_append (rebuild, lines[i]);
                g_string_append_c (rebuild, '\n');
            }
            g_string_append (rebuild, entry);
            for (i = end_idx; lines[i]; i++) {
                g_string_append (rebuild, lines[i]);
                g_string_append_c (rebuild, '\n');
            }
            new_contents = g_string_free (rebuild, FALSE);
        }
        g_strfreev (lines);
    }

    if (!new_contents) {
        if (contents) {
            new_contents = g_strdup_printf ("%s%s", contents, entry);
        } else {
            new_contents = g_strdup (entry);
        }
    }

    g_file_set_contents (path, new_contents, -1, NULL);
    g_free (new_contents);
    g_free (contents);
    g_free (entry);
    g_free (path);
}

static void
apply_expand_mode (SeparatorData *sep)
{
    if (sep->auto_expand) {
        gtk_widget_set_hexpand (sep->drawing_area, TRUE);
        gtk_widget_set_size_request (sep->drawing_area, -1, -1);
    } else {
        gtk_widget_set_hexpand (sep->drawing_area, FALSE);
        gtk_widget_set_size_request (sep->drawing_area, sep->width, -1);
    }
}

static gboolean
on_draw (GtkWidget *widget, cairo_t *cr, gpointer data)
{
    SeparatorData *sep = data;
    if (!sep->transparent) {
        gdk_cairo_set_source_rgba (cr, &sep->color);
        cairo_paint (cr);
    }
    return TRUE;
}

static gboolean
on_size_changed (XfcePanelPlugin *plugin, gint size, gpointer data)
{
    SeparatorData *sep = get_data (plugin);
    if (!sep->auto_expand)
        gtk_widget_set_size_request (sep->drawing_area, sep->width, -1);
    return TRUE;
}

static void
on_configure_response (GtkWidget *dialog, gint response, gpointer data)
{
    XfcePanelPlugin *plugin = XFCE_PANEL_PLUGIN (data);

    if (response == GTK_RESPONSE_OK || response == GTK_RESPONSE_DELETE_EVENT) {
        SeparatorData *sep = get_data (plugin);
        GtkWidget *width_spin = g_object_get_data (G_OBJECT (dialog), "width_spin");
        GtkWidget *check = g_object_get_data (G_OBJECT (dialog), "transparent_check");
        GtkWidget *color_btn = g_object_get_data (G_OBJECT (dialog), "color_button");
        GtkWidget *expand_check = g_object_get_data (G_OBJECT (dialog), "expand_check");

        sep->width = (gint) gtk_spin_button_get_value (GTK_SPIN_BUTTON (width_spin));
        sep->transparent = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (check));
        sep->auto_expand = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (expand_check));
        gtk_color_chooser_get_rgba (GTK_COLOR_CHOOSER (color_btn), &sep->color);

        save_config (sep);
        apply_expand_mode (sep);
        gtk_widget_queue_draw (sep->drawing_area);
    }
    gtk_widget_destroy (dialog);
}

static void
on_color_toggled (GtkToggleButton *check, GtkWidget *color_btn)
{
    gtk_widget_set_sensitive (color_btn, !gtk_toggle_button_get_active (check));
}

static void
on_expand_toggled (GtkToggleButton *check, GtkWidget *width_spin)
{
    gboolean auto_expand = gtk_toggle_button_get_active (check);
    gtk_widget_set_sensitive (width_spin, !auto_expand);
}

static void
on_configure (XfcePanelPlugin *plugin, gpointer data)
{
    SeparatorData *sep = get_data (plugin);
    GtkWidget *dialog, *content, *grid;
    GtkWidget *label, *width_spin, *transparent_check, *color_button, *expand_check;
    GtkAdjustment *adj;

    dialog = gtk_dialog_new_with_buttons (
        _("Separator Settings"),
        NULL,
        GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
        _("Cancel"), GTK_RESPONSE_CANCEL,
        _("OK"), GTK_RESPONSE_OK,
        NULL);

    gtk_window_set_default_size (GTK_WINDOW (dialog), 350, -1);

    content = gtk_dialog_get_content_area (GTK_DIALOG (dialog));
    grid = gtk_grid_new ();
    gtk_grid_set_column_spacing (GTK_GRID (grid), 12);
    gtk_grid_set_row_spacing (GTK_GRID (grid), 8);
    gtk_container_set_border_width (GTK_CONTAINER (grid), 12);
    gtk_box_pack_start (GTK_BOX (content), grid, TRUE, TRUE, 0);

    expand_check = gtk_check_button_new_with_label (_("Auto-expand (fill available space)"));
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (expand_check), sep->auto_expand);
    gtk_grid_attach (GTK_GRID (grid), expand_check, 0, 0, 2, 1);

    label = gtk_label_new (_("Width (px):"));
    gtk_widget_set_halign (label, GTK_ALIGN_START);
    gtk_grid_attach (GTK_GRID (grid), label, 0, 1, 1, 1);

    adj = gtk_adjustment_new (sep->width, 1, 5000, 1, 10, 0);
    width_spin = gtk_spin_button_new (GTK_ADJUSTMENT (adj), 1, 0);
    gtk_widget_set_hexpand (width_spin, TRUE);
    gtk_widget_set_sensitive (width_spin, !sep->auto_expand);
    gtk_grid_attach (GTK_GRID (grid), width_spin, 1, 1, 1, 1);

    transparent_check = gtk_check_button_new_with_label (_("Transparent"));
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (transparent_check), sep->transparent);
    gtk_grid_attach (GTK_GRID (grid), transparent_check, 0, 2, 2, 1);

    label = gtk_label_new (_("Color:"));
    gtk_widget_set_halign (label, GTK_ALIGN_START);
    gtk_grid_attach (GTK_GRID (grid), label, 0, 3, 1, 1);

    color_button = gtk_color_button_new ();
    gtk_color_chooser_set_rgba (GTK_COLOR_CHOOSER (color_button), &sep->color);
    gtk_widget_set_sensitive (color_button, !sep->transparent);
    gtk_grid_attach (GTK_GRID (grid), color_button, 1, 3, 1, 1);

    g_signal_connect (transparent_check, "toggled",
                      G_CALLBACK (on_color_toggled), color_button);
    g_signal_connect (expand_check, "toggled",
                      G_CALLBACK (on_expand_toggled), width_spin);

    g_object_set_data (G_OBJECT (dialog), "width_spin", width_spin);
    g_object_set_data (G_OBJECT (dialog), "transparent_check", transparent_check);
    g_object_set_data (G_OBJECT (dialog), "color_button", color_button);
    g_object_set_data (G_OBJECT (dialog), "expand_check", expand_check);

    g_signal_connect (dialog, "response", G_CALLBACK (on_configure_response), plugin);

    gtk_widget_show_all (dialog);
}

static void
construct (XfcePanelPlugin *plugin)
{
    SeparatorData *sep;

    xfce_textdomain (GETTEXT_PACKAGE, LOCALEDIR, "UTF-8");

    sep = g_new0 (SeparatorData, 1);
    sep->plugin_id = read_plugin_id_from_cmdline ();
    sep->plugin = plugin;

    if (sep->plugin_id < 0) {
        g_warning ("custom-separator: could not determine plugin ID from cmdline");
        sep->plugin_id = GPOINTER_TO_INT (g_object_get_data (G_OBJECT (plugin), "xfce-plugin-id"));
    }

    g_object_set_data_full (G_OBJECT (plugin), SEP_DATA_KEY, sep, g_free);

    load_config (sep);

    sep->drawing_area = gtk_drawing_area_new ();
    g_signal_connect (sep->drawing_area, "draw", G_CALLBACK (on_draw), sep);

    apply_expand_mode (sep);

    gtk_container_add (GTK_CONTAINER (plugin), sep->drawing_area);
    gtk_widget_show_all (GTK_WIDGET (plugin));

    g_signal_connect (plugin, "size-changed", G_CALLBACK (on_size_changed), NULL);
    g_signal_connect (plugin, "configure-plugin", G_CALLBACK (on_configure), NULL);
    xfce_panel_plugin_menu_show_configure (plugin);
}

XFCE_PANEL_PLUGIN_REGISTER (construct)
