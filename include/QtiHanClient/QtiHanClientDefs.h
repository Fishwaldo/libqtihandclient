/* controlpanel - QtiHanClientDefs.h
** Copyright (c) 2010 Justin Hammond
**
**  This program is free software; you can redistribute it and/or modify
**  it under the terms of the GNU General Public License as published by
**  the Free Software Foundation; either version 2 of the License, or
**  (at your option) any later version.
**
**  This program is distributed in the hope that it will be useful,
**  but WITHOUT ANY WARRANTY; without even the implied warranty of
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**  GNU General Public License for more details.
**
**  You should have received a copy of the GNU General Public License
**  along with this program; if not, write to the Free Software
**  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307
**  USA
**
** controlpanel SVN Identification:
** $Rev$
*/

/** @file QtiHanClientDefs.h
 *  @brief
 */

#ifndef QTIHANCLIENTDEF_H_
#define QTIHANCLIENTDEF_H_

#include <QtCore\QtGlobal>


#if (defined _WINDOWS || defined WIN32 || defined _MSC_VER) && !defined MINGW
#	if defined QTIHANCLIENT_MAKEDLL	// Create the dynamic library.
#		define QTIHANCLIENT_EXPORT    Q_DECL_EXPORT
#	elif defined QTIHANCLIENT_USEDLL	// Use the dynamic library
#		define QTIHANCLIENT_EXPORT    Q_DECL_IMPORT
#	else							// Create/Use the static library
#		define QTIHANCLIENT_EXPORT
#	endif
// Disable export warnings
#	define QTIHANCLIENT_EXPORT_WARNINGS_OFF	__pragma( warning(push) )\
											__pragma( warning(disable: 4251 4275) )
#	define QTIHANCLIENT_EXPORT_WARNINGS_ON		__pragma( warning(pop) )
#else
#	define QTIHANCLIENT_EXPORT
#	define QTIHANCLIENT_EXPORT_WARNINGS_OFF
#	define QTIHANCLIENT_EXPORT_WARNINGS_ON
#endif

#endif