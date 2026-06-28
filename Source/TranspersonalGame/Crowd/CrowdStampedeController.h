// CrowdStampedeController.h
// Agent #13 — Crowd & Traffic Simulation
// Stampede system: herbivore herds react to predator presence with panic flight

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CrowdStampedeController.generated.h"

UENUM(BlueprintType)
enum class ECrowd_StampedeState : uint8
{
    Calm        UMETA(DisplayName = "Calm"),
    Alert       UMETA(DisplayName = "Alert"),
    Panic       UMETA(DisplayName = "Panic"),
    Dispersing  UMETA(DisplayName = "Dispersing"),
    Regrouping  UMETA(DisplayName = "Regrouping")
};

UENUM(BlueprintType)
enum class ECrowd_TribalReaction : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Flee        UMETA(DisplayName = "Flee"),
    Defend      UMETA(DisplayName = "Defend"),
    ProtectYoung UMETA(DisplayName = "ProtectYoung"),
    Scatter     UMETA(DisplayName = "Scatter")
};

USTRUCT(BlueprintType)
struct FCrowd_HerdAgent
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    FVector Position;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    FVector Velocity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    ECrowd_StampedeState State;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    float PanicRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    float SeparationWeight;

    FCrowd_HerdAgent()
        : Position(FVector::ZeroVector)
        , Velocity(FVector::ZeroVector)
        , State(ECrowd_StampedeState::Calm)
        , PanicRadius(800.0f)
        , SeparationWeight(1.5f)
    {}
};

USTRUCT(BlueprintType)
struct FCrowd_TribalAgent
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    FVector Position;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    ECrowd_TribalReaction Reaction;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    FVector FleeTarget;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    bool bIsProtector;

    FCrowd_TribalAgent()
        : Position(FVector::ZeroVector)
        , Reaction(ECrowd_TribalReaction::Idle)
        , FleeTarget(FVector::ZeroVector)
        , bIsProtector(false)
    {}
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ACrowdStampedeController : public AActor
{
    GENERATED_BODY()

public:
    ACrowdStampedeController();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Stampede trigger — called when predator enters herd radius
    UFUNCTION(BlueprintCallable, Category = "Crowd|Stampede")
    void TriggerStampede(FVector PredatorLocation, float PredatorThreatRadius);

    // Update herd flocking behavior (separation, alignment, cohesion)
    UFUNCTION(BlueprintCallable, Category = "Crowd|Stampede")
    void UpdateHerdFlocking(float DeltaTime);

    // Tribal camp reaction to nearby stampede
    UFUNCTION(BlueprintCallable, Category = "Crowd|Tribal")
    void TriggerTribalReaction(FVector StampedeDirection);

    // Get current stampede state
    UFUNCTION(BlueprintPure, Category = "Crowd|Stampede")
    ECrowd_StampedeState GetStampedeState() const { return CurrentStampedeState; }

    // Calm the herd after threat passes
    UFUNCTION(BlueprintCallable, Category = "Crowd|Stampede")
    void CalmHerd();

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    int32 HerdSize;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    float StampedeSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    float CalmSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    float PanicDecayTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    float SeparationDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    float CohesionStrength;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    float AlignmentStrength;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Crowd|State")
    ECrowd_StampedeState CurrentStampedeState;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Crowd|State")
    FVector StampedeDirection;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Crowd|State")
    float PanicTimer;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Crowd|State")
    TArray<FCrowd_HerdAgent> HerdAgents;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Crowd|State")
    TArray<FCrowd_TribalAgent> TribalAgents;

private:
    FVector ComputeSeparationForce(int32 AgentIndex) const;
    FVector ComputeAlignmentForce(int32 AgentIndex) const;
    FVector ComputeCohesionForce(int32 AgentIndex) const;
};
