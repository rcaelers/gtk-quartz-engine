/* GTK+ theme engine for the Quartz backend
 *
 * Copyright (C) 2007-2008 Imendio AB
 *
 * This library is free software; you can redistribute it and/or
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
#include <gtk/gtk.h>
#include <Carbon/Carbon.h>

#define IS_DETAIL(d,x) (d && strcmp (d, x) == 0)

/* FIXME: Fix GTK+ to export those in a quartz header file. */
CGContextRef gdk_quartz_drawable_get_context     (GdkDrawable  *drawable,
                                                  gboolean      antialias);
void         gdk_quartz_drawable_release_context (GdkDrawable  *drawable,
                                                  CGContextRef  context);

static void
quartz_measure_button (HIThemeButtonDrawInfo *draw_info,
                       gint                   width,
                       gint                   height)
{
  HIRect in_rect, out_rect;
  HIShapeRef shape;

  in_rect.origin.x = 0;
  in_rect.origin.y = 0;
  in_rect.size.width = width;
  in_rect.size.height = height;

  HIThemeGetButtonShape (&in_rect, draw_info, &shape);
  HIShapeGetBounds (shape, &out_rect);
  g_print ("Shape: %d %d, %d %d  ",
           (int) out_rect.origin.x,
           (int) out_rect.origin.y,
           (int) out_rect.size.width - (int) out_rect.origin.x,
           (int) out_rect.size.height - (int) out_rect.origin.y);

  HIThemeGetButtonBackgroundBounds (&in_rect, draw_info, &out_rect);
  g_print ("Bounds: %d %d, %d %d  ",
           (int) out_rect.origin.x,
           (int) out_rect.origin.y,
           (int) out_rect.size.width - (int) out_rect.origin.x,
           (int) out_rect.size.height - (int) out_rect.origin.y);

  HIThemeGetButtonContentBounds (&in_rect, draw_info, &out_rect);
  g_print ("Content: %d %d, %d %d\n",
           (int) out_rect.origin.x,
           (int) out_rect.origin.y,
           (int) out_rect.size.width - (int) out_rect.origin.x,
           (int) out_rect.size.height - (int) out_rect.origin.y);
}


void
quartz_draw_button (GtkStyle        *style,
                    GdkWindow       *window,
                    GtkStateType     state_type,
                    GtkShadowType    shadow_type,
                    GtkWidget       *widget,
                    const gchar     *detail,
                    ThemeButtonKind  kind,
                    gint             x,
                    gint             y,
                    gint             width,
                    gint             height)
{
  CGContextRef context;
  HIRect rect;
  HIThemeButtonDrawInfo draw_info;
  gint line_width;

  draw_info.version = 0;
  draw_info.kind = kind;
  draw_info.adornment = kThemeAdornmentNone;
  draw_info.value = kThemeButtonOff;

  if (GTK_IS_TOGGLE_BUTTON (widget) &&
      gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget)))
    {
      if (kind == kThemeBevelButton)
        draw_info.kind = kThemeBevelButtonInset;
      else
        draw_info.kind = kThemePushButtonInset;

      //draw_info.value = kThemeButtonOn;
    }

  if (state_type == GTK_STATE_ACTIVE ||
      (GTK_IS_TOGGLE_BUTTON (widget) &&
       gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget))))
    draw_info.state = kThemeStatePressed;
  else if (state_type == GTK_STATE_INSENSITIVE)
    draw_info.state = kThemeStateInactive;
  else
    draw_info.state = kThemeStateActive;

  if (GTK_WIDGET_HAS_FOCUS (widget))
    draw_info.adornment |= kThemeAdornmentFocus;

  if (IS_DETAIL (detail, "buttondefault"))
    draw_info.adornment |= kThemeAdornmentDefault;

  /* FIXME: Emulate default button pulsing. */

  gtk_widget_style_get (widget,
                        "focus-line-width", &line_width,
                        NULL);

  rect = CGRectMake (x + line_width, y + line_width,
                     width - 2 * line_width, height - 2 * line_width - 1);

  if (0)
    {
      g_print ("%d %d ", width, height);
      quartz_measure_button (&draw_info, rect.size.width, rect.size.height);
      //g_print ("%d %d\n", (int) rect.size.width, (int) rect.size.height);
    }

  context = gdk_quartz_drawable_get_context (GDK_WINDOW_OBJECT (window)->impl, FALSE);
  if (!context)
    return;

  HIThemeDrawButton (&rect,
                     &draw_info,
                     context,
                     kHIThemeOrientationNormal,
                     NULL);

  gdk_quartz_drawable_release_context (GDK_WINDOW_OBJECT (window)->impl, context);
}

