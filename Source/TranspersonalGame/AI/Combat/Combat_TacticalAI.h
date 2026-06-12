#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "SharedTypes.h"
#include "Combat_TacticalAI.generated.h"

UENUM(BlueprintType)
enum class ECombat_TacticalState : uint8
{
    Patrol,
    Alert,
    Hunting,
    Attacking,
    Retreating,
    Flanking,
    Ambush
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCombat_TacticalData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical")
    float ThreatLevel = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical")
    FVector LastKnownPlayerLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical")
    float TimesSincePlayerSeen = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical")
    bool bHasLineOfSight = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical")
    float DistanceToPlayer = 999999.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical")
    ECombat_TacticalState CurrentState = ECombat_TacticalState::Patrol;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UCombat_TacticalAI : public UActorComponent
{
    GENERATED_BODY()

public:
    UCombat_TacticalAI();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical AI")
    FCombat_TacticalData TacticalData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical AI")
    float AlertRadius = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical AI")
    float AttackRadius = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical AI")
    float FlankingDistance = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical AI")
    float RetreatHealthThreshold = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical AI")
    bool bCanFlank = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical AI")
    bool bCanAmbush = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical AI")
    float StateChangeDelay = 2.0f;

    UFUNCTION(BlueprintCallable, Category = "Tactical AI")
    void UpdateTacticalState();

    UFUNCTION(BlueprintCallable, Category = "Tactical AI")
    void SetTacticalState(ECombat_TacticalState NewState);

    UFUNCTION(BlueprintCallable, Category = "Tactical AI")
    FVector GetFlankingPosition();

    UFUNCTION(BlueprintCallable, Category = "Tactical AI")
    bool ShouldRetreat();

    UFUNCTION(BlueprintCallable, Category = "Tactical AI")
    void OnPlayerDetected(FVector PlayerLocation);

    UFUNCTION(BlueprintCallable, Category = "Tactical AI")
    void OnPlayerLost();

private:
    float LastStateChangeTime = 0.0f;
    AActor* PlayerTarget = nullptr;

    void FindPlayerTarget();
    bool HasLineOfSightToPlayer();
    float CalculateThreatLevel();
};