#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "../Core/ConsciousnessComponent.h"
#include "CombatSystem.generated.h"

UENUM(BlueprintType)
enum class ECombatState : uint8
{
    Peaceful        UMETA(DisplayName = "Peaceful"),
    Alert          UMETA(DisplayName = "Alert"),
    Combat         UMETA(DisplayName = "Combat"),
    Retreating     UMETA(DisplayName = "Retreating"),
    Meditating     UMETA(DisplayName = "Meditating")
};

UENUM(BlueprintType)
enum class EAttackType : uint8
{
    Physical       UMETA(DisplayName = "Physical"),
    Psychic        UMETA(DisplayName = "Psychic"),
    Spiritual      UMETA(DisplayName = "Spiritual"),
    Elemental      UMETA(DisplayName = "Elemental")
};

USTRUCT(BlueprintType)
struct FAttackData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Damage = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EAttackType AttackType = EAttackType::Physical;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Range = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Cooldown = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float ConsciousnessImpact = 5.0f;

    FAttackData()
    {
        Damage = 10.0f;
        AttackType = EAttackType::Physical;
        Range = 100.0f;
        Cooldown = 1.0f;
        ConsciousnessImpact = 5.0f;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCombatSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UCombatSystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Combat State Management
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    ECombatState CurrentCombatState = ECombatState::Peaceful;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float Health = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float MaxHealth = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float DetectionRange = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float AttackRange = 150.0f;

    // Attack System
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    TArray<FAttackData> AvailableAttacks;

    UPROPERTY(BlueprintReadOnly, Category = "Combat")
    float LastAttackTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Combat")
    AActor* CurrentTarget = nullptr;

    // Consciousness Integration
    UPROPERTY(BlueprintReadOnly, Category = "Combat")
    UConsciousnessComponent* ConsciousnessComponent = nullptr;

    // Combat Functions
    UFUNCTION(BlueprintCallable, Category = "Combat")
    void SetCombatState(ECombatState NewState);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    bool CanAttack() const;

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void PerformAttack(int32 AttackIndex);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void TakeDamage(float DamageAmount, EAttackType AttackType, AActor* Attacker);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void FindNearestTarget();

    UFUNCTION(BlueprintCallable, Category = "Combat")
    float GetDistanceToTarget() const;

    UFUNCTION(BlueprintCallable, Category = "Combat")
    bool IsTargetInRange(float Range) const;

    // Consciousness-Based Combat Modifiers
    UFUNCTION(BlueprintCallable, Category = "Combat")
    float GetConsciousnessAttackModifier() const;

    UFUNCTION(BlueprintCallable, Category = "Combat")
    float GetConsciousnessDefenseModifier() const;

    UFUNCTION(BlueprintCallable, Category = "Combat")
    bool ShouldEnterMeditativeState() const;

    // Events
    UFUNCTION(BlueprintImplementableEvent, Category = "Combat")
    void OnCombatStateChanged(ECombatState OldState, ECombatState NewState);

    UFUNCTION(BlueprintImplementableEvent, Category = "Combat")
    void OnAttackPerformed(const FAttackData& AttackData);

    UFUNCTION(BlueprintImplementableEvent, Category = "Combat")
    void OnDamageTaken(float Damage, EAttackType AttackType);

    UFUNCTION(BlueprintImplementableEvent, Category = "Combat")
    void OnTargetFound(AActor* Target);

private:
    void UpdateCombatBehavior(float DeltaTime);
    void HandleConsciousnessIntegration();
    FAttackData SelectBestAttack() const;
};