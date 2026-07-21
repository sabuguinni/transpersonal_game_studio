#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CrowdBehaviorController.generated.h"

UENUM(BlueprintType)
enum class ECrowd_AgentState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Foraging    UMETA(DisplayName = "Foraging"),
    Fleeing     UMETA(DisplayName = "Fleeing"),
    Stampeding  UMETA(DisplayName = "Stampeding"),
    Grazing     UMETA(DisplayName = "Grazing"),
    Migrating   UMETA(DisplayName = "Migrating")
};

UENUM(BlueprintType)
enum class ECrowd_HerdSpecies : uint8
{
    Triceratops     UMETA(DisplayName = "Triceratops"),
    Parasaurolophus UMETA(DisplayName = "Parasaurolophus"),
    Gallimimus      UMETA(DisplayName = "Gallimimus"),
    Ankylosaurus    UMETA(DisplayName = "Ankylosaurus"),
    Brachiosaurus   UMETA(DisplayName = "Brachiosaurus")
};

USTRUCT(BlueprintType)
struct FCrowd_HerdAgent
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    FVector Location;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    FVector Velocity;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    ECrowd_AgentState State;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    ECrowd_HerdSpecies Species;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    float FearLevel;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    float StaminaLevel;

    FCrowd_HerdAgent()
        : Location(FVector::ZeroVector)
        , Velocity(FVector::ZeroVector)
        , State(ECrowd_AgentState::Idle)
        , Species(ECrowd_HerdSpecies::Gallimimus)
        , FearLevel(0.0f)
        , StaminaLevel(100.0f)
    {}
};

USTRUCT(BlueprintType)
struct FCrowd_StampedeData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    FVector TriggerLocation;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    FVector FleeDirection;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    float TriggerRadius;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    float StampedeSpeed;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    int32 AffectedAgentCount;

    FCrowd_StampedeData()
        : TriggerLocation(FVector::ZeroVector)
        , FleeDirection(FVector::ForwardVector)
        , TriggerRadius(500.0f)
        , StampedeSpeed(1200.0f)
        , AffectedAgentCount(0)
    {}
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ACrowdBehaviorController : public AActor
{
    GENERATED_BODY()

public:
    ACrowdBehaviorController();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    int32 MaxHerdSize;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    ECrowd_HerdSpecies HerdSpecies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    float SpawnRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    float FlockingRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    float SeparationRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    float MaxSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    float FearDecayRate;

    UPROPERTY(BlueprintReadOnly, Category = "Crowd|State")
    TArray<FCrowd_HerdAgent> HerdAgents;

    UPROPERTY(BlueprintReadOnly, Category = "Crowd|State")
    bool bStampedeActive;

    UPROPERTY(BlueprintReadOnly, Category = "Crowd|State")
    FCrowd_StampedeData ActiveStampede;

    UFUNCTION(BlueprintCallable, Category = "Crowd")
    void InitializeHerd();

    UFUNCTION(BlueprintCallable, Category = "Crowd")
    void TriggerStampede(FVector ThreatLocation, float ThreatRadius);

    UFUNCTION(BlueprintCallable, Category = "Crowd")
    void UpdateFlockingBehavior(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Crowd")
    int32 GetActiveAgentCount() const;

    UFUNCTION(BlueprintCallable, Category = "Crowd")
    void SetHerdFearLevel(float FearLevel);

private:
    FVector ComputeSeparation(int32 AgentIndex) const;
    FVector ComputeAlignment(int32 AgentIndex) const;
    FVector ComputeCohesion(int32 AgentIndex) const;
};
