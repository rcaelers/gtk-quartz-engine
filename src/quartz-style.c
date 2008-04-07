/* This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include <config.h>
#include <math.h>
#include <string.h>
#include <stdio.h>
#include <gtk/gtk.h>

#include <Carbon/Carbon.h>
#import <AppKit/AppKit.h>

#include "quartz-style.h"
#include "quartz-draw.h"

static GtkStyleClass *parent_class;

/* FIXME: Fix GTK+ to export those in a quartz header file. */
CGContextRef gdk_quartz_drawable_get_context (GdkDrawable *drawable, gboolean antialias);
void gdk_quartz_drawable_release_context (GdkDrawable *drawable, CGContextRef context);
static gboolean is_combo_box_child (GtkWidget *widget);



/* TODO:
 *
 * Put the button and frame draw functions in helper functions and
 * handle all the state/focus/active/etc stuff there.
 *
 * Tweak the spacing between check/radio and the label.
 */

static gchar *debug = NULL;
#define DEBUG_DRAW if (debug && (strcmp (debug, "all") == 0 || strcmp (debug, G_OBJECT_TYPE_NAME (widget)) == 0)) \
  g_print ("%s, %s, %s\n", __PRETTY_FUNCTION__, G_OBJECT_TYPE_NAME (widget), detail);

#define IS_DETAIL(d,x) (d && strcmp (d, x) == 0)

static void
style_setup_settings (void)
{
    GtkSettings *settings;
    gint blink_time;

    debug = g_strdup (g_getenv ("DEBUG_DRAW"));

    settings = gtk_settings_get_default ();
    if (!settings)
	return;

    /* FIXME: Get settings from the system or hardcodes default for
     * things like popup menu delay, double click time/pixels defaults
     * etc.
     */

    blink_time = 2*500;
    g_object_set (settings,
			"gtk-cursor-blink", blink_time > 0,
			"gtk-cursor-blink-time", blink_time,
			NULL);
    g_object_set (settings,
			"gtk-double-click-distance", 2,
			"gtk-double-click-time", 500,
			NULL);
    g_object_set (settings,
			"gtk-dnd-drag-threshold", 5,
			NULL);
}

static void
style_setup_system_font (GtkStyle *style)
{
  gchar *font;

  /* FIXME: This should be fetched from the correct preference value. See:
   * http://developer.apple.com/documentation/UserExperience/\
   * Conceptual/OSXHIGuidelines/XHIGText/chapter_13_section_2.html
   */
  font = "Lucida Grande 13";

  if (style->font_desc)
    pango_font_description_free (style->font_desc);

  style->font_desc = pango_font_description_from_string (font);
}

/*

System font, kThemeSystemFont:

  "Lucida Grande Regular 13" is used for text in menus, dialogs, and
  full-size controls.

Emphasized system font, kThemeEmphasizedSystemFont:

  "Lucida Grande Bold 13", use sparingly. It is used for the message
  text in alerts.

Small system font, kThemeSmallSystemFont

  "Lucida Grande Regular 11" is used for informative text in
  alerts. It is also the default font for column headings in lists,
  for help tags, and for small controls. You can also use it to
  provide additional information about settings.

Emphasized small system font, kThemeSmallEmphazisedSystemFont

  "Lucida Grande Bold 11", use sparingly. You might use it to title a
  group of settings that appear without a group box, or for brief
  informative text below a text field.

Mini system font, kThemeMiniSystemFont

  "Lucida Grande Regular 9" is used for mini controls. It can also be
  used for utility window labels and text.

Emphasized mini system font, N/A

  "Lucida Grande Bold 9" is available for cases in which the
  emphasized small system font is too large.

Application font, kThemeApplicationFont

  "Lucida Grande Regular 13", the default font for user-created text
  documents.

Label font, kThemeLabelFont

  "Lucida Grande Regular 10" is used for the labels on toolbar buttons
  and to label tick marks on full-size sliders.

View font, kThemeViewsFont

  "Lucida Grande Regular 12" as the default font of text in lists and
  tables.

*/

#if 0
static gchar *
style_get_system_font_string (NSControlSize size)
{
  NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
  NSFont            *font;
  gchar             *str;

  font = [NSFont systemFontOfSize:[NSFont systemFontSizeForControlSize: size]];
  str = g_strdup_printf ("%s, size: %.2f", [[font familyName] UTF8String], [font pointSize]);

  [pool release];

  return str;
}
#endif

static void
style_setup_rc_styles (void)
{
  gchar *str;
  gchar buf[1024];

#define RC_WIDGET(name,match,body,...) \
  g_snprintf (buf, sizeof (buf), \
              "style \"" name "\"=\"quartz-default\" {\n" \
              body \
              "} widget \"" match "\" style \"" name "\"\n", __VA_ARGS__); \
  gtk_rc_parse_string (buf);

#define RC_WIDGET_CLASS(name,match,body,...) \
  g_snprintf (buf, sizeof (buf), \
              "style \"" name "\"=\"quartz-default\" {\n" \
              body \
              "} widget_class \"" match "\" style \"" name "\"\n", __VA_ARGS__); \
  gtk_rc_parse_string (buf);

#define RC_CLASS(name,match,body,...) \
  g_snprintf (buf, sizeof (buf), \
              "style \"" name "\"=\"quartz-default\" {\n" \
              body \
              "} class \"" match "\" style \"" name "\"\n", __VA_ARGS__); \
  gtk_rc_parse_string (buf);

  /* FIXME: Comment out for now, doesn't seem to do anything? */
  /*RC_WIDGET ("quartz-tooltips-caption", "gtk-tooltips.GtkLabel",
             "fg[NORMAL] = { %d, %d, %d }\n"
             "font_name = \"%s\"\n",
             0, 0, 0, "Lucida Grande 11");
  */

  /* FIXME: Get the right background and frame colors. */
  RC_WIDGET ("quartz-tooltips", "gtk-tooltip*",
             "fg[NORMAL] = { 0, 0, 0 }\n"
             /*"bg[NORMAL] = { %d, %d, %d }\n"*/
             "font_name = \"%s\"\n"
             "xthickness = 4\n"
             "ythickness = 4\n",
             /*0xf8ff07, 0xf9ff06, 0x7eff81,*/
             "Lucida Grande 11");

#if 0
  SInt32 height;
  GetThemeMetric (kThemeMetricPushButtonHeight, &height);
  g_print ("Button height: %d\n", (int) height);
#endif

  /* Button. The button has 1 pixel around itself, and draw the focus
   * there + 2 more pixels outside its allocation.
   */
  RC_WIDGET_CLASS ("quartz-button", "*Button*",
                   "GtkWidget::draw-border = { 2, 2, 2, 2 }\n"
                   "GtkWidget::focus-line-width = 1\n"
                   "GtkButton::inner-border = { 8, 8, 2, 4 }\n"
                   "%s", "");

  /* Small font. */
  RC_WIDGET ("quartz-small-font", "*small-font*",
             "font_name = \"%s\"\n",
             "Lucida Grande 11");

  /* Mini font. */
  RC_WIDGET ("quartz-mini-font", "*mini-font*",
             "font_name = \"%s\"\n",
             "Lucida Grande 9");

  /* TreeView column header (button). */
  str = g_strdup_printf ("style \"quartz-tree-header\" = \"quartz-default\"\n"
			 "{\n"
			 "font_name = \"%s\"\n"
			 "GtkWidget::focus-line-width = 0\n"
			 "GtkWidget::draw-border = { 1, 1, 1, 1 }\n"
			 "GtkButton::inner-border = { 3, 3, 1, 3 }\n"
			 "}widget_class \"*.GtkTreeView.*Button*\" style \"quartz-tree-header\"\n"
			 "widget_class \"*.GtkCTree.*Button*\" style \"quartz-tree-header\"\n",
			 "Lucida Grande 11");
  gtk_rc_parse_string (str);
  g_free (str);

  /* TreeView font. */
  str = g_strdup_printf ("style \"quartz-tree-row\" = \"quartz-default\"\n"
			 "{\n"
			 "font_name = \"%s\"\n"
			 "}widget_class \"*.GtkTreeView\" style \"quartz-tree-row\"\n",
			 "Lucida Grande 11");
  gtk_rc_parse_string (str);
  g_free (str);

  /* Entry. FIXME: This has some problems, we have to use exterior
   * focus to get any expose events at all for widget->window and not
   * just entry->text_area. We still don't get the focus draw properly
   * outside the entry though.
   */
  RC_WIDGET_CLASS ("quartz-entry", "*Entry*",
                   "GtkWidget::interior-focus = 0\n"
                   "GtkWidget::focus-line-width = 1\n"
                   "GtkEntry::inner-border = { 4, 4, 4, 3 }\n"
                   "%s", "");

  /* SpinButton. FIXME: This needs tweaking, the arrow part is cut off
   * and needs to draw a placard behind itself.
   */
  RC_WIDGET_CLASS ("quartz-spinbutton", "*SpinButton*",
                   "GtkWidget::interior-focus = 0\n"
                   "GtkWidget::focus-line-width = 1\n"
                   "GtkEntry::inner-border = { 4, 4, 4, 3 }\n"
                   "%s", "");

  /* MenuItem. */
  str = g_strdup_printf ("style \"quartz-menu\" = \"quartz-default\"\n"
                         "{\n"
                         "font_name = \"%s\"\n"
                         "xthickness = 0\n"
                         "ythickness = 2\n"
                         "}widget_class \"*MenuItem*\" style \"quartz-menu\"\n",
                         "Lucida Grande 14");
  gtk_rc_parse_string (str);
  g_free (str);

  /* ComboBox. We have to use thickness since the text isn't actually
   * a child of the button. This doesn't work perfectly, we get a
   * pixel off in the height (+1), since it's a multiple of 2.
   */
  RC_WIDGET_CLASS ("quartz-combobox-button", "*GtkComboBox.*GtkToggleButton",
                   "xthickness = 6\n"
                   "ythickness = 2\n"
                   "%s", "");

  RC_WIDGET_CLASS ("quartz-combobox-arrow", "*GtkComboBox",
                   "GtkComboBox::arrow-size = 9\n"
                   "%s", "");

  RC_WIDGET_CLASS ("quartz-combobox-cellview", "*GtkComboBox*.GtkCellView*",
                   "%s", "");

  /* ProgressBar. */
  RC_WIDGET_CLASS ("quartz-progressbar", "*ProgressBar*",
                   "font_name = \"%s\"\n",
                   "Lucida Grande 11");

  /* Scrollbar. */
  RC_WIDGET_CLASS ("quartz-scrollbar", "*Scrollbar*",
                   "GtkScrollbar::has-backward-stepper = 0\n"
                   "GtkScrollbar::has-secondary-backward-stepper = 1\n"
                   "GtkScrollbar::min-slider-length = 10\n" // check value
                   "GtkRange::trough-border = 0\n"
                   "GtkRange::stepper-spacing = 0\n"
                   "GtkRange::stepper-size = 14\n" // ?
                   "GtkRange::trough-under-steppers = 1\n"
                   "%s", "");

  /* ButtonBox. Remove the extra padding of children. */
  RC_WIDGET_CLASS ("quartz-button-box", "*ButtonBox*",
                   "GtkButtonBox::child-min-width = 0\n"
                   "GtkButtonBox::child-min-height = 0\n"
                   "%s", "");
}

static void
draw_arrow (GtkStyle      *style,
			GdkWindow     *window,
			GtkStateType   state,
			GtkShadowType  shadow,
			GdkRectangle  *area,
			GtkWidget     *widget,
			const gchar   *detail,
			GtkArrowType   arrow_type,
			gboolean       fill,
			gint           x,
			gint           y,
			gint           width,
			gint           height)
{
  CGContextRef context;
  HIRect rect;
	HIThemePopupArrowDrawInfo arrow_info;

  DEBUG_DRAW;

  if (GTK_IS_SCROLLBAR (widget))
    return;
  if (is_combo_box_child (widget))
    return;

  rect = CGRectMake (x, y, width, height);

  context = gdk_quartz_drawable_get_context (GDK_WINDOW_OBJECT (window)->impl, FALSE);
  if (!context)
    return;

  arrow_info.version = 0;
  arrow_info.state = kThemeStateActive;
	switch (arrow_type)
    {
      case GTK_ARROW_UP:
        arrow_info.orientation = kThemeArrowUp;
        break;

      default:
      case GTK_ARROW_DOWN:
        arrow_info.orientation = kThemeArrowDown;
        break;

      case GTK_ARROW_RIGHT:
        arrow_info.orientation = kThemeArrowRight;
        break;

      case GTK_ARROW_LEFT:
        arrow_info.orientation = kThemeArrowLeft;
        break;
   }

  arrow_info.size = kThemeArrow9pt;

  HIThemeDrawPopupArrow (&rect, &arrow_info, context, kHIThemeOrientationNormal);

  gdk_quartz_drawable_release_context (GDK_WINDOW_OBJECT (window)->impl, FALSE);
  return;  
}

static gboolean
is_combo_box_child (GtkWidget *widget)
{
    GtkWidget *tmp;

    for (tmp = widget->parent; tmp; tmp = tmp->parent)
      {
        if (GTK_IS_COMBO_BOX (tmp))
          return TRUE;
      }

    return FALSE;
}

static gboolean
is_tree_view_child (GtkWidget *widget)
{
    GtkWidget *tmp;

    for (tmp = widget->parent; tmp; tmp = tmp->parent)
      {
        if (GTK_IS_TREE_VIEW (tmp))
          return TRUE;
      }

    return FALSE;
}

static gboolean
is_path_bar_button (GtkWidget *widget)
{
    GtkWidget *tmp;

    if (!GTK_IS_BUTTON (widget))
      return FALSE;

    for (tmp = widget->parent; tmp; tmp = tmp->parent)
      {
        if (strcmp (G_OBJECT_TYPE_NAME (tmp), "GtkPathBar") == 0)
          return TRUE;
      }

    return FALSE;
}

/* Checks if the button is displaying just an icon and no text, used to
 * decide whether to show a square or aqua button. FIXME: Implement.
 */
static gboolean
is_icon_only_button (GtkWidget *widget)
{
    if (!GTK_IS_BUTTON (widget))
      return FALSE;

    /* FIXME: implement. */

    return FALSE;
}

static void
draw_box (GtkStyle      *style,
		GdkWindow     *window,
		GtkStateType   state_type,
		GtkShadowType  shadow_type,
		GdkRectangle  *area,
		GtkWidget     *widget,
		const gchar   *detail,
		gint           x,
		gint           y,
		gint           width,
		gint           height)
{
  DEBUG_DRAW;

  if (GTK_IS_BUTTON (widget) && is_tree_view_child (widget))
    {
      /* FIXME: Refactor and share with the rest of the button
       * drawing.
       */

      CGContextRef context;
      HIRect rect;
      HIThemeButtonDrawInfo draw_info;

      draw_info.version = 0;
      draw_info.kind = kThemeListHeaderButton;
      draw_info.adornment = kThemeAdornmentNone;
      draw_info.value = kThemeButtonOff;

      if (state_type == GTK_STATE_ACTIVE)
        draw_info.state = kThemeStatePressed;
      else if (state_type == GTK_STATE_INSENSITIVE)
        draw_info.state = kThemeStateInactive;
      else
        draw_info.state = kThemeStateActive;

      if (GTK_WIDGET_HAS_FOCUS (widget))
        draw_info.adornment |= kThemeAdornmentFocus;

      /* We draw outside the allocation to cover the ugly frame from
       * the treeview.
       */
      rect = CGRectMake (x - 1, y - 1, width + 2, height + 2);

      context = gdk_quartz_drawable_get_context (GDK_WINDOW_OBJECT (window)->impl, FALSE);
      if (!context)
        return;

      HIThemeDrawButton (&rect,
                         &draw_info,
                         context,
                         kHIThemeOrientationNormal,
                         NULL);

      gdk_quartz_drawable_release_context (GDK_WINDOW_OBJECT (window)->impl, context);

      return;
    }
  else if (GTK_IS_TOGGLE_BUTTON (widget) && is_combo_box_child (widget))
    {
      /* FIXME: Support GtkComboBoxEntry too (using kThemeComboBox). */
      CGContextRef context;
      HIRect rect;
      HIThemeButtonDrawInfo draw_info;
      gint line_width;

      draw_info.version = 0;
      draw_info.kind = kThemePopupButton;
      draw_info.adornment = kThemeAdornmentNone;
      draw_info.value = kThemeButtonOff;

      if (state_type == GTK_STATE_ACTIVE)
				draw_info.state = kThemeStatePressed;
      else if (state_type == GTK_STATE_INSENSITIVE)
				draw_info.state = kThemeStateInactive;
      else
				draw_info.state = kThemeStateActive;

      if (GTK_WIDGET_HAS_FOCUS (widget))
				draw_info.adornment |= kThemeAdornmentFocus;

      /* FIXME: We should make the button "greyed out" when the window
       * doesn't have focus, but I can't see how it can be done (it's
       * not the same as insensitive).
       */

      gtk_widget_style_get (widget,
                            "focus-line-width", &line_width,
                            NULL);

      rect = CGRectMake (x + line_width, y + line_width,
                         width - 2 * line_width, height - 2 * line_width - 1);

      context = gdk_quartz_drawable_get_context (GDK_WINDOW_OBJECT (window)->impl, FALSE);
      if (!context)
        return;

      HIThemeDrawButton (&rect,
			 &draw_info,
			 context,
			 kHIThemeOrientationNormal,
			 NULL);

      gdk_quartz_drawable_release_context (GDK_WINDOW_OBJECT (window)->impl, context);

      return;
    }
  else if (IS_DETAIL (detail, "button") || IS_DETAIL (detail, "buttondefault"))
    {
      if (GTK_IS_TREE_VIEW (widget->parent) || GTK_IS_CLIST (widget->parent))
	{
		/* FIXME: refactor so that we can share this code with
		 * normal buttons.
		 */
		CGContextRef context;
		HIRect rect;
		HIThemeButtonDrawInfo draw_info;

		draw_info.version = 0;
		draw_info.kind = kThemeListHeaderButton;
		draw_info.adornment = kThemeAdornmentNone;
		draw_info.value = kThemeButtonOff;

		if (state_type == GTK_STATE_ACTIVE)
			draw_info.state = kThemeStatePressed;
		else if (state_type == GTK_STATE_INSENSITIVE)
			draw_info.state = kThemeStateInactive;
		else
			draw_info.state = kThemeStateActive;

		if (GTK_WIDGET_HAS_FOCUS (widget))
			draw_info.adornment |= kThemeAdornmentFocus;

		if (IS_DETAIL (detail, "buttondefault"))
			draw_info.adornment |= kThemeAdornmentDefault;

		rect = CGRectMake (x, y, width, height);

		context = gdk_quartz_drawable_get_context (GDK_WINDOW_OBJECT (window)->impl, FALSE);
          if (!context)
            return;

		HIThemeDrawButton (&rect,
					 &draw_info,
					 context,
					 kHIThemeOrientationNormal,
					 NULL);

		gdk_quartz_drawable_release_context (GDK_WINDOW_OBJECT (window)->impl, context);

		return;
	}
      else /* Normal button. */
	{
          ThemeButtonKind kind;

          if (is_path_bar_button (widget) || is_icon_only_button (widget))
            {
              kind = kThemeBevelButton;
            }
          else
            {
              gdouble ratio;
              int     max_height = 35; /* FIXME: This should be calculated somehow. */

              /* Use weird heuristics for now... */
              ratio = (gdouble) height / (gdouble) width;
              if (height >= max_height || (ratio > 0.4 && ratio < 1.5))
                kind = kThemeBevelButton;
              else
                kind = kThemePushButtonNormal;
            }

          quartz_draw_button (style, window, state_type, shadow_type,
                              widget, detail,
                              kind,
                              x, y,
                              width, height);
		return;
	}
    }
  else if (IS_DETAIL (detail, "toolbar"))
    {
      /* FIXME: Might want to draw something here, like the window title
       * background... not sure how to do it. It would be good to get the
       * "unified toolbar look if possible.
       */

      HIThemeBackgroundDrawInfo draw_info;
      HIRect rect;
      CGContextRef context;

      draw_info.version = 0;
      draw_info.state = kThemeStateActive;
      draw_info.kind = kThemeBackgroundWindowHeader;

      rect = CGRectMake (x, y, width, height);

      //g_print ("%d %d %d %d\n", x, y, width, height);

      context = gdk_quartz_drawable_get_context (GDK_WINDOW_OBJECT (window)->impl, FALSE);
      if (!context)
        return;

      // this only covers the -color-, not the gradient
      //HIThemeSetFill (kThemeBrushToolbarBackground, NULL, context, kHIThemeOrientationNormal);
      //CGContextFillRect (context, (CGRect) rect);

      //HIThemeApplyBackground (&rect, &draw_info, context, kHIThemeOrientationNormal);
      //HIThemeDrawBackground (&rect, &draw_info, context, kHIThemeOrientationNormal);

      gdk_quartz_drawable_release_context (GDK_WINDOW_OBJECT (window)->impl, context);

      return;
    }
  else if (IS_DETAIL (detail, "menubar"))
    {
		  /* TODO: What about vertical menubars? */
      HIThemeMenuBarDrawInfo draw_info;
			HIThemePlacardDrawInfo bg_draw_info;
      HIRect rect;
			HIRect bg_rect;
      CGContextRef context;

      draw_info.version = 0;
      draw_info.state = kThemeMenuBarNormal;

      bg_draw_info.version = 0;
      bg_draw_info.state = kThemeStateActive;

      /* We paint the Rect with a shift of 10 pixels on both sides to avoid round borders */
			rect = CGRectMake (x-10, y, width+20, 22);

			/* FIXME?: We shift one pixel to avoid the grey border */
			bg_rect = CGRectMake (x-1, y-1, width+2, height+2);

      context = gdk_quartz_drawable_get_context (GDK_WINDOW_OBJECT (window)->impl, FALSE);
      if (!context)
        return;

			/* We fill the whole area with the background since the menubar has a fixed height
			of 22 pixels, screw people with more than one text line in menubars */
			HIThemeDrawPlacard (&bg_rect, &bg_draw_info, context, kHIThemeOrientationNormal);
      HIThemeDrawMenuBarBackground (&rect, &draw_info, context, kHIThemeOrientationNormal);

      gdk_quartz_drawable_release_context (GDK_WINDOW_OBJECT (window)->impl, context);

      return;
    }
  else if (IS_DETAIL (detail, "menu"))
    {
      GtkWidget *toplevel;
      HIThemeMenuDrawInfo draw_info;
      HIRect rect;
      CGContextRef context;

      draw_info.version = 0;
      draw_info.menuType = kThemeMenuTypePopUp;

      toplevel = gtk_widget_get_toplevel (widget);
      gdk_window_get_size (toplevel->window, &width, &height);

      rect = CGRectMake (x, y, width, height);

      context = gdk_quartz_drawable_get_context (GDK_WINDOW_OBJECT (window)->impl, FALSE);
      if (!context)
        return;

      HIThemeDrawMenuBackground (&rect, &draw_info, context, kHIThemeOrientationNormal);

      gdk_quartz_drawable_release_context (GDK_WINDOW_OBJECT (window)->impl, context);

      return;
    }
  else if (IS_DETAIL (detail, "menuitem"))
    {
      CGRect menu_rect, item_rect;
      HIThemeMenuItemDrawInfo draw_info;
      CGContextRef context;
      GtkWidget *toplevel;

      // FIXME: For toplevel menuitems, we should probably use HIThemeDrawMenuTitle().

      draw_info.version = 0;
      draw_info.itemType = kThemeMenuItemPlain;
      // FIXME: We need to OR the type with different values depending
      // on the type (option menu, etc), if it has an icon, etc.
      draw_info.itemType |= kThemeMenuItemPopUpBackground;

      if (state_type == GTK_STATE_INSENSITIVE)
	draw_info.state = kThemeMenuDisabled;
      else if (state_type == GTK_STATE_PRELIGHT)
	draw_info.state = kThemeMenuSelected;
      else
	draw_info.state = kThemeMenuActive;

      item_rect = CGRectMake (x, y, width, height);

      toplevel = gtk_widget_get_toplevel (widget);
      gdk_window_get_size (toplevel->window, &width, &height);
      menu_rect = CGRectMake (0, 0, width, height);

      context = gdk_quartz_drawable_get_context (GDK_WINDOW_OBJECT (window)->impl, FALSE);
      if (!context)
        return;

      HIThemeDrawMenuItem (&menu_rect,
				 &item_rect,
				 &draw_info,
				 context,
				 kHIThemeOrientationNormal,
				 NULL);

      gdk_quartz_drawable_release_context (GDK_WINDOW_OBJECT (window)->impl, context);

      return;
    }
  else if (IS_DETAIL (detail, "spinbutton"))
    {
      CGContextRef context;
      HIRect rect;
      HIThemePlacardDrawInfo placard_info;
      HIThemeButtonDrawInfo draw_info;

      /* Draw the background texture to paint over the bg background
       * that the spinbutton draws.
       */
      placard_info.version = 0;
      placard_info.state = kThemeStateActive;

      rect = CGRectMake (x - 1, y - 1, width + 2, height + 2);

      context = gdk_quartz_drawable_get_context (GDK_WINDOW_OBJECT (window)->impl, FALSE);
      if (!context)
        return;

      HIThemeDrawPlacard (&rect, &placard_info, context, kHIThemeOrientationNormal);

      /* And the arrows... */
      draw_info.version = 0;
      draw_info.kind = kThemeIncDecButton;
      draw_info.adornment = kThemeAdornmentNone;
      draw_info.value = kThemeButtonOff;

      if (state_type == GTK_STATE_INSENSITIVE)
        draw_info.state = kThemeStateInactive;
      else if (GTK_SPIN_BUTTON (widget)->click_child == GTK_ARROW_DOWN)
        draw_info.state = kThemeStatePressedDown;
      else if (GTK_SPIN_BUTTON (widget)->click_child == GTK_ARROW_UP)
        draw_info.state = kThemeStatePressedUp;
      else
        draw_info.state = kThemeStateActive;

      rect = CGRectMake (x-2, y+1, width, height);

      HIThemeDrawButton (&rect,
                         &draw_info,
                         context,
                         kHIThemeOrientationNormal,
                         NULL);

      gdk_quartz_drawable_release_context (GDK_WINDOW_OBJECT (window)->impl, context);

      return;
    }
  else if (IS_DETAIL (detail, "spinbutton_up") || 
           IS_DETAIL (detail, "spinbutton_down"))
    {
      return; /* Ignore. */
    }
  else if (GTK_IS_PROGRESS_BAR (widget) && IS_DETAIL (detail, "trough"))
    {
      CGContextRef context;
      HIRect rect;
      HIThemeTrackDrawInfo draw_info;

      draw_info.version = 0;
      draw_info.reserved = 0;
      draw_info.filler1 = 0;
      draw_info.kind = kThemeLargeProgressBar;

      if (state_type == GTK_STATE_INSENSITIVE)
        draw_info.enableState = kThemeTrackInactive;
      else
        draw_info.enableState = kThemeTrackActive;

      rect = CGRectMake (x, y, width, height);

      draw_info.bounds = rect;
      draw_info.min = 0;
      draw_info.max = 100;
      draw_info.value = gtk_progress_bar_get_fraction (GTK_PROGRESS_BAR (widget)) * 100;
      draw_info.attributes = kThemeTrackHorizontal;
      draw_info.trackInfo.progress.phase = 0; // for indeterminate ones

      /* FIXME: This is really weird, might be a bg in the backend. We
       * have to flip the context while drawing the progess bar,
       * otherwise it ends up with a strange offset. 
       */

      if (GDK_IS_PIXMAP (window))
        {
          context = gdk_quartz_drawable_get_context (GDK_PIXMAP_OBJECT (window)->impl, FALSE);
          if (!context)
            return;

          CGContextSaveGState (context);
          CGContextTranslateCTM (context, 0, CGBitmapContextGetHeight (context));
          CGContextScaleCTM (context, 1.0, -1.0);
        }
      else
        {
          context = gdk_quartz_drawable_get_context (GDK_WINDOW_OBJECT (window)->impl, FALSE);
          if (!context)
            return;
        }

      HIThemeDrawTrack (&draw_info, NULL, context, kHIThemeOrientationNormal);

      if (GDK_IS_PIXMAP (window))
        {
          CGContextRestoreGState (context);
          gdk_quartz_drawable_release_context (GDK_PIXMAP_OBJECT (window)->impl, context);
        }
      else
        gdk_quartz_drawable_release_context (GDK_WINDOW_OBJECT (window)->impl, context);

      return;
    }
  else if (GTK_IS_PROGRESS_BAR (widget) && IS_DETAIL (detail, "bar"))
    {
      return; /* Ignore. */
    }
  else if (GTK_IS_SCALE (widget) && IS_DETAIL (detail, "trough"))
    {
      CGContextRef context;
      HIRect rect;
      HIThemeTrackDrawInfo draw_info;
      GtkAdjustment *adj;

      draw_info.version = 0;
      draw_info.reserved = 0;
      draw_info.filler1 = 0;
      draw_info.kind = kThemeSlider;

      if (state_type == GTK_STATE_INSENSITIVE)
        draw_info.enableState = kThemeTrackInactive;
      else
        draw_info.enableState = kThemeTrackActive;

      rect = CGRectMake (x, y, width, height);

      draw_info.bounds = rect;

      adj = gtk_range_get_adjustment (GTK_RANGE (widget));
      
      draw_info.min = adj->lower;
      draw_info.max = adj->upper;
      draw_info.value = adj->value;

      draw_info.attributes = kThemeTrackShowThumb | kThemeTrackThumbRgnIsNotGhost;

      if (GTK_IS_HSCALE (widget))
        draw_info.attributes |= kThemeTrackHorizontal;

      if (GTK_WIDGET_HAS_FOCUS (widget))
        draw_info.attributes |= kThemeTrackHasFocus;

      draw_info.trackInfo.slider.thumbDir = kThemeThumbPlain;

      context = gdk_quartz_drawable_get_context (GDK_WINDOW_OBJECT (window)->impl, FALSE);
      if (!context)
        return;

      HIThemeDrawTrack (&draw_info, NULL, context, kHIThemeOrientationNormal);

      gdk_quartz_drawable_release_context (GDK_WINDOW_OBJECT (window)->impl, context);

      return;
    }
  else if (GTK_IS_SCALE (widget))
    {
      return; /* Ignore. */
    }
  else if (GTK_IS_SCROLLBAR (widget) && IS_DETAIL (detail, "trough"))
    {
      CGContextRef context;
      HIRect rect;
      HIThemeTrackDrawInfo draw_info;
      GtkAdjustment *adj;
      gint view_size;

      draw_info.version = 0;
      draw_info.reserved = 0;
      draw_info.filler1 = 0;
      draw_info.kind = kThemeScrollBarMedium;

      if (state_type == GTK_STATE_INSENSITIVE)
        draw_info.enableState = kThemeTrackInactive;
      else
        draw_info.enableState = kThemeTrackActive;

      rect = CGRectMake (x, y, width, height);

      draw_info.bounds = rect;

      adj = gtk_range_get_adjustment (GTK_RANGE (widget));

      /* NOTE: view_size != page_size, see:
       * http://lists.apple.com/archives/Carbon-development/2002/Sep/msg01922.html
       */
      view_size = (adj->page_size / (adj->upper - adj->lower) * INT_MAX);

      /* Hackery needed because min, max, and value only accept integers. */
      draw_info.min = 0;
      draw_info.max = INT_MAX - view_size;
      draw_info.value = ((adj->value - adj->lower) / adj->upper) * INT_MAX;

      draw_info.trackInfo.scrollbar.viewsize = view_size;

      draw_info.attributes = kThemeTrackShowThumb | kThemeTrackThumbRgnIsNotGhost;

      if (GTK_IS_HSCROLLBAR (widget))
        draw_info.attributes |= kThemeTrackHorizontal;

      if (GTK_WIDGET_HAS_FOCUS (widget))
        draw_info.attributes |= kThemeTrackHasFocus;

      //draw_info.trackInfo.slider.thumbDir = kThemeThumbPlain;

      context = gdk_quartz_drawable_get_context (GDK_WINDOW_OBJECT (window)->impl, FALSE);
      if (!context)
        return;

      HIThemeDrawTrack (&draw_info, NULL, context, kHIThemeOrientationNormal);

      gdk_quartz_drawable_release_context (GDK_WINDOW_OBJECT (window)->impl, context);

      return;
    }
  else if (GTK_IS_SCROLLBAR (widget))
    {
      return; /* Ignore. */
    }

  const gchar *name = gtk_widget_get_name (widget);
  if (name && strcmp (name, "gtk-tooltips") == 0)
    {
      /* FIXME: draw tooltips. */
    }

  parent_class->draw_box (style, window, state_type, shadow_type, area,
				widget, detail, x, y, width, height);
}

static void
draw_check (GtkStyle      *style,
			GdkWindow     *window,
			GtkStateType   state_type,
			GtkShadowType  shadow_type,
			GdkRectangle  *area,
			GtkWidget     *widget,
			const gchar   *detail,
			gint           x,
			gint           y,
			gint           width,
			gint           height)
{
  DEBUG_DRAW;

  /* FIXME: Refactor and share with the other button drawing
   * functions, and radiobuttons.
   */
  if (IS_DETAIL (detail, "checkbutton"))
    {
      CGContextRef context;
      HIRect rect;
      HIThemeButtonDrawInfo draw_info;

      draw_info.version = 0;
      draw_info.kind = kThemeCheckBox;
      draw_info.adornment = kThemeAdornmentNone;

      // FIXME: might want this? kThemeAdornmentDrawIndicatorOnly

      if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget)))
	draw_info.value = kThemeButtonOn;
      else
	draw_info.value = kThemeButtonOff;

      if (state_type == GTK_STATE_ACTIVE)
	draw_info.state = kThemeStatePressed;
      else if (state_type == GTK_STATE_INSENSITIVE)
	draw_info.state = kThemeStateInactive;
      else
	draw_info.state = kThemeStateActive;

      if (GTK_WIDGET_HAS_FOCUS (widget))
	draw_info.adornment |= kThemeAdornmentFocus;

      if (IS_DETAIL (detail, "buttondefault"))
	draw_info.adornment |= kThemeAdornmentDefault;

      rect = CGRectMake (x, y+1, width, height);

      context = gdk_quartz_drawable_get_context (GDK_WINDOW_OBJECT (window)->impl, FALSE);
      if (!context)
        return;

      HIThemeDrawButton (&rect,
			 &draw_info,
			 context,
			 kHIThemeOrientationNormal,
			 NULL);

      gdk_quartz_drawable_release_context (GDK_WINDOW_OBJECT (window)->impl, context);

      return;
    }
}

static void
draw_option (GtkStyle      *style,
			 GdkWindow     *window,
			 GtkStateType   state_type,
			 GtkShadowType  shadow,
			 GdkRectangle  *area,
			 GtkWidget     *widget,
			 const gchar   *detail,
			 gint           x,
			 gint           y,
			 gint           width,
			 gint           height)
{
  DEBUG_DRAW;

  /* FIXME: Refactor and share with the other button drawing
   * functions, and radiobuttons.
   */
  if (IS_DETAIL (detail, "radiobutton"))
    {
      CGContextRef context;
      HIRect rect;
      HIThemeButtonDrawInfo draw_info;

      draw_info.version = 0;
      draw_info.kind = kThemeRadioButton;
      draw_info.adornment = kThemeAdornmentNone;

      if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget)))
	draw_info.value = kThemeButtonOn;
      else
	draw_info.value = kThemeButtonOff;

      if (state_type == GTK_STATE_ACTIVE)
	draw_info.state = kThemeStatePressed;
      else if (state_type == GTK_STATE_INSENSITIVE)
	draw_info.state = kThemeStateInactive;
      else
	draw_info.state = kThemeStateActive;

      if (GTK_WIDGET_HAS_FOCUS (widget))
	draw_info.adornment |= kThemeAdornmentFocus;

      if (IS_DETAIL (detail, "buttondefault"))
	draw_info.adornment |= kThemeAdornmentDefault;

      rect = CGRectMake (x, y, width, height);

      context = gdk_quartz_drawable_get_context (GDK_WINDOW_OBJECT (window)->impl, FALSE);
      if (!context)
        return;

      HIThemeDrawButton (&rect,
			 &draw_info,
			 context,
			 kHIThemeOrientationNormal,
			 NULL);

      gdk_quartz_drawable_release_context (GDK_WINDOW_OBJECT (window)->impl, context);

      return;
    }
}

static void
draw_tab (GtkStyle      *style,
		GdkWindow     *window,
		GtkStateType   state,
		GtkShadowType  shadow,
		GdkRectangle  *area,
		GtkWidget     *widget,
		const gchar   *detail,
		gint           x,
		gint           y,
		gint           width,
		gint           height)
{
  DEBUG_DRAW;
 return;
}

static void
draw_extension (GtkStyle        *style,
		GdkWindow       *window,
		GtkStateType     state_type,
		GtkShadowType    shadow_type,
		GdkRectangle    *area,
		GtkWidget       *widget,
		const gchar     *detail,
		gint             x,
		gint             y,
		gint             width,
		gint             height,
		GtkPositionType  gap_side)
{
  DEBUG_DRAW;
  if (widget
      && GTK_IS_NOTEBOOK (widget)
			&& detail
			&& !strcmp (detail, "tab"))
    {
      HIRect rect, out_rect;
      HIThemeTabDrawInfo draw_info;
			CGContextRef context;
	  /* bool first, last;
	     gint border_width; */

		if (height > 22 &&
				(gap_side == GTK_POS_RIGHT || gap_side == GTK_POS_LEFT))
			rect = CGRectMake (x, y + height/2 - 22/2, width, height);
	  else
			rect = CGRectMake (x, y, width, height);

	  context = gdk_quartz_drawable_get_context (GDK_WINDOW_OBJECT (window)->impl, FALSE);
	  if (!context)
        return;

		draw_info.version = 1;
		draw_info.direction = kThemeTabNorth;
		draw_info.size = kHIThemeTabSizeNormal;
		draw_info.adornment = kHIThemeTabAdornmentNone;
		draw_info.kind = kHIThemeTabKindNormal;

		if (state_type == GTK_STATE_ACTIVE)
			draw_info.style = kThemeTabNonFront;
	  else if (state_type == GTK_STATE_INSENSITIVE)
		  draw_info.style = kThemeTabNonFrontInactive;
		else
			draw_info.style = kThemeTabFront;
		

		/* TODO: figure out the last one and take thickness into account

		border_width = gtk_container_get_border_width (GTK_CONTAINER (widget));
	  first = widget->allocation.x == x + border_width;
	  last = FALSE;

	  if (first && last)
			  draw_info.position = kHIThemeTabPositionFirst;
	  else if (first)
			  draw_info.position = kHIThemeTabPositionFirst;
	  else if (last)
			  draw_info.position = kHIThemeTabPositionLast;
	  else
			  draw_info.position = kHIThemeTabPositionMiddle;*/

	  draw_info.position = kHIThemeTabPositionOnly;


		HIThemeDrawTab (&rect,
                    &draw_info,
										context,
											kHIThemeOrientationNormal,
											&out_rect);
		gdk_quartz_drawable_release_context (GDK_WINDOW_OBJECT (window)->impl, context);

	}

  return;
  parent_class->draw_extension (style, window, state_type,
				shadow_type, area, widget, detail,
				x, y, width, height, gap_side);
}

static void
draw_box_gap (GtkStyle * style, GdkWindow * window, GtkStateType state_type,
				GtkShadowType shadow_type, GdkRectangle * area,
				GtkWidget * widget, const gchar * detail, gint x,
				gint y, gint width, gint height, GtkPositionType gap_side,
				gint gap_x, gint gap_width)
{
  DEBUG_DRAW;

  return;
  parent_class->draw_box_gap (style, window, state_type, shadow_type,
						area, widget, detail, x, y, width, height,
						gap_side, gap_x, gap_width);
}

static void
draw_flat_box (GtkStyle      *style,
				 GdkWindow     *window,
				 GtkStateType   state_type,
				 GtkShadowType  shadow_type,
				 GdkRectangle  *area,
				 GtkWidget     *widget,
				 const gchar   *detail,
				 gint           x,
				 gint           y,
				 gint           width,
				 gint           height)
{
  DEBUG_DRAW;

  if (IS_DETAIL (detail, "base") ||
      IS_DETAIL (detail, "viewportbin") ||
      IS_DETAIL (detail, "eventbox"))
    {
      HIThemePlacardDrawInfo draw_info;
      HIRect rect;
      CGContextRef context;

      gdk_window_get_size (window, &width, &height);

      /* The background is not drawn to include the corners if we don't
       * pad with one pixel
       */
      rect = CGRectMake (-1, -1, width+2, height+2);

      draw_info.version = 0;
      draw_info.state = kThemeStateActive;

      context = gdk_quartz_drawable_get_context (GDK_WINDOW_OBJECT (window)->impl, FALSE);
      if (!context)
        return;

      /* Is this really the right method? It seems to work though. */
      HIThemeDrawPlacard (&rect, &draw_info, context, kHIThemeOrientationNormal);

      gdk_quartz_drawable_release_context (GDK_WINDOW_OBJECT (window)->impl, context);

      return;
    }
  else if (IS_DETAIL (detail, "entry_bg"))
    {
      CGContextRef context;
      HIRect rect;
      HIThemeFrameDrawInfo draw_info;
      gint line_width;

      draw_info.version = 0;
      draw_info.kind = kHIThemeFrameTextFieldSquare;
      if (state_type == GTK_STATE_INSENSITIVE)
	draw_info.state = kThemeStateInactive;
      else
	draw_info.state = kThemeStateActive;
      draw_info.isFocused = GTK_WIDGET_HAS_FOCUS (widget);

      gtk_widget_style_get (widget,
                            "focus-line-width", &line_width,
                            NULL);

      rect = CGRectMake (x + line_width, y + line_width,
                         width - 2 * line_width, height - 2 * line_width);

      context = gdk_quartz_drawable_get_context (GDK_WINDOW_OBJECT (window)->impl, FALSE);
      if (!context)
        return;

      HIThemeDrawFrame (&rect,
			&draw_info,
			context,
			kHIThemeOrientationNormal);

      gdk_quartz_drawable_release_context (GDK_WINDOW_OBJECT (window)->impl, context);

      return;
    }
  else if (GTK_IS_PROGRESS_BAR (widget) && IS_DETAIL (detail, "trough"))
    {
      CGContextRef context;
      HIRect rect;
      HIThemePlacardDrawInfo placard_info;

      /* Draw the background texture to paint over the black bg that
       * the progressbar draws.
       */
      placard_info.version = 0;
      placard_info.state = kThemeStateActive;

      gdk_drawable_get_size (window, &width, &height);

      rect = CGRectMake (x - 2, y - 1, width + 4, height + 4);

      if (GDK_IS_PIXMAP (window))
        context = gdk_quartz_drawable_get_context (GDK_PIXMAP_OBJECT (window)->impl, FALSE);
      else
        context = gdk_quartz_drawable_get_context (GDK_WINDOW_OBJECT (window)->impl, FALSE);

      if (!context)
        return;

      HIThemeDrawPlacard (&rect, &placard_info, context, kHIThemeOrientationNormal);

      if (GDK_IS_PIXMAP (window))
        gdk_quartz_drawable_release_context (GDK_PIXMAP_OBJECT (window)->impl, context);
      else
        gdk_quartz_drawable_release_context (GDK_WINDOW_OBJECT (window)->impl, context);

      return;
    }
  else if (IS_DETAIL (detail, "checkbutton"))
    {
      /* We don't want any background, no prelight etc. */
      return;
    }
  else if (IS_DETAIL (detail, "cell_even"))
    {
#if 0
      CGContextRef context;
      HIRect rect;

      rect = CGRectMake (x, y, width, height);

      context = gdk_quartz_drawable_get_context (GDK_WINDOW_OBJECT (window)->impl, FALSE);
      if (!context)
        return;

      /* FIXME: Draw... */

      gdk_quartz_drawable_release_context (GDK_WINDOW_OBJECT (window)->impl, context);

      return;
#endif
    }

  parent_class->draw_flat_box (style, window, state_type, shadow_type,
						 area, widget, detail, x, y, width, height);
}

static void
draw_expander (GtkStyle         *style,
				 GdkWindow        *window,
				 GtkStateType      state,
				 GdkRectangle     *area,
				 GtkWidget        *widget,
				 const gchar      *detail,
				 gint              x,
				 gint              y,
				 GtkExpanderStyle  expander_style)
{
  DEBUG_DRAW;

  parent_class->draw_expander (style, window, state, area, widget,
						 detail, x, y, expander_style);
}

static void
draw_shadow (GtkStyle      *style,
			 GdkWindow     *window,
			 GtkStateType   state_type,
			 GtkShadowType  shadow_type,
			 GdkRectangle  *area,
			 GtkWidget     *widget,
			 const gchar   *detail,
			 gint           x,
			 gint           y,
			 gint           width,
			 gint           height)
{
  GtkWidget *child = NULL;

  DEBUG_DRAW;

  if (GTK_IS_BIN (widget))
    child = gtk_bin_get_child (GTK_BIN (widget));
    
  /* Scrolled window with treeview in it. */
  if (IS_DETAIL (detail, "scrolled_window") && GTK_IS_TREE_VIEW (child))
    {
      CGContextRef context;
      HIRect rect;
      HIThemeFrameDrawInfo draw_info;

      draw_info.version = 0;
      draw_info.kind = kHIThemeFrameListBox;
      if (state_type == GTK_STATE_INSENSITIVE)
	draw_info.state = kThemeStateInactive;
      else
	draw_info.state = kThemeStateActive;
      draw_info.isFocused = GTK_WIDGET_HAS_FOCUS (widget);

      rect = CGRectMake (x, y, width, height);

      context = gdk_quartz_drawable_get_context (GDK_WINDOW_OBJECT (window)->impl, FALSE);
      if (!context)
        return;

      HIThemeDrawFrame (&rect,
			&draw_info,
			context,
			kHIThemeOrientationNormal);

      gdk_quartz_drawable_release_context (GDK_WINDOW_OBJECT (window)->impl, context);

      return;
    }
  /* Scrolled window with text view in it. */
  else if (IS_DETAIL (detail, "scrolled_window") && GTK_IS_TEXT_VIEW (child))
    {
      CGContextRef context;
      HIRect rect;
      HIThemeFrameDrawInfo draw_info;

      draw_info.version = 0;
      draw_info.kind = kHIThemeFrameListBox;
      if (state_type == GTK_STATE_INSENSITIVE)
	draw_info.state = kThemeStateInactive;
      else
	draw_info.state = kThemeStateActive;
      draw_info.isFocused = GTK_WIDGET_HAS_FOCUS (widget);

      rect = CGRectMake (x, y, width, height);

      context = gdk_quartz_drawable_get_context (GDK_WINDOW_OBJECT (window)->impl, FALSE);
      if (!context)
        return;

      HIThemeDrawFrame (&rect,
			&draw_info,
			context,
			kHIThemeOrientationNormal);

      gdk_quartz_drawable_release_context (GDK_WINDOW_OBJECT (window)->impl, context);

      return;
    }
  else if (IS_DETAIL (detail, "entry"))
    {
      CGContextRef context;
      HIRect rect;
      HIThemePlacardDrawInfo placard_info;

      /* Draw the background texture to paint over the white base
       * background that the entry draws.
       */

      placard_info.version = 0;
      placard_info.state = kThemeStateActive;

      rect = CGRectMake (x - 1, y - 1, width + 2, height + 2);

      context = gdk_quartz_drawable_get_context (GDK_WINDOW_OBJECT (window)->impl, FALSE);
      if (!context)
        return;

      HIThemeDrawPlacard (&rect, &placard_info, context, kHIThemeOrientationNormal);

      gdk_quartz_drawable_release_context (GDK_WINDOW_OBJECT (window)->impl, context);

      return;
    }
  else if (GTK_IS_SCALE (widget))
    return; /* Ignore. */

  return;
  g_print ("Missing implementation of draw_shadow for %s\n", detail);
  parent_class->draw_shadow (style, window, state_type, shadow_type, area,
					 widget, detail, x, y, width, height);
}

static void
draw_shadow_gap (GtkStyle        *style,
		 GdkWindow       *window,
		 GtkStateType     state_type,
		 GtkShadowType    shadow_type,
		 GdkRectangle    *area,
		 GtkWidget       *widget,
		 const gchar     *detail,
		 gint             x,
		 gint             y,
		 gint             width,
		 gint             height,
		 GtkPositionType  gap_side,
		 gint             gap_x,
		 gint             gap_width)
{
  DEBUG_DRAW;

  return;
  g_print ("Missing implementation of draw_shadow_gap for %s\n", detail);
  parent_class->draw_shadow_gap (style, window, state_type, shadow_type, area,
                                 widget, detail, x, y, width, height,
                                 gap_side, gap_x, gap_width);
}

static void
draw_hline (GtkStyle     *style,
			GdkWindow    *window,
			GtkStateType  state_type,
			GdkRectangle *area,
			GtkWidget    *widget,
			const gchar  *detail,
			gint          x1,
			gint          x2,
			gint          y)
{
  DEBUG_DRAW;

  if (IS_DETAIL (detail, "menuitem"))
    {
      CGRect menu_rect, item_rect;
      HIThemeMenuItemDrawInfo draw_info;
      CGContextRef context;
      GtkWidget *toplevel;
      gint width, height;

      // FIXME: refactor out and share with draw_box::menuitem

      draw_info.version = 0;
      draw_info.itemType = kThemeMenuItemPlain;
      draw_info.itemType |= kThemeMenuItemPopUpBackground;

      if (state_type == GTK_STATE_INSENSITIVE)
	draw_info.state = kThemeMenuDisabled;
      else if (state_type == GTK_STATE_PRELIGHT)
	draw_info.state = kThemeMenuSelected;
      else
	draw_info.state = kThemeMenuActive;

      item_rect = CGRectMake (x1, y, x2-x1, height);

      toplevel = gtk_widget_get_toplevel (widget);
      gdk_window_get_size (toplevel->window, &width, &height);
      menu_rect = CGRectMake (0, 0, width, height);

      context = gdk_quartz_drawable_get_context (GDK_WINDOW_OBJECT (window)->impl, FALSE);
      if (!context)
        return;

      HIThemeDrawMenuSeparator (&menu_rect,
                                &item_rect,
                                &draw_info,
                                context,
                                kHIThemeOrientationNormal);

      gdk_quartz_drawable_release_context (GDK_WINDOW_OBJECT (window)->impl, context);

      return;
    }
}

static void
draw_vline (GtkStyle     *style,
			GdkWindow    *window,
			GtkStateType  state_type,
			GdkRectangle *area,
			GtkWidget    *widget,
			const gchar  *detail,
			gint          y1,
			gint          y2,
			gint          x)
{
  DEBUG_DRAW;
}

static void
draw_slider (GtkStyle       *style,
			 GdkWindow      *window,
			 GtkStateType    state_type,
			 GtkShadowType   shadow_type,
			 GdkRectangle   *area,
			 GtkWidget      *widget,
			 const gchar    *detail,
			 gint            x,
			 gint            y,
			 gint            width,
			 gint            height,
			 GtkOrientation  orientation)
{
  DEBUG_DRAW;

  if (0  && GTK_IS_SCROLLBAR (widget))
    {
      return; /* Ignore. */
    }

  /*  parent_class->draw_slider (style, window, state_type, shadow_type, area,
					 widget, detail, x, y, width, height,
					 orientation);
  */
}

static void
draw_resize_grip (GtkStyle      *style,
			GdkWindow     *window,
			GtkStateType   state_type,
			GdkRectangle  *area,
			GtkWidget     *widget,
			const gchar   *detail,
			GdkWindowEdge  edge,
			gint           x,
			gint           y,
			gint           width,
			gint           height)
{
  DEBUG_DRAW;

  /*  parent_class->draw_resize_grip (style, window, state_type, area,
					widget, detail, edge, x, y, width,
					height);
  */
}

static void
draw_handle (GtkStyle       *style,
			 GdkWindow      *window,
			 GtkStateType    state_type,
			 GtkShadowType   shadow_type,
			 GdkRectangle   *area,
			 GtkWidget      *widget,
			 const gchar    *detail,
			 gint            x,
			 gint            y,
			 gint            width,
			 gint            height,
			 GtkOrientation  orientation)
{
  DEBUG_DRAW;
}

static void
draw_focus (GtkStyle     *style,
			GdkWindow    *window,
			GtkStateType  state_type,
            GdkRectangle *area,
			GtkWidget    *widget,
			const gchar  *detail,
            gint          x,
			gint          y,
			gint          width,
			gint          height)
{
  DEBUG_DRAW;

#if 0
  CGRect rect;
  CGContext context;

  context = gdk_quartz_drawable_get_context (GDK_WINDOW_OBJECT (window)->impl, FALSE);
  if (!context)
    return;

  rect = CGRectMake (x, y, width, height);

  HIThemeDrawFocusRect (&rect, TRUE, context, kHIThemeOrientationNormal);

 gdk_quartz_drawable_release_context (GDK_WINDOW_OBJECT (window)->impl, context);
#endif
}

static void
draw_layout (GtkStyle        *style,
             GdkWindow       *window,
             GtkStateType     state_type,
             gboolean         use_text,
             GdkRectangle    *area,
             GtkWidget       *widget,
             const gchar     *detail,
             gint             x,
             gint             y,
             PangoLayout     *layout)
{
  DEBUG_DRAW;

  if (state_type == GTK_STATE_PRELIGHT && 
      GTK_IS_PROGRESS_BAR (widget) && IS_DETAIL (detail, "progressbar"))
    {
      /* Ignore this, it looks very out of place on the mac (I think
       * the intention is to make the text clearly visible
       * independetly of the backgroun).
       */
      return;
    }

  parent_class->draw_layout (style, window, state_type, use_text,
                             NULL, NULL, NULL,
                             x, y, layout);
}

static void
quartz_style_init_from_rc (GtkStyle * style, GtkRcStyle * rc_style)
{
    style_setup_system_font (style);

    parent_class->init_from_rc (style, rc_style);
}

static void
quartz_style_realize (GtkStyle *style)
{
  GdkGCValues gc_values;
  GdkGCValuesMask gc_values_mask;
  gint i;

  style->black.red = 0x0000;
  style->black.green = 0x0000;
  style->black.blue = 0x0000;
  gdk_colormap_alloc_color (style->colormap, &style->black, FALSE, TRUE);

  style->white.red = 0xffff;
  style->white.green = 0xffff;
  style->white.blue = 0xffff;
  gdk_colormap_alloc_color (style->colormap, &style->white, FALSE, TRUE);

  gc_values_mask = GDK_GC_FOREGROUND | GDK_GC_BACKGROUND;

  gc_values.foreground = style->black;
  gc_values.background = style->white;
  style->black_gc = gtk_gc_get (style->depth, style->colormap, &gc_values, gc_values_mask);

  gc_values.foreground = style->white;
  gc_values.background = style->black;
  style->white_gc = gtk_gc_get (style->depth, style->colormap, &gc_values, gc_values_mask);

  gc_values_mask = GDK_GC_FOREGROUND;

  for (i = 0; i < 5; i++)
    {
      /* FIXME: Implement bg pixmaps? */
      if (style->rc_style && style->rc_style->bg_pixmap_name[i])
	style->bg_pixmap[i] = (GdkPixmap *) GDK_PARENT_RELATIVE;

      gdk_colormap_alloc_color (style->colormap, &style->fg[i], FALSE, TRUE);
      gdk_colormap_alloc_color (style->colormap, &style->bg[i], FALSE, TRUE);
      gdk_colormap_alloc_color (style->colormap, &style->light[i], FALSE, TRUE);
      gdk_colormap_alloc_color (style->colormap, &style->dark[i], FALSE, TRUE);
      gdk_colormap_alloc_color (style->colormap, &style->mid[i], FALSE, TRUE);
      gdk_colormap_alloc_color (style->colormap, &style->text[i], FALSE, TRUE);
      gdk_colormap_alloc_color (style->colormap, &style->base[i], FALSE, TRUE);
      gdk_colormap_alloc_color (style->colormap, &style->text_aa[i], FALSE, TRUE);

      gc_values.foreground = style->fg[i];
      style->fg_gc[i] = gtk_gc_get (style->depth, style->colormap, &gc_values, gc_values_mask);

      gc_values.foreground = style->bg[i];
      style->bg_gc[i] = gtk_gc_get (style->depth, style->colormap, &gc_values, gc_values_mask);

      gc_values.foreground = style->light[i];
      style->light_gc[i] = gtk_gc_get (style->depth, style->colormap, &gc_values, gc_values_mask);

      gc_values.foreground = style->dark[i];
      style->dark_gc[i] = gtk_gc_get (style->depth, style->colormap, &gc_values, gc_values_mask);

      gc_values.foreground = style->mid[i];
      style->mid_gc[i] = gtk_gc_get (style->depth, style->colormap, &gc_values, gc_values_mask);

      gc_values.foreground = style->text[i];
      style->text_gc[i] = gtk_gc_get (style->depth, style->colormap, &gc_values, gc_values_mask);

      gc_values.foreground = style->base[i];
      style->base_gc[i] = gtk_gc_get (style->depth, style->colormap, &gc_values, gc_values_mask);

      gc_values.foreground = style->text_aa[i];
      style->text_aa_gc[i] = gtk_gc_get (style->depth, style->colormap, &gc_values, gc_values_mask);
    }
}

static void
quartz_style_unrealize (GtkStyle * style)
{
	parent_class->unrealize (style);
}

static void
quartz_style_class_init (QuartzStyleClass * klass)
{
  GtkStyleClass *style_class = GTK_STYLE_CLASS (klass);

  parent_class = g_type_class_peek_parent (klass);

  style_class->draw_arrow = draw_arrow;
  style_class->draw_box = draw_box;
  style_class->draw_check = draw_check;
  style_class->draw_option = draw_option;
  style_class->draw_tab = draw_tab;
  style_class->draw_flat_box = draw_flat_box;
  style_class->draw_expander = draw_expander;
  style_class->draw_extension = draw_extension;
  style_class->draw_box_gap = draw_box_gap;
  style_class->draw_shadow = draw_shadow;
  style_class->draw_shadow_gap = draw_shadow_gap;
  style_class->draw_hline = draw_hline;
  style_class->draw_vline = draw_vline;
  style_class->draw_handle = draw_handle;
  style_class->draw_focus = draw_focus;
  style_class->draw_resize_grip = draw_resize_grip;
  style_class->draw_slider = draw_slider;
  style_class->draw_layout = draw_layout;

  style_class->init_from_rc = quartz_style_init_from_rc;
  style_class->realize = quartz_style_realize;
  style_class->unrealize = quartz_style_unrealize;
}

GType quartz_type_style = 0;

void
quartz_style_register_type (GTypeModule * module)
{
    static const GTypeInfo object_info = {
	sizeof (QuartzStyleClass),
	(GBaseInitFunc) NULL,
	(GBaseFinalizeFunc) NULL,
	(GClassInitFunc) quartz_style_class_init,
	NULL,			/* class_finalize */
	NULL,			/* class_data */
	sizeof (QuartzStyle),
	0,			/* n_preallocs */
	(GInstanceInitFunc) NULL,
    };

    quartz_type_style = g_type_module_register_type (module,
								 GTK_TYPE_STYLE,
								 "QuartzStyle",
								 &object_info, 0);
}

void
quartz_style_init (void)
{
  style_setup_settings ();
  style_setup_rc_styles ();
}
