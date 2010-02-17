/*
-------------------------------------------------------------------------------
	This file is part of OgreKit.
	http://gamekit.googlecode.com/

	Copyright (c) 2009 Charlie C.
-------------------------------------------------------------------------------
 This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
	 claim that you wrote the original software. If you use this software
	 in a product, an acknowledgment in the product documentation would be
	 appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
	 misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.
-------------------------------------------------------------------------------
*/
#include <stdio.h>
#include <stdarg.h>
#include "OgrePrint.h"
#include "OgreStringDefines.h"
#include "OgreLogManager.h"

#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
//  for IsDebuggerPresent
# if OGRE_COMPILER == OGRE_COMPILER_MSVC
#   define _WIN32_WINNT 0x403
# endif
# ifndef WIN32_LEAN_AND_MEAN
#  define WIN32_LEAN_AND_MEAN 1
# endif
# include <windows.h>
# include <io.h>
#endif


#define BUFSIZE (0xFFFF)
using namespace Ogre;


void Ogre::Logf(const char *fmt, ...)
{
    LogManager *mgr = LogManager::getSingletonPtr();
    Ogre::Log *log = mgr->getDefaultLog();
    if (!log)
        return;
    if (log->getLogDetail() == Ogre::LL_LOW)
        return;






    static char ConsoleBuffer[BUFSIZE+1];
    va_list lst;
    va_start(lst, fmt);
    int size = ut_vsnprintf(ConsoleBuffer, BUFSIZE, fmt, lst);
    va_end(lst);

    if (size < 0)
    {
        ConsoleBuffer[BUFSIZE] = 0;
        size = BUFSIZE;
    }

    if (size > 0)
    {
        /// out to log stream so user def flags work
        ConsoleBuffer[size] = 0;

#if OGRE_COMPILER == OGRE_COMPILER_MSVC
        if (IsDebuggerPresent())
        {
            /// display in debug console
            OutputDebugString(ConsoleBuffer);
			if (ConsoleBuffer[size-1] != '\n')
				OutputDebugString("\n");
        }
        else
#endif
        {
           log->logMessage(ConsoleBuffer);
        }
    }
}