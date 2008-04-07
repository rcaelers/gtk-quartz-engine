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

#ifndef QUARTZ_STYLE_H
#define QUARTZ_STYLE_H

#include <gtk/gtkstyle.h>

typedef struct _QuartzStyle QuartzStyle;
typedef struct _QuartzStyleClass QuartzStyleClass;

extern GType quartz_type_style;

#define QUARTZ_TYPE_STYLE              quartz_type_style
#define QUARTZ_STYLE(object)           (G_TYPE_CHECK_INSTANCE_CAST ((object), QUARTZ_TYPE_STYLE, QuartzStyle))
#define QUARTZ_STYLE_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST ((klass), QUARTZ_TYPE_STYLE, QuartzStyleClass))
#define QUARTZ_IS_STYLE(object)        (G_TYPE_CHECK_INSTANCE_TYPE ((object), QUARTZ_TYPE_STYLE))
#define QUARTZ_IS_STYLE_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), QUARTZ_TYPE_STYLE))
#define QUARTZ_STYLE_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS ((obj), QUARTZ_TYPE_STYLE, QuartzStyleClass))

struct _QuartzStyle
{
  GtkStyle parent_instance;
};

struct _QuartzStyleClass
{
  GtkStyleClass parent_class;
};

void quartz_style_register_type (GTypeModule *module);
void quartz_style_init          (void);

#endif /* QUARTZ_STYLE_H */
