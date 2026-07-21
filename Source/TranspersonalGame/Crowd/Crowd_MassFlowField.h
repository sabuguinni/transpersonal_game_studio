#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "MassEntityTypes.h"
#include "MassCommonTypes.h"
#include "SharedTypes.h"
#include "Crowd_MassFlowField.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_FlowFieldCell
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Flow Field")
    FVector FlowDirection;

    UPROPERTY(BlueprintReadOnly, Category = "Flow Field")
    float Cost;

    UPROPERTY(BlueprintReadOnly, Category = "Flow Field")
    bool bIsObstacle;

    UPROPERTY(BlueprintReadOnly, Category = "Flow Field")
    float DistanceToTarget;

    FCrowd_FlowFieldCell()
        : FlowDirection(FVector::ZeroVector)
        , Cost(1.0f)
        , bIsObstacle(false)
        , DistanceToTarget(FLT_MAX)
    {}
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_FlowFieldGrid
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Flow Field")
    TArray<FCrowd_FlowFieldCell> Cells;

    UPROPERTY(BlueprintReadOnly, Category = "Flow Field")
    int32 GridWidth;

    UPROPERTY(BlueprintReadOnly, Category = "Flow Field")
    int32 GridHeight;

    UPROPERTY(BlueprintReadOnly, Category = "Flow Field")
    float CellSize;

    UPROPERTY(BlueprintReadOnly, Category = "Flow Field")
    FVector WorldOrigin;

    FCrowd_FlowFieldGrid()
        : GridWidth(0)
        , GridHeight(0)
        , CellSize(100.0f)
        , WorldOrigin(FVector::ZeroVector)
    {}
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCrowd_MassFlowField : public UActorComponent
{
    GENERATED_BODY()

public:
    UCrowd_MassFlowField();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Flow field generation
    UFUNCTION(BlueprintCallable, Category = "Flow Field")
    void GenerateFlowField(const FVector& TargetLocation, float MaxDistance = 5000.0f);

    UFUNCTION(BlueprintCallable, Category = "Flow Field")
    void ClearFlowField();

    // Flow field queries
    UFUNCTION(BlueprintCallable, Category = "Flow Field")
    FVector GetFlowDirection(const FVector& WorldPosition) const;

    UFUNCTION(BlueprintCallable, Category = "Flow Field")
    float GetFlowCost(const FVector& WorldPosition) const;

    UFUNCTION(BlueprintCallable, Category = "Flow Field")
    bool IsObstacle(const FVector& WorldPosition) const;

    // Grid management
    UFUNCTION(BlueprintCallable, Category = "Flow Field")
    void InitializeGrid(const FVector& Origin, int32 Width, int32 Height, float CellSize = 100.0f);

    UFUNCTION(BlueprintCallable, Category = "Flow Field")
    void AddObstacle(const FVector& WorldPosition, float Radius = 50.0f);

    UFUNCTION(BlueprintCallable, Category = "Flow Field")
    void RemoveObstacle(const FVector& WorldPosition, float Radius = 50.0f);

    // Mass Entity integration
    UFUNCTION(BlueprintCallable, Category = "Mass Entity")
    void UpdateMassAgentFlowField(FMassEntityHandle EntityHandle, const FVector& AgentPosition);

    UFUNCTION(BlueprintCallable, Category = "Mass Entity")
    FVector CalculateMassAgentDirection(const FVector& AgentPosition, const FVector& AgentVelocity) const;

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Flow Field")
    FCrowd_FlowFieldGrid FlowGrid;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flow Field")
    float DefaultCellSize;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flow Field")
    float MaxFlowDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flow Field")
    float ObstacleCost;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flow Field")
    bool bAutoUpdateFlowField;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flow Field")
    float UpdateFrequency;

private:
    // Grid coordinate conversion
    FIntPoint WorldToGrid(const FVector& WorldPosition) const;
    FVector GridToWorld(const FIntPoint& GridPosition) const;
    int32 GridToIndex(const FIntPoint& GridPosition) const;
    FIntPoint IndexToGrid(int32 Index) const;

    // Flow field calculation
    void CalculateDistanceField(const FVector& TargetLocation);
    void CalculateFlowField();
    void PropagateDistances(const FIntPoint& StartCell);

    // Obstacle detection
    void DetectStaticObstacles();
    bool IsWorldPositionBlocked(const FVector& WorldPosition) const;

    float LastUpdateTime;
    FVector CurrentTargetLocation;
    bool bFlowFieldValid;
};