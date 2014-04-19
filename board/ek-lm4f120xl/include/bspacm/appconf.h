/* BSPACM - config header for ek-lm4f120xl
 *
 * Written in 2014 by Peter A. Bigot <http://pabigot.github.io/bspacm/>
 *
 * To the extent possible under law, the author(s) have dedicated all
 * copyright and related and neighboring rights to this software to
 * the public domain worldwide. This software is distributed without
 * any warranty.
 *
 * You should have received a copy of the CC0 Public Domain Dedication
 * along with this software. If not, see
 * <http://creativecommons.org/publicdomain/zero/1.0/>.
 */

/** @file
 *
 * @brief Board-specific configuration header for ek-lm4f120xl
 *
 * This file is included by <bspacm/config.h> which defines general
 * defaults that are overridden/extended/used here to apply
 * information to the board-specific configuration. Its contents
 * should be consistent with the peripheral configuration identified
 * in the @c PERIPH_CONFIG_SRC make variable.
 *
 * This file may be copied, modified, and installed in an
 * application-specific include directory to supersede the copy in the
 * board-specific include hierarchy.
 *
 * @homepage http://github.com/pabigot/bspacm
 * @publicdomain @cc0_doc
 */

#ifndef BSPACM_APPCONF_H
#define BSPACM_APPCONF_H

/* Use the common file to default various settings */
#include <bspacm/board/default_appconf.h>

#endif /* BSPACM_APPCONF_H */
