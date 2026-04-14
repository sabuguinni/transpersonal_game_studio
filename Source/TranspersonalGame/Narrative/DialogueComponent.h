#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Components/AudioComponent.h"
#include "../SharedTypes.h"
#include "NarrativeManager.h"
#include "DialogueComponent.generated.h"

class UNarrativeManager;

UENUM(BlueprintType)
enum class ENarr_DialogueTrigger : uint8
{
    OnInteraction   UMETA(DisplayName = "On Interaction"),
    OnProximity     UMETA(DisplayName = "On Proximity"),
    OnStoryBeat     UMETA(DisplayName = "On Story Beat"),
    OnConsciousness UMETA(DisplayName = "On Consciousness Level"),
    Manual          UMETA(DisplayName = "Manual")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_DialogueTriggerCondition
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    ENarr_DialogueTrigger TriggerType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString DialogueID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    ENarr_StoryBeat RequiredStoryBeat;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    float MinConsciousnessLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    float ProximityDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    bool bOnlyTriggerOnce;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    bool bHasTriggered;

    FNarr_DialogueTriggerCondition()
    {
        TriggerType = ENarr_DialogueTrigger::OnInteraction;
        DialogueID = TEXT("");
        RequiredStoryBeat = ENarr_StoryBeat::Awakening;
        MinConsciousnessLevel = 0.0f;
        ProximityDistance = 500.0f;
        bOnlyTriggerOnce = true;
        bHasTriggered = false;
    }
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDialogueTriggered, const FString&, DialogueID, AActor*, Speaker);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDialogueCompleted, const FString&, DialogueID);

/**
 * Component that handles dialogue interactions for NPCs and interactive objects
 * Integrates with the NarrativeManager for story-driven conversations
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UDialogueComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UDialogueComponent();

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Dialogue interaction
    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    bool TriggerDialogue(AActor* Interactor, const FString& DialogueID = TEXT(""));

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void EndCurrentDialogue();

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    bool IsInDialogue() const;

    // Trigger conditions
    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void AddDialogueTrigger(const FNarr_DialogueTriggerCondition& TriggerCondition);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void RemoveDialogueTrigger(const FString& DialogueID);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    bool CanTriggerDialogue(const FString& DialogueID, AActor* Interactor) const;

    // Audio playback
    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void PlayDialogueAudio(const FString& AudioAssetPath);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void StopDialogueAudio();

    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TArray<FNarr_DialogueTriggerCondition> DialogueTriggers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString CharacterName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    bool bAutoPlayAudio;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    float AudioVolume;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    bool bInterruptible;

    // Events
    UPROPERTY(BlueprintAssignable, Category = "Dialogue")
    FOnDialogueTriggered OnDialogueTriggered;

    UPROPERTY(BlueprintAssignable, Category = "Dialogue")
    FOnDialogueCompleted OnDialogueCompleted;

protected:
    UPROPERTY()
    UNarrativeManager* NarrativeManager;

    UPROPERTY()
    UAudioComponent* AudioComponent;

    UPROPERTY()
    FString CurrentDialogueID;

    UPROPERTY()
    TWeakObjectPtr<AActor> CurrentInteractor;

private:
    void InitializeComponent();
    void CheckProximityTriggers();
    void CheckStoryBeatTriggers();
    void CheckConsciousnessTriggers();
    
    UFUNCTION()
    void OnStoryBeatCompleted(ENarr_StoryBeat CompletedBeat);
    
    UFUNCTION()
    void OnConsciousnessLevelChanged(float NewLevel);

    bool EvaluateTriggerCondition(const FNarr_DialogueTriggerCondition& Condition, AActor* Interactor) const;
    FNarr_DialogueTriggerCondition* FindTriggerCondition(const FString& DialogueID);
};