/* ScummVM - Scumm Interpreter
 * Copyright (C) 2002-2006 The ScummVM project
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
 * $URL$
 * $Id$
 */

#include "common/stdafx.h"

#include "base/engine.h"
#include "base/game.h"
#include "base/plugins.h"
#include "base/version.h"

#include "common/config-manager.h"
#include "common/fs.h"
#include "common/util.h"
#include "common/system.h"

#include "gui/about.h"
#include "gui/browser.h"
#include "gui/chooser.h"
#include "gui/eval.h"
#include "gui/launcher.h"
#include "gui/message.h"
#include "gui/newgui.h"
#include "gui/options.h"
#include "gui/EditTextWidget.h"
#include "gui/ListWidget.h"
#include "gui/TabWidget.h"
#include "gui/PopUpWidget.h"

#include "sound/mididrv.h"


using Common::ConfigManager;

namespace GUI {

enum {
	kStartCmd = 'STRT',
	kAboutCmd = 'ABOU',
	kOptionsCmd = 'OPTN',
	kAddGameCmd = 'ADDG',
	kEditGameCmd = 'EDTG',
	kRemoveGameCmd = 'REMG',
	kQuitCmd = 'QUIT',


	kCmdGlobalGraphicsOverride = 'OGFX',
	kCmdGlobalAudioOverride = 'OSFX',
	kCmdGlobalMIDIOverride = 'OMID',
	kCmdGlobalVolumeOverride = 'OVOL',

	kCmdChooseSoundFontCmd = 'chsf',

	kCmdExtraBrowser = 'PEXT',
	kCmdGameBrowser = 'PGME',
	kCmdSaveBrowser = 'PSAV'
};

/*
 * TODO: Clean up this ugly design: we subclass EditTextWidget to perform
 * input validation. It would be much more elegant to use a decorator pattern,
 * or a validation callback, or something like that.
 */
class DomainEditTextWidget : public EditTextWidget {
public:
	DomainEditTextWidget(GuiObject *boss, const String &name, const String &text)
		: EditTextWidget(boss, name, text) {
	}

protected:
	bool tryInsertChar(byte c, int pos) {
		if (isalnum(c) || c == '-' || c == '_') {
			_editString.insertChar(c, pos);
			return true;
		}
		return false;
	}
};

/*
 * A dialog that allows the user to edit a config game entry.
 * TODO: add widgets for some/all of the following
 * - Maybe scaler/graphics mode. But there are two problems:
 *   1) Different backends can have different scalers with different names,
 *      so we first have to add a way to query those... no Ender, I don't
 *      think a bitmasked property() value is nice for this,  because we would
 *      have to add to the bitmask values whenever a backends adds a new scaler).
 *   2) At the time the launcher is running, the GFX backend is already setup.
 *      So when a game is run via the launcher, the custom scaler setting for it won't be
 *      used. So we'd also have to add an API to change the scaler during runtime
 *      (the SDL backend can already do that based on user input, but there is no API
 *      to achieve it)
 *   If the APIs for 1&2 are in place, we can think about adding this to the Edit&Option dialogs
 */

class EditGameDialog : public OptionsDialog {
	typedef Common::String String;
	typedef Common::StringList StringList;
public:
	EditGameDialog(const String &domain, const String &desc);

	virtual void handleScreenChanged();	

	void open();
	void close();
	virtual void handleCommand(CommandSender *sender, uint32 cmd, uint32 data);

protected:
	EditTextWidget *_descriptionWidget;
	DomainEditTextWidget *_domainWidget;

	StaticTextWidget *_gamePathWidget;
	StaticTextWidget *_extraPathWidget;
	StaticTextWidget *_savePathWidget;

	PopUpWidget *_langPopUp;
	PopUpWidget *_platformPopUp;

	CheckboxWidget *_globalGraphicsOverride;
	CheckboxWidget *_globalAudioOverride;
	CheckboxWidget *_globalMIDIOverride;
	CheckboxWidget *_globalVolumeOverride;
};

EditGameDialog::EditGameDialog(const String &domain, const String &desc)
	: OptionsDialog(domain, "gameoptions") {

	int labelWidth = g_gui.evaluator()->getVar("gameOptionsLabelWidth");

	// GAME: Path to game data (r/o), extra data (r/o), and save data (r/w)
	String gamePath(ConfMan.get("path", _domain));
	String extraPath(ConfMan.get("extrapath", _domain));
	String savePath(ConfMan.get("savepath", _domain));

	// GAME: Determine the description string
	String description(ConfMan.get("description", domain));
	if (description.empty() && !desc.empty()) {
		description = desc;
	}

	// GUI:  Add tab widget
	TabWidget *tab = new TabWidget(this, "gameoptions_tabwidget");
	tab->setHints(THEME_HINT_FIRST_DRAW | THEME_HINT_SAVE_BACKGROUND);

	//
	// 1) The game tab
	//
	tab->addTab("Game");

	// GUI:  Label & edit widget for the game ID
	new StaticTextWidget(tab, "gameoptions_id", "ID: ");
	_domainWidget = new DomainEditTextWidget(tab, "gameoptions_domain", _domain);

	// GUI:  Label & edit widget for the description
	new StaticTextWidget(tab, "gameoptions_name", "Name: ");
	_descriptionWidget = new EditTextWidget(tab, "gameoptions_desc", description);

	// Language popup
	_langPopUp = new PopUpWidget(tab, "gameoptions_lang", "Language: ", labelWidth);
	_langPopUp->appendEntry("<default>");
	_langPopUp->appendEntry("");
	const Common::LanguageDescription *l = Common::g_languages;
	for (; l->code; ++l) {
		_langPopUp->appendEntry(l->description, l->id);
	}

	// Platform popup
	_platformPopUp = new PopUpWidget(tab, "gameoptions_platform", "Platform: ", labelWidth);
	_platformPopUp->appendEntry("<default>");
	_platformPopUp->appendEntry("");
	const Common::PlatformDescription *p = Common::g_platforms;
	for (; p->code; ++p) {
		_platformPopUp->appendEntry(p->description, p->id);
	}

	//
	// 3) The graphics tab
	//
	tab->addTab("Graphics");

	_globalGraphicsOverride = new CheckboxWidget(tab, "gameoptions_graphicsCheckbox", "Override global graphic settings", kCmdGlobalGraphicsOverride, 0);

	addGraphicControls(tab, "gameoptions_");

	//
	// 4) The audio tab
	//
	tab->addTab("Audio");

	_globalAudioOverride = new CheckboxWidget(tab, "gameoptions_audioCheckbox", "Override global audio settings", kCmdGlobalAudioOverride, 0);

	addAudioControls(tab, "gameoptions_");
	addSubtitleControls(tab, "gameoptions_");

	//
	// 5) The volume tab
	//
	int volControlPos = g_gui.evaluator()->getVar("volumeControlsInAudio", true);

	if (!volControlPos) {
		tab->addTab("Volume");

		_globalVolumeOverride = new CheckboxWidget(tab, "gameoptions_volumeCheckbox", "Override global volume settings", kCmdGlobalVolumeOverride, 0);
	} else {
		_globalVolumeOverride = NULL;
	}

	addVolumeControls(tab, "gameoptions_");

	//
	// 6) The MIDI tab
	//
	tab->addTab("MIDI");

	_globalMIDIOverride = new CheckboxWidget(tab, "gameoptions_midiCheckbox", "Override global MIDI settings", kCmdGlobalMIDIOverride, 0);

	addMIDIControls(tab, "gameoptions_");

	//
	// 2) The 'Path' tab
	//
	tab->addTab("Paths");

	// These buttons have to be extra wide, or the text will be truncated
	// in the small version of the GUI.

	// GUI:  Button + Label for the game path
	new ButtonWidget(tab, "gameoptions_gamepath", "Game Path: ", kCmdGameBrowser, 0);
	_gamePathWidget = new StaticTextWidget(tab, "gameoptions_gamepathText", gamePath);

	// GUI:  Button + Label for the additional path
	new ButtonWidget(tab, "gameoptions_extrapath", "Extra Path:", kCmdExtraBrowser, 0);
	_extraPathWidget = new StaticTextWidget(tab, "gameoptions_extrapathText", extraPath);
	if (extraPath.empty() || !ConfMan.hasKey("extrapath", _domain)) {
		_extraPathWidget->setLabel("None");
	}

	// GUI:  Button + Label for the save path
	new ButtonWidget(tab, "gameoptions_savepath", "Save Path: ", kCmdSaveBrowser, 0);
	_savePathWidget = new StaticTextWidget(tab, "gameoptions_savepathText", savePath);
	if (savePath.empty() || !ConfMan.hasKey("savepath", _domain)) {
		_savePathWidget->setLabel("Default");
	}

	// Activate the first tab
	tab->setActiveTab(0);

	// Add OK & Cancel buttons
	new ButtonWidget(this, "gameoptions_cancel", "Cancel", kCloseCmd, 0);
	new ButtonWidget(this, "gameoptions_ok", "OK", kOKCmd, 0);
}

void EditGameDialog::handleScreenChanged() {
	OptionsDialog::handleScreenChanged();

	int labelWidth = g_gui.evaluator()->getVar("gameOptionsLabelWidth");

	if (_langPopUp)
		_langPopUp->changeLabelWidth(labelWidth);
	if (_platformPopUp)
		_platformPopUp->changeLabelWidth(labelWidth);
}

void EditGameDialog::open() {
	OptionsDialog::open();

	int sel, i;
	bool e;

	// En-/disable dialog items depending on whether overrides are active or not.

	e = ConfMan.hasKey("fullscreen", _domain) ||
		ConfMan.hasKey("aspect_ratio", _domain);
	_globalGraphicsOverride->setState(e);

	e = ConfMan.hasKey("music_driver", _domain) ||
		ConfMan.hasKey("subtitles", _domain) ||
		ConfMan.hasKey("talkspeed", _domain);
	_globalAudioOverride->setState(e);

	e = ConfMan.hasKey("multi_midi", _domain) ||
		ConfMan.hasKey("native_mt32", _domain)||
		ConfMan.hasKey("enable_gs", _domain);
	_globalMIDIOverride->setState(e);

	e = ConfMan.hasKey("music_volume", _domain) ||
		ConfMan.hasKey("sfx_volume", _domain) ||
		ConfMan.hasKey("speech_volume", _domain);

	if (_globalVolumeOverride)
		_globalVolumeOverride->setState(e);

	// TODO: game path

	const Common::LanguageDescription *l = Common::g_languages;
	const Common::Language lang = Common::parseLanguage(ConfMan.get("language", _domain));

	sel = 0;
	if (ConfMan.hasKey("language", _domain)) {
		for (i = 0; l->code; ++l, ++i) {
			if (lang == l->id)
				sel = i + 2;
		}
	}
	_langPopUp->setSelected(sel);


	const Common::PlatformDescription *p = Common::g_platforms;
	const Common::Platform platform = Common::parsePlatform(ConfMan.get("platform", _domain));
	sel = 0;
	for (i = 0; p->code; ++p, ++i) {
		if (platform == p->id)
			sel = i + 2;
	}
	_platformPopUp->setSelected(sel);
}


void EditGameDialog::close() {
	if (getResult()) {
		ConfMan.set("description", _descriptionWidget->getEditString(), _domain);

		Common::Language lang = (Common::Language)_langPopUp->getSelectedTag();
		if (lang < 0)
			ConfMan.removeKey("language", _domain);
		else
			ConfMan.set("language", Common::getLanguageCode(lang), _domain);

		String gamePath(_gamePathWidget->getLabel());
		if (!gamePath.empty())
			ConfMan.set("path", gamePath, _domain);

		String extraPath(_extraPathWidget->getLabel());
		if (!extraPath.empty() && (extraPath != "None"))
			ConfMan.set("extrapath", extraPath, _domain);

		String savePath(_savePathWidget->getLabel());
		if (!savePath.empty() && (savePath != "Default"))
			ConfMan.set("savepath", savePath, _domain);

		Common::Platform platform = (Common::Platform)_platformPopUp->getSelectedTag();
		if (platform < 0)
			ConfMan.removeKey("platform", _domain);
		else
			ConfMan.set("platform", Common::getPlatformCode(platform), _domain);
	}
	OptionsDialog::close();
}

void EditGameDialog::handleCommand(CommandSender *sender, uint32 cmd, uint32 data) {
	switch (cmd) {
	case kCmdGlobalGraphicsOverride:
		setGraphicSettingsState(data != 0);
		draw();
		break;
	case kCmdGlobalAudioOverride:
		setAudioSettingsState(data != 0);
		setSubtitleSettingsState(data != 0);
		if(_globalVolumeOverride == NULL)
			setVolumeSettingsState(data != 0);
		draw();
		break;
	case kCmdGlobalMIDIOverride:
		setMIDISettingsState(data != 0);
		draw();
		break;
	case kCmdGlobalVolumeOverride:
		setVolumeSettingsState(data != 0);
		draw();
		break;
	case kCmdChooseSoundFontCmd: {
		BrowserDialog browser("Select SoundFont", false);

		if (browser.runModal() > 0) {
			// User made this choice...
			FilesystemNode file(browser.getResult());
			_soundFont->setLabel(file.path());
			draw();
		}
		break;
	}

	// Change path for the game
	case kCmdGameBrowser: {
		BrowserDialog browser("Select directory with game data", true);
		if (browser.runModal() > 0) {
			// User made his choice...
			FilesystemNode dir(browser.getResult());

			// TODO: Verify the game can be found in the new directory... Best
			// done with optional specific gameid to pluginmgr detectgames?
			// FSList files = dir.listDir(FilesystemNode::kListFilesOnly);

			_gamePathWidget->setLabel(dir.path());
			draw();
		}
		draw();
		break;
	}

	// Change path for extra game data (eg, using sword cutscenes when playing via CD)
	case kCmdExtraBrowser: {
		BrowserDialog browser("Select additional game directory", true);
		if (browser.runModal() > 0) {
			// User made his choice...
			FilesystemNode dir(browser.getResult());
			_extraPathWidget->setLabel(dir.path());
			draw();
		}
		draw();
		break;
	}
	// Change path for stored save game (perm and temp) data
	case kCmdSaveBrowser: {
		BrowserDialog browser("Select directory for saved games", true);
		if (browser.runModal() > 0) {
			// User made his choice...
			FilesystemNode dir(browser.getResult());
			_savePathWidget->setLabel(dir.path());
			draw();
		}
		draw();
		break;
	}

	case kOKCmd: {
		// Write back changes made to config object
		String newDomain(_domainWidget->getEditString());
		if (newDomain != _domain) {
			if (newDomain.empty() || ConfMan.hasGameDomain(newDomain)) {
				MessageDialog alert("This game ID is already taken. Please choose another one.");
				alert.runModal();
				return;
			}
			ConfMan.renameGameDomain(_domain, newDomain);
			_domain = newDomain;
		}
		}
		// FALL THROUGH to default case
	default:
		OptionsDialog::handleCommand(sender, cmd, data);
	}
}


#pragma mark -

LauncherDialog::LauncherDialog()
	: Dialog(0, 0, 320, 200) {
	_drawingHints |= THEME_HINT_MAIN_DIALOG;

	const int screenW = g_system->getOverlayWidth();
	const int screenH = g_system->getOverlayHeight();

	_w = screenW;
	_h = screenH;

#ifndef DISABLE_FANCY_THEMES
	_logo = 0;
	if (g_gui.evaluator()->getVar("launcher_logo.visible") == 1) {
		_logo = new GraphicsWidget(this, "launcher_logo");
		_logo->useThemeTransparency(true);
		ThemeNew *th = (ThemeNew *)g_gui.theme();

		_logo->setGfx(th->getImageSurface(th->kThemeLogo));

		new StaticTextWidget(this, "launcher_version", gScummVMVersionDate);
	} else
		new StaticTextWidget(this, "launcher_version", gScummVMFullVersion);
#else
	// Show ScummVM version
	new StaticTextWidget(this, "launcher_version", gScummVMFullVersion);
#endif

	new ButtonWidget(this, "launcher_quit_button", "Quit", kQuitCmd, 'Q');
	new ButtonWidget(this, "launcher_about_button", "About", kAboutCmd, 'B');
	new ButtonWidget(this, "launcher_options_button", "Options", kOptionsCmd, 'O');
	_startButton =
			new ButtonWidget(this, "launcher_start_button", "Start", kStartCmd, 'S');

	// Above the lowest button rows: two more buttons (directly below the list box)
	new ButtonWidget(this, "launcher_addGame_button", "Add Game...", kAddGameCmd, 'A');
	_editButton =
		new ButtonWidget(this, "launcher_editGame_button", "Edit Game...", kEditGameCmd, 'E');
	_removeButton =
		new ButtonWidget(this, "launcher_removeGame_button", "Remove Game", kRemoveGameCmd, 'R');


	// Add list with game titles
	_list = new ListWidget(this, "launcher_list");
	_list->setEditable(false);
	_list->setNumberingMode(kListNumberingOff);


	// Populate the list
	updateListing();

	// Restore last selection
	String last(ConfMan.get("lastselectedgame", ConfigManager::kApplicationDomain));
	selectGame(last);

	// En-/disable the buttons depending on the list selection
	updateButtons();

	// Create file browser dialog
	_browser = new BrowserDialog("Select directory with game data", true);
}

void LauncherDialog::selectGame(const String &name) {
	if (!name.empty()) {
		int itemToSelect = 0;
		StringList::const_iterator iter;
		for (iter = _domains.begin(); iter != _domains.end(); ++iter, ++itemToSelect) {
			if (name == *iter) {
				_list->setSelected(itemToSelect);
				break;
			}
		}
	}
}

LauncherDialog::~LauncherDialog() {
	delete _browser;
}

void LauncherDialog::close() {
	// Save last selection
	const int sel = _list->getSelected();
	if (sel >= 0)
		ConfMan.set("lastselectedgame", _domains[sel], ConfigManager::kApplicationDomain);
	else
		ConfMan.removeKey("lastselectedgame", ConfigManager::kApplicationDomain);

	ConfMan.flushToDisk();
	Dialog::close();
}

void LauncherDialog::updateListing() {
	Common::StringList l;

	// Retrieve a list of all games defined in the config file
	_domains.clear();
	const ConfigManager::DomainMap &domains = ConfMan.getGameDomains();
	ConfigManager::DomainMap::const_iterator iter = domains.begin();
	for (iter = domains.begin(); iter != domains.end(); ++iter) {
		String gameid(iter->_value.get("gameid"));
		String description(iter->_value.get("description"));

		if (gameid.empty())
			gameid = iter->_key;
		if (description.empty()) {
			GameDescriptor g = Base::findGame(gameid);
			if (!g.description.empty())
				description = g.description;
		}

		if (!gameid.empty() && !description.empty()) {
			// Insert the game into the launcher list
			int pos = 0, size = l.size();

			while (pos < size && (scumm_stricmp(description.c_str(), l[pos].c_str()) > 0))
				pos++;
			l.insert_at(pos, description);
			_domains.insert_at(pos, iter->_key);
		}
	}

	const int oldSel = _list->getSelected();
	_list->setList(l);
	if (oldSel < (int)l.size())
		_list->setSelected(oldSel);	// Restore the old selection
	updateButtons();
}

void LauncherDialog::addGame() {
	// Allow user to add a new game to the list.
	// 1) show a dir selection dialog which lets the user pick the directory
	//    the game data resides in.
	// 2) try to auto detect which game is in the directory, if we cannot
	//    determine it uniquely preent a list of candidates to the user
	//    to pick from
	// 3) Display the 'Edit' dialog for that item, letting the user specify
	//    an alternate description (to distinguish multiple versions of the
	//    game, e.g. 'Monkey German' and 'Monkey English') and set default
	//    options for that game.

	if (_browser->runModal() > 0) {
		// User made his choice...
		FilesystemNode dir(_browser->getResult());
		FSList files;
		if (!dir.listDir(files, FilesystemNode::kListAll)) {
			error("browser returned a node that is not a directory: '%s'",
					dir.path().c_str());
		}

		// ...so let's determine a list of candidates, games that
		// could be contained in the specified directory.
		DetectedGameList candidates(PluginManager::instance().detectGames(files));

		int idx;
		if (candidates.empty()) {
			// No game was found in the specified directory
			MessageDialog alert("ScummVM could not find any game in the specified directory!");
			alert.runModal();
			idx = -1;
		} else if (candidates.size() == 1) {
			// Exact match
			idx = 0;
		} else {
			// Display the candidates to the user and let her/him pick one
			StringList list;
			for (idx = 0; idx < (int)candidates.size(); idx++)
				list.push_back(candidates[idx].description);

			ChooserDialog dialog("Pick the game:");
			dialog.setList(list);
			idx = dialog.runModal();
		}
		if (0 <= idx && idx < (int)candidates.size()) {
			DetectedGame result = candidates[idx];

			// The auto detector or the user made a choice.
			// Pick a domain name which does not yet exist (after all, we
			// are *adding* a game to the config, not replacing).
			String domain(result.gameid);
			if (ConfMan.hasGameDomain(domain)) {
				char suffix = 'a';
				domain += suffix;
				while (ConfMan.hasGameDomain(domain)) {
					assert(suffix < 'z');
					domain.deleteLastChar();
					suffix++;
					domain += suffix;
				}
			}

			// Add the name domain
			ConfMan.addGameDomain(domain);
			
			// TODO: Setting the description field here has the drawback
			// that the user does never notice when we upgrade our descriptions.
			// It might be nice ot leave this field empty, and only set it to
			// a value when the user edits the description string. 
			// However, at this point, that's impractical. Once we have a method
			// to query all backends for the proper & full description of a given
			// game target, we can change this (currently, you can only query
			// for the generic gameid description; it's not possible to obtain
			// a description which contains extended information like language, etc.).
			ConfMan.set("description", result.description, domain);

			ConfMan.set("gameid", result.gameid, domain);
			ConfMan.set("path", dir.path(), domain);

			// Set language if specified
			if (result.language != Common::UNK_LANG)
				ConfMan.set("language", Common::getLanguageCode(result.language), domain);

			// Set platform if specified
			if (result.platform != Common::kPlatformUnknown)
				ConfMan.set("platform", Common::getPlatformCode(result.platform), domain);

			// Display edit dialog for the new entry
			EditGameDialog editDialog(domain, result.description);
			if (editDialog.runModal() > 0) {
				// User pressed OK, so make changes permanent

				// Write config to disk
				ConfMan.flushToDisk();

				// Update the ListWidget, select the new item, and force a redraw
				updateListing();
				selectGame(domain);
				draw();
			} else {
				// User aborted, remove the the new domain again
				ConfMan.removeGameDomain(domain);
			}
		}
	}
}

void LauncherDialog::removeGame(int item) {
	MessageDialog alert("Do you really want to remove this game configuration?", "Yes", "No");

	if (alert.runModal() == GUI::kMessageOK) {
		// Remove the currently selected game from the list
		assert(item >= 0);
		ConfMan.removeGameDomain(_domains[item]);

		// Write config to disk
		ConfMan.flushToDisk();

		// Update the ListWidget and force a redraw
		updateListing();
		draw();
	}
}

void LauncherDialog::editGame(int item) {
	// Set game specifc options. Most of these should be "optional", i.e. by
	// default set nothing and use the global ScummVM settings. E.g. the user
	// can set here an optional alternate music volume, or for specific games
	// a different music driver etc.
	// This is useful because e.g. MonkeyVGA needs Adlib music to have decent
	// music support etc.
	assert(item >= 0);
	String gameId(ConfMan.get("gameid", _domains[item]));
	if (gameId.empty())
		gameId = _domains[item];
	EditGameDialog editDialog(_domains[item], Base::findGame(gameId).description);
	if (editDialog.runModal() > 0) {
		// User pressed OK, so make changes permanent

		// Write config to disk
		ConfMan.flushToDisk();

		// Update the ListWidget and force a redraw
		updateListing();
		draw();
	}
}

void LauncherDialog::handleCommand(CommandSender *sender, uint32 cmd, uint32 data) {
	int item = _list->getSelected();

	switch (cmd) {
	case kAddGameCmd:
		addGame();
		break;
	case kRemoveGameCmd:
		removeGame(item);
		break;
	case kEditGameCmd:
		editGame(item);
		break;
	case kOptionsCmd: {
		GlobalOptionsDialog options;
		options.runModal();
		}
		break;
	case kAboutCmd: {
		AboutDialog about;
		about.runModal();
		}
		break;
	case kStartCmd:
	case kListItemActivatedCmd:
	case kListItemDoubleClickedCmd:
		// Print out what was selected
		assert(item >= 0);
		ConfMan.setActiveDomain(_domains[item]);
		close();
		break;
	case kListSelectionChangedCmd:
		updateButtons();
		break;
	case kQuitCmd:
		ConfMan.setActiveDomain("");
		setResult(-1);
		close();
		break;
	default:
		Dialog::handleCommand(sender, cmd, data);
	}
}

void LauncherDialog::updateButtons() {
	bool enable = (_list->getSelected() >= 0);
	if (enable != _startButton->isEnabled()) {
		_startButton->setEnabled(enable);
		_startButton->draw();
	}
	if (enable != _editButton->isEnabled()) {
		_editButton->setEnabled(enable);
		_editButton->draw();
	}
	if (enable != _removeButton->isEnabled()) {
		_removeButton->setEnabled(enable);
		_removeButton->draw();
	}
}

void LauncherDialog::handleScreenChanged() {
#ifndef DISABLE_FANCY_THEMES
	if (g_gui.evaluator()->getVar("launcher_logo.visible") == 1) {
		StaticTextWidget *ver = (StaticTextWidget*)findWidget("launcher_version");
		if (ver) {
			ver->setAlign((Graphics::TextAlignment)g_gui.evaluator()->getVar("launcher_version.align"));
			ver->setLabel(gScummVMVersionDate);
		}

		if (!_logo)
			_logo = new GraphicsWidget(this, "launcher_logo");
		_logo->useThemeTransparency(true);
		ThemeNew *th = (ThemeNew *)g_gui.theme();

		_logo->setGfx(th->getImageSurface(th->kThemeLogo));
	} else {
		StaticTextWidget *ver = (StaticTextWidget*)findWidget("launcher_version");
		if (ver) {
			ver->setAlign((Graphics::TextAlignment)g_gui.evaluator()->getVar("launcher_version.align"));
			ver->setLabel(gScummVMFullVersion);
		}

		if (_logo) {
			deleteWidget(_logo);
			_logo->setNext(0);
			delete _logo;
			_logo = 0;
		}
	}
#endif

	_w = g_system->getOverlayWidth();
	_h = g_system->getOverlayHeight();

	Dialog::handleScreenChanged();
}

} // End of namespace GUI
