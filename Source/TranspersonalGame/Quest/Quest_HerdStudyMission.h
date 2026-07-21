#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Quest_HerdStudyMission.generated.h"

// Forward declarations
class UCrowd_MassSimulationManager;
class ATranspersonalCharacter;

UENUM(BlueprintType)
enum class EQuest_HerdStudyObjective : uint8
{
    ObserveFeeding,
    ObserveMigration,
    ObserveDefense,
    ObserveSocialBehavior,
    DocumentSpecies
};

USTRUCT(BlueprintType)
struct FQuest_HerdObservation
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Herd Study")
    FString SpeciesName;

    UPROPERTY(BlueprintReadOnly, Category = "Herd Study")
    EQuest_HerdStudyObjective ObjectiveType;

    UPROPERTY(BlueprintReadOnly, Category = "Herd Study")
    FVector ObservationLocation;

    UPROPERTY(BlueprintReadOnly, Category = "Herd Study")
    float ObservationDuration;

    UPROPERTY(BlueprintReadOnly, Category = "Herd Study")
    int32 HerdSize;

    FQuest_HerdObservation()
    {
        SpeciesName = TEXT("");
        ObjectiveType = EQuest_HerdStudyObjective::ObserveFeeding;
        ObservationLocation = FVector::ZeroVector;
        ObservationDuration = 0.0f;
        HerdSize = 0;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AQuest_HerdStudyMission : public AActor
{
    GENERATED_BODY()

public:
    AQuest_HerdStudyMission();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Quest Management
    UFUNCTION(BlueprintCallable, Category = "Quest")
    void StartHerdStudyMission();

    UFUNCTION(BlueprintCallable, Category = "Quest")
    void CompleteHerdStudyMission();

    UFUNCTION(BlueprintCallable, Category = "Quest")
    void FailHerdStudyMission();

    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool IsQuestActive() const { return bIsQuestActive; }

    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool IsQuestComplete() const { return bIsQuestComplete; }

    // Observation System
    UFUNCTION(BlueprintCallable, Category = "Observation")
    void RegisterHerdObservation(const FString& SpeciesName, EQuest_HerdStudyObjective ObjectiveType, 
                                FVector Location, float Duration, int32 HerdSize);

    UFUNCTION(BlueprintCallable, Category = "Observation")
    bool CheckObservationProgress();

    UFUNCTION(BlueprintCallable, Category = "Observation")
    TArray<FQuest_HerdObservation> GetCompletedObservations() const { return CompletedObservations; }

    // Integration with Crowd System
    UFUNCTION(BlueprintCallable, Category = "Integration")
    void DetectNearbyHerds();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    void AnalyzeHerdBehavior(AActor* HerdActor);

protected:
    // Quest State
    UPROPERTY(BlueprintReadOnly, Category = "Quest State")
    bool bIsQuestActive;

    UPROPERTY(BlueprintReadOnly, Category = "Quest State")
    bool bIsQuestComplete;

    UPROPERTY(BlueprintReadOnly, Category = "Quest State")
    float QuestTimeLimit;

    UPROPERTY(BlueprintReadOnly, Category = "Quest State")
    float QuestTimeRemaining;

    // Observation Requirements
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Config")
    int32 RequiredObservations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Config")
    float MinObservationDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Config")
    float MaxObservationDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Config")
    float MinObservationDuration;

    // Observation Data
    UPROPERTY(BlueprintReadOnly, Category = "Observation Data")
    TArray<FQuest_HerdObservation> CompletedObservations;

    UPROPERTY(BlueprintReadOnly, Category = "Observation Data")
    TArray<FString> TargetSpecies;

    // Timer Handles
    FTimerHandle QuestTimerHandle;
    FTimerHandle HerdDetectionTimerHandle;
    FTimerHandle ObservationTimerHandle;

    // References
    UPROPERTY(BlueprintReadOnly, Category = "References")
    ATranspersonalCharacter* PlayerCharacter;

    UPROPERTY(BlueprintReadOnly, Category = "References")
    UCrowd_MassSimulationManager* CrowdManager;

private:
    // Internal Quest Logic
    void UpdateQuestTimer();
    void OnQuestTimeExpired();
    void ValidateObservationRequirements();
    bool IsPlayerInObservationRange(const FVector& HerdLocation) const;
    void ProcessHerdBehaviorData(AActor* HerdActor, EQuest_HerdStudyObjective ObjectiveType);
    void UpdateQuestObjectives();
    void NotifyQuestProgress();
};