/* ScummVM - Scumm Interpreter
 * Copyright (C) 2003-2005 Andreas 'Sprawl' Karlsson - Original EPOC port, ESDL
 * Copyright (C) 2003-2005 Lars 'AnotherGuest' Persson - Original EPOC port, Audio System
 * Copyright (C) 2005 Jurgen 'SumthinWicked' Braam - EPOC/CVS maintainer
 * Copyright (C) 2005-2006 The ScummVM project
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef ScummVMapph
#define ScummVMapph

#include <eikapp.h>
#include <e32base.h>
#include <coecntrl.h>
#include <eikenv.h>
#include <coeview.h>
#include <eikappui.h>

class CScummVM : public CEikApplication {
public:
	CScummVM();
	~CScummVM();

	CApaDocument *CreateDocumentL();
	TUid AppDllUid() const;
};


#include <eikdoc.h>

class CScummVMDoc : public CEikDocument {
public:
	CScummVMDoc(CEikApplication &aApplicaiton);
	~CScummVMDoc();

	CEikAppUi *CreateAppUiL();
	void ConstructL();
};

#include <eikappui.h>
class CScummVMUi;
class CScummWatcher : public CActive {
public:
	CScummWatcher();
	~CScummWatcher();

	void DoCancel();
	void RunL();
	CScummVMUi *iAppUi;
};

class CScummVMUi : public CEikAppUi {
public:
	CScummVMUi();
	~CScummVMUi();

	void ConstructL();
	void HandleCommandL(TInt aCommand);
	void HandleForegroundEventL(TBool aForeground);
	void BringUpEmulatorL();

private:
	TThreadId iThreadId;
	TInt iExeWgId;
	RThread iThreadWatch;
	CScummWatcher *iWatcher;
};
#endif
