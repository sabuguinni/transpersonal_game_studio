#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "DialogueManager.h"
#include "DialogueSequences.generated.h"

// ============================================================
// DIALOGUE SEQUENCES — Agent #15 Narrative & Dialogue Agent
// Quest arc: "First Tools" → "Survive the Night" → "Stampede"
// All sequences reference audio URLs from ElevenLabs TTS
// ============================================================

// -------------------------------------------------------
// ELDER KARA — Quest Giver NPC
// Location: Camp centre, near the fire pit
// Role: Tribal leader, survival mentor
// -------------------------------------------------------

/**
 * Quest Arc 1: "First Tools"
 * Trigger: OnQuestStart (player enters camp for first time)
 * Completion condition: AQuest_CraftingSystemManager::HasCraftedStoneAxe()
 */
USTRUCT(BlueprintType)
struct FNarr_ElderKara_FirstTools : public FTableRowBase
{
	GENERATED_BODY()

	// Line 1 — Quest intro, spoken on first camp entry
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|ElderKara")
	FText Line1_Text = FText::FromString(
		TEXT("The first tool you make is not just a tool. It is proof that you can survive. "
			 "Take the flint. Strike it against the hard stone. Shape it. "
			 "Bind it to the branch with vine. When you hold that axe, you hold your future. "
			 "Now go — the night comes faster than you think."));

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|ElderKara")
	FString Line1_AudioURL = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1782741460705_Elder_Kara_FirstTools.mp3");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|ElderKara")
	float Line1_Duration = 19.0f;

	// Line 2 — Reminder if player idles near camp without crafting
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|ElderKara")
	FText Line2_Text = FText::FromString(
		TEXT("You are still here? The flint will not shape itself. "
			 "Look around — the rocks near the river edge are the sharpest. "
			 "Bring two pieces back. That is all I ask."));

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|ElderKara")
	float Line2_Duration = 12.0f;

	// Line 3 — Quest complete, stone axe crafted
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|ElderKara")
	FText Line3_Text = FText::FromString(
		TEXT("You made it. A real tool. Now you are not just surviving — "
			 "you are building something. Keep it close. You will need it tonight."));

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|ElderKara")
	float Line3_Duration = 11.0f;
};

/**
 * Quest Arc 2: "Survive the Night"
 * Trigger: OnDangerNear (sun drops below horizon — time-of-day check)
 * Completion condition: Player survives until dawn with campfire active
 * Prerequisite: HasCraftedStoneAxe() == true
 */
USTRUCT(BlueprintType)
struct FNarr_ElderKara_SurviveNight : public FTableRowBase
{
	GENERATED_BODY()

	// Line 1 — Dusk warning, campfire instruction
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|ElderKara")
	FText Line1_Text = FText::FromString(
		TEXT("Three sticks. Dry tinder. That is all you need to push back the dark. "
			 "The campfire is not just warmth — it is the line between you and everything that hunts at night. "
			 "Build it before the sun drops below the ridge. Do not wait until you are already afraid."));

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|ElderKara")
	FString Line1_AudioURL = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1782741463807_Elder_Kara_SurviveNight.mp3");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|ElderKara")
	float Line1_Duration = 17.0f;

	// Line 2 — Midnight danger escalation (if no campfire built)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|ElderKara")
	FText Line2_Text = FText::FromString(
		TEXT("I hear them. The hunters. They circle at night — they smell the dark. "
			 "If you have no fire, stay still. Do not move. Do not breathe loud. "
			 "Wait for the light."));

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|ElderKara")
	float Line2_Duration = 13.0f;

	// Line 3 — Dawn survival confirmation
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|ElderKara")
	FText Line3_Text = FText::FromString(
		TEXT("You made it through. The first night is always the hardest. "
			 "From here, every dawn you see is one you earned."));

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|ElderKara")
	float Line3_Duration = 10.0f;
};

/**
 * Quest Arc 3: "Stop the Stampede"
 * Trigger: OnQuestStart (herd detected moving toward camp)
 * Completion condition: Herd redirected — torch used within 50m of lead animal
 * Prerequisite: HasCraftedTorch() == true
 * References: Agent #13 Crowd Simulation stampede trigger
 */
USTRUCT(BlueprintType)
struct FNarr_ElderKara_Stampede : public FTableRowBase
{
	GENERATED_BODY()

	// Line 1 — Quest briefing, torch strategy
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|ElderKara")
	FText Line1_Text = FText::FromString(
		TEXT("The torch is your weapon against the herd. When the stampede comes, do not run. "
			 "Stand your ground. Hold the flame high. The animals fear fire — they will turn. "
			 "They will scatter. But you must not flinch. "
			 "One step back and the whole herd follows you into camp."));

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|ElderKara")
	FString Line1_AudioURL = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1782741485382_Elder_Kara_StampedeQuest.mp3");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|ElderKara")
	float Line1_Duration = 18.0f;

	// Line 2 — Mid-quest, herd approaching (proximity trigger 200m)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|ElderKara")
	FText Line2_Text = FText::FromString(
		TEXT("They are coming. I can feel the ground shake. "
			 "Get to the front of the herd — cut them off before they reach the valley mouth. "
			 "The torch. Now."));

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|ElderKara")
	float Line2_Duration = 11.0f;

	// Line 3 — Quest complete, herd redirected
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|ElderKara")
	FText Line3_Text = FText::FromString(
		TEXT("They turned. You held the line. "
			 "Remember this — fire is the oldest tool we have. "
			 "Older than the axe. Older than the spear. "
			 "As long as you can make fire, you can survive anything this world throws at you."));

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|ElderKara")
	float Line3_Duration = 16.0f;
};

// -------------------------------------------------------
// CRAFTER BRON — Tutorial NPC
// Location: Crafting station, east of camp
// Role: Artisan, tool-making teacher
// -------------------------------------------------------

/**
 * Crafter Bron — Tutorial dialogue sequence
 * Trigger: OnProximity (player within 150 units of crafting station)
 */
USTRUCT(BlueprintType)
struct FNarr_CrafterBron_Tutorial : public FTableRowBase
{
	GENERATED_BODY()

	// Line 1 — First approach to crafting station
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|CrafterBron")
	FText Line1_Text = FText::FromString(
		TEXT("You found the flint cache. Good. Two sharp stones and a sturdy branch — "
			 "that is all you need for a hand axe. Watch how I hold the stone. "
			 "Strike at an angle. Not straight down — you will split it wrong. "
			 "At an angle. Like this."));

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|CrafterBron")
	FString Line1_AudioURL = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1782741317137_Crafter_Bron_Tutorial.mp3");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|CrafterBron")
	float Line1_Duration = 17.0f;

	// Line 2 — Crafting in progress (shown during 3s craft timer)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|CrafterBron")
	FText Line2_Text = FText::FromString(
		TEXT("Keep striking. The edge will come. You will feel when it is right — "
			 "the stone gets lighter in your hand. That is the sign."));

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|CrafterBron")
	float Line2_Duration = 9.0f;

	// Line 3 — Craft complete
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|CrafterBron")
	FText Line3_Text = FText::FromString(
		TEXT("There. Your first axe. It is rough — but it will cut. "
			 "Come back when you have bone. I will show you how to make a spear tip "
			 "that will stop a raptor mid-charge."));

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|CrafterBron")
	float Line3_Duration = 12.0f;

	// Line 4 — Spear tip unlock (after HasCraftedStoneAxe)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|CrafterBron")
	FText Line4_Text = FText::FromString(
		TEXT("You brought bone. Good. Two sticks, the bone tip, and vine to bind it. "
			 "The spear gives you distance. Distance keeps you alive against the fast ones. "
			 "Never let a raptor get inside your arm's reach."));

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|CrafterBron")
	float Line4_Duration = 14.0f;
};

// -------------------------------------------------------
// QUEST ARC REGISTRY — maps quest IDs to dialogue sequences
// Used by DialogueManager to load correct sequence per quest
// -------------------------------------------------------
USTRUCT(BlueprintType)
struct FNarr_QuestDialogueRegistry : public FTableRowBase
{
	GENERATED_BODY()

	/** Quest identifier — matches FQuest_QuestData.QuestID */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Registry")
	FName QuestID;

	/** NPC speaker for this quest */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Registry")
	FName SpeakerID;

	/** DataTable row name for the dialogue sequence struct */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Registry")
	FName SequenceRowName;

	/** Trigger type for the opening line */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Registry")
	ENarr_DialogueTrigger OpeningTrigger = ENarr_DialogueTrigger::OnQuestStart;

	/** Proximity radius for OnProximity triggers (cm) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Registry")
	float ProximityRadius = 150.0f;

	/** Cooldown between repeat plays (seconds) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Registry")
	float RepeatCooldown = 30.0f;
};
