#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Kismet/GameplayStatics.h"
#include "SharedTypes.h"
#include "CrowdTrafficManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_TrafficRoute
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traffic Route")
    TArray<FVector> Waypoints;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traffic Route")
    float RouteSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traffic Route")
    int32 MaxAgents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traffic Route")
    FString RouteName;

    FCrowd_TrafficRoute()
    {
        RouteSpeed = 300.0f;
        MaxAgents = 50;
        RouteName = TEXT("DefaultRoute");
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_TrafficAgent
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traffic Agent")
    FVector CurrentPosition;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traffic Agent")
    FVector TargetPosition;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traffic Agent")
    float MovementSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traffic Agent")
    int32 CurrentWaypointIndex;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traffic Agent")
    int32 RouteID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traffic Agent")
    bool bIsActive;

    FCrowd_TrafficAgent()
    {
        CurrentPosition = FVector::ZeroVector;
        TargetPosition = FVector::ZeroVector;
        MovementSpeed = 300.0f;
        CurrentWaypointIndex = 0;
        RouteID = 0;
        bIsActive = true;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ACrowdTrafficManager : public AActor
{
    GENERATED_BODY()

public:
    ACrowdTrafficManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootSceneComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traffic System")
    TArray<FCrowd_TrafficRoute> TrafficRoutes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traffic System")
    TArray<FCrowd_TrafficAgent> ActiveAgents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traffic System")
    int32 MaxTotalAgents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traffic System")
    float SpawnInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traffic System")
    float CullingDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traffic System")
    TSubclassOf<AActor> AgentActorClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bEnableLODSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float HighDetailDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float MediumDetailDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float LowDetailDistance;

private:
    float LastSpawnTime;
    TArray<AActor*> SpawnedAgentActors;

public:
    UFUNCTION(BlueprintCallable, Category = "Traffic System")
    void InitializeTrafficSystem();

    UFUNCTION(BlueprintCallable, Category = "Traffic System")
    void AddTrafficRoute(const FCrowd_TrafficRoute& NewRoute);

    UFUNCTION(BlueprintCallable, Category = "Traffic System")
    void SpawnTrafficAgent(int32 RouteIndex);

    UFUNCTION(BlueprintCallable, Category = "Traffic System")
    void UpdateAgentMovement(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Traffic System")
    void UpdateLODSystem();

    UFUNCTION(BlueprintCallable, Category = "Traffic System")
    void ClearAllAgents();

    UFUNCTION(BlueprintCallable, Category = "Traffic System", CallInEditor)
    void CreateDefaultRoutes();

    UFUNCTION(BlueprintCallable, Category = "Traffic System")
    int32 GetActiveAgentCount() const { return ActiveAgents.Num(); }

    UFUNCTION(BlueprintCallable, Category = "Traffic System")
    void SetMaxAgents(int32 NewMax) { MaxTotalAgents = NewMax; }
};