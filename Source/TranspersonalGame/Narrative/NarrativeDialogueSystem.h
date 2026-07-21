#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NarrativeDialogueSystem.generated.h"

// ============================================================
// Enums — must be at global scope (UHT rule)
// ============================================================

UENUM(BlueprintType)
enum class ENarr_QuestOutcome : uint8
{
    None            UMETA(DisplayName = "None"),
    SurvivedFast    UMETA(DisplayName = "Survived Quickly (<10s)"),
    SurvivedBarely  UMETA(DisplayName = "Survived Barely (10-20s)"),
    Failed          UMETA(DisplayName = "Failed and Restarted"),
    NotStarted      UMETA(DisplayName = "Quest Not Started")
};

UENUM(BlueprintType)
enum class ENarr_StampedeCause : uint8
{
    Unknown     UMETA(DisplayName = "Unknown"),
    Predator    UMETA(DisplayName = "Predator"),
    Earthquake  UMETA(DisplayName = "Earthquake"),
    Fire        UMETA(DisplayName = "Fire"),
    Thunder     UMETA(DisplayName = "Thunder")
};

UENUM(BlueprintType)
enum class ENarr_NPCRole : uint8
{
    TribalElder  UMETA(DisplayName = "Tribal Elder"),
    ChiefHunter  UMETA(DisplayName = "Chief Hunter"),
    ScoutRunner  UMETA(DisplayName = "Scout Runner"),
    CampBuilder  UMETA(DisplayName = "Camp Builder"),
    Tracker      UMETA(DisplayName = "Tracker")
};

// ============================================================
// Structs — must be at global scope (UHT rule)
// ============================================================

USTRUCT(BlueprintType)
struct FNarr_DialogueLine
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FText SpeakerName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FText LineText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    ENarr_NPCRole SpeakerRole;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    ENarr_QuestOutcome TriggerOutcome;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    ENarr_StampedeCause TriggerCause;

    // Audio URL from ElevenLabs TTS (set at runtime or in editor)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString AudioURL;

    // Duration in seconds (for subtitle timing)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    float AudioDurationSeconds;

    FNarr_DialogueLine()
        : SpeakerRole(ENarr_NPCRole::TribalElder)
        , TriggerOutcome(ENarr_QuestOutcome::None)
        , TriggerCause(ENarr_StampedeCause::Unknown)
        , AudioDurationSeconds(5.0f)
    {}
};

USTRUCT(BlueprintType)
struct FNarr_DialogueSet
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    ENarr_QuestOutcome Outcome;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    ENarr_StampedeCause Cause;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    TArray<FNarr_DialogueLine> Lines;

    FNarr_DialogueSet()
        : Outcome(ENarr_QuestOutcome::None)
        , Cause(ENarr_StampedeCause::Unknown)
    {}
};

// ============================================================
// Delegate declarations
// ============================================================

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
    FOnNarr_DialogueTriggered,
    FNarr_DialogueLine, Line,
    ENarr_QuestOutcome, Outcome
);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
    FOnNarr_DialogueSequenceComplete,
    ENarr_QuestOutcome, Outcome
);

// ============================================================
// Main class
// ============================================================

UCLASS(ClassGroup = "Narrative", meta = (DisplayName = "Narrative Dialogue System"))
class TRANSPERSONALGAME_API ANarrativeDialogueSystem : public AActor
{
    GENERATED_BODY()

public:
    ANarrativeDialogueSystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // --------------------------------------------------------
    // Core dialogue trigger — called by QuestStampedeSystem
    // --------------------------------------------------------

    /** Trigger dialogue set matching outcome + cause. Plays lines in sequence. */
    UFUNCTION(BlueprintCallable, Category = "Narrative|Dialogue")
    void TriggerDialogue(ENarr_QuestOutcome Outcome, ENarr_StampedeCause Cause);

    /** Trigger dialogue by outcome only (cause = Unknown) */
    UFUNCTION(BlueprintCallable, Category = "Narrative|Dialogue")
    void TriggerDialogueByOutcome(ENarr_QuestOutcome Outcome);

    /** Stop current dialogue sequence */
    UFUNCTION(BlueprintCallable, Category = "Narrative|Dialogue")
    void StopDialogue();

    /** Get current active line text for HUD subtitle display */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Narrative|Dialogue")
    FText GetCurrentSubtitleText() const;

    /** Get current speaker name for HUD */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Narrative|Dialogue")
    FText GetCurrentSpeakerName() const;

    /** Is a dialogue sequence currently playing? */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Narrative|Dialogue")
    bool IsDialoguePlaying() const;

    /** Get audio URL for current line (for runtime audio playback) */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Narrative|Dialogue")
    FString GetCurrentLineAudioURL() const;

    // --------------------------------------------------------
    // Dialogue data — populated in constructor with all variants
    // --------------------------------------------------------

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Data")
    TArray<FNarr_DialogueSet> DialogueSets;

    // --------------------------------------------------------
    // Delegates — bind from Blueprint or QuestStampedeSystem
    // --------------------------------------------------------

    UPROPERTY(BlueprintAssignable, Category = "Narrative|Events")
    FOnNarr_DialogueTriggered OnDialogueTriggered;

    UPROPERTY(BlueprintAssignable, Category = "Narrative|Events")
    FOnNarr_DialogueSequenceComplete OnDialogueSequenceComplete;

    // --------------------------------------------------------
    // Runtime state
    // --------------------------------------------------------

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Narrative|State",
        meta = (AllowPrivateAccess = "true"))
    ENarr_QuestOutcome ActiveOutcome;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Narrative|State",
        meta = (AllowPrivateAccess = "true"))
    int32 CurrentLineIndex;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Narrative|State",
        meta = (AllowPrivateAccess = "true"))
    bool bDialoguePlaying;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Narrative|State",
        meta = (AllowPrivateAccess = "true"))
    float LineTimer;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Narrative|State",
        meta = (AllowPrivateAccess = "true"))
    TArray<FNarr_DialogueLine> ActiveSequence;

private:
    void PopulateDefaultDialogue();
    void AdvanceToNextLine();
    FNarr_DialogueSet* FindDialogueSet(ENarr_QuestOutcome Outcome, ENarr_StampedeCause Cause);
};
