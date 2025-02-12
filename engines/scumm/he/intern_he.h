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

#ifndef SCUMM_INTERN_HE_H
#define SCUMM_INTERN_HE_H

#include "scumm/intern.h"
#ifndef DISABLE_HE
#include "scumm/he/floodfill_he.h"
#include "scumm/he/wiz_he.h"
#endif

namespace Common {
class SeekableReadStream;
class WriteStream;
}

namespace Scumm {

#ifndef DISABLE_HE
class ResExtractor;
class LogicHE;
class MoviePlayer;
class Sprite;
#endif

class ScummEngine_v60he : public ScummEngine_v6 {
protected:
	typedef void (ScummEngine_v60he::*OpcodeProcv60he)();
	struct OpcodeEntryv60he {
		OpcodeProcv60he proc;
		const char *desc;
	};

	const OpcodeEntryv60he *_opcodesv60he;

public:
	//Common::File _hFileTable[17];
	Common::SeekableReadStream *_hInFileTable[17];
	Common::WriteStream *_hOutFileTable[17];

	int _heTimers[16];
	int getHETimer(int timer);
	void setHETimer(int timer);

public:
	ScummEngine_v60he(OSystem *syst, const DetectorResult &dr);
	~ScummEngine_v60he();

	virtual void resetScumm();

protected:
	virtual void setupOpcodes();
	virtual void executeOpcode(byte i);
	virtual const char *getOpcodeDesc(byte i);

	virtual void saveOrLoad(Serializer *s);

	void localizeArray(int slot, byte scriptSlot);
	void redimArray(int arrayId, int newX, int newY, int d);
	int readFileToArray(int slot, int32 size);
	void writeFileFromArray(int slot, int resID);

	int virtScreenSave(byte *dst, int x1, int y1, int x2, int y2);
	void virtScreenLoad(int resIdx, int x1, int y1, int x2, int y2);

	int convertFilePath(byte *dst);
	virtual void decodeParseString(int a, int b);
	void swapObjects(int object1, int object2);

	/* HE version 60 script opcodes */
	void o60_setState();
	void o60_roomOps();
	void o60_actorOps();
	void o60_kernelSetFunctions();
	void o60_kernelGetFunctions();
	void o60_openFile();
	void o60_closeFile();
	void o60_deleteFile();
	void o60_readFile();
	void o60_rename();
	void o60_writeFile();
	void o60_soundOps();
	void o60_seekFilePos();
	void o60_localizeArrayToScript();
	void o60_redimArray();
	void o60_readFilePos();
};

#ifndef DISABLE_HE
class ScummEngine_v70he : public ScummEngine_v60he {
	friend class ResExtractor;
	friend class Wiz;

protected:
	typedef void (ScummEngine_v70he::*OpcodeProcv70he)();
	struct OpcodeEntryv70he {
		OpcodeProcv70he proc;
		const char *desc;
	};

	const OpcodeEntryv70he *_opcodesv70he;

	ResExtractor *_resExtractor;

	byte *_heV7RoomOffsets;

	int32 _heSndSoundId, _heSndOffset, _heSndChannel, _heSndFlags, _heSndSoundFreq;

	bool _skipProcessActors;

public:
	ScummEngine_v70he(OSystem *syst, const DetectorResult &dr);
	~ScummEngine_v70he();

	Wiz *_wiz;

	byte *heFindResourceData(uint32 tag, byte *ptr);
	byte *heFindResource(uint32 tag, byte *ptr);
	byte *findWrappedBlock(uint32 tag, byte *ptr, int state, bool flagError);

protected:
	virtual void setupOpcodes();
	virtual void executeOpcode(byte i);
	virtual const char *getOpcodeDesc(byte i);

	virtual void setupScummVars();
	virtual void resetScummVars();

	virtual void saveOrLoad(Serializer *s);

	virtual void readRoomsOffsets();
	virtual void readGlobalObjects();
	virtual void readIndexBlock(uint32 blocktype, uint32 itemsize);

	virtual int getActorFromPos(int x, int y);

	int getStringCharWidth(byte chr);
	virtual int setupStringArray(int size);
	void appendSubstring(int dst, int src, int len2, int len);
	void adjustRect(Common::Rect &rect);

	virtual void setCursorFromImg(uint img, uint room, uint imgindex);
	virtual void setDefaultCursor();

	virtual void clearDrawQueues();

	void remapHEPalette(const uint8 *src, uint8 *dst);

	/* HE version 70 script opcodes */
	void o70_startSound();
	void o70_pickupObject();
	void o70_getActorRoom();
	void o70_resourceRoutines();
	void o70_systemOps();
	void o70_kernelSetFunctions();
	void o70_copyString();
	void o70_getStringWidth();
	void o70_getStringLen();
	void o70_appendString();
	void o70_concatString();
	void o70_compareString();
	void o70_isResourceLoaded();
	void o70_readINI();
	void o70_writeINI();
	void o70_getStringLenForWidth();
	void o70_getCharIndexInString();
	void o70_setFilePath();
	void o70_setSystemMessage();
	void o70_polygonOps();
	void o70_polygonHit();

	byte VAR_NUM_SOUND_CHANNELS;
	byte VAR_WIZ_TCOLOR;
};

class ScummEngine_v71he : public ScummEngine_v70he {
public:
	ScummEngine_v71he(OSystem *syst, const DetectorResult &dr);

protected:
	virtual void saveOrLoad(Serializer *s);

	virtual void redrawBGAreas();

	virtual void processActors();
	void preProcessAuxQueue();
	void postProcessAuxQueue();

public:
	/* Actor AuxQueue stuff (HE) */
	AuxBlock _auxBlocks[16];
	uint16 _auxBlocksNum;
	AuxEntry _auxEntries[16];
	uint16 _auxEntriesNum;

	void queueAuxBlock(Actor *a);
	void queueAuxEntry(int actorNum, int subIndex);
};

class ScummEngine_v72he : public ScummEngine_v71he {
protected:
	typedef void (ScummEngine_v72he::*OpcodeProcV72he)();
	struct OpcodeEntryV72he {
		OpcodeProcV72he proc;
		const char *desc;
	};

#if !defined(__GNUC__)
	#pragma START_PACK_STRUCTS
#endif

	struct ArrayHeader {
		int32 type;      //0
		int32 dim1start; //4
		int32 dim1end;   //8
		int32 dim2start; //0C
		int32 dim2end;   //10
		byte data[1];    //14
	} GCC_PACK;

#if !defined(__GNUC__)
	#pragma END_PACK_STRUCTS
#endif

	const OpcodeEntryV72he *_opcodesV72he;

	int _stringLength;
	byte _stringBuffer[4096];

	WizParameters _wizParams;

public:
	ScummEngine_v72he(OSystem *syst, const DetectorResult &dr);

	virtual void resetScumm();

protected:
	virtual void setupOpcodes();
	virtual void executeOpcode(byte i);
	virtual const char *getOpcodeDesc(byte i);

	virtual void setupScummVars();
	virtual void resetScummVars();
	virtual void readArrayFromIndexFile();

	virtual byte *getStringAddress(int i);
	virtual void readMAXS(int blockSize);

	virtual void redrawBGAreas();

	ArrayHeader *defineArray(int array, int type, int dim2start, int dim2end, int dim1start, int dim1end);
	virtual int readArray(int array, int idx2, int idx1);
	virtual void writeArray(int array, int idx2, int idx1, int value);
	void redimArray(int arrayId, int newDim2start, int newDim2end,
					int newDim1start, int newDim1end, int type);
	void checkArrayLimits(int array, int dim2start, int dim2end, int dim1start, int dim1end);
	void copyArray(int array1, int a1_dim2start, int a1_dim2end, int a1_dim1start, int a1_dim1end,
					int array2, int a2_dim2start, int a2_dim2end, int a2_dim1start, int a2_dim1end);
	void copyArrayHelper(ArrayHeader *ah, int idx2, int idx1, int len1, byte **data, int *size, int *num);
	virtual int setupStringArray(int size);
	int readFileToArray(int slot, int32 size);
	void writeFileFromArray(int slot, int32 resID);

	virtual void decodeParseString(int a, int b);
	void decodeScriptString(byte *dst, bool scriptString = false);
	void copyScriptString(byte *dst, int dstSize);

	int findObject(int x, int y, int num, int *args);
	int getSoundResourceSize(int id);

	virtual bool handleNextCharsetCode(Actor *a, int *c);

	/* HE version 72 script opcodes */
	void o72_pushDWord();
	void o72_getScriptString();
	void o72_isAnyOf();
	void o72_resetCutscene();
	void o72_findObjectWithClassOf();
	void o72_getObjectImageX();
	void o72_getObjectImageY();
	void o72_captureWizImage();
	void o72_getTimer();
	void o72_setTimer();
	void o72_getSoundPosition();
	void o72_startScript();
	void o72_startObject();
	void o72_drawObject();
	void o72_printWizImage();
	void o72_getArrayDimSize();
	void o72_getNumFreeArrays();
	void o72_roomOps();
	void o72_actorOps();
	void o72_verbOps();
	void o72_findObject();
	void o72_arrayOps();
	void o72_systemOps();
	void o72_talkActor();
	void o72_talkEgo();
	void o72_dimArray();
	void o72_dim2dimArray();
	void o72_traceStatus();
	void o72_debugInput();
	void o72_drawWizImage();
	void o72_kernelGetFunctions();
	void o72_jumpToScript();
	void o72_openFile();
	void o72_readFile();
	void o72_writeFile();
	void o72_findAllObjects();
	void o72_deleteFile();
	void o72_rename();
	void o72_getPixel();
	void o72_pickVarRandom();
	void o72_redimArray();
	void o72_readINI();
	void o72_writeINI();
	void o72_getResourceSize();
	void o72_setFilePath();
	void o72_setSystemMessage();

	byte VAR_NUM_ROOMS;
	byte VAR_NUM_SCRIPTS;
	byte VAR_NUM_SOUNDS;
	byte VAR_NUM_COSTUMES;
	byte VAR_NUM_IMAGES;
	byte VAR_NUM_CHARSETS;

	byte VAR_POLYGONS_ONLY;
};

class ScummEngine_v80he : public ScummEngine_v72he {
protected:
	typedef void (ScummEngine_v80he::*OpcodeProcV80he)();
	struct OpcodeEntryV80he {
		OpcodeProcV80he proc;
		const char *desc;
	};

	const OpcodeEntryV80he *_opcodesV80he;

	int32 _heSndResId, _curSndId, _sndPtrOffs, _sndTmrOffs;

public:
	ScummEngine_v80he(OSystem *syst, const DetectorResult &dr);

protected:
	virtual void setupOpcodes();
	virtual void executeOpcode(byte i);
	virtual const char *getOpcodeDesc(byte i);

	virtual void setupScummVars();
	virtual void resetScummVars();

	virtual void initCharset(int charset);

	virtual void clearDrawQueues();

	void createSound(int snd1id, int snd2id);

	void drawLine(int x1, int y1, int x, int unk1, int unk2, int type, int id);
	void drawPixel(int x, int y, int flags);

	/* HE version 80 script opcodes */
	void o80_createSound();
	void o80_getFileSize();
	void o80_stringToInt();
	void o80_getSoundVar();
	void o80_localizeArrayToRoom();
	void o80_sourceDebug();
	void o80_readConfigFile();
	void o80_writeConfigFile();
	void o80_cursorCommand();
	void o80_setState();
	void o80_drawWizPolygon();
	void o80_drawLine();
	void o80_pickVarRandom();

	byte VAR_PLATFORM;
	byte VAR_WINDOWS_VERSION;
	byte VAR_CURRENT_CHARSET;
	byte VAR_COLOR_DEPTH;
};

class ScummEngine_v90he : public ScummEngine_v80he {
	friend class LogicHE;
	friend class MoviePlayer;
	friend class Sprite;

protected:
	typedef void (ScummEngine_v90he::*OpcodeProcV90he)();
	struct OpcodeEntryV90he {
		OpcodeProcV90he proc;
		const char *desc;
	};

	const OpcodeEntryV90he *_opcodesV90he;

	FloodFillParameters _floodFillParams;

	struct VideoParameters {
		byte filename[260];
		int32 status;
		int32 flags;
		int32 unk2;
		int32 wizResNum;
	};

	VideoParameters _videoParams;

	int32 _heObject, _heObjectNum;
	int32 _hePaletteNum;

	int32 _curMaxSpriteId;
	int32 _curSpriteId;
	int32 _curSpriteGroupId;

public:
	ScummEngine_v90he(OSystem *syst, const DetectorResult &dr);
	~ScummEngine_v90he();

	virtual void resetScumm();

	LogicHE *_logicHE;
	MoviePlayer *_moviePlay;
	Sprite *_sprite;

protected:
	virtual void allocateArrays();
	virtual void setupOpcodes();
	virtual void executeOpcode(byte i);
	virtual const char *getOpcodeDesc(byte i);

	virtual void scummLoop_handleDrawing();

	virtual void setupScummVars();
	virtual void resetScummVars();

	virtual void saveOrLoad(Serializer *s);

	virtual void readMAXS(int blockSize);

	virtual void processActors();

	int computeWizHistogram(int resnum, int state, int x, int y, int w, int h);
	void getArrayDim(int array, int *dim2start, int *dim2end, int *dim1start, int *dim1end);
	void sortArray(int array, int dim2start, int dim2end, int dim1start, int dim1end, int sortOrder);

public:
	int getGroupSpriteArray(int spriteGroupId);

protected:
	uint8 *getHEPaletteIndex(int palSlot);
	int getHEPaletteColor(int palSlot, int color);
	int getHEPaletteSimilarColor(int palSlot, int red, int green, int start, int end);
	int getHEPaletteColorComponent(int palSlot, int color, int component);
	void setHEPaletteColor(int palSlot, uint8 color, uint8 r, uint8 g, uint8 b);
	void setHEPaletteFromPtr(int palSlot, const uint8 *palData);
	void setHEPaletteFromCostume(int palSlot, int resId);
	void setHEPaletteFromImage(int palSlot, int resId, int state);
	void setHEPaletteFromRoom(int palSlot, int resId, int state);
	void restoreHEPalette(int palSlot);
	void copyHEPalette(int dstPalSlot, int srcPalSlot);
	void copyHEPaletteColor(int palSlot, uint8 dstColor, uint8 srcColor);

protected:
	/* HE version 90 script opcodes */
	void o90_dup_n();
	void o90_min();
	void o90_max();
	void o90_sin();
	void o90_cos();
	void o90_sqrt();
	void o90_atan2();
	void o90_getSegmentAngle();
	void o90_getActorData();
	void o90_startScriptUnk();
	void o90_jumpToScriptUnk();
	void o90_videoOps();
	void o90_getVideoData();
	void o90_wizImageOps();
	void o90_getDistanceBetweenPoints();
	void o90_getSpriteInfo();
	void o90_setSpriteInfo();
	void o90_getSpriteGroupInfo();
	void o90_setSpriteGroupInfo();
	void o90_getWizData();
	void o90_floodFill();
	void o90_mod();
	void o90_shl();
	void o90_shr();
	void o90_xor();
	void o90_findAllObjectsWithClassOf();
	void o90_getPolygonOverlap();
	void o90_cond();
	void o90_dim2dim2Array();
	void o90_redim2dimArray();
	void o90_getLinesIntersectionPoint();
	void o90_sortArray();
	void o90_getObjectData();
	void o90_getPaletteData();
	void o90_paletteOps();
	void o90_fontUnk();
	void o90_getActorAnimProgress();
	void o90_kernelGetFunctions();
	void o90_kernelSetFunctions();

	byte VAR_NUM_SPRITE_GROUPS;
	byte VAR_NUM_SPRITES;
	byte VAR_NUM_PALETTES;
	byte VAR_NUM_UNK;

	byte VAR_U32_VERSION;
	byte VAR_U32_ARRAY_UNK;
};

class ScummEngine_v99he : public ScummEngine_v90he {
public:
	ScummEngine_v99he(OSystem *syst, const DetectorResult &dr) : ScummEngine_v90he(syst, dr) {}

	virtual void resetScumm();

protected:
	virtual void resetScummVars();

	virtual void readMAXS(int blockSize);

	virtual void saveOrLoad(Serializer *s);

	virtual void copyPalColor(int dst, int src);
	virtual void darkenPalette(int redScale, int greenScale, int blueScale, int startColor, int endColor);
	virtual void setPaletteFromPtr(const byte *ptr, int numcolor = -1);
	virtual void setPalColor(int index, int r, int g, int b);
	virtual void updatePalette();
};

class ScummEngine_v100he : public ScummEngine_v99he {
protected:
	typedef void (ScummEngine_v100he::*OpcodeProcV100he)();
	struct OpcodeEntryV100he {
		OpcodeProcV100he proc;
		const char *desc;
	};

	int32 _heResId, _heResType;

	const OpcodeEntryV100he *_opcodesV100he;

public:
	ScummEngine_v100he(OSystem *syst, const DetectorResult &dr) : ScummEngine_v99he(syst, dr) {}

protected:
	virtual void setupOpcodes();
	virtual void executeOpcode(byte i);
	virtual const char *getOpcodeDesc(byte i);

	virtual void saveOrLoad(Serializer *s);

	virtual void decodeParseString(int a, int b);

	/* HE version 100 script opcodes */
	void o100_actorOps();
	void o100_arrayOps();
	void o100_dim2dimArray();
	void o100_redim2dimArray();
	void o100_dimArray();
	void o100_drawLine();
	void o100_drawObject();
	void o100_floodFill();
	void o100_setSpriteGroupInfo();
	void o100_resourceRoutines();
	void o100_wizImageOps();
	void o100_jumpToScript();
	void o100_createSound();
	void o100_dim2dim2Array();
	void o100_paletteOps();
	void o100_jumpToScriptUnk();
	void o100_startScriptUnk();
	void o100_redimArray();
	void o100_roomOps();
	void o100_setSystemMessage();
	void o100_startSound();
	void o100_setSpriteInfo();
	void o100_startScript();
	void o100_systemOps();
	void o100_cursorCommand();
	void o100_videoOps();
	void o100_wait();
	void o100_writeFile();
	void o100_isResourceLoaded();
	void o100_getResourceSize();
	void o100_getSpriteGroupInfo();
	void o100_getPaletteData();
	void o100_readFile();
	void o100_getSpriteInfo();
	void o100_getWizData();
	void o100_getVideoData();
};
#endif


} // End of namespace Scumm

#endif
