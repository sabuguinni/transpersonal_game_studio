#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Components/TextRenderComponent.h"
#include "SharedTypes.h"
#include "Quest_PracticalSurvivalMissions.generated.h"

UENUM(BlueprintType)
enum class EQuest_SurvivalMissionType : uint8
{
    FindWater UMETA(DisplayName = "Find Water Source"),
    BuildShelter UMETA(DisplayName = "Build Basic Shelter"),
    CraftTool UMETA(DisplayName = "Craft Stone Tool"),
    HuntPrey UMETA(DisplayName = "Hunt Small Prey"),
    ExploreCave UMETA(DisplayName = "Explore Safe Cave"),
    AvoidPredator UMETA(DisplayName = "Avoid Large Predator"),
    GatherResources UMETA(DisplayName = "Gather Basic Resources"),
    ScoutArea UMETA(DisplayName = "Scout Safe Territory")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_SurvivalMissionData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    EQuest_SurvivalMissionType MissionType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    FString MissionTitle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    FString MissionDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    FVector TargetLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    float CompletionRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    bool bIsActive;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    bool bIsCompleted;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    int32 Priority;

    FQuest_SurvivalMissionData()
    {
        MissionType = EQuest_SurvivalMissionType::FindWater;
        MissionTitle = TEXT("Survival Mission");
        MissionDescription = TEXT("Complete this mission to survive");
        TargetLocation = FVector::ZeroVector;
        CompletionRadius = 500.0f;
        bIsActive = false;
        bIsCompleted = false;
        Priority = 1;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AQuest_PracticalSurvivalMissions : public AActor
{
    GENERATED_BODY()

public:
    AQuest_PracticalSurvivalMissions();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootSceneComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USphereComponent* DetectionSphere;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UTextRenderComponent* MissionText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission System")
    TArray<FQuest_SurvivalMissionData> AvailableMissions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission System")
    FQuest_SurvivalMissionData CurrentMission;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission System")
    bool bAutoStartNextMission;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission System")
    float MissionCheckInterval;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Mission System")
    void InitializeSurvivalMissions();

    UFUNCTION(BlueprintCallable, Category = "Mission System")
    void StartMission(EQuest_SurvivalMissionType MissionType);

    UFUNCTION(BlueprintCallable, Category = "Mission System")
    void CompleteMission();

    UFUNCTION(BlueprintCallable, Category = "Mission System")
    bool CheckMissionProgress();

    UFUNCTION(BlueprintCallable, Category = "Mission System")
    FQuest_SurvivalMissionData GetNextPriorityMission();

    UFUNCTION(BlueprintCallable, Category = "Mission System")
    void UpdateMissionDisplay();

    UFUNCTION(BlueprintCallable, Category = "Mission System")
    TArray<FQuest_SurvivalMissionData> GetActiveMissions();

    UFUNCTION(BlueprintCallable, Category = "Mission System")
    float GetDistanceToMissionTarget();

protected:
    UFUNCTION()
    void OnPlayerEnterDetection(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnPlayerExitDetection(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

private:
    void CreateBasicSurvivalMissions();
    void CheckPlayerProximityToTarget();
    
    UPROPERTY()
    class ATranspersonalCharacter* PlayerCharacter;

    float LastMissionCheck;
    bool bPlayerInRange;
};