#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "Narr_StoryController.generated.h"

// Forward declarations
class ATranspersonalCharacter;
class UCrowdSimulationManager;

UENUM(BlueprintType)
enum class ENarr_StoryPhase : uint8
{
    Awakening,          // Player wakes up in prehistoric world
    FirstContact,       // First dinosaur encounter
    LearningToSurvive, // Basic survival mechanics
    TerritoryMapping,   // Exploring and understanding territories
    PredatorAwareness, // Learning about dangerous predators
    HerdObservation,   // Studying herbivore behavior
    SurvivalMastery,   // Advanced survival techniques
    TerritorialConflict // Dealing with territorial disputes
};

UENUM(BlueprintType)
enum class ENarr_EmotionalState : uint8
{
    Terror,        // Overwhelming fear
    Anxiety,       // Constant worry
    Caution,       // Careful awareness
    Curiosity,     // Scientific interest
    Confidence,    // Growing competence
    Determination, // Focused survival
    Respect        // Understanding of ecosystem
};

USTRUCT(BlueprintType)
struct FNarr_StoryEvent
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    FString EventName;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    FString EventDescription;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    ENarr_StoryPhase RequiredPhase;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    FVector TriggerLocation;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    float TriggerRadius;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    bool bTriggered;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    FString AudioCueName;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    float EventCooldown;

    FNarr_StoryEvent()
    {
        EventName = TEXT("Unknown Event");
        EventDescription = TEXT("A story event occurred");
        RequiredPhase = ENarr_StoryPhase::Awakening;
        TriggerLocation = FVector::ZeroVector;
        TriggerRadius = 500.0f;
        bTriggered = false;
        AudioCueName = TEXT("");
        EventCooldown = 30.0f;
    }
};

USTRUCT(BlueprintType)
struct FNarr_CharacterProgress
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    ENarr_StoryPhase CurrentPhase;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    ENarr_EmotionalState EmotionalState;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    int32 DinosaurEncounters;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    int32 SuccessfulObservations;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    int32 NearDeathExperiences;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    float SurvivalTime;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    TArray<FString> UnlockedKnowledge;

    FNarr_CharacterProgress()
    {
        CurrentPhase = ENarr_StoryPhase::Awakening;
        EmotionalState = ENarr_EmotionalState::Terror;
        DinosaurEncounters = 0;
        SuccessfulObservations = 0;
        NearDeathExperiences = 0;
        SurvivalTime = 0.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ANarr_StoryController : public AActor
{
    GENERATED_BODY()

public:
    ANarr_StoryController();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Core narrative components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Story System")
    class USceneComponent* RootSceneComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story Progress")
    FNarr_CharacterProgress PlayerProgress;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story Events")
    TArray<FNarr_StoryEvent> StoryEvents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story Events")
    TArray<FNarr_StoryEvent> ActiveEvents;

    // Audio integration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    TMap<FString, FString> AudioCueURLs;

    // Player reference
    UPROPERTY(BlueprintReadOnly, Category = "Player")
    TObjectPtr<ATranspersonalCharacter> PlayerCharacter;

    // Crowd simulation integration
    UPROPERTY(BlueprintReadOnly, Category = "Crowd Integration")
    TObjectPtr<UCrowdSimulationManager> CrowdManager;

    // Timing and progression
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timing")
    float PhaseTransitionCooldown;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timing")
    float LastEventTime;

public:
    // Story progression
    UFUNCTION(BlueprintCallable, Category = "Story System")
    void AdvanceStoryPhase(ENarr_StoryPhase NewPhase);

    UFUNCTION(BlueprintCallable, Category = "Story System")
    void TriggerStoryEvent(const FString& EventName);

    UFUNCTION(BlueprintCallable, Category = "Story System")
    void UpdateEmotionalState(ENarr_EmotionalState NewState);

    // Event management
    UFUNCTION(BlueprintCallable, Category = "Story Events")
    void CheckLocationBasedEvents(FVector PlayerLocation);

    UFUNCTION(BlueprintCallable, Category = "Story Events")
    void RegisterDinosaurEncounter(const FString& DinosaurType, bool bSurvived);

    UFUNCTION(BlueprintCallable, Category = "Story Events")
    void RegisterSuccessfulObservation(const FString& ObservationType);

    // Audio and feedback
    UFUNCTION(BlueprintCallable, Category = "Audio")
    void PlayNarrativeAudio(const FString& AudioCueName);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void RegisterAudioCue(const FString& CueName, const FString& AudioURL);

    // Knowledge system
    UFUNCTION(BlueprintCallable, Category = "Knowledge")
    void UnlockKnowledge(const FString& KnowledgeItem);

    UFUNCTION(BlueprintCallable, Category = "Knowledge")
    bool HasKnowledge(const FString& KnowledgeItem) const;

    UFUNCTION(BlueprintCallable, Category = "Knowledge")
    TArray<FString> GetAvailableKnowledge() const;

    // Survival tracking
    UFUNCTION(BlueprintCallable, Category = "Survival")
    void UpdateSurvivalTime(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Survival")
    void RecordNearDeathExperience();

    // Getters
    UFUNCTION(BlueprintPure, Category = "Story System")
    ENarr_StoryPhase GetCurrentPhase() const { return PlayerProgress.CurrentPhase; }

    UFUNCTION(BlueprintPure, Category = "Story System")
    ENarr_EmotionalState GetEmotionalState() const { return PlayerProgress.EmotionalState; }

    UFUNCTION(BlueprintPure, Category = "Story System")
    float GetSurvivalTime() const { return PlayerProgress.SurvivalTime; }

    UFUNCTION(BlueprintPure, Category = "Story System")
    FString GetCurrentPhaseDescription() const;

    UFUNCTION(BlueprintPure, Category = "Story System")
    FString GetEmotionalStateDescription() const;

    // Blueprint events
    UFUNCTION(BlueprintImplementableEvent, Category = "Story Events")
    void OnPhaseTransition(ENarr_StoryPhase OldPhase, ENarr_StoryPhase NewPhase);

    UFUNCTION(BlueprintImplementableEvent, Category = "Story Events")
    void OnEmotionalStateChange(ENarr_EmotionalState OldState, ENarr_EmotionalState NewState);

    UFUNCTION(BlueprintImplementableEvent, Category = "Story Events")
    void OnStoryEventTriggered(const FNarr_StoryEvent& Event);

private:
    // Internal helpers
    void InitializeStoryEvents();
    void UpdateActiveEvents();
    bool CanTriggerEvent(const FNarr_StoryEvent& Event) const;
    void ProcessEventTrigger(FNarr_StoryEvent& Event);
};