#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTags.h"
#include "CombatAIController.h"
#include "CombatBehaviorComponent.generated.h"

UENUM(BlueprintType)
enum class EAttackType : uint8
{
    Bite,
    Claw,
    Tail,
    Charge,
    Stomp,
    Roar // Intimidação
};

USTRUCT(BlueprintType)
struct FAttackPattern
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EAttackType AttackType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Damage = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Range = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Cooldown = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float WindupTime = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float RecoveryTime = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bRequiresLineOfSight = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FGameplayTag AnimationTag;
};

USTRUCT(BlueprintType)
struct FCombatStats
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MaxHealth = 100.0f;

    UPROPERTY(BlueprintReadOnly)
    float CurrentHealth = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MovementSpeed = 600.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float AttackSpeed = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float DefenseRating = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float StaminaMax = 100.0f;

    UPROPERTY(BlueprintReadOnly)
    float CurrentStamina = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float StaminaRegenRate = 10.0f;
};

/**
 * Componente que gerencia comportamentos de combate específicos dos dinossauros
 * Implementa padrões de ataque, stamina, e lógica de combate tático
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCombatBehaviorComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UCombatBehaviorComponent();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // === COMBAT STATS ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Stats")
    FCombatStats CombatStats;

    // === ATTACK PATTERNS ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Patterns")
    TArray<FAttackPattern> AttackPatterns;

    UPROPERTY(BlueprintReadOnly, Category = "Combat State")
    int32 CurrentAttackIndex = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Combat State")
    float LastAttackTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Combat State")
    bool bIsAttacking = false;

    UPROPERTY(BlueprintReadOnly, Category = "Combat State")
    bool bIsInCombat = false;

    // === TACTICAL BEHAVIOR ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical Behavior")
    float CircleDistance = 400.0f; // Distância para circular o alvo

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical Behavior")
    float FlankingAngle = 45.0f; // Ângulo preferido para flanquear

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical Behavior")
    float AmbushWaitTime = 5.0f; // Tempo de espera em emboscada

    UPROPERTY(BlueprintReadOnly, Category = "Tactical State")
    FVector LastKnownTargetLocation;

    UPROPERTY(BlueprintReadOnly, Category = "Tactical State")
    float AmbushStartTime = 0.0f;

    // === FEAR AND INTIMIDATION ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Intimidation")
    float IntimidationRadius = 800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Intimidation")
    float RoarCooldown = 15.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Intimidation")
    float LastRoarTime = 0.0f;

public:
    // === PUBLIC INTERFACE ===
    UFUNCTION(BlueprintCallable, Category = "Combat Behavior")
    void UpdateBehavior(float DeltaTime, ECombatState CombatState, AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Combat Behavior")
    bool TryExecuteAttack(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Combat Behavior")
    FVector GetTacticalPosition(AActor* Target, ECombatState CombatState);

    UFUNCTION(BlueprintCallable, Category = "Combat Behavior")
    void TakeDamage(float Damage, AActor* DamageSource);

    UFUNCTION(BlueprintCallable, Category = "Combat Behavior")
    void OnStateChanged(ECombatState PreviousState, ECombatState NewState);

    // === GETTERS ===
    UFUNCTION(BlueprintPure, Category = "Combat Behavior")
    float GetHealthPercentage() const { return CombatStats.CurrentHealth / CombatStats.MaxHealth; }

    UFUNCTION(BlueprintPure, Category = "Combat Behavior")
    float GetStaminaPercentage() const { return CombatStats.CurrentStamina / CombatStats.StaminaMax; }

    UFUNCTION(BlueprintPure, Category = "Combat Behavior")
    bool CanAttack() const;

    UFUNCTION(BlueprintPure, Category = "Combat Behavior")
    bool IsInCombat() const { return bIsInCombat; }

    UFUNCTION(BlueprintPure, Category = "Combat Behavior")
    FAttackPattern GetCurrentAttackPattern() const;

    // === INTIMIDATION ===
    UFUNCTION(BlueprintCallable, Category = "Combat Behavior")
    void ExecuteRoar();

    UFUNCTION(BlueprintCallable, Category = "Combat Behavior")
    void IntimidateNearbyActors();

protected:
    // === INTERNAL METHODS ===
    void UpdateStamina(float DeltaTime);
    void UpdateCombatState(float DeltaTime);
    void SelectOptimalAttack(AActor* Target);
    FVector CalculateFlankingPosition(AActor* Target);
    FVector CalculateAmbushPosition(AActor* Target);
    FVector CalculateCirclingPosition(AActor* Target);
    bool HasLineOfSight(AActor* Target) const;
    void BroadcastIntimidation(float Radius);

    // === DELEGATES ===
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnAttackExecuted, EAttackType, AttackType, AActor*, Target, float, Damage);
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHealthChanged, float, NewHealth, float, MaxHealth);
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCombatStateChanged, bool, bInCombat);

public:
    UPROPERTY(BlueprintAssignable, Category = "Combat Events")
    FOnAttackExecuted OnAttackExecuted;

    UPROPERTY(BlueprintAssignable, Category = "Combat Events")
    FOnHealthChanged OnHealthChanged;

    UPROPERTY(BlueprintAssignable, Category = "Combat Events")
    FOnCombatStateChanged OnCombatStateChanged;
};