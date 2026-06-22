#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "NPCBehaviorComponent.generated.h"

UENUM(BlueprintType)
enum class ENPC_BehaviorState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Patrol      UMETA(DisplayName = "Patrol"),
    Alert       UMETA(DisplayName = "Alert"),
    Chase       UMETA(DisplayName = "Chase"),
    Attack      UMETA(DisplayName = "Attack"),
    Flee        UMETA(DisplayName = "Flee"),
    Eat         UMETA(DisplayName = "Eat"),
    Rest        UMETA(DisplayName = "Rest"),
};

UENUM(BlueprintType)
enum class ENPC_DinoSpecies : uint8
{
    TRex        UMETA(DisplayName = "T-Rex"),
    Raptor      UMETA(DisplayName = "Raptor"),
    Brachiosaurus UMETA(DisplayName = "Brachiosaurus"),
    Generic     UMETA(DisplayName = "Generic"),
};

USTRUCT(BlueprintType)
struct FNPC_BehaviorConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Behavior")
    float PatrolRadius = 3000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Behavior")
    float ChaseRange = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Behavior")
    float AttackRange = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Behavior")
    float MoveSpeed = 600.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Behavior")
    bool bIsPassive = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Behavior")
    int32 PackSize = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Behavior")
    float FearRadiusNear = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Behavior")
    float FearRadiusFar = 3000.0f;
};

USTRUCT(BlueprintType)
struct FNPC_PerceptionData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "NPC|Perception")
    bool bPlayerDetected = false;

    UPROPERTY(BlueprintReadOnly, Category = "NPC|Perception")
    float DistanceToPlayer = 99999.0f;

    UPROPERTY(BlueprintReadOnly, Category = "NPC|Perception")
    FVector LastKnownPlayerLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadOnly, Category = "NPC|Perception")
    float TimeSinceLastSeen = 0.0f;
};

UCLASS(ClassGroup=(TranspersonalGame), meta=(BlueprintSpawnableComponent), BlueprintType)
class TRANSPERSONALGAME_API UNPCBehaviorComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UNPCBehaviorComponent();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // State machine
    UFUNCTION(BlueprintCallable, Category = "NPC|Behavior")
    void SetBehaviorState(ENPC_BehaviorState NewState);

    UFUNCTION(BlueprintPure, Category = "NPC|Behavior")
    ENPC_BehaviorState GetBehaviorState() const { return NPC_CurrentState; }

    // Perception
    UFUNCTION(BlueprintCallable, Category = "NPC|Perception")
    void UpdatePerception(float DeltaTime);

    UFUNCTION(BlueprintPure, Category = "NPC|Perception")
    float GetDistanceToPlayer() const { return NPC_Perception.DistanceToPlayer; }

    UFUNCTION(BlueprintPure, Category = "NPC|Perception")
    bool IsPlayerDetected() const { return NPC_Perception.bPlayerDetected; }

    // Config
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Config")
    ENPC_DinoSpecies NPC_Species = ENPC_DinoSpecies::Generic;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Config")
    FNPC_BehaviorConfig NPC_Config;

    // Current patrol waypoint index
    UPROPERTY(BlueprintReadOnly, Category = "NPC|Patrol")
    int32 NPC_CurrentWaypointIndex = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Patrol")
    TArray<FVector> NPC_PatrolWaypoints;

    // Fear output — read by AnimInstance
    UPROPERTY(BlueprintReadOnly, Category = "NPC|Output")
    float NPC_FearNormalized = 0.0f;

private:
    UPROPERTY()
    ENPC_BehaviorState NPC_CurrentState = ENPC_BehaviorState::Idle;

    UPROPERTY()
    FNPC_PerceptionData NPC_Perception;

    UPROPERTY()
    float NPC_StateTimer = 0.0f;

    UPROPERTY()
    float NPC_PatrolTimer = 0.0f;

    void UpdateStateMachine(float DeltaTime);
    void ApplySpeciesDefaults();
    APawn* FindPlayerPawn() const;
};
