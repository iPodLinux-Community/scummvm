/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001-2006 The ScummVM project
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#ifndef SIMON_H
#define SIMON_H

#include "base/engine.h"

#include "common/rect.h"
#include "common/util.h"

#include "simon/animation.h"
#include "simon/midi.h"
#include "simon/sound.h"
#include "simon/vga.h"

namespace Simon {

/* Various other settings */
//#define DUMP_FILE_NR 8
//#define DUMP_BITMAPS_FILE_NR 8

uint fileReadItemID(Common::File *in);

#define CHECK_BOUNDS(x, y) assert((uint)(x) < ARRAYSIZE(y))

struct Child;
struct SubObject;

struct Item;
struct WindowBlock;
struct Subroutine;
struct SubroutineLine;
struct TimeEvent;

struct TextLocation {
	int16 x, y, width;
	TextLocation() { memset(this, 0, sizeof(*this)); }
};

struct HitArea {
	uint16 x, y;
	uint16 width, height;
	uint16 flags;
	uint16 id;
	uint16 data;
	WindowBlock *window;
	Item *item_ptr;
	uint16 verb;
	uint16 priority;
	HitArea() { memset(this, 0, sizeof(*this)); }
};

struct VgaPointersEntry {
	byte *vgaFile1;
	byte *vgaFile1End;
	byte *vgaFile2;
	byte *vgaFile2End;
	byte *sfxFile;
	byte *sfxFileEnd;
	VgaPointersEntry() { memset(this, 0, sizeof(*this)); }
};

struct VgaSprite {
	uint16 id;
	uint16 image;
	uint16 palette;
	uint16 x, y;									/* actually signed numbers */
	uint16 flags;
	uint16 priority;
	uint16 windowNum, zoneNum;
	VgaSprite() { memset(this, 0, sizeof(*this)); }
};

struct VgaSleepStruct {
	uint16 ident;
	const byte *code_ptr;
	uint16 sprite_id;
	uint16 cur_vga_file;
	VgaSleepStruct() { memset(this, 0, sizeof(*this)); }
};

struct VgaTimerEntry {
	int16 delay;
	const byte *script_pointer;
	uint16 sprite_id;
	uint16 cur_vga_file;
	int32 param;
	VgaTimerEntry() { memset(this, 0, sizeof(*this)); }
};

enum SIMONGameType {
	GType_FF = 0,
	GType_SIMON1 = 1,
	GType_SIMON2 = 2
};

struct GameFileDescription {
	const char *fileName;
	uint16 fileType;
	const char *md5;
};

struct GameDescription {
	const char *name;
	SIMONGameType gameType;
	GameIds gameId;
	const char *extra;
	int filesCount;
	GameFileDescription *filesDescriptions;
	uint32 features;
	Common::Language language;
	Common::Platform platform;
};

struct GameSpecificSettings;

class Debugger;

class SimonEngine : public Engine {
	friend class Debugger;
	friend class MoviePlayer;

	void errorString(const char *buf_input, char *buf_output);

	typedef void (SimonEngine::*OpcodeProc) ();
	void setupOpcodes();
	const OpcodeProc *_opcode_table;
	int _numOpcodes;

	typedef void (SimonEngine::*VgaOpcodeProc) ();
	void setupVgaOpcodes();
	const VgaOpcodeProc *_vga_opcode_table;

public:
	GameDescription *_gameDescription;

	bool initGame(void);

	int getGameId() const { return _gameDescription->gameId; }
	int getGameType() const { return _gameDescription->gameType; }
	uint32 getFeatures() const { return _gameDescription->features; }
	Common::Language getLanguage() const { return _gameDescription->language; }
	Common::Platform getPlatform() const { return _gameDescription->platform; }
	const char *getFileName(int type) const { 
		for (int i = 0; i < _gameDescription->filesCount; i++) {
			if (_gameDescription->filesDescriptions[i].fileType == type)
				return _gameDescription->filesDescriptions[i].fileName; 
		}
		error("getFileName: Invalid type %d", type);
	}

protected:
	void playSting(uint a);

	const byte *_vcPtr;								/* video code ptr */
	uint16 _vc_get_out_of_code;


	uint32 *_gameOffsetsPtr;

	uint _vgaBaseDelay;
	uint _tableIndexBase;
	uint _textIndexBase;
	uint _numVideoOpcodes;
	uint _vgaMemSize;
	uint _tableMemSize;
	uint _musicIndexBase;
	uint _soundIndexBase;
	const GameSpecificSettings *gss;

	byte _keyPressed;

	typedef enum {
		FORMAT_NONE,
		FORMAT_MP3,
		FORMAT_WAV,
		FORMAT_VOC
	} SoundFormat;

	Common::File *_gameFile;

	byte *_strippedTxtMem;
	byte *_textMem;
	uint _textSize;
	uint _stringTabNum, _stringTabPos, _stringtab_numalloc;
	byte **_stringTabPtr;

	Item **_itemArrayPtr;
	uint _itemArraySize;
	uint _itemArrayInited;

	byte *_itemHeapPtr;
	uint _itemHeapCurPos;
	uint _itemHeapSize;

	byte *_iconFilePtr;

	byte *_tblList;

	const byte *_codePtr;

	byte **_localStringtable;
	uint _stringIdLocalMin, _stringIdLocalMax;

	byte *_tablesHeapPtr, *_tablesHeapPtrOrg, *_tablesheapPtrNew;
	uint _tablesHeapSize, _tablesHeapCurPos, _tablesHeapCurPosOrg;
	uint _tablesHeapCurPosNew;

	Subroutine *_subroutineList, *_subroutineListOrg;
	uint _subroutine;

	uint _dxSurfacePitch;

	uint _recursionDepth;

	uint32 _lastVgaTick;

	uint16 _marks;

	bool _scriptVar2;
	bool _runScriptReturn1;
	bool _runScriptCondition[40];
	int _runScriptReturn[40];
	bool _skipVgaWait;
	bool _noParentNotify;
	bool _beardLoaded;
	bool _hitarea_unk_3;
	bool _mortalFlag;
	bool _updateScreen;
	bool _usePaletteDelay;
	bool _syncFlag2;
	bool _inCallBack;
	bool _cepeFlag;
	byte _copyPartialMode;
	bool _fastMode;
	bool _useBackGround;

	uint16 _debugMode;
	uint16 _language;
	bool _pause;
	bool _startMainScript;
	bool _continousMainScript;
	bool _startVgaScript;
	bool _continousVgaScript;
	bool _drawImagesDebug;
	bool _dumpImages;
	bool _speech;
	bool _subtitles;
	bool _vgaVar9;
	int16 _chanceModifier;
	bool _restoreWindow6;
	int _scrollX, _scrollXMax, _scrollWidth;
	int _scrollY, _scrollYMax, _scrollHeight;
	int _scrollCount, _scrollFlag;
	const byte *_scrollImage;
	byte _boxStarHeight;

 	uint16 _hyperLink, _newLines;
 	uint16 _oracleMaxScrollY, _noOracleScroll;
 	uint16 _interactY;

	int16 _scriptVerb, _scriptNoun1, _scriptNoun2;
	int16 _scriptAdj1, _scriptAdj2;

	uint16 _curWindow;
	WindowBlock *_textWindow;

	Item *_subjectItem, *_objectItem;
	Item *_currentPlayer;

	Item *_hitAreaObjectItem;
	HitArea *_lastHitArea;
	HitArea *_lastNameOn;
	HitArea *_lastHitArea3;
	Item *_hitAreaSubjectItem;
	HitArea *_currentVerbBox, *_lastVerbOn;
	uint _needHitAreaRecalc;
	uint _verbHitArea;
	uint16 _defaultVerb;
	uint _currentBoxNumber;
	uint _iOverflow;

	uint16 _windowNum;

	uint _printCharCurPos, _printCharMaxPos, _printCharPixelCount;
	uint _numLettersToPrint;

	uint _numTextBoxes;

	uint _lastTime;
	uint32 _clockStopped, _gameStoppedClock;
	time_t _timeStore;

	TimeEvent *_firstTimeStruct, *_pendingDeleteTimeEvent;

	int _mouseX, _mouseY;
	int _mouseXOld, _mouseYOld;

	enum {
	    kMaxCursorWidth = 40,
	    kMaxCursorHeight = 40
	};

	byte _mouseData[kMaxCursorWidth * kMaxCursorHeight];
	byte _animatePointer;
	byte _mouseCursor, _mouseAnim, _mouseAnimMax;
	byte _currentMouseCursor, _currentMouseAnim;
	byte _oldMouseCursor, _oldMouseAnimMax;
	uint _mouseHideCount;
	bool _mouseToggle;

	byte _leftButtonDown;
	byte _rightButtonDown;
	bool _noRightClick;

	Item *_dummyItem1;
	Item *_dummyItem2;
	Item *_dummyItem3;

	volatile uint16 _lockWord;
	uint16 _scrollUpHitArea;
	uint16 _scrollDownHitArea;

	bool _fastFadeOutFlag;
	bool _unkPalFlag;
	byte _paletteFlag;
	uint _fastFadeCount;
	volatile uint16 _fastFadeInFlag;

	int _screenWidth, _screenHeight;

	uint16 _noOverWrite;
	bool _rejectBlock;

	bool _exitCutscene;

	uint _soundFileId;
	int16 _lastMusicPlayed;
	int16 _nextMusicToPlay;

	bool _showPreposition;
	bool _showMessageFlag;

	uint _vgaSpriteChanged;

	byte *_vgaMemPtr, *_vgaMemEnd, *_vgaMemBase;
	byte *_vgaFrozenBase, *_vgaRealBase;
	byte *_zoneBuffers;

	byte *_curVgaFile1;
	byte *_curVgaFile2;
	byte *_curSfxFile;

	uint16 _syncCount, _timer5, _timer4;

	uint16 _frameRate;

	uint16 _zoneNumber;
	uint16 _vgaWaitFor, _lastVgaWaitFor;
	uint16 _vgaCurSpriteId, _vgaCurZoneNum;
	uint16 _vgaCurSpritePriority;

	int16 _baseY;
	float _scale;
	Common::Rect _feebleRect;
	int _scaleX, _scaleY, _scaleWidth, _scaleHeight;

	VgaTimerEntry *_nextVgaTimerToProcess;

	Item *_objectArray[20];
	Item *_itemStore[20];

	uint16 _stringIdArray2[40];
	uint16 _stringIdArray3[40];
	uint16 _speechIdArray4[40];

	uint16 _bitArray[16];
	uint16 _bitArrayTwo[16];
	uint16 _bitArrayThree[16];
	int16 _variableArray[256];
	int16 _variableArray2[256];
	int16 *_variableArrayPtr;

	WindowBlock *_windowArray[16];

	byte _fcsData1[8];
	bool _fcsData2[8];

	TextLocation _textLocation1, _textLocation2, _textLocation3, _textLocation4;

	int _freeStringSlot;

	byte _stringReturnBuffer[2][180];

	HitArea _hitAreas[250];

	VgaPointersEntry _vgaBufferPointers[450];
	VgaSprite _vgaSprites[180];
	VgaSleepStruct _vgaSleepStructs[60];

	const uint16 *_pathFindArray[100];

	uint8 _pathValues[400];
	uint16 _PVCount;
	uint16 _GPVCount;

	uint8 _pathValues1[400];
	uint16 _PVCount1;
	uint16 _GPVCount1;

	uint8 _currentPalette[1024];
	uint8 _displayPalette[1024];

	byte _videoBuf1[3000];

	VgaTimerEntry _vgaTimerList[900];

	WindowBlock *_windowList;

	byte _lettersToPrintBuf[80];

	MidiPlayer midi;
	bool _native_mt32;

	int _vgaTickCounter;

	MoviePlayer *_moviePlay;

	Sound *_sound;

	bool _effectsPaused;
	bool _ambientPaused;
	bool _musicPaused;

	Debugger *_debugger;

	int _saveLoadRowCurPos;
	int _numSaveGameRows;
	bool _saveDialogFlag;
	bool _saveOrLoad;
	bool _saveLoadEdit;

	byte _saveLoadType, _saveLoadSlot;
	char _saveLoadName[108];

	int _sdlMouseX, _sdlMouseY;

	byte *_backGroundBuf;
	byte *_frontBuf;
	byte *_backBuf;
	byte *_scaleBuf;

	Common::RandomSource _rnd;

	const byte *_vc10BasePtrOld;
	byte _hebrewCharWidths[32];

public:
	SimonEngine(OSystem *syst);
	virtual ~SimonEngine();

protected:
	uint16 to16Wrapper(uint value);
	uint16 readUint16Wrapper(const void *src);
	uint32 readUint32Wrapper(const void *src);

	int allocGamePcVars(Common::File *in);
	void setUserFlag(Item *item, int a, int b);
	void createPlayer();
	void allocateStringTable(int num);
	void setupStringTable(byte *mem, int num);
	void setupLocalStringTable(byte *mem, int num);
	void readGamePcText(Common::File *in);
	void readItemChildren(Common::File *in, Item *item, uint tmp);
	void readItemFromGamePc(Common::File *in, Item *item);
	void loadGamePcFile();
	void decompressData(const char *srcName, byte *dst, uint32 offset, uint32 srcSize, uint32 dstSize);
	void loadOffsets(const char *filename, int number, uint32 &file, uint32 &offset, uint32 &compressedSize, uint32 &size);
	void loadSound(uint sound, int pan, int vol, uint type);
	void loadVoice(uint speechId);

	void paletteFadeOut(byte *palPtr, uint num, uint size);
	
	byte *allocateItem(uint size);
	byte *allocateTable(uint size);
	void alignTableMem();

	Child *findChildOfType(Item *i, uint child);
	Child *allocateChildBlock(Item *i, uint type, uint size);

	void allocItemHeap();
	void allocTablesHeap();

	Subroutine *createSubroutine(uint a);
	void readSubroutine(Common::File *in, Subroutine *sub);
	SubroutineLine *createSubroutineLine(Subroutine *sub, int a);
	void readSubroutineLine(Common::File *in, SubroutineLine *new_table, Subroutine *sub);
	byte *readSingleOpcode(Common::File *in, byte *ptr);
	void readSubroutineBlock(Common::File *in);

	Subroutine *getSubroutineByID(uint subroutine_id);

	/* used in debugger */
	void dumpSubroutines();
	void dumpSubroutine(Subroutine *sub);
	void dumpSubroutineLine(SubroutineLine *sl, Subroutine *sub);
	const byte *dumpOpcode(const byte *p);

	int startSubroutine(Subroutine *sub);
	int startSubroutineEx(Subroutine *sub);

	bool checkIfToRunSubroutineLine(SubroutineLine *sl, Subroutine *sub);

	int runScript();

	Item *getNextItemPtr();
	uint getNextItemID();
	uint getItem1ID() {return 1;}
	Item *me();
	Item *actor();

	byte getByte();
	int getNextWord();

	uint getNextVarContents();
	uint getVarOrWord();
	uint getVarOrByte();
	uint readVariable(uint variable);
	void writeNextVarContents(uint16 contents);
	void writeVariable(uint variable, uint16 contents);

	void setItemParent(Item *item, Item *parent);

	uint itemPtrToID(Item *id);

	Item *derefItem(uint item);
	void setItemState(Item *item, int value);

	void showMessageFormat(const char *s, ...);
	const byte *getStringPtrByID(uint stringId);
	const byte *getLocalStringByID(uint stringId);
	uint getNextStringID();

	void addTimeEvent(uint timeout, uint subroutine_id);
	void delTimeEvent(TimeEvent *te);

	bool isRoom(Item *item);
	bool isObject(Item *item);

	void itemChildrenChanged(Item *item);
	void unlinkItem(Item *item);
	void linkItem(Item *item, Item *parent);

	void stopAnimateSimon1(uint a);
	void stopAnimateSimon2(uint a, uint b);

	void enableBox(uint hitarea);
	void disableBox(uint hitarea);
	void moveBox(uint hitarea, int x, int y);
	bool isBoxDead(uint hitarea);
	void undefineBox(uint hitarea);
	void defineBox(int id, int x, int y, int width, int height, int flags, int verb, Item *item_ptr);
	HitArea *findEmptyHitArea();

	void resetVerbs();
	void setVerb(HitArea * ha);
	void hitarea_leave(HitArea * ha, bool state = false);
	void leaveHitAreaById(uint hitarea_id);

	void sendSync(uint a);
	void waitForSync(uint a);

	uint getOffsetOfChild2Param(SubObject *child, uint prop);
	void setTextColor(uint color);
	void scriptMouseOn();
	void scriptMouseOff();
	void freezeBottom();
	void unfreezeBottom();

	TextLocation *getTextLocation(uint a);
	void setup_cond_c_helper();

	void checkLinkBox();
 	void hyperLinkOn(uint16 x);
 	void hyperLinkOff();
	void linksUp();
	void linksDown();
	void oracleTextUp();
	void oracleTextDown();
	void listSaveGames(int n);
	void saveUserGame(int slot);
	void windowBackSpace(WindowBlock *window);

	void oracleLogo();
	void scrollOracle();
	void scrollOracleUp();
	void scrollOracleDown();
	void swapCharacterLogo();

	void mouseOff();
	void mouseOn();

	void loadTextIntoMem(uint stringId);
	void loadTablesIntoMem(uint subr_id);

	uint loadTextFile(const char *filename, byte *dst);
	Common::File *openTablesFile(const char *filename);
	void closeTablesFile(Common::File *in);

	uint loadTextFile_simon1(const char *filename, byte *dst);
	Common::File *openTablesFile_simon1(const char *filename);

	uint loadTextFile_gme(const char *filename, byte *dst);
	Common::File *openTablesFile_gme(const char *filename);

	void invokeTimeEvent(TimeEvent *te);
	bool kickoffTimeEvents();
	void killAllTimers();

	void endCutscene();
	void runSubroutine101();

	void checkUp(WindowBlock *window);
	void checkDown(WindowBlock *window);
	void inventoryUp(WindowBlock *window);
	void inventoryDown(WindowBlock *window);

	WindowBlock *openWindow(uint x, uint y, uint w, uint h, uint flags, uint fill_color, uint text_color);
	uint getWindowNum(WindowBlock *window);
	void clearWindow(WindowBlock *window);
	void changeWindow(uint a);
	void closeWindow(uint a);
	void windowPutChar(WindowBlock *window, byte c, byte b = 0);

	HitArea *findBox(uint hitarea_id);
	void boxController(uint x, uint y, uint mode);
	void handleVerbClicked(uint verb);
	void clearName();
	void displayName(HitArea * ha);
	void resetNameWindow();
	void displayBoxStars();
	void hitarea_stuff();
	void invertBox_FF(HitArea *ha, bool state);
	void invertBox(HitArea * ha, byte a, byte b, byte c, byte d);

	void handleMouseMoved();
	void pollMouseXY();
	void drawMousePointer();
	void drawMousePointer_FF();
	void drawMousePart(int image, byte x, byte y);

	void defineArrowBoxes(WindowBlock *window);
	void removeArrows(WindowBlock *window, uint num);

	void draw_icon_c(WindowBlock *window, uint icon, uint x, uint y);
	bool has_item_childflag_0x10(Item *item);
	uint itemGetIconNumber(Item *item);
	uint setupIconHitArea(WindowBlock *window, uint num, uint x, uint y, Item *item_ptr);
	void drawIconArray(uint i, Item *item_ptr, int line, int classMask);
	void drawIconArray_FF(uint i, Item *item_ptr, int line, int classMask);
	void drawIconArray_Simon(uint i, Item *item_ptr, int line, int classMask);
	void removeIconArray(uint num);

	void loadIconData();	
	void loadIconFile();

	void processSpecialKeys();
	void hitarea_stuff_helper();

	void permitInput();

	uint getFeebleFontSize(byte chr);
	void showmessage_helper_3(uint a, uint b);
	void showmessage_print_char(byte chr);

	void set_video_mode_internal(uint16 mode, uint16 vga_res_id);

	void loadZone(uint vga_res);

	void loadSprite(uint windowNum, uint vga_res, uint vga_sprite_id, uint x, uint y, uint palette);
	void playSpeech(uint speech_id, uint vga_sprite_id);
	void skipSpeech();

	bool printNameOf(Item *item, uint x, uint y);
	bool printTextOf(uint a, uint x, uint y);
	void printVerbOf(uint hitarea_id);
	void showActionString(const byte *string);

	void printScreenText(uint vga_sprite_id, uint color, const char *string_ptr, int16 x, int16 y, int16 width);
	void sendInteractText(uint16 num, const char *fmt, ...);
	void printInteractText(uint16 num, const char *string);

	void renderStringAmiga(uint vga_sprite_id, uint color, uint width, uint height, const char *txt);
	void renderString(uint vga_sprite_id, uint color, uint width, uint height, const char *txt);

	byte *allocBlock(uint32 size);
	void checkNoOverWrite(byte *end);
	void checkRunningAnims(byte *end);
	void checkAnims(uint a, byte *end);
	void checkZonePtrs(byte *end);
	void setZoneBuffers();

	void runVgaScript();

public:
	bool getBitFlag(uint bit);
	void setBitFlag(uint bit, bool value);

	// Simon1/Simon2 video script opcodes
	void vc1_fadeOut();
	void vc2_call();
	void vc3_loadSprite();
	void vc4_fadeIn();
	void vc5_skip_if_neq();
	void vc6_skip_ifn_sib_with_a();
	void vc7_skip_if_sib_with_a();
	void vc8_skip_if_parent_is();
	void vc9_skip_if_unk3_is();
	void vc10_draw();
	void vc11_clearPathFinder();
	void vc12_delay();
	void vc13_addToSpriteX();
	void vc14_addToSpriteY();
	void vc15_sync();
	void vc16_waitSync();
	void vc17_setPathfinderItem();
	void vc18_jump();
	void vc19_chain_to_script();
	void vc20_setRepeat();
	void vc21_endRepeat();
	void vc22_setSpritePalette();
	void vc23_setSpritePriority();
	void vc24_setSpriteXY();
	void vc25_halt_sprite();
	void vc26_setSubWindow();
	void vc27_resetSprite();
	void vc28_dummy_op();
	void vc29_stopAllSounds();
	void vc30_setFrameRate();
	void vc31_setWindow();
	void vc32_copyVar();
	void vc33_setMouseOn();
	void vc34_setMouseOff();
	void vc35_clearWindow();
	void vc36_setWindowImage();
	void vc37_addToSpriteY();
	void vc38_skipIfVarZero();
	void vc39_setVar();
	void vc40();
	void vc41();
	void vc42_delayIfNotEQ();
	void vc43_skipIfBitClear();
	void vc44_skipIfBitSet();
	void vc45_setSpriteX();
	void vc46_setSpriteY();
	void vc47_addToVar();
	void vc48_setPathFinder();
	void vc49_setBit();
	void vc50_clearBit();
	void vc51_enableBox();
	void vc52_playSound();
	void vc53_panSFX();
	void vc54_no_op();
	void vc55_moveBox();
	void vc56_delay();
	void vc57_no_op();
	void vc58();
	void vc59();
	void vc60_killSprite();
	void vc61_setMaskImage();
	void vc62_fastFadeOut();
	void vc63_fastFadeIn();

	// Simon2 specific Video Script Opcodes
	void vc64_skipIfSpeechEnded();
	void vc65_slowFadeIn();
	void vc66_skipIfNotEqual();
	void vc67_skipIfGE();
	void vc68_skipIfLE();
	void vc69_playTrack();
	void vc70_queueMusic();
	void vc71_checkMusicQueue();
	void vc72_play_track_2();
	void vc73_setMark();
	void vc74_clearMark();

	// Feeble specific Video Script Opcodes
	void vc75_setScale();
	void vc76_setScaleXOffs();
	void vc77_setScaleYOffs();
	void vc78_computeXY();
	void vc79_computePosNum();
	void vc80_setOverlayImage();
	void vc81_setRandom();
	void vc82_getPathValue();
	void vc83_playSoundLoop();
	void vc84_stopSoundLoop();

	void setScriptCondition(bool cond);
	bool getScriptCondition();
	void setScriptReturn(int ret);
	int getScriptReturn();

	// Opcodes, Simon 1 and later
	void o_at();
	void o_notAt();
	void o_carried();
	void o_notCarried();
	void o_isAt();
	void o_zero();
	void o_notZero();
	void o_eq();
	void o_notEq();
	void o_gt();
	void o_lt();
	void o_eqf();
	void o_notEqf();
	void o_ltf();
	void o_gtf();
	void o_chance();
	void o_isRoom();
	void o_isObject();
	void o_state();
	void o_oflag();
	void o_destroy();
	void o_place();
	void o_copyff();
	void o_clear();
	void o_let();
	void o_add();
	void o_sub();
	void o_addf();
	void o_subf();
	void o_mul();
	void o_div();
	void o_mulf();
	void o_divf();
	void o_mod();
	void o_modf();
	void o_random();
	void o_goto();
	void o_oset();
	void o_oclear();
	void o_putBy();
	void o_inc();
	void o_dec();
	void o_setState();
	void o_print();
	void o_message();
	void o_msg();
	void o_addTextBox();
	void o_setShortText();
	void o_setLongText();
	void o_end();
	void o_done();
	void o_process();
	void o_when();
	void o_if1();
	void o_if2();
	void o_isCalled();
	void o_is();
	void o_debug();
	void o_comment();
	void o_haltAnimation();
	void o_restartAnimation();
	void o_getParent();
	void o_getNext();
	void o_getChildren();
	void o_picture();
	void o_loadZone();
	void o_killAnimate();
	void o_defWindow();
	void o_window();
	void o_cls();
	void o_closeWindow();
	void o_addBox();
	void o_delBox();
	void o_enableBox();
	void o_disableBox();
	void o_moveBox();
	void o_doIcons();
	void o_isClass();
	void o_setClass();
	void o_unsetClass();
	void o_waitSync();
	void o_sync();
	void o_defObj();
	void o_here();
	void o_doClassIcons();
	void o_waitEndTune();
	void o_ifEndTune();
	void o_setAdjNoun();
	void o_saveUserGame();
	void o_loadUserGame();
	void o_stopTune();
	void o_pauseGame();
	void o_copysf();
	void o_restoreIcons();
	void o_freezeZones();
	void o_placeNoIcons();
	void o_clearTimers();
	void o_setDollar();
	void o_isBox();
	void o_doTable();
	void o_storeItem();
	void o_getItem();
	void o_bSet();
	void o_bClear();
	void o_bZero();
	void o_bNotZero();
	void o_getOValue();
	void o_setOValue();
	void o_ink();
	void o_screenTextBox();
	void o_screenTextMsg();
	void o_playEffect();
	void o_getDollar2();
	void o_isAdjNoun();
	void o_b2Set();
	void o_b2Clear();
	void o_b2Zero();
	void o_b2NotZero();
	void o_lockZones();
	void o_unlockZones();
	void o_getPathPosn();
	void o_scnTxtLongText();
	void o_mouseOn();
	void o_unloadZone();
	void o_unfreezeZones();

	// Opcodes, Simon 1 only
	void o1_printLongText();
	void o1_rescan();
	void o1_animate();
	void o1_stopAnimate();
	void o1_playTune();
	void o1_screenTextPObj();
	void o1_mouseOff();
	void o1_loadBeard();
	void o1_unloadBeard();
	void o1_loadStrings();
	void o1_specialFade();

	// Opcodes, Simon 2 and later
	void o2_printLongText();
	void o2_rescan();
	void o2_animate();
	void o2_stopAnimate();
	void o2_playTune();
	void o2_screenTextPObj();
	void o2_mouseOff();
	void o2_isShortText();
	void o2_clearMarks();
	void o2_waitMark();

	// Opcodes, Feeble Files only
	void o3_chance();
	void o3_jumpOut();
	void o3_addTextBox();
	void o3_printLongText();
	void o3_addBox();
	void o3_oracleTextDown();
	void o3_oracleTextUp();
	void o3_ifTime();
	void o3_playTune();
	void o3_setTime();
	void o3_saveUserGame();
	void o3_loadUserGame();
	void o3_listSaveGames();
	void o3_checkCD();
	void o3_screenTextBox();
	void o3_isAdjNoun();
	void o3_hyperLinkOn();
	void o3_hyperLinkOff();
	void o3_checkPaths();
	void o3_screenTextPObj();
	void o3_mouseOff();
	void o3_loadVideo();
	void o3_playVideo();
	void o3_centreScroll();
	void o3_resetPVCount();
	void o3_setPathValues();
	void o3_stopClock();
	void o3_restartClock();
	void o3_setColour();
	void o3_b3Set();
	void o3_b3Clear();
	void o3_b3Zero();
	void o3_b3NotZero();

protected:
	void drawImages(VC10_state *state);
	void drawImages_Feeble(VC10_state *state);
	bool drawImages_clip(VC10_state *state);
	void scaleClip(int16 h, int16 w, int16 y, int16 x, int16 scrollY);
	void horizontalScroll(VC10_state *state);
	void verticalScroll(VC10_state *state);

	int vcReadVarOrWord();
	uint vcReadNextWord();
	uint vcReadNextByte();
	uint vcReadVar(uint var);
	void vcWriteVar(uint var, int16 value);
	void vcSkipNextInstruction();

	int getScale(int16 y, int16 x);
	void checkScrollX(int16 x, int16 xpos);
	void checkScrollY(int16 y, int16 ypos);
	void centreScroll();

	bool itemIsSiblingOf(uint16 val);
	bool itemIsParentOf(uint16 a, uint16 b);
	bool vc_maybe_skip_proc_1(uint16 a, int16 b);

	void addVgaEvent(uint16 num, const byte *code_ptr, uint16 cur_sprite, uint16 curZoneNum, int32 param = 0);
	void deleteVgaEvent(VgaTimerEntry * vte);
	void processVgaEvents();
	void animateEvent(const byte *code_ptr, uint16 curZoneNum, uint16 cur_sprite);
	void panEvent(uint16 curZoneNum, uint16 cur_sprite, int32 param);
	void scrollEvent();

	VgaSprite *findCurSprite();

	bool isSpriteLoaded(uint16 id, uint16 zoneNum);

	void resetWindow(WindowBlock *window);
	void delete_hitarea_by_index(uint index);

	void windowPutChar(uint a);

	void restoreWindow(WindowBlock *window);
	void colorWindow(WindowBlock *window);

	void restoreBlock(uint h, uint w, uint y, uint x);

	byte *getFrontBuf();
	byte *getBackBuf();
	byte *getBackGround();
	byte *getScaleBuf();

	byte *loadVGAFile(uint id, uint type, uint32 &dstSize);
	void loadSimonVGAFile(uint vga_id);

	int init();
	int go();

	void openGameFile();
	void readGameFile(void *dst, uint32 offs, uint32 size);

	void timer_callback();
	void timer_proc1();

	void animateSprites();
	void animateSpritesDebug();
	void animateSpritesByY();

	void dx_clear_surfaces(uint num_lines);
	void dx_update_screen_and_palette();

	void dump_video_script(const byte *src, bool one_opcode_only);
	void dump_vga_file(const byte *vga);
	void dump_vga_script(const byte *ptr, uint res, uint sprite_id);
	void dump_vga_script_always(const byte *ptr, uint res, uint sprite_id);
	void dump_vga_bitmaps(const byte *vga, byte *vga1, int res);
	void dump_single_bitmap(int file, int image, const byte *offs, int w, int h, byte base);
	void dump_bitmap(const char *filename, const byte *offs, int w, int h, int flags, const byte *palette, byte base);

	void clearBackFromTop(uint lines);
	void fillFrontFromBack(uint x, uint y, uint w, uint h);
	void fillBackGroundFromBack(uint lines);
	void fillBackFromFront(uint x, uint y, uint w, uint h);

	void print_char_helper_1(const byte *src, uint len);
	void print_char_helper_5(WindowBlock *window);

	void quickLoadOrSave();
	void shutdown();

	byte *vc10_uncompressFlip(const byte *src, uint w, uint h);
	byte *vc10_flip(const byte *src, uint w, uint h);

	Item *getNextItemPtrStrange();

	bool saveGame(uint slot, char *caption);
	bool loadGame(uint slot);

	void openTextWindow();
	void tidyIconArray(uint i);

	void video_putchar_newline(WindowBlock *window);
	void video_putchar_drawchar(WindowBlock *window, uint x, uint y, byte chr);

	void loadMusic(uint music);
	void checkTimerCallback();
	void delay(uint delay);
	void pause();

	void waitForMark(uint i);
	void scrollScreen();

	void decodeColumn(byte *dst, const byte *src, int height);
	void decodeRow(byte *dst, const byte *src, int width);
	void hitarea_stuff_helper_2();
	void fastFadeIn();
	void slowFadeIn();

	void vc_kill_sprite(uint file, uint sprite);

	void set_dummy_cursor();

	void set_volume(int volume);

	void userGame(bool load);
	void disableFileBoxes();
	int userGameGetKey(bool *b, char *buf);
	void userGameBackSpace(WindowBlock *window, int x, byte b = 0);
	void listSaveGames(char *buf);
	void fileError(WindowBlock *window, bool save_error);

	int countSaveGames();
	int displaySaveGameList(int curpos, bool load, char *dst);

	char *genSaveName(int slot);
};

} // End of namespace Simon

#endif
