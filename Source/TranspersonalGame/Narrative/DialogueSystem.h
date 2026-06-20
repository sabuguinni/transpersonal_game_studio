#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "DialogueSystem.generated.h"

// ============================================================
// Narrative & Dialogue Agent #15 — DialogueSystem.h
// Cycle: PROD_CYCLE_AUTO_20260620_001
// Provides: quest dialogue lines, NPC bark system, trigger zones
// ============================================================

UENUM(BlueprintType)
enum class ENarr_DialogueSpeaker : uint8
{
    None            UMETA(DisplayName = "None"),
    QuestNarrator   UMETA(DisplayName = "Quest Narrator"),
    TribalElder     UMETA(DisplayName = "Tribal Elder"),
    PlayerCharacter UMETA(DisplayName = "Player Character"),
    AmbientBark     UMETA(DisplayName = "Ambient Bark")
};

UENUM(BlueprintType)
enum class ENarr_DialogueTriggerType : uint8
{
    OnQuestStart        UMETA(DisplayName = "On Quest Start"),
    OnItemCrafted       UMETA(DisplayName = "On Item Crafted"),
    OnDinoNearby        UMETA(DisplayName = "On Dino Nearby"),
    OnWaterFound        UMETA(DisplayName = "On Water Found"),
    OnPlayerDamaged     UMETA(DisplayName = "On Player Damaged"),
    OnNightfall         UMETA(DisplayName = "On Nightfall"),
    OnShelterBuilt      UMETA(DisplayName = "On Shelter Built")
};

USTRUCT(BlueprintType)
struct FNarr_DialogueLine
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FText LineText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    ENarr_DialogueSpeaker Speaker = ENarr_DialogueSpeaker::QuestNarrator;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    float DisplayDuration = 4.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString AudioURL;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    bool bBlockPlayerInput = false;
};

USTRUCT(BlueprintType)
struct FNarr_DialogueSequence
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FName SequenceID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    ENarr_DialogueTriggerType TriggerType = ENarr_DialogueTriggerType::OnQuestStart;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    TArray<FNarr_DialogueLine> Lines;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    bool bHasPlayed = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    bool bPlayOnce = true;
};

// ============================================================
// UDialogueSystemComponent — attach to PlayerController or GameMode
// ============================================================
UCLASS(ClassGroup = (Narrative), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UDialogueSystemComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UDialogueSystemComponent();

    virtual void BeginPlay() override;

    // Trigger a dialogue sequence by ID
    UFUNCTION(BlueprintCallable, Category = "Narrative|Dialogue")
    void TriggerDialogue(FName SequenceID);

    // Trigger all sequences matching a trigger type
    UFUNCTION(BlueprintCallable, Category = "Narrative|Dialogue")
    void TriggerByType(ENarr_DialogueTriggerType TriggerType);

    // Register a new sequence at runtime
    UFUNCTION(BlueprintCallable, Category = "Narrative|Dialogue")
    void RegisterSequence(FNarr_DialogueSequence Sequence);

    // Get current active line (for HUD display)
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Narrative|Dialogue")
    FNarr_DialogueLine GetCurrentLine() const;

    // Is dialogue currently active?
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Narrative|Dialogue")
    bool IsDialogueActive() const;

    // Advance to next line manually (for player-paced dialogue)
    UFUNCTION(BlueprintCallable, Category = "Narrative|Dialogue")
    void AdvanceLine();

    // Skip entire sequence
    UFUNCTION(BlueprintCallable, Category = "Narrative|Dialogue")
    void SkipSequence();

    // All registered dialogue sequences
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    TArray<FNarr_DialogueSequence> RegisteredSequences;

    // Currently playing sequence index (-1 = none)
    UPROPERTY(BlueprintReadOnly, Category = "Narrative|Dialogue")
    int32 ActiveSequenceIndex = -1;

    // Currently playing line index within sequence
    UPROPERTY(BlueprintReadOnly, Category = "Narrative|Dialogue")
    int32 ActiveLineIndex = 0;

    // Delegate for HUD to bind to
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDialogueLineChanged, FNarr_DialogueLine, NewLine);
    UPROPERTY(BlueprintAssignable, Category = "Narrative|Dialogue")
    FOnDialogueLineChanged OnDialogueLineChanged;

    DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDialogueSequenceEnded);
    UPROPERTY(BlueprintAssignable, Category = "Narrative|Dialogue")
    FOnDialogueSequenceEnded OnDialogueSequenceEnded;

private:
    void InitializeDefaultSequences();
    void PlayLine(int32 SequenceIdx, int32 LineIdx);
    void OnLineTimerExpired();

    FTimerHandle LineTimerHandle;
};
