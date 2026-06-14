#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Crowd_MassEntityManager.generated.h"

USTRUCT(BlueprintType)
struct FCrowd_AgentData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    FVector Position;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    FVector Velocity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    float Speed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    int32 AgentID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    bool bIsActive;

    FCrowd_AgentData()
    {
        Position = FVector::ZeroVector;
        Velocity = FVector::ZeroVector;
        Speed = 100.0f;
        AgentID = 0;
        bIsActive = true;
    }
};

USTRUCT(BlueprintType)
struct FCrowd_ZoneData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zone")
    FVector Center;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zone")
    float Radius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zone")
    int32 MaxAgents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zone")
    TArray<int32> ActiveAgents;

    FCrowd_ZoneData()
    {
        Center = FVector::ZeroVector;
        Radius = 500.0f;
        MaxAgents = 50;
    }
};

UENUM(BlueprintType)
enum class ECrowd_LODLevel : uint8
{
    High    UMETA(DisplayName = "High Detail"),
    Medium  UMETA(DisplayName = "Medium Detail"),
    Low     UMETA(DisplayName = "Low Detail"),
    Culled  UMETA(DisplayName = "Culled")
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ACrowd_MassEntityManager : public AActor
{
    GENERATED_BODY()

public:
    ACrowd_MassEntityManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* RootMeshComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Settings")
    int32 MaxTotalAgents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Settings")
    float UpdateFrequency;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Settings")
    float LODDistance_High;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Settings")
    float LODDistance_Medium;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Settings")
    float LODDistance_Low;

    UPROPERTY(BlueprintReadOnly, Category = "Runtime Data")
    TArray<FCrowd_AgentData> ActiveAgents;

    UPROPERTY(BlueprintReadOnly, Category = "Runtime Data")
    TArray<FCrowd_ZoneData> CrowdZones;

    UPROPERTY(BlueprintReadOnly, Category = "Runtime Data")
    int32 CurrentAgentCount;

public:
    UFUNCTION(BlueprintCallable, Category = "Crowd Management")
    void SpawnCrowdZone(FVector Center, float Radius, int32 AgentCount);

    UFUNCTION(BlueprintCallable, Category = "Crowd Management")
    void UpdateCrowdLOD(FVector PlayerPosition);

    UFUNCTION(BlueprintCallable, Category = "Crowd Management")
    void SetCrowdBehavior(int32 ZoneIndex, const FString& BehaviorType);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    int32 GetActiveAgentCount() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    float GetCurrentFPS() const;

protected:
    UFUNCTION()
    void UpdateAgentPositions(float DeltaTime);

    UFUNCTION()
    ECrowd_LODLevel CalculateLODLevel(float DistanceToPlayer) const;

    UFUNCTION()
    void OptimizePerformance();

private:
    float LastUpdateTime;
    float PerformanceTimer;
    bool bIsInitialized;
};