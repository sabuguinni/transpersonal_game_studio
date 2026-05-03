#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundBase.h"
#include "Engine/World.h"
#include "../Core/SharedTypes.h"
#include "NarrativeManager.generated.h"

/**
 * NARRATIVE MANAGER - Sistema de narrativa dinâmica
 * Agente #15 - Narrative & Dialogue Agent
 * 
 * Gere narrativa contextual baseada em:
 * - Localização do jogador
 * - Proximidade de dinossauros
 * - Estado de sobrevivência
 * - Eventos do mundo
 */

UENUM(BlueprintType)
enum class ENarr_NarrativeType : uint8
{
    ResearchLog = 0     UMETA(DisplayName = "Research Log"),
    SafetyWarning = 1   UMETA(DisplayName = "Safety Warning"),
    StoryNarration = 2  UMETA(DisplayName = "Story Narration"),
    SurvivalGuide = 3   UMETA(DisplayName = "Survival Guide"),
    Discovery = 4       UMETA(DisplayName = "Discovery"),
    Danger = 5          UMETA(DisplayName = "Danger")
};

UENUM(BlueprintType)
enum class ENarr_TriggerCondition : uint8
{
    PlayerLocation = 0      UMETA(DisplayName = "Player Location"),
    DinosaurProximity = 1   UMETA(DisplayName = "Dinosaur Proximity"),
    HealthStatus = 2        UMETA(DisplayName = "Health Status"),
    TimeOfDay = 3           UMETA(DisplayName = "Time of Day"),
    FirstVisit = 4          UMETA(DisplayName = "First Visit"),
    CombatEvent = 5         UMETA(DisplayName = "Combat Event")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_NarrativeEntry
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString EntryID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    ENarr_NarrativeType Type = ENarr_NarrativeType::ResearchLog;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString AudioURL;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString TranscriptText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    ENarr_TriggerCondition TriggerCondition = ENarr_TriggerCondition::PlayerLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FVector TriggerLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    float TriggerRadius = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    bool bHasPlayed = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    float Cooldown = 60.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    float LastPlayedTime = 0.0f;

    FNarr_NarrativeEntry()
    {
        EntryID = TEXT("");
        Type = ENarr_NarrativeType::ResearchLog;
        AudioURL = TEXT("");
        TranscriptText = TEXT("");
        TriggerCondition = ENarr_TriggerCondition::PlayerLocation;
        TriggerLocation = FVector::ZeroVector;
        TriggerRadius = 500.0f;
        bHasPlayed = false;
        Cooldown = 60.0f;
        LastPlayedTime = 0.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ANarrativeManager : public AActor
{
    GENERATED_BODY()

public:
    ANarrativeManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Componentes
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UAudioComponent* AudioComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class USceneComponent* RootSceneComponent;

    // Base de dados de narrativa
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    TArray<FNarr_NarrativeEntry> NarrativeEntries;

    // Configurações
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    float CheckInterval = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    float MaxNarrativeDistance = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    bool bEnableDebugOutput = true;

    // Estado interno
    UPROPERTY(BlueprintReadOnly, Category = "State")
    float LastCheckTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    class APawn* PlayerPawn;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    bool bIsPlayingNarrative = false;

public:
    // Métodos principais
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void InitializeNarrativeDatabase();

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void CheckNarrativeTriggers();

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    bool PlayNarrativeEntry(const FNarr_NarrativeEntry& Entry);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void StopCurrentNarrative();

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void AddNarrativeEntry(const FNarr_NarrativeEntry& NewEntry);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    TArray<FNarr_NarrativeEntry> GetNarrativeEntriesInRadius(FVector Location, float Radius);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    bool IsNarrativeEntryAvailable(const FNarr_NarrativeEntry& Entry);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void ResetNarrativeEntry(const FString& EntryID);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void ResetAllNarrativeEntries();

    // Eventos Blueprint
    UFUNCTION(BlueprintImplementableEvent, Category = "Narrative")
    void OnNarrativeStarted(const FNarr_NarrativeEntry& Entry);

    UFUNCTION(BlueprintImplementableEvent, Category = "Narrative")
    void OnNarrativeFinished(const FNarr_NarrativeEntry& Entry);

    UFUNCTION(BlueprintImplementableEvent, Category = "Narrative")
    void OnNarrativeTriggered(ENarr_TriggerCondition Condition, FVector Location);

private:
    // Métodos internos
    void CheckLocationTriggers();
    void CheckProximityTriggers();
    void CheckHealthTriggers();
    void CheckTimeOfDayTriggers();
    
    FNarr_NarrativeEntry* FindNarrativeEntry(const FString& EntryID);
    bool CanPlayNarrative(const FNarr_NarrativeEntry& Entry);
    void UpdateNarrativeState(FNarr_NarrativeEntry& Entry);
};