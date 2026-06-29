#include "DialogueSequences.h"
#include "DialogueManager.h"
#include "Engine/DataTable.h"

// ============================================================
// DialogueSequences.cpp — Agent #15 Narrative & Dialogue Agent
//
// This file provides the compiled translation unit for all
// dialogue sequence structs. The structs are DataTable-compatible
// (FTableRowBase) and are loaded at runtime by UNarr_DialogueManager.
//
// Quest Arc Summary:
//   Arc 1 — "First Tools"       (FNarr_ElderKara_FirstTools)
//   Arc 2 — "Survive the Night" (FNarr_ElderKara_SurviveNight)
//   Arc 3 — "Stop the Stampede" (FNarr_ElderKara_Stampede)
//   Tutorial — Crafter Bron     (FNarr_CrafterBron_Tutorial)
//
// Audio URLs (ElevenLabs TTS — Supabase Storage):
//   Elder_Kara_FirstTools:
//     https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1782741460705_Elder_Kara_FirstTools.mp3
//   Elder_Kara_SurviveNight:
//     https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1782741463807_Elder_Kara_SurviveNight.mp3
//   Elder_Kara_StampedeQuest:
//     https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1782741485382_Elder_Kara_StampedeQuest.mp3
//   Crafter_Bron_Tutorial:
//     https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1782741317137_Crafter_Bron_Tutorial.mp3
//
// Integration points:
//   - AQuest_CraftingSystemManager::HasCraftedStoneAxe()  → unlocks Arc 2
//   - AQuest_CraftingSystemManager::HasCraftedCampfire()  → Arc 2 completion
//   - AQuest_CraftingSystemManager::HasCraftedTorch()     → unlocks Arc 3
//   - Agent #13 Crowd Simulation stampede trigger         → Arc 3 start
// ============================================================

// -------------------------------------------------------
// Static helper: Build the default quest dialogue registry
// Called by UNarr_DialogueManager::InitializeDefaultRegistry()
// -------------------------------------------------------
namespace NarrDialogueDefaults
{
	/**
	 * Returns the default registry entries for all quest arcs.
	 * These are loaded into the DialogueManager on BeginPlay.
	 */
	TArray<FNarr_QuestDialogueRegistry> BuildDefaultRegistry()
	{
		TArray<FNarr_QuestDialogueRegistry> Registry;

		// Arc 1 — First Tools
		{
			FNarr_QuestDialogueRegistry Entry;
			Entry.QuestID         = FName("Quest_FirstTools");
			Entry.SpeakerID       = FName("Elder_Kara");
			Entry.SequenceRowName = FName("ElderKara_FirstTools");
			Entry.OpeningTrigger  = ENarr_DialogueTrigger::OnQuestStart;
			Entry.ProximityRadius = 200.0f;
			Entry.RepeatCooldown  = 60.0f;
			Registry.Add(Entry);
		}

		// Arc 2 — Survive the Night
		{
			FNarr_QuestDialogueRegistry Entry;
			Entry.QuestID         = FName("Quest_SurviveNight");
			Entry.SpeakerID       = FName("Elder_Kara");
			Entry.SequenceRowName = FName("ElderKara_SurviveNight");
			Entry.OpeningTrigger  = ENarr_DialogueTrigger::OnDangerNear;
			Entry.ProximityRadius = 500.0f;  // Danger radius — night predators
			Entry.RepeatCooldown  = 120.0f;
			Registry.Add(Entry);
		}

		// Arc 3 — Stop the Stampede
		{
			FNarr_QuestDialogueRegistry Entry;
			Entry.QuestID         = FName("Quest_StopStampede");
			Entry.SpeakerID       = FName("Elder_Kara");
			Entry.SequenceRowName = FName("ElderKara_Stampede");
			Entry.OpeningTrigger  = ENarr_DialogueTrigger::OnQuestStart;
			Entry.ProximityRadius = 300.0f;
			Entry.RepeatCooldown  = 90.0f;
			Registry.Add(Entry);
		}

		// Tutorial — Crafter Bron
		{
			FNarr_QuestDialogueRegistry Entry;
			Entry.QuestID         = FName("Quest_CraftingTutorial");
			Entry.SpeakerID       = FName("Crafter_Bron");
			Entry.SequenceRowName = FName("CrafterBron_Tutorial");
			Entry.OpeningTrigger  = ENarr_DialogueTrigger::OnProximity;
			Entry.ProximityRadius = 150.0f;
			Entry.RepeatCooldown  = 30.0f;
			Registry.Add(Entry);
		}

		return Registry;
	}

	/**
	 * Returns the display name for a speaker ID.
	 * Used by the HUD subtitle system.
	 */
	FText GetSpeakerDisplayName(const FName& SpeakerID)
	{
		static TMap<FName, FText> DisplayNames = {
			{ FName("Elder_Kara"),    FText::FromString(TEXT("Elder Kara"))   },
			{ FName("Crafter_Bron"), FText::FromString(TEXT("Crafter Bron")) },
		};

		const FText* Found = DisplayNames.Find(SpeakerID);
		return Found ? *Found : FText::FromName(SpeakerID);
	}

	/**
	 * Returns the subtitle colour for a speaker role.
	 * Elder Kara = amber (authority), Crafter Bron = grey-green (practical).
	 */
	FLinearColor GetSpeakerSubtitleColour(const FName& SpeakerID)
	{
		if (SpeakerID == FName("Elder_Kara"))
		{
			return FLinearColor(1.0f, 0.75f, 0.1f, 1.0f);  // Amber
		}
		if (SpeakerID == FName("Crafter_Bron"))
		{
			return FLinearColor(0.5f, 0.8f, 0.4f, 1.0f);  // Grey-green
		}
		return FLinearColor::White;
	}

} // namespace NarrDialogueDefaults
