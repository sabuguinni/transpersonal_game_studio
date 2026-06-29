#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "NarrativeDialogueSystem.generated.h"

// ============================================================
// ENarr_SpeakerRole — Who is speaking
// ============================================================
UENUM(BlueprintType)
enum class ENarr_SpeakerRole : uint8
{
    None        UMETA(DisplayName = "None"),
    Hunter      UMETA(DisplayName = "Hunter"),
    Scout       UMETA(DisplayName = "Scout"),
    Elder       UMETA(DisplayName = "Elder"),
    Survivor    UMETA(DisplayName = "Survivor"),
    TribalLeader UMETA(DisplayName = "Tribal Leader"),
    Narrator    UMETA(DisplayName = "Narrator")
};

// ============================================================
// ENarr_DialogueCondition — When does this line trigger?
// ============================================================
UENUM(BlueprintType)
enum class ENarr_DialogueCondition : uint8
{
    Always          UMETA(DisplayName = "Always"),
    OnFirstVisit    UMETA(DisplayName = "On First Visit"),
    OnDangerNear    UMETA(DisplayName = "On Danger Nearby"),
    OnLowHealth     UMETA(DisplayName = "On Low Health"),
    OnQuestActive   UMETA(DisplayName = "On Quest Active"),
    OnNightfall     UMETA(DisplayName = "On Nightfall"),
    OnRainStart     UMETA(DisplayName = "On Rain Start"),
    OnDinoSpotted   UMETA(DisplayName = "On Dinosaur Spotted")
};

// ============================================================
// FNarr_DialogueLine — A single spoken line
// ============================================================
USTRUCT(BlueprintType)
struct FNarr_DialogueLine
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    FText LineText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    ENarr_SpeakerRole Speaker = ENarr_SpeakerRole::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    ENarr_DialogueCondition Condition = ENarr_DialogueCondition::Always;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    USoundBase* VoiceAudio = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    float DisplayDuration = 4.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    bool bPlayOnce = true;

    UPROPERTY(Transient)
    bool bHasPlayed = false;
};

// ============================================================
// FNarr_DialogueSequence — An ordered set of lines
// ============================================================
USTRUCT(BlueprintType)
struct FNarr_DialogueSequence
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Sequence")
    FName SequenceID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Sequence")
    TArray<FNarr_DialogueLine> Lines;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Sequence")
    bool bLoopSequence = false;

    UPROPERTY(Transient)
    int32 CurrentLineIndex = 0;
};

// ============================================================
// UNarr_DialogueComponent — Attaches to any NPC/trigger actor
// ============================================================
UCLASS(ClassGroup = (Narrative), meta = (BlueprintSpawnableComponent), DisplayName = "Narr Dialogue Component")
class TRANSPERSONALGAME_API UNarr_DialogueComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UNarr_DialogueComponent();

    // All dialogue sequences available on this actor
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    TArray<FNarr_DialogueSequence> DialogueSequences;

    // Radius within which the player triggers dialogue
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    float ProximityRadius = 400.0f;

    // Currently active sequence ID
    UPROPERTY(BlueprintReadOnly, Category = "Narrative")
    FName ActiveSequenceID;

    // Play a specific sequence by ID
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void PlaySequence(FName SequenceID);

    // Advance to next line in active sequence
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void AdvanceLine();

    // Stop current sequence
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void StopSequence();

    // Get current line text for HUD display
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Narrative")
    FText GetCurrentLineText() const;

    // Check if a sequence has been completed
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Narrative")
    bool IsSequenceComplete(FName SequenceID) const;

    // Trigger dialogue based on game condition
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void TriggerByCondition(ENarr_DialogueCondition Condition);

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
    // Timer for auto-advancing lines
    float LineTimer = 0.0f;
    bool bSequencePlaying = false;

    FNarr_DialogueSequence* FindSequence(FName SequenceID);
};
