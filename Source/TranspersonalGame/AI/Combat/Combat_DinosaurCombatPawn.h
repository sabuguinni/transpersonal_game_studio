#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "GameFramework/FloatingPawnMovement.h"
#include "SharedTypes.h"
#include "Combat_DinosaurCombatPawn.generated.h"

class ACombat_CombatAIController;
class UAnimationAsset;

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ACombat_DinosaurCombatPawn : public APawn
{
    GENERATED_BODY()

public:
    ACombat_DinosaurCombatPawn();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Mesh Component
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* DinosaurMesh;

    // Collision Component
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USphereComponent* CollisionSphere;

    // Movement Component
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UFloatingPawnMovement* MovementComponent;

    // Attack Range Collision
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USphereComponent* AttackRangeCollision;

    // Detection Range Collision
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USphereComponent* DetectionRangeCollision;

public:
    // Dinosaur Properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Stats")
    EDinosaurSpecies DinosaurType = EDinosaurSpecies::TRex;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Stats")
    float Health = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Stats")
    float MaxHealth = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Stats")
    float AttackDamage = 25.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Stats")
    float AttackSpeed = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Stats")
    float MovementSpeed = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Stats")
    float DetectionRadius = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Stats")
    float AttackRadius = 200.0f;

    // Combat Behavior
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Behavior")
    ECombat_DinosaurBehavior CombatBehavior = ECombat_DinosaurBehavior::Territorial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Behavior")
    float AggressionLevel = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Behavior")
    bool bIsPackHunter = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Behavior")
    bool bCanFlee = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Behavior")
    float FleeHealthThreshold = 0.3f;

    // Pack Behavior
    UPROPERTY(BlueprintReadOnly, Category = "Pack Behavior")
    TArray<ACombat_DinosaurCombatPawn*> PackMembers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Behavior")
    int32 MaxPackSize = 6;

    UPROPERTY(BlueprintReadOnly, Category = "Pack Behavior")
    ACombat_DinosaurCombatPawn* PackLeader;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Behavior")
    float PackCoordinationRadius = 1000.0f;

    // Territory
    UPROPERTY(BlueprintReadOnly, Category = "Territory")
    FVector TerritoryCenter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Territory")
    float TerritoryRadius = 2000.0f;

    // Combat State
    UPROPERTY(BlueprintReadOnly, Category = "Combat State")
    ECombat_AIState CurrentCombatState = ECombat_AIState::Idle;

    UPROPERTY(BlueprintReadOnly, Category = "Combat State")
    AActor* CurrentTarget;

    UPROPERTY(BlueprintReadOnly, Category = "Combat State")
    float LastAttackTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Combat State")
    bool bIsAttacking = false;

public:
    // Health Management
    UFUNCTION(BlueprintCallable, Category = "Combat")
    void TakeDamage(float DamageAmount, AActor* DamageSource);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void Heal(float HealAmount);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    bool IsAlive() const { return Health > 0.0f; }

    UFUNCTION(BlueprintCallable, Category = "Combat")
    float GetHealthPercentage() const { return Health / MaxHealth; }

    // Combat Actions
    UFUNCTION(BlueprintCallable, Category = "Combat")
    void StartAttack(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void ExecuteAttack();

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void StopAttack();

    UFUNCTION(BlueprintCallable, Category = "Combat")
    bool CanAttack() const;

    // Movement and Positioning
    UFUNCTION(BlueprintCallable, Category = "Movement")
    void MoveToTarget(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Movement")
    void MoveToLocation(FVector Location);

    UFUNCTION(BlueprintCallable, Category = "Movement")
    void StopMovement();

    // Pack Management
    UFUNCTION(BlueprintCallable, Category = "Pack")
    void JoinPack(ACombat_DinosaurCombatPawn* Leader);

    UFUNCTION(BlueprintCallable, Category = "Pack")
    void LeavePack();

    UFUNCTION(BlueprintCallable, Category = "Pack")
    void AddPackMember(ACombat_DinosaurCombatPawn* Member);

    UFUNCTION(BlueprintCallable, Category = "Pack")
    void RemovePackMember(ACombat_DinosaurCombatPawn* Member);

    UFUNCTION(BlueprintCallable, Category = "Pack")
    bool IsPackLeader() const { return PackLeader == this; }

    UFUNCTION(BlueprintCallable, Category = "Pack")
    void CoordinatePackAttack(AActor* Target);

    // Territory Management
    UFUNCTION(BlueprintCallable, Category = "Territory")
    void SetTerritory(FVector Center, float Radius);

    UFUNCTION(BlueprintCallable, Category = "Territory")
    bool IsInTerritory() const;

    UFUNCTION(BlueprintCallable, Category = "Territory")
    bool IsTargetInTerritory(AActor* Target) const;

    // Behavior Configuration
    UFUNCTION(BlueprintCallable, Category = "Behavior")
    void SetDinosaurType(EDinosaurSpecies NewType);

    UFUNCTION(BlueprintCallable, Category = "Behavior")
    void ConfigureBehaviorForSpecies();

    // State Queries
    UFUNCTION(BlueprintCallable, Category = "State")
    bool IsInCombat() const { return CurrentCombatState == ECombat_AIState::Combat; }

    UFUNCTION(BlueprintCallable, Category = "State")
    bool ShouldFlee() const;

    UFUNCTION(BlueprintCallable, Category = "State")
    float GetDistanceToTarget() const;

protected:
    // Collision Events
    UFUNCTION()
    void OnAttackRangeBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnAttackRangeEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

    UFUNCTION()
    void OnDetectionRangeBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnDetectionRangeEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

    // Internal Methods
    void UpdateCombatBehavior(float DeltaTime);
    void UpdatePackBehavior(float DeltaTime);
    void UpdateMovement(float DeltaTime);
    void HandleDeath();

    // AI Controller Reference
    ACombat_CombatAIController* CombatAIController;

private:
    // Internal state
    TArray<AActor*> ActorsInAttackRange;
    TArray<AActor*> ActorsInDetectionRange;
    FVector LastKnownTargetLocation;
    float StateChangeTime;
};