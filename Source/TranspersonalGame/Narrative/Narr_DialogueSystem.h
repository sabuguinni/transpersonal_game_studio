#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/AudioComponent.h"
#include "Components/SceneComponent.h"
#include "../SharedTypes.h"
#include "Narr_DialogueSystem.generated.h"

UENUM(BlueprintType)
enum class ENarr_DialogueType : uint8
{
    Discovery       UMETA(DisplayName = "Discovery"),
    Warning         UMETA(DisplayName = "Warning"),
    Tutorial        UMETA(DisplayName = "Tutorial"),
    Emergency       UMETA(DisplayName = "Emergency"),
    Observation     UMETA(DisplayName = "Observation"),
    Achievement     UMETA(DisplayName = "Achievement")
};

UENUM(BlueprintType)
enum class ENarr_SpeakerRole : uint8
{
    FieldPaleontologist     UMETA(DisplayName = "Field Paleontologist"),
    ExplorationGuide        UMETA(DisplayName = "Exploration Guide"),
    SeniorPaleontologist    UMETA(DisplayName = "Senior Paleontologist"),
    SafetyCoordinator       UMETA(DisplayName = "Safety Coordinator"),
    ResearchDirector        UMETA(DisplayName = "Research Director"),
    SurvivalExpert          UMETA(DisplayName = "Survival Expert")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_DialogueEntry
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString DialogueText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    ENarr_SpeakerRole Speaker;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    ENarr_DialogueType DialogueType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    float Duration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString AudioURL;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    EEng_BiomeType RelevantBiome;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    EEng_DinosaurSpecies RelevantDinosaur;

    FNarr_DialogueEntry()
    {
        DialogueText = "";
        Speaker = ENarr_SpeakerRole::FieldPaleontologist;
        DialogueType = ENarr_DialogueType::Observation;
        Duration = 0.0f;
        AudioURL = "";
        RelevantBiome = EEng_BiomeType::Savanna;
        RelevantDinosaur = EEng_DinosaurSpecies::TRex;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_NarrativeContext
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Context")
    EEng_BiomeType CurrentBiome;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Context")
    TArray<EEng_DinosaurSpecies> NearbyDinosaurs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Context")
    EEng_ThreatLevel ThreatLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Context")
    EEng_TimeOfDay TimeOfDay;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Context")
    EEng_WeatherType Weather;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Context")
    bool bInEmergency;

    FNarr_NarrativeContext()
    {
        CurrentBiome = EEng_BiomeType::Savanna;
        ThreatLevel = EEng_ThreatLevel::Safe;
        TimeOfDay = EEng_TimeOfDay::Morning;
        Weather = EEng_WeatherType::Clear;
        bInEmergency = false;
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

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootSceneComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UAudioComponent* AudioComponent;

    // Dialogue Database
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue System")
    TArray<FNarr_DialogueEntry> DialogueDatabase;

    // Current Context
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative Context")
    FNarr_NarrativeContext CurrentContext;

    // System Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    float DialogueCooldown;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    float ContextUpdateInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    bool bAutoTriggerDialogue;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    float MaxDialogueDistance;

private:
    FTimerHandle ContextUpdateTimer;
    FTimerHandle DialogueCooldownTimer;
    bool bDialogueOnCooldown;

public:
    virtual void Tick(float DeltaTime) override;

    // Core Functions
    UFUNCTION(BlueprintCallable, Category = "Dialogue System")
    void TriggerDialogue(ENarr_DialogueType DialogueType, EEng_DinosaurSpecies RelevantDinosaur = EEng_DinosaurSpecies::TRex);

    UFUNCTION(BlueprintCallable, Category = "Dialogue System")
    void UpdateNarrativeContext();

    UFUNCTION(BlueprintCallable, Category = "Dialogue System")
    FNarr_DialogueEntry GetRelevantDialogue(ENarr_DialogueType DialogueType, EEng_BiomeType Biome, EEng_DinosaurSpecies Dinosaur);

    UFUNCTION(BlueprintCallable, Category = "Dialogue System")
    void PlayDialogue(const FNarr_DialogueEntry& DialogueEntry);

    UFUNCTION(BlueprintCallable, Category = "Dialogue System")
    void InitializeDialogueDatabase();

    // Context Detection
    UFUNCTION(BlueprintCallable, Category = "Context Detection")
    EEng_BiomeType DetectCurrentBiome();

    UFUNCTION(BlueprintCallable, Category = "Context Detection")
    TArray<EEng_DinosaurSpecies> DetectNearbyDinosaurs();

    UFUNCTION(BlueprintCallable, Category = "Context Detection")
    EEng_ThreatLevel CalculateThreatLevel();

    // Emergency Integration
    UFUNCTION(BlueprintCallable, Category = "Emergency Integration")
    void OnEmergencyTriggered(const FString& EmergencyType);

    UFUNCTION(BlueprintCallable, Category = "Emergency Integration")
    void OnMissionCompleted(const FString& MissionType);

    // Utility Functions
    UFUNCTION(BlueprintCallable, Category = "Utility")
    void AddCustomDialogue(const FNarr_DialogueEntry& NewDialogue);

    UFUNCTION(BlueprintCallable, Category = "Utility")
    void ClearDialogueDatabase();

private:
    void OnContextUpdate();
    void OnDialogueCooldownEnd();
    bool IsPlayerNearby();
    FString GetBiomeName(EEng_BiomeType BiomeType);
    FString GetDinosaurName(EEng_DinosaurSpecies Species);
};