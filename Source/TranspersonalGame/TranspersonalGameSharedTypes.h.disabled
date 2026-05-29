// TranspersonalGameSharedTypes.h
// Canonical shared type definitions for the TranspersonalGame module.
// All agents MUST use types from this file instead of defining their own.
// Last reorganized: 2026-04-14 (Phase 1 — zero-risk cleanup)
#pragma once

#include "CoreMinimal.h"
#include "Animation/Core/TranspersonalAnimationTypes.h"
#include "TranspersonalGameSharedTypes.generated.h"

// ============================================================
// Section 1: World & Environment Enums
// ============================================================

UENUM(BlueprintType)
enum class EWeatherState : uint8
{
	Clear,
	Cloudy,
	Rain,
	Storm,
	Fog,
	Snow,
	Sandstorm
};

UENUM(BlueprintType)
enum class EPerformanceTarget : uint8 { PC_HighEnd, Console_NextGen, Console_Current, Mobile_High };

// ============================================================
// Section 2: Character Enums
// ============================================================

UENUM(BlueprintType)
enum class ECharacterArchetype : uint8
{
	Protagonist_Paleontologist,
	Paleontologist,
	Ranger,
	Scientist,
	Hunter,
	Guide,
	Survivor,
	Villager,
	Military,
	Corporate,
	Child,
	Elder,
	Warrior,
	Healer,
	Scout,
	Shaman,
	Crafter,
	Explorer,
	Leader,
	Mystic,
	Guardian,
	Trickster
};

UENUM(BlueprintType)
enum class ECharacterGender : uint8
{
	Male,
	Female,
	NonBinary
};

UENUM(BlueprintType)
enum class EBehaviorState : uint8
{
	Idle,
	Active,
	Paused,
	Completed,
	Failed,
	Wandering,
	Hunting,
	Fleeing,
	Socializing
};

// ============================================================
// Section 3: Dinosaur Enums & Structs
// ============================================================

UENUM(BlueprintType)
enum class EDinosaurBehaviorState : uint8 { Idle, Wandering, Grazing, Hunting, Fleeing, Sleeping, Socializing, Aggressive, Territorial, Migrating };

UENUM(BlueprintType)
enum class EDinosaurArchetype : uint8 { Herbivore, Carnivore, Omnivore, Scavenger, Apex, Pack, Solitary };

UENUM(BlueprintType)
enum class EDomesticationLevel : uint8 { Wild, Wary, Neutral, Friendly, Tamed, Bonded };

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FDinosaurMemoryEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName EventType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector Location = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Timestamp = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Importance = 0.5f;
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FDinosaurPersonality
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Aggression = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Curiosity = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Sociability = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Courage = 0.5f;
};

// ============================================================
// Section 4: Quest & Narrative
// ============================================================

UENUM(BlueprintType)
enum class EQuestType : uint8 { MainQuest, SideQuest, DailyQuest, EventQuest, Tutorial };

UENUM(BlueprintType)
enum class EQuestStatus : uint8 { NotStarted, InProgress, Completed, Failed, Abandoned };

UENUM(BlueprintType)
enum class EDialogueContext : uint8 { Normal, Combat, Exploration, Cutscene, Tutorial };

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FDialogueLine
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString SpeakerID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString DialogueText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Duration = 3.0f;
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FStoryBeat
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName BeatID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Description;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Priority = 0;
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuestData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName QuestID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString QuestName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Description;
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuestObjective
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName ObjectiveID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Description;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bCompleted = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CurrentProgress = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 RequiredProgress = 1;
};

// ============================================================
// Section 5: NPC & AI Structs
// ============================================================

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNPCPersonality
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Aggression = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Curiosity = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Sociability = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Courage = 0.5f;
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FDailyRoutineEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float StartHour = 8.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float EndHour = 17.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName ActivityName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector Location = FVector::ZeroVector;
};

// ============================================================
// Section 6: Performance & Technical
// ============================================================

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerformanceBudget
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxTriangleCount = 5000000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxDrawCalls = 2000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxTextureMemoryMB = 2048.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TargetFrameRate = 60.0f;
};

// ============================================================
// Section 7: VFX
// ============================================================

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FVFXEffectData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName EffectName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString EffectPath;
};

UENUM(BlueprintType)
enum class EVFXCategory : uint8
{
	Environmental,
	Combat,
	Creature,
	Weather,
	Destruction
};
