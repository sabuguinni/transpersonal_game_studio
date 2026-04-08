#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Engine/DataTable.h"
#include "CrowdManager.generated.h"

USTRUCT(BlueprintType)
struct FCrowdPersonData : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    UStaticMesh* Mesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float WalkSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float ConsciousnessLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FString> Behaviors;

    FCrowdPersonData()
    {
        Mesh = nullptr;
        WalkSpeed = 150.0f;
        ConsciousnessLevel = 0.5f;
    }
};

USTRUCT(BlueprintType)
struct FCrowdAgent
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector Position;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector Velocity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector Destination;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Speed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float ConsciousnessLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 BehaviorState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float StressLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsInCombat;

    FCrowdAgent()
    {
        Position = FVector::ZeroVector;
        Velocity = FVector::ZeroVector;
        Destination = FVector::ZeroVector;
        Speed = 150.0f;
        ConsciousnessLevel = 0.5f;
        BehaviorState = 0;
        StressLevel = 0.0f;
        bIsInCombat = false;
    }
};

UCLASS()
class TRANSPERSONALGAME_API ACrowdManager : public AActor
{
    GENERATED_BODY()

public:
    ACrowdManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Crowd Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Settings")
    int32 MaxCrowdSize;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Settings")
    float SpawnRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Settings")
    float DespawnRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Settings")
    UDataTable* CrowdDataTable;

    // Behavior Parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float SeparationRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float AlignmentRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float CohesionRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float AvoidanceRadius;

    // Consciousness Integration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Consciousness")
    float ConsciousnessInfluenceRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Consciousness")
    float StressDecayRate;

    // Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UInstancedStaticMeshComponent* CrowdMeshComponent;

    // Crowd Data
    UPROPERTY(BlueprintReadOnly, Category = "Crowd Data")
    TArray<FCrowdAgent> CrowdAgents;

    // Public Functions
    UFUNCTION(BlueprintCallable, Category = "Crowd Management")
    void SpawnCrowdAgent(FVector Location);

    UFUNCTION(BlueprintCallable, Category = "Crowd Management")
    void RemoveCrowdAgent(int32 AgentIndex);

    UFUNCTION(BlueprintCallable, Category = "Crowd Management")
    void SetCrowdDestination(FVector NewDestination);

    UFUNCTION(BlueprintCallable, Category = "Crowd Management")
    void TriggerCombatResponse(FVector CombatLocation, float Radius);

    UFUNCTION(BlueprintCallable, Category = "Consciousness")
    void UpdateConsciousnessInfluence(FVector Location, float Influence, float Radius);

private:
    // Internal Functions
    void UpdateCrowdBehavior(float DeltaTime);
    void UpdateCrowdPositions(float DeltaTime);
    void UpdateInstancedMeshes();
    
    FVector CalculateSeparation(const FCrowdAgent& Agent, int32 AgentIndex);
    FVector CalculateAlignment(const FCrowdAgent& Agent, int32 AgentIndex);
    FVector CalculateCohesion(const FCrowdAgent& Agent, int32 AgentIndex);
    FVector CalculateAvoidance(const FCrowdAgent& Agent);
    FVector CalculateSeek(const FCrowdAgent& Agent, FVector Target);
    
    void HandleCombatBehavior(FCrowdAgent& Agent, float DeltaTime);
    void HandleNormalBehavior(FCrowdAgent& Agent, float DeltaTime);
    
    // Spatial partitioning for performance
    TMap<FIntPoint, TArray<int32>> SpatialGrid;
    float GridCellSize;
    
    void UpdateSpatialGrid();
    TArray<int32> GetNearbyAgents(const FVector& Position, float Radius);
    
    // Performance tracking
    float LastUpdateTime;
    int32 UpdateBatchSize;
    int32 CurrentUpdateIndex;
};