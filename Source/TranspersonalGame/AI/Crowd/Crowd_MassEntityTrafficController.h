#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MassEntityTypes.h"
#include "MassProcessor.h"
#include "MassCommonTypes.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "SharedTypes.h"
#include "Crowd_MassEntityTrafficController.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_TrafficFlowData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traffic Flow")
    FVector FlowDirection;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traffic Flow")
    float FlowSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traffic Flow")
    int32 EntityCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traffic Flow")
    float DensityFactor;

    FCrowd_TrafficFlowData()
    {
        FlowDirection = FVector::ForwardVector;
        FlowSpeed = 200.0f;
        EntityCount = 0;
        DensityFactor = 1.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_TrafficLane
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traffic Lane")
    FVector StartPoint;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traffic Lane")
    FVector EndPoint;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traffic Lane")
    float LaneWidth;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traffic Lane")
    int32 MaxEntities;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traffic Lane")
    bool bIsBidirectional;

    FCrowd_TrafficLane()
    {
        StartPoint = FVector::ZeroVector;
        EndPoint = FVector(1000.0f, 0.0f, 0.0f);
        LaneWidth = 200.0f;
        MaxEntities = 500;
        bIsBidirectional = false;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ACrowd_MassEntityTrafficController : public AActor
{
    GENERATED_BODY()

public:
    ACrowd_MassEntityTrafficController();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Traffic control components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* ControllerMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* TrafficRoot;

    // Traffic flow configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traffic Control")
    TArray<FCrowd_TrafficLane> TrafficLanes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traffic Control")
    FCrowd_TrafficFlowData CurrentTrafficFlow;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traffic Control")
    float TrafficDensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traffic Control")
    int32 MaxConcurrentEntities;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traffic Control")
    bool bEnableTrafficControl;

    // Performance settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float UpdateFrequency;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float CullingDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bUseDistanceCulling;

public:
    // Traffic control methods
    UFUNCTION(BlueprintCallable, Category = "Traffic Control")
    void InitializeTrafficSystem();

    UFUNCTION(BlueprintCallable, Category = "Traffic Control")
    void UpdateTrafficFlow(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Traffic Control")
    void AddTrafficLane(const FCrowd_TrafficLane& NewLane);

    UFUNCTION(BlueprintCallable, Category = "Traffic Control")
    void RemoveTrafficLane(int32 LaneIndex);

    UFUNCTION(BlueprintCallable, Category = "Traffic Control")
    void SetTrafficDensity(float NewDensity);

    UFUNCTION(BlueprintCallable, Category = "Traffic Control")
    int32 GetActiveEntityCount() const;

    UFUNCTION(BlueprintCallable, Category = "Traffic Control")
    void PauseTrafficFlow();

    UFUNCTION(BlueprintCallable, Category = "Traffic Control")
    void ResumeTrafficFlow();

    UFUNCTION(BlueprintCallable, Category = "Traffic Control")
    void ClearAllTraffic();

    // Lane management
    UFUNCTION(BlueprintCallable, Category = "Lane Management")
    bool IsLaneAvailable(int32 LaneIndex) const;

    UFUNCTION(BlueprintCallable, Category = "Lane Management")
    FVector GetLaneDirection(int32 LaneIndex) const;

    UFUNCTION(BlueprintCallable, Category = "Lane Management")
    float GetLaneOccupancy(int32 LaneIndex) const;

    UFUNCTION(BlueprintCallable, Category = "Lane Management")
    void OptimizeLaneDistribution();

private:
    // Internal traffic state
    TArray<int32> LaneEntityCounts;
    float LastUpdateTime;
    bool bTrafficPaused;
    int32 TotalActiveEntities;

    // Internal methods
    void UpdateLaneOccupancy();
    void BalanceTrafficLoad();
    void CullDistantEntities();
    void ValidateTrafficLanes();
};