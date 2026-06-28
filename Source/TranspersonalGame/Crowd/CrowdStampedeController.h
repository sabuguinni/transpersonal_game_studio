#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CrowdStampedeController.generated.h"

UENUM(BlueprintType)
enum class ECrowd_StampedeState : uint8
{
    Grazing       UMETA(DisplayName = "Grazing"),
    Alert         UMETA(DisplayName = "Alert"),
    Fleeing       UMETA(DisplayName = "Fleeing"),
    Scattered     UMETA(DisplayName = "Scattered"),
    Regrouping    UMETA(DisplayName = "Regrouping")
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
    ECrowd_StampedeState State;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd|Stampede")
    float FearLevel;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd|Stampede")
    float LODDistance;

    FCrowd_StampedeAgent()
        : Location(FVector::ZeroVector)
        , Velocity(FVector::ZeroVector)
        , State(ECrowd_StampedeState::Grazing)
        , FearLevel(0.0f)
        , LODDistance(0.0f)
    {}
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ACrowd_StampedeController : public AActor
{
    GENERATED_BODY()

public:
    ACrowd_StampedeController();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Trigger stampede from a threat location
    UFUNCTION(BlueprintCallable, Category = "Crowd|Stampede")
    void TriggerStampede(FVector ThreatLocation, float ThreatRadius);

    // Update all agent states based on current threat
    UFUNCTION(BlueprintCallable, Category = "Crowd|Stampede")
    void UpdateAgentStates(float DeltaTime);

    // Get flee direction away from threat
    UFUNCTION(BlueprintPure, Category = "Crowd|Stampede")
    FVector GetFleeDirection(FVector AgentLocation, FVector ThreatLocation) const;

    // LOD: returns simplified representation distance
    UFUNCTION(BlueprintPure, Category = "Crowd|Stampede")
    float GetLODDistance(int32 AgentIndex) const;

    // Herd configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    int32 MaxAgents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    float HerdRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    float StampedeSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    float FearDecayRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    float AlertRadius;

    // LOD thresholds (cm)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|LOD")
    float LODNearDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|LOD")
    float LODMidDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|LOD")
    float LODFarDistance;

    // Runtime state
    UPROPERTY(BlueprintReadOnly, Category = "Crowd|State")
    TArray<FCrowd_StampedeAgent> Agents;

    UPROPERTY(BlueprintReadOnly, Category = "Crowd|State")
    ECrowd_StampedeState HerdState;

    UPROPERTY(BlueprintReadOnly, Category = "Crowd|State")
    FVector ActiveThreatLocation;

    UPROPERTY(BlueprintReadOnly, Category = "Crowd|State")
    bool bStampedeActive;

private:
    float StampedeTimer;
    float RegroupTimer;
};
