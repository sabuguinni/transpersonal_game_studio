#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CrowdStampedeController.generated.h"

UENUM(BlueprintType)
enum class ECrowd_StampedeState : uint8
{
    Calm        UMETA(DisplayName = "Calm"),
    Alert       UMETA(DisplayName = "Alert"),
    Fleeing     UMETA(DisplayName = "Fleeing"),
    Stampeding  UMETA(DisplayName = "Stampeding"),
    Scattered   UMETA(DisplayName = "Scattered")
};

USTRUCT(BlueprintType)
struct FCrowd_StampedeAgent
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Crowd|Stampede")
    FVector Location;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd|Stampede")
    FVector Velocity;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd|Stampede")
    float Speed;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd|Stampede")
    float PanicLevel;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd|Stampede")
    ECrowd_StampedeState State;

    FCrowd_StampedeAgent()
        : Location(FVector::ZeroVector)
        , Velocity(FVector::ZeroVector)
        , Speed(0.0f)
        , PanicLevel(0.0f)
        , State(ECrowd_StampedeState::Calm)
    {}
};

USTRUCT(BlueprintType)
struct FCrowd_StampedeTrigger
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Crowd|Stampede")
    FVector TriggerLocation;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd|Stampede")
    float TriggerRadius;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd|Stampede")
    float PanicStrength;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd|Stampede")
    float Duration;

    FCrowd_StampedeTrigger()
        : TriggerLocation(FVector::ZeroVector)
        , TriggerRadius(500.0f)
        , PanicStrength(1.0f)
        , Duration(10.0f)
    {}
};

UCLASS(ClassGroup = (Crowd), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCrowdStampedeController : public UActorComponent
{
    GENERATED_BODY()

public:
    UCrowdStampedeController();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UFUNCTION(BlueprintCallable, Category = "Crowd|Stampede")
    void TriggerStampede(FVector Origin, float Radius, float PanicStrength);

    UFUNCTION(BlueprintCallable, Category = "Crowd|Stampede")
    void AddAgent(FVector Location, float Speed);

    UFUNCTION(BlueprintCallable, Category = "Crowd|Stampede")
    void ClearAllAgents();

    UFUNCTION(BlueprintPure, Category = "Crowd|Stampede")
    int32 GetActiveAgentCount() const;

    UFUNCTION(BlueprintPure, Category = "Crowd|Stampede")
    ECrowd_StampedeState GetOverallHerdState() const;

    UFUNCTION(BlueprintCallable, Category = "Crowd|Stampede")
    void SetMaxAgents(int32 NewMax);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Stampede")
    int32 MaxAgents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Stampede")
    float SeparationRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Stampede")
    float CohesionRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Stampede")
    float AlignmentRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Stampede")
    float PanicDecayRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Stampede")
    float StampedeSpeedMultiplier;

private:
    UPROPERTY()
    TArray<FCrowd_StampedeAgent> Agents;

    UPROPERTY()
    TArray<FCrowd_StampedeTrigger> ActiveTriggers;

    void UpdateAgentFlocking(FCrowd_StampedeAgent& Agent, float DeltaTime);
    void UpdateAgentPanic(FCrowd_StampedeAgent& Agent, float DeltaTime);
    FVector ComputeSeparationForce(const FCrowd_StampedeAgent& Agent) const;
    FVector ComputeCohesionForce(const FCrowd_StampedeAgent& Agent) const;
    FVector ComputeAlignmentForce(const FCrowd_StampedeAgent& Agent) const;
    FVector ComputePanicFleeForce(const FCrowd_StampedeAgent& Agent) const;
};
