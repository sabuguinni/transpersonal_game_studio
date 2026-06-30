#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/DataTable.h"
#include "SharedTypes.h"
#include "DialogueSystem.generated.h"

// ─── Enums ────────────────────────────────────────────────────────────────────

UENUM(BlueprintType)
enum class ENarr_DialogueSpeaker : uint8
{
    None            UMETA(DisplayName = "None"),
    TribalLeader    UMETA(DisplayName = "Tribal Leader"),
    ElderHunter     UMETA(DisplayName = "Elder Hunter"),
    SurvivorScout   UMETA(DisplayName = "Survivor Scout"),
    YoungTracker    UMETA(DisplayName = "Young Tracker"),
    Craftsman       UMETA(DisplayName = "Craftsman"),
    Player          UMETA(DisplayName = "Player")
};

UENUM(BlueprintType)
enum class ENarr_DialogueContext : uint8
{
    Ambient         UMETA(DisplayName = "Ambient"),
    Danger          UMETA(DisplayName = "Danger"),
    Discovery       UMETA(DisplayName = "Discovery"),
    Migration       UMETA(DisplayName = "Migration"),
    Combat          UMETA(DisplayName = "Combat"),
    Rest            UMETA(DisplayName = "Rest"),
    QuestGive       UMETA(DisplayName = "Quest Give"),
    QuestComplete   UMETA(DisplayName = "Quest Complete")
};

UENUM(BlueprintType)
enum class ENarr_DialogueResult : uint8
{
    Continue        UMETA(DisplayName = "Continue"),
    BranchA         UMETA(DisplayName = "Branch A"),
    BranchB         UMETA(DisplayName = "Branch B"),
    End             UMETA(DisplayName = "End"),
    TriggerQuest    UMETA(DisplayName = "Trigger Quest"),
    TriggerEvent    UMETA(DisplayName = "Trigger Event")
};

// ─── Structs ──────────────────────────────────────────────────────────────────

USTRUCT(BlueprintType)
struct FNarr_DialogueLine : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    FName LineID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    ENarr_DialogueSpeaker Speaker = ENarr_DialogueSpeaker::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    FText LineText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    ENarr_DialogueContext Context = ENarr_DialogueContext::Ambient;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    FString AudioURL;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    float DisplayDuration = 4.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    FName NextLineID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    ENarr_DialogueResult Result = ENarr_DialogueResult::Continue;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    FName TriggerQuestID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    FName TriggerEventID;
};

USTRUCT(BlueprintType)
struct FNarr_DialogueSequence
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    FName SequenceID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    TArray<FNarr_DialogueLine> Lines;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    bool bIsRepeatable = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    bool bHasBeenPlayed = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    ENarr_DialogueContext RequiredContext = ENarr_DialogueContext::Ambient;
};

USTRUCT(BlueprintType)
struct FNarr_VoiceLineRegistry
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Audio")
    FName LineID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Audio")
    ENarr_DialogueSpeaker Speaker = ENarr_DialogueSpeaker::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Audio")
    FString AudioURL;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Audio")
    float DurationSeconds = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Audio")
    ENarr_DialogueContext Context = ENarr_DialogueContext::Ambient;
};

// ─── Dialogue Manager Actor ────────────────────────────────────────────────────

UCLASS(BlueprintType, Blueprintable, ClassGroup = "Narrative")
class TRANSPERSONALGAME_API ANarr_DialogueManager : public AActor
{
    GENERATED_BODY()

public:
    ANarr_DialogueManager();

    // Active sequences registered in this manager
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    TArray<FNarr_DialogueSequence> RegisteredSequences;

    // Voice line registry (populated from ElevenLabs TTS output)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Audio")
    TArray<FNarr_VoiceLineRegistry> VoiceLineRegistry;

    // Currently active sequence
    UPROPERTY(BlueprintReadOnly, Category = "Narrative|Dialogue",
        meta = (AllowPrivateAccess = "true"))
    FName ActiveSequenceID;

    // Current line index within active sequence
    UPROPERTY(BlueprintReadOnly, Category = "Narrative|Dialogue",
        meta = (AllowPrivateAccess = "true"))
    int32 CurrentLineIndex = 0;

    // Is dialogue currently playing
    UPROPERTY(BlueprintReadOnly, Category = "Narrative|Dialogue",
        meta = (AllowPrivateAccess = "true"))
    bool bDialogueActive = false;

    // Trigger a dialogue sequence by ID
    UFUNCTION(BlueprintCallable, Category = "Narrative|Dialogue")
    bool StartDialogueSequence(FName SequenceID);

    // Advance to next line in active sequence
    UFUNCTION(BlueprintCallable, Category = "Narrative|Dialogue")
    bool AdvanceDialogue();

    // End the current dialogue sequence
    UFUNCTION(BlueprintCallable, Category = "Narrative|Dialogue")
    void EndDialogue();

    // Get the current dialogue line
    UFUNCTION(BlueprintCallable, Category = "Narrative|Dialogue")
    bool GetCurrentLine(FNarr_DialogueLine& OutLine) const;

    // Register a voice line URL from TTS generation
    UFUNCTION(BlueprintCallable, Category = "Narrative|Audio")
    void RegisterVoiceLine(FName LineID, ENarr_DialogueSpeaker Speaker,
        const FString& AudioURL, float Duration, ENarr_DialogueContext Context);

    // Get audio URL for a given line ID
    UFUNCTION(BlueprintCallable, Category = "Narrative|Audio")
    FString GetVoiceLineURL(FName LineID) const;

    // Trigger context-appropriate ambient dialogue
    UFUNCTION(BlueprintCallable, Category = "Narrative|Dialogue")
    bool TriggerContextDialogue(ENarr_DialogueContext Context);

    // Check if a sequence has been played
    UFUNCTION(BlueprintCallable, Category = "Narrative|Dialogue")
    bool HasSequenceBeenPlayed(FName SequenceID) const;

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

private:
    // Timer for auto-advancing dialogue
    float DialogueTimer = 0.0f;
    bool bAutoAdvance = true;

    // Find sequence by ID (returns index or -1)
    int32 FindSequenceIndex(FName SequenceID) const;

    // Populate built-in voice line registry from TTS audio URLs
    void InitializeVoiceLineRegistry();
};
