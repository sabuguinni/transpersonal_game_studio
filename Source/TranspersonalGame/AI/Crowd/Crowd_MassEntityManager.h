#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "SharedTypes.h"
#include "Crowd_MassEntityManager.generated.h"

UENUM(BlueprintType)
enum class ECrowd_BehaviorState : uint8
{
    IdleStanding        UMETA(DisplayName = "Idle Standing"),
    WalkingPatrol       UMETA(DisplayName = "Walking Patrol"),
    GatheringSocial     UMETA(DisplayName = "Gathering Social"),
    FleeingDanger       UMETA(DisplayName = "Fleeing Danger"),
    WorkingActivity     UMETA(DisplayName = "Working Activity")
};

UENUM(BlueprintType)
enum class ECrowd_LODLevel : uint8
{
    High        UMETA(DisplayName = "High Detail"),
    Medium      UMETA(DisplayName = "Medium Detail"),
    Low         UMETA(DisplayName = "Low Detail"),
    Culled      UMETA(DisplayName = "Culled")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_NPCData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd NPC")
    FVector Location;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd NPC")
    FRotator Rotation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd NPC")
    ECrowd_BehaviorState BehaviorState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd NPC")
    ECrowd_LODLevel LODLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd NPC")
    float MovementSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd NPC")
    FVector PatrolTarget;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd NPC")
    bool bIsVisible;

    FCrowd_NPCData()
    {
        Location = FVector::ZeroVector;
        Rotation = FRotator::ZeroRotator;
        BehaviorState = ECrowd_BehaviorState::IdleStanding;
        LODLevel = ECrowd_LODLevel::High;
        MovementSpeed = 100.0f;
        PatrolTarget = FVector::ZeroVector;
        bIsVisible = true;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_BiomeDistribution
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FString BiomeName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FVector CenterLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    int32 MaxNPCs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    int32 CurrentNPCs;

    FCrowd_BiomeDistribution()
    {
        BiomeName = TEXT("Unknown");
        CenterLocation = FVector::ZeroVector;
        MaxNPCs = 200;
        CurrentNPCs = 0;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ACrowd_MassEntityManager : public AActor
{
    GENERATED_BODY()

public:
    ACrowd_MassEntityManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Core crowd management
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Management")
    int32 MaxCrowdSize;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Management")
    float UpdateFrequency;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Management")
    bool bEnableLODSystem;

    // LOD distances
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD System")
    float LODCloseDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD System")
    float LODMediumDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD System")
    float LODFarDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD System")
    float LODCullDistance;

    // Biome distribution
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Distribution")
    TArray<FCrowd_BiomeDistribution> BiomeData;

    // NPC management
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NPC Management")
    TArray<FCrowd_NPCData> ActiveNPCs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Management")
    float MovementUpdateInterval;

    // Performance monitoring
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Performance")
    int32 VisibleNPCCount;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Performance")
    float LastUpdateTime;

    // Crowd behavior functions
    UFUNCTION(BlueprintCallable, Category = "Crowd Management")
    void InitializeCrowdSystem();

    UFUNCTION(BlueprintCallable, Category = "Crowd Management")
    void SpawnCrowdNPCs(int32 Count);

    UFUNCTION(BlueprintCallable, Category = "Crowd Management")
    void UpdateCrowdLOD();

    UFUNCTION(BlueprintCallable, Category = "Crowd Management")
    void UpdateCrowdBehavior();

    UFUNCTION(BlueprintCallable, Category = "LOD System")
    ECrowd_LODLevel CalculateLODLevel(const FVector& NPCLocation, const FVector& PlayerLocation);

    UFUNCTION(BlueprintCallable, Category = "Biome Distribution")
    FVector GetRandomLocationInBiome(const FString& BiomeName);

    UFUNCTION(BlueprintCallable, Category = "Biome Distribution")
    void DistributeNPCsAcrossBiomes();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void OptimizeCrowdPerformance();

    UFUNCTION(BlueprintCallable, Category = "Behavior")
    void AssignRandomBehavior(int32 NPCIndex);

    UFUNCTION(BlueprintCallable, Category = "Behavior")
    void ProcessFleeingBehavior(int32 NPCIndex);

    UFUNCTION(BlueprintCallable, Category = "Behavior")
    void ProcessGatheringBehavior(int32 NPCIndex);

    UFUNCTION(BlueprintCallable, Category = "Behavior")
    void ProcessPatrolBehavior(int32 NPCIndex);

private:
    float LastMovementUpdate;
    float LastLODUpdate;
    
    void UpdateNPCMovement(float DeltaTime);
    void CleanupInvalidNPCs();
    void EnforceCrowdLimits();
};