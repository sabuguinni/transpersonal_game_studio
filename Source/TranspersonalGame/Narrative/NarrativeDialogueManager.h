#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/GameModeBase.h"
#include "Components/ActorComponent.h"
#include "Engine/DataTable.h"
#include "../SharedTypes.h"
#include "NarrativeDialogueManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_DialogueEntry
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString DialogueID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString SpeakerName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString DialogueText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString AudioPath;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    float Duration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    bool bIsQuestRelated;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString TriggerCondition;

    FNarr_DialogueEntry()
    {
        DialogueID = TEXT("");
        SpeakerName = TEXT("Narrator");
        DialogueText = TEXT("");
        AudioPath = TEXT("");
        Duration = 3.0f;
        bIsQuestRelated = false;
        TriggerCondition = TEXT("");
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_NarrativeEvent
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString EventID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString EventDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    TArray<FNarr_DialogueEntry> DialogueSequence;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    bool bIsTriggered;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    float TriggerRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FVector TriggerLocation;

    FNarr_NarrativeEvent()
    {
        EventID = TEXT("");
        EventDescription = TEXT("");
        bIsTriggered = false;
        TriggerRadius = 1000.0f;
        TriggerLocation = FVector::ZeroVector;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UNarrativeDialogueManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UNarrativeDialogueManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Core narrative system
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative Database")
    TArray<FNarr_DialogueEntry> DialogueDatabase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative Events")
    TArray<FNarr_NarrativeEvent> NarrativeEvents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Current State")
    FString CurrentDialogueID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Current State")
    bool bIsDialogueActive;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Current State")
    float DialogueTimer;

    // Player proximity tracking
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Tracking")
    class APawn* PlayerPawn;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Tracking")
    float ProximityCheckRadius;

    // Audio integration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    class UAudioComponent* DialogueAudioComponent;

    // Core dialogue functions
    UFUNCTION(BlueprintCallable, Category = "Dialogue System")
    void TriggerDialogue(const FString& DialogueID);

    UFUNCTION(BlueprintCallable, Category = "Dialogue System")
    void StopCurrentDialogue();

    UFUNCTION(BlueprintCallable, Category = "Dialogue System")
    bool IsDialogueActive() const;

    UFUNCTION(BlueprintCallable, Category = "Dialogue System")
    FNarr_DialogueEntry GetDialogueEntry(const FString& DialogueID);

    // Narrative event system
    UFUNCTION(BlueprintCallable, Category = "Narrative Events")
    void TriggerNarrativeEvent(const FString& EventID);

    UFUNCTION(BlueprintCallable, Category = "Narrative Events")
    void CheckProximityTriggers();

    UFUNCTION(BlueprintCallable, Category = "Narrative Events")
    void RegisterNarrativeEvent(const FNarr_NarrativeEvent& NewEvent);

    // Quest integration
    UFUNCTION(BlueprintCallable, Category = "Quest Integration")
    void OnQuestStarted(const FString& QuestID);

    UFUNCTION(BlueprintCallable, Category = "Quest Integration")
    void OnQuestCompleted(const FString& QuestID);

    UFUNCTION(BlueprintCallable, Category = "Quest Integration")
    void OnQuestObjectiveUpdated(const FString& QuestID, const FString& ObjectiveID);

    // Survival context awareness
    UFUNCTION(BlueprintCallable, Category = "Survival Context")
    void OnPlayerHealthLow();

    UFUNCTION(BlueprintCallable, Category = "Survival Context")
    void OnPlayerHungerHigh();

    UFUNCTION(BlueprintCallable, Category = "Survival Context")
    void OnPlayerThirstHigh();

    UFUNCTION(BlueprintCallable, Category = "Survival Context")
    void OnDangerDetected(const FString& DangerType);

    // Dinosaur encounter narratives
    UFUNCTION(BlueprintCallable, Category = "Dinosaur Encounters")
    void OnDinosaurSighted(const FString& DinosaurType, float Distance);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Encounters")
    void OnDinosaurAggressive(const FString& DinosaurType);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Encounters")
    void OnDinosaurFleeing(const FString& DinosaurType);

private:
    // Internal helper functions
    void UpdateDialogueTimer(float DeltaTime);
    void ProcessNarrativeQueue();
    FNarr_DialogueEntry* FindDialogueEntry(const FString& DialogueID);
    FNarr_NarrativeEvent* FindNarrativeEvent(const FString& EventID);
    void InitializeDefaultDialogue();
    void InitializeDefaultNarrativeEvents();
};