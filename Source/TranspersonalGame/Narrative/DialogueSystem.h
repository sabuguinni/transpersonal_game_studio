#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Components/ActorComponent.h"
#include "Sound/SoundCue.h"
#include "Components/AudioComponent.h"
#include "../SharedTypes.h"
#include "DialogueSystem.generated.h"

// Dialogue trigger types for survival narrative
UENUM(BlueprintType)
enum class ENarr_DialogueTrigger : uint8
{
    ProximityToDinosaur     UMETA(DisplayName = "Proximity to Dinosaur"),
    BiomeEntry              UMETA(DisplayName = "Biome Entry"),
    DangerDetection         UMETA(DisplayName = "Danger Detection"),
    ResourceDiscovery       UMETA(DisplayName = "Resource Discovery"),
    WeatherChange           UMETA(DisplayName = "Weather Change"),
    TimeOfDay               UMETA(DisplayName = "Time of Day"),
    HealthCritical          UMETA(DisplayName = "Health Critical"),
    FirstEncounter          UMETA(DisplayName = "First Encounter")
};

// Narrative voice types for different contexts
UENUM(BlueprintType)
enum class ENarr_VoiceType : uint8
{
    TacticalNarrator        UMETA(DisplayName = "Tactical Narrator"),
    FieldResearcher         UMETA(DisplayName = "Field Researcher"),
    SafetyGuide             UMETA(DisplayName = "Safety Guide"),
    BehaviorAnalyst         UMETA(DisplayName = "Behavior Analyst"),
    SurvivalInstructor      UMETA(DisplayName = "Survival Instructor")
};

// Dialogue line data structure
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_DialogueLine
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString DialogueText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    ENarr_VoiceType VoiceType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    ENarr_DialogueTrigger TriggerType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    EEng_BiomeType TargetBiome;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    EEng_DinosaurSpecies TargetDinosaur;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    float TriggerDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    bool bPlayOnce;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    bool bHasBeenPlayed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    USoundBase* AudioClip;

    FNarr_DialogueLine()
    {
        DialogueText = "Default dialogue line";
        VoiceType = ENarr_VoiceType::TacticalNarrator;
        TriggerType = ENarr_DialogueTrigger::ProximityToDinosaur;
        TargetBiome = EEng_BiomeType::Savanna;
        TargetDinosaur = EEng_DinosaurSpecies::TRex;
        TriggerDistance = 1000.0f;
        bPlayOnce = true;
        bHasBeenPlayed = false;
        AudioClip = nullptr;
    }
};

// Dynamic narrative context tracking
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_NarrativeContext
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Context")
    EEng_BiomeType CurrentBiome;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Context")
    EEng_ThreatLevel CurrentThreatLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Context")
    TArray<EEng_DinosaurSpecies> NearbyDinosaurs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Context")
    float ClosestDinosaurDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Context")
    EEng_TimeOfDay CurrentTimeOfDay;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Context")
    EEng_WeatherType CurrentWeather;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Context")
    FEng_SurvivalStats PlayerStats;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Context")
    TArray<FString> TriggeredDialogueIDs;

    FNarr_NarrativeContext()
    {
        CurrentBiome = EEng_BiomeType::Savanna;
        CurrentThreatLevel = EEng_ThreatLevel::Safe;
        ClosestDinosaurDistance = 10000.0f;
        CurrentTimeOfDay = EEng_TimeOfDay::Morning;
        CurrentWeather = EEng_WeatherType::Clear;
    }
};

/**
 * Dynamic Dialogue System for Prehistoric Survival Game
 * Provides contextual narrative based on player situation, location, and threats
 * NO SPIRITUAL CONTENT - Focus on survival, danger, and scientific observation
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UNarr_DialogueSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UNarr_DialogueSystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Dialogue database
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue Database")
    TArray<FNarr_DialogueLine> DialogueLines;

    // Current narrative context
    UPROPERTY(BlueprintReadOnly, Category = "Narrative Context")
    FNarr_NarrativeContext CurrentContext;

    // Audio playback component
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio")
    UAudioComponent* DialogueAudioComponent;

    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    float ContextUpdateInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    float DinosaurDetectionRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    bool bEnableDebugLogging;

    // Core dialogue functions
    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void UpdateNarrativeContext();

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void CheckDialogueTriggers();

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    bool PlayDialogueLine(const FNarr_DialogueLine& DialogueLine);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void StopCurrentDialogue();

    // Context detection functions
    UFUNCTION(BlueprintCallable, Category = "Context Detection")
    EEng_BiomeType DetectCurrentBiome(const FVector& PlayerLocation);

    UFUNCTION(BlueprintCallable, Category = "Context Detection")
    TArray<AActor*> FindNearbyDinosaurs(float SearchRadius);

    UFUNCTION(BlueprintCallable, Category = "Context Detection")
    EEng_ThreatLevel CalculateThreatLevel();

    // Dialogue management
    UFUNCTION(BlueprintCallable, Category = "Dialogue Management")
    void AddDialogueLine(const FNarr_DialogueLine& NewDialogue);

    UFUNCTION(BlueprintCallable, Category = "Dialogue Management")
    void InitializeDefaultDialogues();

    UFUNCTION(BlueprintCallable, Category = "Dialogue Management")
    FNarr_DialogueLine* FindBestDialogueForContext();

private:
    float LastContextUpdate;
    bool bIsPlayingDialogue;
    FString CurrentDialogueID;

    // Helper functions
    bool ShouldTriggerDialogue(const FNarr_DialogueLine& DialogueLine);
    float CalculateDialoguePriority(const FNarr_DialogueLine& DialogueLine);
    void LogNarrativeEvent(const FString& Event);
};