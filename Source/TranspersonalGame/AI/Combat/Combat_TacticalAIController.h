#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Components/ActorComponent.h"
#include "GameFramework/Actor.h"
#include "SharedTypes.h"
#include "Combat_TacticalAIController.generated.h"

UENUM(BlueprintType)
enum class ECombat_TacticalState : uint8
{
    Patrol      UMETA(DisplayName = "Patrol"),
    Alert       UMETA(DisplayName = "Alert"),
    Hunt        UMETA(DisplayName = "Hunt"),
    Attack      UMETA(DisplayName = "Attack"),
    Retreat     UMETA(DisplayName = "Retreat"),
    Regroup     UMETA(DisplayName = "Regroup"),
    Flank       UMETA(DisplayName = "Flank"),
    Ambush      UMETA(DisplayName = "Ambush")
};

UENUM(BlueprintType)
enum class ECombat_FormationType : uint8
{
    Single      UMETA(DisplayName = "Single"),
    Pack        UMETA(DisplayName = "Pack"),
    Swarm       UMETA(DisplayName = "Swarm"),
    Pincer      UMETA(DisplayName = "Pincer"),
    Surround    UMETA(DisplayName = "Surround")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCombat_TacticalData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    ECombat_TacticalState CurrentState = ECombat_TacticalState::Patrol;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    ECombat_FormationType FormationType = ECombat_FormationType::Single;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float ThreatLevel = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float AttackRange = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float FlankingDistance = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    int32 PackSize = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    bool bIsPackLeader = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    bool bCanCallReinforcements = true;

    FCombat_TacticalData()
    {
        CurrentState = ECombat_TacticalState::Patrol;
        FormationType = ECombat_FormationType::Single;
        ThreatLevel = 0.0f;
        AttackRange = 200.0f;
        FlankingDistance = 300.0f;
        PackSize = 1;
        bIsPackLeader = false;
        bCanCallReinforcements = true;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCombat_DamageZone
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
    FVector Center = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
    float Radius = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
    float BaseDamage = 20.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
    FString DamageType = TEXT("Physical");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
    bool bIsActive = true;

    FCombat_DamageZone()
    {
        Center = FVector::ZeroVector;
        Radius = 100.0f;
        BaseDamage = 20.0f;
        DamageType = TEXT("Physical");
        bIsActive = true;
    }
};

UCLASS(Blueprintable, BlueprintType)
class TRANSPERSONALGAME_API UCombat_TacticalAIController : public UAIController
{
    GENERATED_BODY()

public:
    UCombat_TacticalAIController();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

public:
    // Tactical AI Properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    FCombat_TacticalData TacticalData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    TArray<FCombat_DamageZone> DamageZones;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    TArray<AActor*> PackMembers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    AActor* CurrentTarget;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    TArray<FVector> TacticalWaypoints;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    float StateTransitionCooldown = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    float LastStateChange = 0.0f;

    // Combat Decision Making
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void EvaluateThreatLevel();

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void UpdateTacticalState();

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void ExecuteFormationMovement();

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void ProcessCombatDecision();

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    bool CanTransitionToState(ECombat_TacticalState NewState);

    // Pack Coordination
    UFUNCTION(BlueprintCallable, Category = "Pack AI")
    void CoordinatePackAttack();

    UFUNCTION(BlueprintCallable, Category = "Pack AI")
    void AssignPackRoles();

    UFUNCTION(BlueprintCallable, Category = "Pack AI")
    void CallReinforcements();

    // Damage System
    UFUNCTION(BlueprintCallable, Category = "Damage")
    void ConfigureDamageZones();

    UFUNCTION(BlueprintCallable, Category = "Damage")
    bool IsTargetInDamageZone(AActor* Target, FCombat_DamageZone& Zone);

    UFUNCTION(BlueprintCallable, Category = "Damage")
    float CalculateDamageToTarget(AActor* Target);

    // Tactical Movement
    UFUNCTION(BlueprintCallable, Category = "Movement")
    FVector GetFlankingPosition(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Movement")
    FVector GetAmbushPosition(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Movement")
    void MoveToTacticalPosition(FVector Position);

private:
    float LastThreatEvaluation = 0.0f;
    float ThreatEvaluationInterval = 1.0f;
    
    bool bIsExecutingManeuver = false;
    FVector CurrentTacticalTarget = FVector::ZeroVector;
};