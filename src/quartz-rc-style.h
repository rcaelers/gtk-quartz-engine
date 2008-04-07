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

#ifndef QUARTZ_RC_STYLE_H
#define QUARTZ_RC_STYLE_H

#include <gtk/gtkrc.h>

typedef struct _QuartzRcStyle QuartzRcStyle;
typedef struct _QuartzRcStyleClass QuartzRcStyleClass;

extern GType quartz_type_rc_style;

#define QUARTZ_TYPE_RC_STYLE              quartz_type_rc_style
#define QUARTZ_RC_STYLE(object)           (G_TYPE_CHECK_INSTANCE_CAST ((object), QUARTZ_TYPE_RC_STYLE, QuartzRcStyle))
#define QUARTZ_RC_STYLE_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST ((klass), QUARTZ_TYPE_RC_STYLE, QuartzRcStyleClass))
#define QUARTZ_IS_RC_STYLE(object)        (G_TYPE_CHECK_INSTANCE_TYPE ((object), QUARTZ_TYPE_RC_STYLE))
#define QUARTZ_IS_RC_STYLE_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), QUARTZ_TYPE_RC_STYLE))
#define QUARTZ_RC_STYLE_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS ((obj), QUARTZ_TYPE_RC_STYLE, QuartzRcStyleClass))

struct _QuartzRcStyle
{
  GtkRcStyle parent_instance;
  
	//GList *img_list;
};

struct _QuartzRcStyleClass
{
  GtkRcStyleClass parent_class;
};

void quartz_rc_style_register_type (GTypeModule *module);

#endif /* QUARTZ_RC_STYLE_H */
