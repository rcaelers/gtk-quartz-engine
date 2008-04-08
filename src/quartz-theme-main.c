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

#include <gmodule.h>
#include <gtk/gtk.h>

#include "quartz-style.h"
#include "quartz-rc-style.h"

G_MODULE_EXPORT void
theme_init (GTypeModule * module)
{
  quartz_rc_style_register_type (module);
  quartz_style_register_type (module);
  
  quartz_style_init ();
}

G_MODULE_EXPORT void
theme_exit (void)
{
}

G_MODULE_EXPORT GtkRcStyle *
theme_create_rc_style (void)
{
  return g_object_new (QUARTZ_TYPE_RC_STYLE, NULL);
}

G_MODULE_EXPORT const gchar *
g_module_check_init (GModule * module)
{
  return gtk_check_version (2, 10, 0);
}
