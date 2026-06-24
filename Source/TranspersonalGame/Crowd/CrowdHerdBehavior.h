#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CrowdHerdBehavior.generated.h"

UENUM(BlueprintType)
enum class ECrowd_HerdState : uint8
{
    Grazing     UMETA(DisplayName = "Grazing"),
    Migrating   UMETA(DisplayName = "Migrating"),
    Fleeing     UMETA(DisplayName = "Fleeing"),
    Resting     UMETA(DisplayName = "Resting")
};

USTRUCT(BlueprintType)
struct FCrowd_HerdAgent
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    FVector Location;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    FVector Velocity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    ECrowd_HerdState State;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    float FearLevel;

    FCrowd_HerdAgent()
        : Location(FVector::ZeroVector)
        , Velocity(FVector::ZeroVector)
        , State(ECrowd_HerdState::Grazing)
        , FearLevel(0.0f)
    {}
};

UCLASS(ClassGroup = (Crowd), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API ACrowd_HerdBehaviorActor : public AActor
{
    GENERATED_BODY()

public:
    ACrowd_HerdBehaviorActor();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Herd")
    int32 HerdSize;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Herd")
    float CohesionRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Herd")
    float SeparationRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Herd")
    float AlignmentRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Herd")
    float MaxSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Herd")
    ECrowd_HerdState CurrentHerdState;

    UPROPERTY(BlueprintReadOnly, Category = "Crowd|Herd")
    TArray<FCrowd_HerdAgent> Agents;

    UFUNCTION(BlueprintCallable, Category = "Crowd|Herd")
    void InitializeHerd(int32 Count, FVector Origin, float SpreadRadius);

    UFUNCTION(BlueprintCallable, Category = "Crowd|Herd")
    void TriggerFlee(FVector ThreatLocation, float ThreatRadius);

    UFUNCTION(BlueprintCallable, Category = "Crowd|Herd")
    void SetHerdState(ECrowd_HerdState NewState);

    UFUNCTION(BlueprintCallable, Category = "Crowd|Herd")
    int32 GetAgentCount() const;

private:
    void UpdateBoids(float DeltaTime);
    FVector ComputeCohesion(int32 AgentIndex);
    FVector ComputeSeparation(int32 AgentIndex);
    FVector ComputeAlignment(int32 AgentIndex);
    FVector ComputeFlee(int32 AgentIndex, FVector ThreatPos);

    FVector ActiveThreatLocation;
    bool bHasThreat;
    float ThreatDecayTimer;
};
