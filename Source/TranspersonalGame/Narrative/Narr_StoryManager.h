#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/TriggerBox.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundBase.h"
#include "../Core/SharedTypes.h"
#include "Narr_StoryManager.generated.h"

/**
 * NARRATIVE AGENT #15 - STORY MANAGER
 * 
 * Manages dynamic storytelling, dialogue triggers, and narrative progression
 * in the prehistoric survival world. Responds to player actions and environmental
 * events to deliver contextual story moments.
 */

UENUM(BlueprintType)
enum class ENarr_StoryState : uint8
{
    Introduction = 0    UMETA(DisplayName = "Introduction"),
    Exploration = 1     UMETA(DisplayName = "Exploration"),
    FirstContact = 2    UMETA(DisplayName = "First Contact"),
    Survival = 3        UMETA(DisplayName = "Survival"),
    Adaptation = 4      UMETA(DisplayName = "Adaptation"),
    Mastery = 5         UMETA(DisplayName = "Mastery")
};

UENUM(BlueprintType)
enum class ENarr_TriggerType : uint8
{
    Discovery = 0       UMETA(DisplayName = "Discovery"),
    Danger = 1          UMETA(DisplayName = "Danger"),
    Safety = 2          UMETA(DisplayName = "Safety"),
    Achievement = 3     UMETA(DisplayName = "Achievement"),
    Environmental = 4   UMETA(DisplayName = "Environmental")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_StoryEvent
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story Event")
    FString EventID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story Event")
    FString DialogueText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story Event")
    ENarr_TriggerType TriggerType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story Event")
    float Priority;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story Event")
    bool bHasBeenTriggered;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story Event")
    TSoftObjectPtr<USoundBase> VoiceClip;

    FNarr_StoryEvent()
    {
        EventID = TEXT("");
        DialogueText = TEXT("");
        TriggerType = ENarr_TriggerType::Discovery;
        Priority = 1.0f;
        bHasBeenTriggered = false;
        VoiceClip = nullptr;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_NarrativeContext
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative Context")
    ENarr_StoryState CurrentState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative Context")
    int32 DinosaurEncounters;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative Context")
    int32 DaysAlive;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative Context")
    float SurvivalScore;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative Context")
    TArray<FString> CompletedEvents;

    FNarr_NarrativeContext()
    {
        CurrentState = ENarr_StoryState::Introduction;
        DinosaurEncounters = 0;
        DaysAlive = 0;
        SurvivalScore = 0.0f;
        CompletedEvents.Empty();
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ANarr_StoryManager : public AActor
{
    GENERATED_BODY()

public:
    ANarr_StoryManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Core narrative state
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative State")
    FNarr_NarrativeContext NarrativeContext;

    // Story events database
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story Events")
    TArray<FNarr_StoryEvent> StoryEvents;

    // Audio system
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio")
    class UAudioComponent* AudioComponent;

    // Trigger monitoring
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Triggers")
    TArray<class ATriggerBox*> NarrativeTriggers;

public:
    // Story progression methods
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void TriggerStoryEvent(const FString& EventID, ENarr_TriggerType TriggerType);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void AdvanceStoryState(ENarr_StoryState NewState);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void RegisterDinosaurEncounter(const FString& DinosaurType);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void UpdateSurvivalMetrics(float HealthPercent, float HungerPercent, float ThirstPercent);

    // Event system
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    bool IsEventCompleted(const FString& EventID) const;

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    FNarr_StoryEvent GetNextPendingEvent() const;

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void PlayNarrativeAudio(const FString& EventID);

    // Context queries
    UFUNCTION(BlueprintPure, Category = "Narrative")
    ENarr_StoryState GetCurrentStoryState() const { return NarrativeContext.CurrentState; }

    UFUNCTION(BlueprintPure, Category = "Narrative")
    int32 GetDinosaurEncounterCount() const { return NarrativeContext.DinosaurEncounters; }

    UFUNCTION(BlueprintPure, Category = "Narrative")
    float GetSurvivalScore() const { return NarrativeContext.SurvivalScore; }

private:
    // Internal story management
    void InitializeStoryEvents();
    void CheckTriggerConditions();
    void ProcessPendingEvents();
    
    // Story progression logic
    void EvaluateStoryProgression();
    bool ShouldAdvanceToNextState() const;
    
    // Timer for periodic checks
    float StoryUpdateTimer;
    static constexpr float StoryUpdateInterval = 2.0f;
};