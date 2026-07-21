#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Crowd_TrafficFlowManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_TrafficNode
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector Location;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Capacity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float CurrentDensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<int32> ConnectedNodes;

    FCrowd_TrafficNode()
    {
        Location = FVector::ZeroVector;
        Capacity = 100.0f;
        CurrentDensity = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_FlowField
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector FlowDirection;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float FlowStrength;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Congestion;

    FCrowd_FlowField()
    {
        FlowDirection = FVector::ForwardVector;
        FlowStrength = 1.0f;
        Congestion = 0.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ACrowd_TrafficFlowManager : public AActor
{
    GENERATED_BODY()

public:
    ACrowd_TrafficFlowManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traffic Flow")
    TArray<FCrowd_TrafficNode> TrafficNodes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traffic Flow")
    TMap<FVector2D, FCrowd_FlowField> FlowFieldGrid;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traffic Flow")
    float GridCellSize;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traffic Flow")
    float MaxFlowSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traffic Flow")
    float CongestionThreshold;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootSceneComponent;

    UFUNCTION(BlueprintCallable, Category = "Traffic Flow")
    void InitializeTrafficNetwork();

    UFUNCTION(BlueprintCallable, Category = "Traffic Flow")
    void UpdateFlowFields();

    UFUNCTION(BlueprintCallable, Category = "Traffic Flow")
    FVector GetFlowDirection(const FVector& WorldLocation);

    UFUNCTION(BlueprintCallable, Category = "Traffic Flow")
    float GetCongestionLevel(const FVector& WorldLocation);

    UFUNCTION(BlueprintCallable, Category = "Traffic Flow")
    void AddTrafficNode(const FVector& Location, float Capacity);

    UFUNCTION(BlueprintCallable, Category = "Traffic Flow")
    void ConnectNodes(int32 NodeA, int32 NodeB);

    UFUNCTION(BlueprintCallable, Category = "Traffic Flow")
    void UpdateNodeDensity(int32 NodeIndex, float DensityChange);

private:
    void CalculateFlowField(const FVector2D& GridPosition);
    void PropagateFlow();
    FVector2D WorldToGrid(const FVector& WorldLocation);
    FVector GridToWorld(const FVector2D& GridPosition);
};