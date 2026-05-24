#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "Engine/TriggerBox.h"
#include "../SharedTypes.h"
#include "Quest_ProgressiveObjectiveSystem.generated.h"

UENUM(BlueprintType)
enum class EQuest_ProgressionPhase : uint8
{
    BasicSurvival = 0,
    FirstTools = 1,
    BeginnerHunter = 2,
    ExperiencedExplorer = 3,
    SurvivalMaster = 4
};

UENUM(BlueprintType)
enum class EQuest_ObjectiveType : uint8
{
    CollectResource,
    CraftItem,
    HuntCreature,
    ExploreLocation,
    SurviveTime,
    BuildStructure
};

USTRUCT(BlueprintType)
struct FQuest_ProgressiveObjective
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
    FString ObjectiveID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
    FString Title;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
    EQuest_ObjectiveType ObjectiveType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
    int32 RequiredCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
    int32 CurrentCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
    bool bIsCompleted;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
    FVector TargetLocation;

    FQuest_ProgressiveObjective()
    {
        ObjectiveID = "";
        Title = "";
        Description = "";
        ObjectiveType = EQuest_ObjectiveType::CollectResource;
        RequiredCount = 1;
        CurrentCount = 0;
        bIsCompleted = false;
        TargetLocation = FVector::ZeroVector;
    }
};

USTRUCT(BlueprintType)
struct FQuest_ProgressionPhaseData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Phase")
    EQuest_ProgressionPhase Phase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Phase")
    FString PhaseTitle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Phase")
    TArray<FQuest_ProgressiveObjective> Objectives;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Phase")
    bool bIsUnlocked;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Phase")
    bool bIsCompleted;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Phase")
    FVector PhaseLocation;

    FQuest_ProgressionPhaseData()
    {
        Phase = EQuest_ProgressionPhase::BasicSurvival;
        PhaseTitle = "";
        bIsUnlocked = false;
        bIsCompleted = false;
        PhaseLocation = FVector::ZeroVector;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AQuest_ProgressiveObjectiveSystem : public AActor
{
    GENERATED_BODY()

public:
    AQuest_ProgressiveObjectiveSystem();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progression System")
    TArray<FQuest_ProgressionPhaseData> ProgressionPhases;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progression System")
    EQuest_ProgressionPhase CurrentPhase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progression System")
    int32 TotalObjectivesCompleted;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progression System")
    float SurvivalTimeMinutes;

public:
    UFUNCTION(BlueprintCallable, Category = "Progression")
    void InitializeProgressionSystem();

    UFUNCTION(BlueprintCallable, Category = "Progression")
    void UpdateObjectiveProgress(const FString& ObjectiveID, int32 ProgressAmount);

    UFUNCTION(BlueprintCallable, Category = "Progression")
    bool CompleteObjective(const FString& ObjectiveID);

    UFUNCTION(BlueprintCallable, Category = "Progression")
    void UnlockNextPhase();

    UFUNCTION(BlueprintCallable, Category = "Progression")
    FQuest_ProgressionPhaseData GetCurrentPhaseData() const;

    UFUNCTION(BlueprintCallable, Category = "Progression")
    TArray<FQuest_ProgressiveObjective> GetActiveObjectives() const;

    UFUNCTION(BlueprintCallable, Category = "Progression")
    bool IsPhaseCompleted(EQuest_ProgressionPhase Phase) const;

    UFUNCTION(BlueprintCallable, Category = "Progression")
    float GetOverallProgressPercentage() const;

    UFUNCTION(BlueprintCallable, Category = "Progression")
    void RegisterResourceCollection(const FString& ResourceType, int32 Amount);

    UFUNCTION(BlueprintCallable, Category = "Progression")
    void RegisterCreatureKill(const FString& CreatureType);

    UFUNCTION(BlueprintCallable, Category = "Progression")
    void RegisterLocationExplored(const FString& LocationName);

    UFUNCTION(BlueprintCallable, Category = "Progression")
    void RegisterItemCrafted(const FString& ItemType);

private:
    void SetupDefaultProgression();
    void CheckPhaseCompletion();
    FQuest_ProgressiveObjective* FindObjectiveByID(const FString& ObjectiveID);
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AQuest_ObjectiveTracker : public ATriggerBox
{
    GENERATED_BODY()

public:
    AQuest_ObjectiveTracker();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective Tracker")
    FString TrackedObjectiveID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective Tracker")
    EQuest_ObjectiveType TrackerType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective Tracker")
    FString RequiredResourceType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective Tracker")
    int32 RequiredAmount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective Tracker")
    bool bIsActive;

public:
    UFUNCTION()
    virtual void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, 
                               UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, 
                               bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION(BlueprintCallable, Category = "Tracker")
    void ActivateTracker();

    UFUNCTION(BlueprintCallable, Category = "Tracker")
    void DeactivateTracker();

    UFUNCTION(BlueprintCallable, Category = "Tracker")
    void SetTrackedObjective(const FString& ObjectiveID, EQuest_ObjectiveType Type);
};