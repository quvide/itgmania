#ifndef MUSIC_WHEEL_ITEM_H
#define MUSIC_WHEEL_ITEM_H

#include <string>

#include "AutoActor.h"
#include "Difficulty.h"
#include "GameConstantsAndTypes.h"
#include "BitmapText.h"
#include "GameCommand.h"
#include "Group.h"
#include "MessageManager.h"
#include "PlayerNumber.h"
#include "RageTypes.h"
#include "RageUtil_AutoPtr.h"
#include "Song.h"
#include "TextBanner.h"
#include "ThemeMetric.h"
#include "WheelItemBase.h"
#include "WheelNotifyIcon.h"

class Course;
class Song;

struct MusicWheelItemData;

enum MusicWheelItemType {
  MusicWheelItemType_Song,
  MusicWheelItemType_ParentExpanded,
  MusicWheelItemType_ParentCollapsed,
  MusicWheelItemType_SectionExpanded,
  MusicWheelItemType_SectionCollapsed,
  MusicWheelItemType_Roulette,
  MusicWheelItemType_Course,
  MusicWheelItemType_Sort,
  MusicWheelItemType_Mode,
  MusicWheelItemType_Random,
  MusicWheelItemType_Portal,
  MusicWheelItemType_Custom,
  NUM_MusicWheelItemType,
  MusicWheelItemType_Invalid,
};
const std::string& MusicWheelItemTypeToString(MusicWheelItemType i);
/** @brief An item on the MusicWheel. */
class MusicWheelItem : public WheelItemBase {
 public:
  MusicWheelItem(std::string sType = "MusicWheelItem");
  MusicWheelItem(const MusicWheelItem& cpy);
  virtual ~MusicWheelItem();
  virtual MusicWheelItem* Copy() const { return new MusicWheelItem(*this); }

  virtual void LoadFromWheelItemData(
      const WheelItemBaseData* pWID, int iIndex, bool bHasFocus,
      int iDrawIndex);
  virtual void Update(float fDeltaTime);
  virtual void HandleMessage(const Message& msg);
  void RefreshGrades();

 private:
  // Re-send "Set" to every child and refresh the grade displays from the
  // current GAMESTATE (steps, difficulty, profile). Deferred to Update() so
  // that the several broadcasts one wheel step produces (steps and trail for
  // each player) cost one refresh per frame instead of one each.
  void RefreshFromGameState();
  // HandleMessage(msg) for the "Set" message, with per-child profiling.
  void DispatchSet(const Message& msg);
  // What the item's contents depend on besides its own song: each player's
  // selected difficulty and steps type (the same derivation RefreshGrades
  // uses). Recorded at every full Set so a queued refresh can be skipped when
  // none of it changed, which is the case on an ordinary scroll step.
  void GetPlayerSelectionKey(PlayerNumber p, Difficulty& dc, StepsType& st) const;
  void RecordSelectionKeys();
  bool m_bRefreshPending = false;
  bool m_bForceRefresh = false;
  bool m_bHasFocus = false;
  Difficulty m_LastDifficulty[NUM_PLAYERS];
  StepsType m_LastStepsType[NUM_PLAYERS];
  ThemeMetric<bool> GRADES_SHOW_MACHINE;

  AutoActor m_sprColorPart[NUM_MusicWheelItemType];
  AutoActor m_sprNormalPart[NUM_MusicWheelItemType];
  AutoActor m_sprOverPart[NUM_MusicWheelItemType];

  TextBanner m_TextBanner;  // used by Type_Song instead of m_pText
  BitmapText* m_pText[NUM_MusicWheelItemType];
  BitmapText* m_pTextSectionCount;

  WheelNotifyIcon m_WheelNotifyIcon;
  AutoActor m_pGradeDisplay[NUM_PLAYERS];
};

struct MusicWheelItemData : public WheelItemBaseData {
  MusicWheelItemData()
      : m_pCourse(nullptr),
        m_pSong(nullptr),
        m_pGroup(nullptr),
        m_Flags(),
        m_iSectionCount(0),
        m_sParentSection(""),
        m_sLabel(""),
        m_pAction() {}
  MusicWheelItemData(
      WheelItemDataType type, Song* pSong, std::string sSectionName,
      Course* pCourse, Group* pGroup, RageColor color, int iSectionCount,
      std::string sParentSection = "");

  Course* m_pCourse;
  Song* m_pSong;
  Group* m_pGroup;
  WheelNotifyIcon::Flags m_Flags;

  // for TYPE_SECTION
  int m_iSectionCount;
  std::string m_sParentSection;

  // for TYPE_SORT
  std::string m_sLabel;
  HiddenPtr<GameCommand> m_pAction;
};

#endif

/**
 * @file
 * @author Chris Danford, Chris Gomez, Glenn Maynard (c) 2001-2004
 * @section LICENSE
 * All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, and/or sell copies of the Software, and to permit persons to
 * whom the Software is furnished to do so, provided that the above
 * copyright notice(s) and this permission notice appear in all copies of
 * the Software and that both the above copyright notice(s) and this
 * permission notice appear in supporting documentation.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT OF
 * THIRD PARTY RIGHTS. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR HOLDERS
 * INCLUDED IN THIS NOTICE BE LIABLE FOR ANY CLAIM, OR ANY SPECIAL INDIRECT
 * OR CONSEQUENTIAL DAMAGES, OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
 * OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */
