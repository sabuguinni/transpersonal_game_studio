#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CrowdSimulationManager.generated.h"

UENUM(BlueprintType)
enum class ECrowd_HerdBehavior : uint8
{
    Grazing     UMETA(DisplayName = "Grazing"),
    Migrating   UMETA(DisplayName = "Migrating"),
    Fleeing     UMETA(DisplayName = "Fleeing"),
    Resting     UMETA(DisplayName = "Resting"),
    Drinking    UMETA(DisplayName = "Drinking")
};

USTRUCT(BlueprintType)
struct FCrowd_WaypointData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    FVector Location = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    float Radius = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    float DensityWeight = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    ECrowd_HerdBehavior BehaviorAtWaypoint = ECrowd_HerdBehavior::Grazing;
};

USTRUCT(BlueprintType)
struct FCrowd_HerdConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    FString SpeciesName = TEXT("Herbivore");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    int32 HerdSize = 10;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    float MigrationSpeed = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    float FleeRadius = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    TArray<FCrowd_WaypointData> MigrationPath;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ACrowdSimulationManager : public AActor
{
    GENERATED_BODY()

public:
    ACrowdSimulationManager();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    int32 MaxSimultaneousAgents = 500;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    float LOD_NearDistance = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    float LOD_FarDistance = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    TArray<FCrowd_HerdConfig> RegisteredHerds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Waypoints")
    TArray<FCrowd_WaypointData> GlobalWaypoints;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Debug")
    bool bDrawDebugPaths = false;

    UFUNCTION(BlueprintCallable, Category = "Crowd")
    void RegisterHerd(const FCrowd_HerdConfig& HerdConfig);

    UFUNCTION(BlueprintCallable, Category = "Crowd")
    void TriggerFleeResponse(FVector ThreatLocation, float ThreatRadius);

    UFUNCTION(BlueprintCallable, Category = "Crowd")
    int32 GetActiveAgentCount() const;

    UFUNCTION(BlueprintCallable, Category = "Crowd")
    void SetHerdBehavior(const FString& SpeciesName, ECrowd_HerdBehavior NewBehavior);

    UFUNCTION(BlueprintCallable, Category = "Crowd")
    FCrowd_WaypointData GetNearestWaypoint(FVector FromLocation) const;

private:
    int32 ActiveAgentCount;
    float SimulationTickAccumulator;
    static constexpr float SimulationTickInterval = 0.1f;

    void UpdateHerdMigration(float DeltaTime);
    void ApplyLODCulling();
};
