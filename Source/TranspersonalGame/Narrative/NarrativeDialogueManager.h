#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NarrativeDialogueManager.generated.h"

// Narr_ prefix — unique across project per UE5 C++ rules

UENUM(BlueprintType)
enum class ENarr_DialogueTriggerType : uint8
{
    Camp        UMETA(DisplayName = "Camp Entrance"),
    River       UMETA(DisplayName = "River Crossing"),
    Predator    UMETA(DisplayName = "Predator Zone"),
    Discovery   UMETA(DisplayName = "Discovery Point"),
    Combat      UMETA(DisplayName = "Combat Zone")
};

USTRUCT(BlueprintType)
struct FNarr_DialogueLine
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString SpeakerName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString LineText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    float DisplayDuration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    ENarr_DialogueTriggerType TriggerContext;

    FNarr_DialogueLine()
        : SpeakerName(TEXT("Unknown"))
        , LineText(TEXT(""))
        , DisplayDuration(3.0f)
        , TriggerContext(ENarr_DialogueTriggerType::Camp)
    {}
};

USTRUCT(BlueprintType)
struct FNarr_DialogueSequence
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FName SequenceID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    TArray<FNarr_DialogueLine> Lines;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    bool bHasPlayed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    ENarr_DialogueTriggerType TriggerType;

    FNarr_DialogueSequence()
        : SequenceID(NAME_None)
        , bHasPlayed(false)
        , TriggerType(ENarr_DialogueTriggerType::Camp)
    {}
};

/**
 * ANarrativeDialogueManager
 * Manages dialogue sequences, trigger zones, and narrative progression.
 * Placed in MinPlayableMap — responds to player proximity triggers.
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ANarrativeDialogueManager : public AActor
{
    GENERATED_BODY()

public:
    ANarrativeDialogueManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // --- Dialogue Data ---

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    TArray<FNarr_DialogueSequence> DialogueLibrary;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|State")
    bool bDialogueActive;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|State")
    int32 CurrentLineIndex;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|State")
    FName ActiveSequenceID;

    // --- Survival Context ---

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Survival")
    float PlayerHungerThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Survival")
    float PlayerDangerRadius;

    // --- Methods ---

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void TriggerDialogueSequence(FName SequenceID);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void AdvanceDialogue();

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void EndDialogue();

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    FNarr_DialogueLine GetCurrentLine() const;

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    bool IsDialogueActive() const;

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void RegisterTriggerZone(ENarr_DialogueTriggerType TriggerType, FVector Location);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void InitializeDefaultDialogues();

private:
    FNarr_DialogueSequence* FindSequenceByID(FName SequenceID);
    void BuildCampDialogue();
    void BuildRiverDialogue();
    void BuildPredatorDialogue();
};
