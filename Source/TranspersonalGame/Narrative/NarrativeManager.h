#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Sound/SoundCue.h"
#include "Components/AudioComponent.h"
#include "../SharedTypes.h"
#include "NarrativeManager.generated.h"

// Narrative-specific enums
UENUM(BlueprintType)
enum class ENarr_NarrativeEvent : uint8
{
    PlayerEntersBiome       UMETA(DisplayName = "Player Enters Biome"),
    DinosaurEncounter       UMETA(DisplayName = "Dinosaur Encounter"),
    ThreatDetected         UMETA(DisplayName = "Threat Detected"),
    ResourceDiscovered     UMETA(DisplayName = "Resource Discovered"),
    QuestCompleted         UMETA(DisplayName = "Quest Completed"),
    SurvivalCritical       UMETA(DisplayName = "Survival Critical"),
    WeatherChange          UMETA(DisplayName = "Weather Change"),
    TimeOfDayChange        UMETA(DisplayName = "Time of Day Change")
};

UENUM(BlueprintType)
enum class ENarr_NarratorType : uint8
{
    TacticalNarrator       UMETA(DisplayName = "Tactical Narrator"),
    FieldResearcher        UMETA(DisplayName = "Field Researcher"),
    EmergencyNarrator      UMETA(DisplayName = "Emergency Narrator"),
    StoryNarrator          UMETA(DisplayName = "Story Narrator"),
    SurvivalGuide          UMETA(DisplayName = "Survival Guide")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_NarrativeTrigger
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    ENarr_NarrativeEvent EventType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    ENarr_NarratorType NarratorType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString DialogueText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    TSoftObjectPtr<USoundCue> AudioCue;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    float Priority;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    float Cooldown;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    bool bIsOneShot;

    FNarr_NarrativeTrigger()
    {
        EventType = ENarr_NarrativeEvent::PlayerEntersBiome;
        NarratorType = ENarr_NarratorType::StoryNarrator;
        DialogueText = TEXT("Default narrative text");
        Priority = 1.0f;
        Cooldown = 30.0f;
        bIsOneShot = false;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_ActiveNarrative
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FNarr_NarrativeTrigger Trigger;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    float LastTriggeredTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    int32 TriggerCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    bool bIsActive;

    FNarr_ActiveNarrative()
    {
        LastTriggeredTime = 0.0f;
        TriggerCount = 0;
        bIsActive = true;
    }
};

/**
 * Sistema de narrativa dinâmica para o jogo de sobrevivência pré-histórico.
 * Gere narrações contextuais baseadas em eventos do jogo, localização do jogador,
 * encontros com dinossauros e estado de sobrevivência.
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UNarrativeManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UNarrativeManager();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Core narrative functions
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void TriggerNarrativeEvent(ENarr_NarrativeEvent EventType, const FVector& Location = FVector::ZeroVector);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void RegisterNarrativeTrigger(const FNarr_NarrativeTrigger& NewTrigger);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void PlayNarration(const FNarr_NarrativeTrigger& Trigger);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void SetNarrativeEnabled(bool bEnabled);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    bool IsNarrativeEnabled() const { return bNarrativeEnabled; }

    // Biome-specific narrative
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void TriggerBiomeNarrative(EEng_BiomeType BiomeType, const FVector& PlayerLocation);

    // Dinosaur encounter narrative
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void TriggerDinosaurEncounter(EEng_DinosaurSpecies Species, float Distance, EEng_ThreatLevel ThreatLevel);

    // Survival state narrative
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void TriggerSurvivalNarrative(EEng_SurvivalStat StatType, float StatValue);

    // Quest narrative
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void TriggerQuestNarrative(EEng_QuestType QuestType, EEng_QuestStatus Status);

    // Utility functions
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void StopCurrentNarration();

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    bool IsNarrationPlaying() const;

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    float GetNarrativeVolume() const { return NarrativeVolume; }

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void SetNarrativeVolume(float NewVolume);

protected:
    // Narrative configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative Settings")
    bool bNarrativeEnabled;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative Settings")
    float NarrativeVolume;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative Settings")
    float GlobalNarrativeCooldown;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative Settings")
    int32 MaxConcurrentNarrations;

    // Active narrative data
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Narrative State")
    TArray<FNarr_ActiveNarrative> ActiveNarratives;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Narrative State")
    TArray<FNarr_NarrativeTrigger> RegisteredTriggers;

    // Audio components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio")
    TArray<UAudioComponent*> NarrativeAudioComponents;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio")
    UAudioComponent* CurrentNarrationComponent;

    // Internal state
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
    float LastNarrativeTime;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
    EEng_BiomeType CurrentBiome;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
    TMap<ENarr_NarrativeEvent, float> EventCooldowns;

private:
    // Internal helper functions
    bool CanTriggerNarrative(ENarr_NarrativeEvent EventType) const;
    FNarr_NarrativeTrigger* FindBestTrigger(ENarr_NarrativeEvent EventType, ENarr_NarratorType PreferredNarrator = ENarr_NarratorType::StoryNarrator);
    void UpdateNarrativeCooldowns(float DeltaTime);
    void CleanupFinishedNarrations();
    UAudioComponent* GetAvailableAudioComponent();
    void InitializeDefaultTriggers();

    // Timer handles
    FTimerHandle NarrativeUpdateTimer;
    FTimerHandle CooldownUpdateTimer;
};