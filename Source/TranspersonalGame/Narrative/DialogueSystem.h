#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/AudioComponent.h"
#include "Engine/DataTable.h"
#include "SharedTypes.h"
#include "DialogueSystem.generated.h"

// Narrative dialogue system for prehistoric survival game
// NO SPIRITUAL CONTENT - focus on survival, research, danger warnings

UENUM(BlueprintType)
enum class ENarr_DialogueType : uint8
{
    TacticalWarning     UMETA(DisplayName = "Tactical Warning"),
    FieldResearch       UMETA(DisplayName = "Field Research"),
    SafetyProtocol      UMETA(DisplayName = "Safety Protocol"),
    Discovery           UMETA(DisplayName = "Discovery"),
    SurvivalTip         UMETA(DisplayName = "Survival Tip"),
    EnvironmentalAlert  UMETA(DisplayName = "Environmental Alert")
};

UENUM(BlueprintType)
enum class ENarr_DialogueTrigger : uint8
{
    ProximityToThreat   UMETA(DisplayName = "Proximity to Threat"),
    BiomeEntry          UMETA(DisplayName = "Biome Entry"),
    DinosaurSighting    UMETA(DisplayName = "Dinosaur Sighting"),
    ResourceDiscovery   UMETA(DisplayName = "Resource Discovery"),
    WeatherChange       UMETA(DisplayName = "Weather Change"),
    TimeOfDay           UMETA(DisplayName = "Time of Day"),
    HealthCritical      UMETA(DisplayName = "Health Critical"),
    QuestProgress       UMETA(DisplayName = "Quest Progress")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_DialogueEntry
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString DialogueID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    ENarr_DialogueType DialogueType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString SpeakerName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString DialogueText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString AudioFilePath;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    float Duration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    ENarr_DialogueTrigger TriggerType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    EEng_BiomeType RequiredBiome;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    EEng_DinosaurSpecies RelatedDinosaur;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    float TriggerRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    int32 Priority;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    bool bCanRepeat;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    float RepeatCooldown;

    FNarr_DialogueEntry()
    {
        DialogueID = "";
        DialogueType = ENarr_DialogueType::TacticalWarning;
        SpeakerName = "Unknown";
        DialogueText = "";
        AudioFilePath = "";
        Duration = 0.0f;
        TriggerType = ENarr_DialogueTrigger::ProximityToThreat;
        RequiredBiome = EEng_BiomeType::Forest;
        RelatedDinosaur = EEng_DinosaurSpecies::Raptor;
        TriggerRadius = 1000.0f;
        Priority = 1;
        bCanRepeat = false;
        RepeatCooldown = 300.0f; // 5 minutes
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ANarr_DialogueSystem : public AActor
{
    GENERATED_BODY()

public:
    ANarr_DialogueSystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Core dialogue system
    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void TriggerDialogue(const FString& DialogueID);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void TriggerDialogueByType(ENarr_DialogueType DialogueType, EEng_BiomeType CurrentBiome);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void TriggerDinosaurDialogue(EEng_DinosaurSpecies DinosaurSpecies, float Distance);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void StopCurrentDialogue();

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    bool IsDialoguePlaying() const;

    // Dialogue management
    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void LoadDialogueDatabase();

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void AddDialogueEntry(const FNarr_DialogueEntry& NewEntry);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    FNarr_DialogueEntry GetDialogueEntry(const FString& DialogueID);

    // Context-aware triggers
    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void CheckProximityTriggers(const FVector& PlayerLocation);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void OnBiomeChanged(EEng_BiomeType NewBiome);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void OnDinosaurDetected(EEng_DinosaurSpecies Species, float Distance, const FVector& DinosaurLocation);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void OnWeatherChanged(EEng_WeatherType NewWeather);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void OnTimeOfDayChanged(EEng_TimeOfDay NewTimeOfDay);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void OnHealthCritical(float HealthPercentage);

protected:
    // Audio system
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio")
    UAudioComponent* DialogueAudioComponent;

    // Dialogue database
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TArray<FNarr_DialogueEntry> DialogueDatabase;

    // Current dialogue state
    UPROPERTY(BlueprintReadOnly, Category = "Dialogue")
    FNarr_DialogueEntry CurrentDialogue;

    UPROPERTY(BlueprintReadOnly, Category = "Dialogue")
    bool bIsPlayingDialogue;

    UPROPERTY(BlueprintReadOnly, Category = "Dialogue")
    float CurrentDialogueTime;

    // Cooldown tracking
    UPROPERTY(BlueprintReadOnly, Category = "Dialogue")
    TMap<FString, float> DialogueCooldowns;

    // Context tracking
    UPROPERTY(BlueprintReadOnly, Category = "Dialogue")
    EEng_BiomeType LastKnownBiome;

    UPROPERTY(BlueprintReadOnly, Category = "Dialogue")
    EEng_WeatherType LastKnownWeather;

    UPROPERTY(BlueprintReadOnly, Category = "Dialogue")
    EEng_TimeOfDay LastKnownTimeOfDay;

private:
    // Internal dialogue logic
    void PlayDialogueAudio(const FString& AudioFilePath);
    void OnDialogueFinished();
    bool CanPlayDialogue(const FString& DialogueID);
    void UpdateCooldowns(float DeltaTime);
    FNarr_DialogueEntry SelectBestDialogue(const TArray<FNarr_DialogueEntry>& CandidateDialogues);
    void InitializeDefaultDialogues();
};