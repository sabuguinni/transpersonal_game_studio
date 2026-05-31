#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Components/SceneComponent.h"
#include "SharedTypes.h"
#include "Crowd_SimulationManager.generated.h"

UENUM(BlueprintType)
enum class ECrowd_BehaviorState : uint8
{
    Wandering       UMETA(DisplayName = "Wandering"),
    Gathering       UMETA(DisplayName = "Gathering"),
    Fleeing         UMETA(DisplayName = "Fleeing"),
    Working         UMETA(DisplayName = "Working"),
    Socializing     UMETA(DisplayName = "Socializing"),
    Idle            UMETA(DisplayName = "Idle")
};

UENUM(BlueprintType)
enum class ECrowd_LODLevel : uint8
{
    High            UMETA(DisplayName = "High Detail"),
    Medium          UMETA(DisplayName = "Medium Detail"),
    Low             UMETA(DisplayName = "Low Detail"),
    Culled          UMETA(DisplayName = "Culled")
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
    int32 SocialGroupID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd NPC")
    float MovementSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd NPC")
    FVector TargetLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd NPC")
    bool bIsActive;

    FCrowd_NPCData()
    {
        Location = FVector::ZeroVector;
        Rotation = FRotator::ZeroRotator;
        BehaviorState = ECrowd_BehaviorState::Idle;
        LODLevel = ECrowd_LODLevel::High;
        SocialGroupID = -1;
        MovementSpeed = 100.0f;
        TargetLocation = FVector::ZeroVector;
        bIsActive = true;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_BiomeData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Biome")
    FString BiomeName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Biome")
    FVector CenterLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Biome")
    float Radius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Biome")
    int32 MaxNPCs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Biome")
    TArray<FVector> PatrolPoints;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Biome")
    int32 CurrentNPCCount;

    FCrowd_BiomeData()
    {
        BiomeName = TEXT("Unknown");
        CenterLocation = FVector::ZeroVector;
        Radius = 15000.0f;
        MaxNPCs = 50;
        CurrentNPCCount = 0;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ACrowd_SimulationManager : public AActor
{
    GENERATED_BODY()

public:
    ACrowd_SimulationManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Core crowd simulation properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Simulation")
    int32 MaxCrowdSize;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Simulation")
    float UpdateInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Simulation")
    float LODUpdateDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Simulation")
    bool bEnableLODSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Simulation")
    bool bEnableSocialDynamics;

    // Biome management
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Management")
    TArray<FCrowd_BiomeData> BiomeData;

    // NPC management
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Management")
    TArray<FCrowd_NPCData> NPCDatabase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Management")
    TArray<AActor*> ActiveNPCs;

    // Performance settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float HighLODDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float MediumLODDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float LowLODDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float CullDistance;

    // Social dynamics
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social Dynamics")
    float SocialGroupRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social Dynamics")
    int32 MaxSocialGroups;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social Dynamics")
    float BehaviorChangeInterval;

    // Public functions
    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void InitializeCrowdSystem();

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void SpawnCrowdNPCs(int32 Count, const FString& BiomeName);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void UpdateLODSystem();

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void UpdateBehaviorStates();

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void CreateSocialGroups();

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void DistributeNPCsAcrossBiomes();

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    int32 GetTotalNPCCount() const;

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void SetMaxCrowdSize(int32 NewMaxSize);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void EnableLODSystem(bool bEnable);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void ClearAllNPCs();

private:
    // Internal functions
    void UpdateNPCLOD(AActor* NPC, float DistanceToPlayer);
    void AssignRandomBehavior(AActor* NPC);
    void SetupBiomePatrolPoints(FCrowd_BiomeData& Biome);
    FVector GetRandomLocationInBiome(const FCrowd_BiomeData& Biome);
    float CalculateDistanceToPlayer(const FVector& Location);
    void OptimizePerformance();

    // Internal state
    float LastUpdateTime;
    float LastBehaviorUpdateTime;
    int32 NextSocialGroupID;
    bool bSystemInitialized;

    UPROPERTY()
    USceneComponent* RootSceneComponent;
};