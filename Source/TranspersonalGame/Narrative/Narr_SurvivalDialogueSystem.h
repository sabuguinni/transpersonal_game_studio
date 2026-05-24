#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameInstanceSubsystem.h"
#include "Engine/Engine.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundWave.h"
#include "Narr_SurvivalDialogueSystem.generated.h"

UENUM(BlueprintType)
enum class ENarr_DialogueContext : uint8
{
    PredatorEncounter UMETA(DisplayName = "Predator Encounter"),
    ResourceDiscovery UMETA(DisplayName = "Resource Discovery"),
    WeatherWarning UMETA(DisplayName = "Weather Warning"),
    NightSurvival UMETA(DisplayName = "Night Survival"),
    HerbalistWisdom UMETA(DisplayName = "Herbalist Wisdom"),
    ScoutReport UMETA(DisplayName = "Scout Report"),
    DangerAlert UMETA(DisplayName = "Danger Alert"),
    SafetyAdvice UMETA(DisplayName = "Safety Advice")
};

USTRUCT(BlueprintType)
struct FNarr_DialogueLine
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString SpeakerName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString DialogueText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    ENarr_DialogueContext Context;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    FString AudioURL;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float Duration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Conditions")
    float TriggerDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Conditions")
    bool bRequiresLineOfSight;

    FNarr_DialogueLine()
    {
        SpeakerName = TEXT("Unknown");
        DialogueText = TEXT("");
        Context = ENarr_DialogueContext::SafetyAdvice;
        AudioURL = TEXT("");
        Duration = 0.0f;
        TriggerDistance = 500.0f;
        bRequiresLineOfSight = true;
    }
};

UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UNarr_SurvivalDialogueSystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UNarr_SurvivalDialogueSystem();

    // Subsystem overrides
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Dialogue management
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void RegisterDialogueLine(const FNarr_DialogueLine& DialogueLine);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void TriggerContextualDialogue(ENarr_DialogueContext Context, const FVector& PlayerLocation);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void PlayDialogueLine(const FNarr_DialogueLine& DialogueLine);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void StopCurrentDialogue();

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    bool IsDialoguePlaying() const;

    // Audio management
    UFUNCTION(BlueprintCallable, Category = "Audio")
    void LoadAudioFromURL(const FString& AudioURL, const FString& DialogueID);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void PlayAudioClip(const FString& DialogueID);

    // Context-based triggers
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void CheckProximityTriggers(const FVector& PlayerLocation);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void RegisterProximityDialogue(const FNarr_DialogueLine& DialogueLine, const FVector& TriggerLocation);

protected:
    // Dialogue storage
    UPROPERTY(BlueprintReadOnly, Category = "Dialogue")
    TArray<FNarr_DialogueLine> RegisteredDialogues;

    UPROPERTY(BlueprintReadOnly, Category = "Dialogue")
    TMap<ENarr_DialogueContext, TArray<FNarr_DialogueLine>> ContextualDialogues;

    // Audio management
    UPROPERTY(BlueprintReadOnly, Category = "Audio")
    TMap<FString, USoundWave*> LoadedAudioClips;

    UPROPERTY(BlueprintReadOnly, Category = "Audio")
    UAudioComponent* DialogueAudioComponent;

    // Current state
    UPROPERTY(BlueprintReadOnly, Category = "State")
    bool bIsPlaying;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    FNarr_DialogueLine CurrentDialogue;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    float DialogueStartTime;

    // Proximity triggers
    UPROPERTY(BlueprintReadOnly, Category = "Triggers")
    TMap<FVector, FNarr_DialogueLine> ProximityTriggers;

    UPROPERTY(BlueprintReadOnly, Category = "Triggers")
    TArray<FVector> TriggeredLocations;

private:
    // Internal methods
    void InitializeDefaultDialogues();
    void SetupAudioComponent();
    bool CanTriggerDialogue(const FNarr_DialogueLine& DialogueLine, const FVector& PlayerLocation);
    FNarr_DialogueLine SelectBestDialogue(const TArray<FNarr_DialogueLine>& Candidates, const FVector& PlayerLocation);
    void OnDialogueFinished();

    // Timers
    FTimerHandle DialogueTimerHandle;
    FTimerHandle ProximityCheckTimerHandle;
};