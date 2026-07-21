#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "SharedTypes.h"
#include "Crowd_TrafficController.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_TrafficNode
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traffic")
    FVector Position;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traffic")
    float Capacity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traffic")
    TArray<int32> ConnectedNodes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traffic")
    ECrowd_TrafficType TrafficType;

    FCrowd_TrafficNode()
    {
        Position = FVector::ZeroVector;
        Capacity = 10.0f;
        TrafficType = ECrowd_TrafficType::Pedestrian;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_TrafficFlow
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traffic")
    int32 SourceNodeIndex;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traffic")
    int32 DestinationNodeIndex;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traffic")
    float FlowRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traffic")
    float CurrentLoad;

    FCrowd_TrafficFlow()
    {
        SourceNodeIndex = -1;
        DestinationNodeIndex = -1;
        FlowRate = 1.0f;
        CurrentLoad = 0.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ACrowd_TrafficController : public AActor
{
    GENERATED_BODY()

public:
    ACrowd_TrafficController();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traffic Network")
    TArray<FCrowd_TrafficNode> TrafficNodes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traffic Network")
    TArray<FCrowd_TrafficFlow> TrafficFlows;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float UpdateInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxActiveFlows;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float LODDistance;

private:
    float LastUpdateTime;
    TArray<int32> ActiveFlowIndices;

public:
    UFUNCTION(BlueprintCallable, Category = "Traffic Control")
    void InitializeTrafficNetwork();

    UFUNCTION(BlueprintCallable, Category = "Traffic Control")
    int32 AddTrafficNode(const FVector& Position, float Capacity, ECrowd_TrafficType TrafficType);

    UFUNCTION(BlueprintCallable, Category = "Traffic Control")
    void ConnectNodes(int32 NodeA, int32 NodeB, float FlowRate);

    UFUNCTION(BlueprintCallable, Category = "Traffic Control")
    void UpdateTrafficFlow(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Traffic Control")
    TArray<int32> FindOptimalPath(int32 StartNode, int32 EndNode);

    UFUNCTION(BlueprintCallable, Category = "Traffic Control")
    void OptimizeTrafficLoad();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void ApplyLODOptimization(const FVector& ViewerPosition);

    UFUNCTION(BlueprintCallable, Category = "Debug")
    void DrawTrafficNetwork();
};