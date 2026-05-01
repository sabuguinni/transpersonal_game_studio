#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/TriggerVolume.h"
#include "Quest_SurvivalMissionController.generated.h"

UENUM(BlueprintType)
enum class EQuest_SurvivalMissionType : uint8
{
    WaterGathering     UMETA(DisplayName = "Water Gathering"),
    FoodForaging      UMETA(DisplayName = "Food Foraging"),
    ShelterBuilding   UMETA(DisplayName = "Shelter Building"),
    ToolCrafting      UMETA(DisplayName = "Tool Crafting"),
    TerritoryMapping  UMETA(DisplayName = "Territory Mapping"),
    PredatorAvoidance UMETA(DisplayName = "Predator Avoidance")
};

UENUM(BlueprintType)
enum class EQuest_MissionPriority : uint8
{
    Critical    UMETA(DisplayName = "Critical - Life Threatening"),
    High        UMETA(DisplayName = "High - Survival Essential"),
    Medium      UMETA(DisplayName = "Medium - Quality of Life"),
    Low         UMETA(DisplayName = "Low - Optional")
};

USTRUCT(BlueprintType)
struct FQuest_SurvivalObjective
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
    FString ObjectiveDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
    EQuest_SurvivalMissionType MissionType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
    EQuest_MissionPriority Priority;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
    float TimeLimit;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
    bool bIsCompleted;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
    FVector TargetLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
    float CompletionRadius;

    FQuest_SurvivalObjective()
    {
        ObjectiveDescription = TEXT("Survive");
        MissionType = EQuest_SurvivalMissionType::WaterGathering;
        Priority = EQuest_MissionPriority::Medium;
        TimeLimit = 300.0f;
        bIsCompleted = false;
        TargetLocation = FVector::ZeroVector;
        CompletionRadius = 200.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AQuest_SurvivalMissionController : public AActor
{
    GENERATED_BODY()

public:
    AQuest_SurvivalMissionController();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class USphereComponent* DetectionSphere;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UStaticMeshComponent* MissionMarker;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    TArray<FQuest_SurvivalObjective> ActiveObjectives;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    EQuest_SurvivalMissionType CurrentMissionType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    float MissionRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    bool bMissionActive;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    FString MissionTitle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    FString MissionDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    FString VoiceLineURL;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Mission")
    void StartMission(EQuest_SurvivalMissionType MissionType);

    UFUNCTION(BlueprintCallable, Category = "Mission")
    void CompleteMission();

    UFUNCTION(BlueprintCallable, Category = "Mission")
    void AddObjective(const FQuest_SurvivalObjective& NewObjective);

    UFUNCTION(BlueprintCallable, Category = "Mission")
    void CompleteObjective(int32 ObjectiveIndex);

    UFUNCTION(BlueprintCallable, Category = "Mission")
    bool CheckObjectiveCompletion(const FQuest_SurvivalObjective& Objective, const FVector& PlayerLocation);

    UFUNCTION(BlueprintCallable, Category = "Mission")
    void UpdateMissionProgress();

    UFUNCTION(BlueprintCallable, Category = "Mission")
    float GetMissionCompletionPercentage() const;

    UFUNCTION(BlueprintImplementableEvent, Category = "Mission")
    void OnMissionStarted(EQuest_SurvivalMissionType MissionType);

    UFUNCTION(BlueprintImplementableEvent, Category = "Mission")
    void OnMissionCompleted();

    UFUNCTION(BlueprintImplementableEvent, Category = "Mission")
    void OnObjectiveCompleted(int32 ObjectiveIndex);

protected:
    UFUNCTION()
    void OnDetectionSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    void InitializeWaterGatheringMission();
    void InitializeFoodForagingMission();
    void InitializeShelterBuildingMission();
    void InitializeToolCraftingMission();
    void InitializeTerritoryMappingMission();
    void InitializePredatorAvoidanceMission();

private:
    float MissionStartTime;
    bool bPlayerInRange;
};