#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Sound/SoundBase.h"
#include "NarrativeDialogueManager.generated.h"

UENUM(BlueprintType)
enum class ENarr_DialogueSpeaker : uint8
{
    TribalLeader    UMETA(DisplayName = "Tribal Leader"),
    Scout           UMETA(DisplayName = "Scout"),
    Elder           UMETA(DisplayName = "Elder"),
    Hunter          UMETA(DisplayName = "Hunter"),
    Narrator        UMETA(DisplayName = "Narrator")
};

UENUM(BlueprintType)
enum class ENarr_DialogueTrigger : uint8
{
    OnProximity     UMETA(DisplayName = "On Proximity"),
    OnQuestStart    UMETA(DisplayName = "On Quest Start"),
    OnQuestComplete UMETA(DisplayName = "On Quest Complete"),
    OnDinosaurSeen  UMETA(DisplayName = "On Dinosaur Seen"),
    OnPlayerDamaged UMETA(DisplayName = "On Player Damaged"),
    OnNightfall     UMETA(DisplayName = "On Nightfall"),
    OnCampfire      UMETA(DisplayName = "On Campfire")
};

USTRUCT(BlueprintType)
struct FNarr_DialogueLine
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString LineID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString SpeakerName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    ENarr_DialogueSpeaker SpeakerType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString DialogueText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    ENarr_DialogueTrigger TriggerCondition;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    USoundBase* VoiceAudio;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    float DisplayDuration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    bool bHasBeenPlayed;

    FNarr_DialogueLine()
        : SpeakerType(ENarr_DialogueSpeaker::Hunter)
        , TriggerCondition(ENarr_DialogueTrigger::OnProximity)
        , VoiceAudio(nullptr)
        , DisplayDuration(5.0f)
        , bHasBeenPlayed(false)
    {}
};

USTRUCT(BlueprintType)
struct FNarr_DialogueSequence
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString SequenceID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    TArray<FNarr_DialogueLine> Lines;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    bool bIsActive;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    int32 CurrentLineIndex;

    FNarr_DialogueSequence()
        : bIsActive(false)
        , CurrentLineIndex(0)
    {}
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ANarrativeDialogueManager : public AActor
{
    GENERATED_BODY()

public:
    ANarrativeDialogueManager();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Active dialogue sequences
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    TArray<FNarr_DialogueSequence> DialogueSequences;

    // Currently playing line
    UPROPERTY(BlueprintReadOnly, Category = "Narrative|Dialogue")
    FNarr_DialogueLine ActiveDialogueLine;

    // Is dialogue currently playing
    UPROPERTY(BlueprintReadOnly, Category = "Narrative|Dialogue")
    bool bDialogueActive;

    // Time remaining on current line
    UPROPERTY(BlueprintReadOnly, Category = "Narrative|Dialogue")
    float DialogueTimeRemaining;

    // Trigger a dialogue sequence by ID
    UFUNCTION(BlueprintCallable, Category = "Narrative|Dialogue")
    void TriggerDialogueSequence(const FString& SequenceID);

    // Trigger dialogue by condition
    UFUNCTION(BlueprintCallable, Category = "Narrative|Dialogue")
    void TriggerDialogueByCondition(ENarr_DialogueTrigger Trigger);

    // Advance to next line in active sequence
    UFUNCTION(BlueprintCallable, Category = "Narrative|Dialogue")
    void AdvanceDialogue();

    // Skip current dialogue
    UFUNCTION(BlueprintCallable, Category = "Narrative|Dialogue")
    void SkipDialogue();

    // Register a new dialogue line at runtime
    UFUNCTION(BlueprintCallable, Category = "Narrative|Dialogue")
    void RegisterDialogueLine(const FString& SequenceID, const FNarr_DialogueLine& Line);

    // Get all lines for a speaker type
    UFUNCTION(BlueprintCallable, Category = "Narrative|Dialogue")
    TArray<FNarr_DialogueLine> GetLinesForSpeaker(ENarr_DialogueSpeaker Speaker);

    // Check if a sequence has been completed
    UFUNCTION(BlueprintCallable, Category = "Narrative|Dialogue")
    bool IsSequenceComplete(const FString& SequenceID) const;

protected:
    void InitializeDefaultDialogue();
    void PlayDialogueLine(const FNarr_DialogueLine& Line);
    void OnDialogueLineComplete();

    FTimerHandle DialogueTimerHandle;
    int32 ActiveSequenceIndex;
};
