#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Components/SceneComponent.h"
#include "../../SharedTypes.h"
#include "Crowd_Manager.generated.h"

class UCrowd_BehaviorComponent;
class UCrowd_PopulationComponent;

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_Zone
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Zone")
    FVector Center;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Zone")
    float Radius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Zone")
    int32 MaxPopulation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Zone")
    ECrowd_ActivityType ActivityType;

    FCrowd_Zone()
    {
        Center = FVector::ZeroVector;
        Radius = 500.0f;
        MaxPopulation = 50;
        ActivityType = ECrowd_ActivityType::Wandering;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_Agent
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Agent")
    FVector Position;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Agent")
    FVector Velocity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Agent")
    ECrowd_BehaviorState BehaviorState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Agent")
    float MovementSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Agent")
    int32 ZoneID;

    FCrowd_Agent()
    {
        Position = FVector::ZeroVector;
        Velocity = FVector::ZeroVector;
        BehaviorState = ECrowd_BehaviorState::Idle;
        MovementSpeed = 150.0f;
        ZoneID = -1;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ACrowd_Manager : public AActor
{
    GENERATED_BODY()

public:
    ACrowd_Manager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootSceneComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Settings")
    TArray<FCrowd_Zone> CrowdZones;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Settings")
    int32 MaxTotalAgents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Settings")
    float UpdateFrequency;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Settings")
    float LODDistance1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Settings")
    float LODDistance2;

    UPROPERTY(BlueprintReadOnly, Category = "Crowd State")
    TArray<FCrowd_Agent> ActiveAgents;

    UPROPERTY(BlueprintReadOnly, Category = "Crowd State")
    int32 CurrentAgentCount;

private:
    float LastUpdateTime;
    TArray<AActor*> SpawnedAgents;

public:
    UFUNCTION(BlueprintCallable, Category = "Crowd Management")
    void InitializeCrowdZones();

    UFUNCTION(BlueprintCallable, Category = "Crowd Management")
    void SpawnAgentsInZone(int32 ZoneIndex, int32 Count);

    UFUNCTION(BlueprintCallable, Category = "Crowd Management")
    void UpdateCrowdBehavior(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Crowd Management")
    void OptimizeLOD();

    UFUNCTION(BlueprintCallable, Category = "Crowd Management")
    FCrowd_Zone GetZoneAtLocation(FVector Location);

    UFUNCTION(BlueprintCallable, Category = "Crowd Management")
    void SetZoneActivity(int32 ZoneIndex, ECrowd_ActivityType NewActivity);

    UFUNCTION(BlueprintCallable, Category = "Crowd Management")
    int32 GetActiveAgentCount() const { return CurrentAgentCount; }
};