#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "NPC_DinosaurAIController.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "NPC_TRexBehavior.generated.h"

UENUM(BlueprintType)
enum class ENPC_TRexBehaviorMode : uint8
{
    Passive UMETA(DisplayName = "Passive"),
    Territorial UMETA(DisplayName = "Territorial"),
    Aggressive UMETA(DisplayName = "Aggressive"),
    Apex UMETA(DisplayName = "Apex Predator")
};

USTRUCT(BlueprintType)
struct FNPC_TRexStats
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "T-Rex Stats")
    float AttackDamage = 75.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "T-Rex Stats")
    float MovementSpeed = 600.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "T-Rex Stats")
    float TerritoryRadius = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "T-Rex Stats")
    float HuntingRange = 3000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "T-Rex Stats")
    float RoarCooldown = 15.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "T-Rex Stats")
    float ChargeSpeed = 1200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "T-Rex Stats")
    float BiteDamage = 100.0f;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ANPC_TRexBehavior : public ACharacter
{
    GENERATED_BODY()

public:
    ANPC_TRexBehavior();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class USkeletalMeshComponent* TRexMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "T-Rex Behavior")
    ENPC_TRexBehaviorMode BehaviorMode;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "T-Rex Behavior")
    FNPC_TRexStats TRexStats;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "T-Rex Behavior")
    float LastRoarTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "T-Rex Behavior")
    bool bIsCharging;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "T-Rex Behavior")
    bool bIsInTerritory;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "T-Rex Behavior")
    FVector TerritoryCenter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "T-Rex Behavior")
    TArray<FVector> PatrolPoints;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "T-Rex Behavior")
    int32 CurrentPatrolIndex;

public:
    UFUNCTION(BlueprintCallable, Category = "T-Rex Behavior")
    void SetBehaviorMode(ENPC_TRexBehaviorMode NewMode);

    UFUNCTION(BlueprintCallable, Category = "T-Rex Behavior")
    void PerformRoar();

    UFUNCTION(BlueprintCallable, Category = "T-Rex Behavior")
    void StartCharge(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "T-Rex Behavior")
    void StopCharge();

    UFUNCTION(BlueprintCallable, Category = "T-Rex Behavior")
    void PerformBiteAttack(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "T-Rex Behavior")
    bool IsPlayerInTerritory(AActor* Player) const;

    UFUNCTION(BlueprintCallable, Category = "T-Rex Behavior")
    void SetupPatrolRoute();

    UFUNCTION(BlueprintCallable, Category = "T-Rex Behavior")
    FVector GetNextPatrolPoint();

    UFUNCTION(BlueprintCallable, Category = "T-Rex Behavior")
    void OnPlayerDetected(AActor* Player);

    UFUNCTION(BlueprintCallable, Category = "T-Rex Behavior")
    void OnPlayerLost();

protected:
    void UpdateBehaviorBasedOnMode(float DeltaTime);
    void HandlePassiveBehavior(float DeltaTime);
    void HandleTerritorialBehavior(float DeltaTime);
    void HandleAggressiveBehavior(float DeltaTime);
    void HandleApexBehavior(float DeltaTime);
    
    bool CanPerformRoar() const;
    void ApplyChargeMovement(float DeltaTime);
    float CalculateDistanceToPlayer() const;
    AActor* FindNearestPlayer() const;

private:
    AActor* CurrentTarget;
    FVector ChargeDirection;
    float ChargeStartTime;
    float MaxChargeTime = 3.0f;
};