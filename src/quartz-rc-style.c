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

#include "quartz-style.h"
#include "quartz-rc-style.h"

static void quartz_rc_style_init (QuartzRcStyle * style);
static void quartz_rc_style_class_init (QuartzRcStyleClass * klass);
static GtkStyle *quartz_rc_style_create_style (GtkRcStyle * rc_style);

static GtkRcStyleClass *parent_class;

GType quartz_type_rc_style = 0;

void
quartz_rc_style_register_type (GTypeModule * module)
{
    static const GTypeInfo object_info = {
	sizeof (QuartzRcStyleClass),
	(GBaseInitFunc) NULL,
	(GBaseFinalizeFunc) NULL,
	(GClassInitFunc) quartz_rc_style_class_init,
	NULL,			/* class_finalize */
	NULL,			/* class_data */
	sizeof (QuartzRcStyle),
	0,			/* n_preallocs */
	(GInstanceInitFunc) quartz_rc_style_init,
    };

    quartz_type_rc_style = g_type_module_register_type (module,
						     GTK_TYPE_RC_STYLE,
						     "QuartzRcStyle",
						     &object_info, 0);
}

static void
quartz_rc_style_init (QuartzRcStyle * style)
{
}

static void
quartz_rc_style_class_init (QuartzRcStyleClass * klass)
{
    GtkRcStyleClass *rc_style_class = GTK_RC_STYLE_CLASS (klass);

    parent_class = g_type_class_peek_parent (klass);

    rc_style_class->create_style = quartz_rc_style_create_style;
}

static GtkStyle *
quartz_rc_style_create_style (GtkRcStyle * rc_style)
{
    return g_object_new (QUARTZ_TYPE_STYLE, NULL);
}
