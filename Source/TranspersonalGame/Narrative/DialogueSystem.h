#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/TriggerBox.h"
#include "DialogueSystem.generated.h"

// ============================================================
// Narrative & Dialogue Agent #15 — DialogueSystem.h
// Dialogue trees for Hunter and Elder NPCs.
// 3 starter quest story beats: FirstHunt, GreatLizard, SurviveNight
// ============================================================

UENUM(BlueprintType)
enum class ENarr_QuestID : uint8
{
    None            UMETA(DisplayName = "None"),
    FirstHunt       UMETA(DisplayName = "First Hunt"),
    GreatLizard     UMETA(DisplayName = "The Great Lizard"),
    SurviveNight    UMETA(DisplayName = "Survive the Night"),
};

UENUM(BlueprintType)
enum class ENarr_DialogueSpeaker : uint8
{
    Player      UMETA(DisplayName = "Player"),
    Hunter      UMETA(DisplayName = "Hunter NPC"),
    Elder       UMETA(DisplayName = "Elder NPC"),
    Narrator    UMETA(DisplayName = "Narrator"),
};

USTRUCT(BlueprintType)
struct FNarr_DialogueLine
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    ENarr_DialogueSpeaker Speaker = ENarr_DialogueSpeaker::Narrator;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FText LineText;

    /** Index of next line. -1 = end of conversation. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    int32 NextLineIndex = -1;

    /** Optional: index of player choice branch (branching dialogue). -1 = linear. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    int32 BranchChoiceIndex = -1;

    /** Audio URL from ElevenLabs TTS */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString AudioURL;
};

USTRUCT(BlueprintType)
struct FNarr_DialogueTree
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    ENarr_QuestID QuestID = ENarr_QuestID::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString TreeID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    TArray<FNarr_DialogueLine> Lines;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    int32 CurrentLineIndex = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    bool bCompleted = false;
};

USTRUCT(BlueprintType)
struct FNarr_StoryBeat
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    ENarr_QuestID QuestID = ENarr_QuestID::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString BeatTitle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FText BeatDescription;

    /** Required crafted item label before this beat unlocks (empty = no requirement) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString RequiredCraftedItem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    bool bUnlocked = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    bool bCompleted = false;
};

// ============================================================
// UNarr_DialogueComponent — attach to NPC actors
// ============================================================
UCLASS(ClassGroup = (Narrative), meta = (BlueprintSpawnableComponent), DisplayName = "Narr Dialogue Component")
class TRANSPERSONALGAME_API UNarr_DialogueComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UNarr_DialogueComponent();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FNarr_DialogueTree DialogueTree;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    ENarr_DialogueSpeaker NPCSpeaker = ENarr_DialogueSpeaker::Hunter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    float InteractionRadius = 300.0f;

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void StartDialogue();

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    bool AdvanceDialogue();

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    FNarr_DialogueLine GetCurrentLine() const;

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    bool IsDialogueActive() const;

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void ResetDialogue();

protected:
    virtual void BeginPlay() override;

private:
    bool bDialogueActive = false;
};

// ============================================================
// ANarr_DialogueTrigger — placed in world, activates dialogue on overlap
// ============================================================
UCLASS(DisplayName = "Narr Dialogue Trigger")
class TRANSPERSONALGAME_API ANarr_DialogueTrigger : public AActor
{
    GENERATED_BODY()

public:
    ANarr_DialogueTrigger();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    ENarr_QuestID LinkedQuestID = ENarr_QuestID::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString TriggerLabel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    bool bOneShot = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FText NarrationText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString AudioURL;

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void ActivateTrigger();

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    bool HasFired() const;

protected:
    virtual void BeginPlay() override;

private:
    bool bHasFired = false;
};

// ============================================================
// UNarr_StoryManager — GameInstance subsystem tracking story state
// ============================================================
UCLASS(DisplayName = "Narr Story Manager")
class TRANSPERSONALGAME_API UNarr_StoryManager : public UObject
{
    GENERATED_BODY()

public:
    UNarr_StoryManager();

    UPROPERTY(BlueprintReadWrite, Category = "Narrative")
    TArray<FNarr_StoryBeat> StoryBeats;

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void InitializeStoryBeats();

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    bool UnlockBeat(ENarr_QuestID QuestID);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    bool CompleteBeat(ENarr_QuestID QuestID);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    FNarr_StoryBeat GetBeat(ENarr_QuestID QuestID) const;

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    int32 GetCompletedBeatCount() const;
};
