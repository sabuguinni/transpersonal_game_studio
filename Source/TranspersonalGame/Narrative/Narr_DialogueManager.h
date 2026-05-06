#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/AudioComponent.h"
#include "Engine/TriggerBox.h"
#include "../SharedTypes.h"
#include "Narr_DialogueManager.generated.h"

// Forward declarations
class USoundBase;
class UNarr_DialogueComponent;

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_DialogueEntry
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString SpeakerName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString DialogueText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    USoundBase* AudioClip;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    float Duration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    EEng_BiomeType TriggerBiome;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    bool bIsEmergencyAlert;

    FNarr_DialogueEntry()
    {
        SpeakerName = "Unknown";
        DialogueText = "";
        AudioClip = nullptr;
        Duration = 0.0f;
        TriggerBiome = EEng_BiomeType::Savanna;
        bIsEmergencyAlert = false;
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
    FVector TriggerLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    float TriggerRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    bool bHasTriggered;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    bool bIsRepeatable;

    FNarr_NarrativeEvent()
    {
        EventID = "DefaultEvent";
        EventDescription = "";
        TriggerLocation = FVector::ZeroVector;
        TriggerRadius = 1000.0f;
        bHasTriggered = false;
        bIsRepeatable = false;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ANarr_DialogueManager : public AActor
{
    GENERATED_BODY()

public:
    ANarr_DialogueManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Core dialogue system
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue System")
    TArray<FNarr_NarrativeEvent> NarrativeEvents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue System")
    TArray<FNarr_DialogueEntry> ActiveDialogueQueue;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UAudioComponent* DialogueAudioComponent;

    // Current dialogue state
    UPROPERTY(BlueprintReadOnly, Category = "Dialogue State")
    bool bIsPlayingDialogue;

    UPROPERTY(BlueprintReadOnly, Category = "Dialogue State")
    FNarr_DialogueEntry CurrentDialogue;

    UPROPERTY(BlueprintReadOnly, Category = "Dialogue State")
    float DialogueTimer;

    // Dialogue functions
    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void PlayDialogue(const FNarr_DialogueEntry& DialogueEntry);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void QueueDialogue(const FNarr_DialogueEntry& DialogueEntry);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void StopCurrentDialogue();

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void ProcessDialogueQueue();

    // Narrative event system
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void TriggerNarrativeEvent(const FString& EventID);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void RegisterNarrativeEvent(const FNarr_NarrativeEvent& NewEvent);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    bool CheckProximityTriggers(const FVector& PlayerLocation);

    // Biome-specific narrative
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void TriggerBiomeNarrative(EEng_BiomeType BiomeType);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void PlayEmergencyAlert(const FString& AlertMessage);

    // Discovery and research logs
    UFUNCTION(BlueprintCallable, Category = "Research")
    void PlayResearchLog(const FString& LogContent);

    UFUNCTION(BlueprintCallable, Category = "Research")
    void PlayDiscoveryNarration(const FString& DiscoveryDescription);

private:
    // Internal dialogue management
    void UpdateDialoguePlayback(float DeltaTime);
    void AdvanceDialogueQueue();
    void InitializeDefaultNarrativeEvents();
    
    // Audio management
    void SetupAudioComponent();
    void PlayAudioClip(USoundBase* AudioClip);
};