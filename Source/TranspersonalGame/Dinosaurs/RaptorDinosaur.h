// RaptorDinosaur.h
// Transpersonal Game Studio — Agent #05 Procedural World Generator
// Velociraptor pack hunter — fast, intelligent, flanking AI

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "TimerManager.h"
#include "RaptorDinosaur.generated.h"

// Raptor role within pack
UENUM(BlueprintType)
enum class EWorld_RaptorRole : uint8
{
    Scout    UMETA(DisplayName = "Scout"),
    Flanker  UMETA(DisplayName = "Flanker"),
    Alpha    UMETA(DisplayName = "Alpha")
};

// Dinosaur AI state
UENUM(BlueprintType)
enum class EWorld_DinoState : uint8
{
    Idle       UMETA(DisplayName = "Idle"),
    Patrolling UMETA(DisplayName = "Patrolling"),
    Chasing    UMETA(DisplayName = "Chasing"),
    Attacking  UMETA(DisplayName = "Attacking"),
    Fleeing    UMETA(DisplayName = "Fleeing"),
    Returning  UMETA(DisplayName = "Returning"),
    Dead       UMETA(DisplayName = "Dead")
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ARaptorDinosaur : public ACharacter
{
    GENERATED_BODY()

public:
    ARaptorDinosaur();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;
    virtual float TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent,
        AController* EventInstigator, AActor* DamageCauser) override;

    // === VITAL STATS ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Raptor|Stats")
    float Health;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Raptor|Stats")
    float MaxHealth;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Raptor|Combat")
    float AttackDamage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Raptor|Combat")
    float AttackRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Raptor|Combat")
    float AttackCooldown;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Raptor|AI")
    float DetectionRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Raptor|AI")
    float PatrolRadius;

    // === PACK BEHAVIOUR ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Raptor|Pack")
    EWorld_RaptorRole PackRole;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Raptor|Pack")
    bool bIsAlpha;

    UPROPERTY(BlueprintReadOnly, Category = "Raptor|Pack")
    bool bIsInPackChase;

    UPROPERTY(BlueprintReadOnly, Category = "Raptor|Pack")
    ARaptorDinosaur* PackLeader;

    // === AI STATE ===
    UPROPERTY(BlueprintReadOnly, Category = "Raptor|AI")
    EWorld_DinoState CurrentState;

    // === AI FUNCTIONS ===
    UFUNCTION(BlueprintCallable, Category = "Raptor|AI")
    void UpdateAI();

    UFUNCTION(BlueprintCallable, Category = "Raptor|AI")
    AActor* FindNearestThreat();

    UFUNCTION(BlueprintCallable, Category = "Raptor|AI")
    void ChaseTarget(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Raptor|Combat")
    void PerformAttack(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Raptor|Pack")
    void AlertPackMembers(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Raptor|Pack")
    void CoordinateWithPack();

    UFUNCTION(BlueprintCallable, Category = "Raptor|AI")
    void Patrol();

    UFUNCTION(BlueprintCallable, Category = "Raptor|AI")
    void ReturnToHome();

    UFUNCTION(BlueprintCallable, Category = "Raptor|AI")
    void OnDeath();

private:
    FVector HomeLocation;
    FVector PatrolTarget;
    float TimeSinceLastAttack;
    FTimerHandle AITickHandle;

    void unreal_log_attack(AActor* Target);
};
