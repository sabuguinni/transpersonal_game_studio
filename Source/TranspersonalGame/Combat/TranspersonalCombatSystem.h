#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/DataTable.h"
#include "TranspersonalCombatSystem.generated.h"

UENUM(BlueprintType)
enum class ECombatState : uint8
{
    Peaceful        UMETA(DisplayName = "Peaceful"),
    Defensive       UMETA(DisplayName = "Defensive"),
    Aggressive      UMETA(DisplayName = "Aggressive"),
    Transcendent    UMETA(DisplayName = "Transcendent"),
    Shadow          UMETA(DisplayName = "Shadow")
};

UENUM(BlueprintType)
enum class EConsciousnessAttackType : uint8
{
    Physical        UMETA(DisplayName = "Physical"),
    Emotional       UMETA(DisplayName = "Emotional"),
    Mental          UMETA(DisplayName = "Mental"),
    Spiritual       UMETA(DisplayName = "Spiritual"),
    Shadow          UMETA(DisplayName = "Shadow"),
    Light           UMETA(DisplayName = "Light")
};

USTRUCT(BlueprintType)
struct FConsciousnessAttack
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EConsciousnessAttackType AttackType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Damage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float ConsciousnessImpact;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Range;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float CooldownTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString AttackName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString Description;

    FConsciousnessAttack()
    {
        AttackType = EConsciousnessAttackType::Physical;
        Damage = 10.0f;
        ConsciousnessImpact = 5.0f;
        Range = 100.0f;
        CooldownTime = 2.0f;
        AttackName = TEXT("Basic Attack");
        Description = TEXT("A basic consciousness-based attack");
    }
};

USTRUCT(BlueprintType)
struct FDefensiveState
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float PhysicalResistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float EmotionalResistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MentalResistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float SpiritualResistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsShielded;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float ShieldStrength;

    FDefensiveState()
    {
        PhysicalResistance = 0.1f;
        EmotionalResistance = 0.1f;
        MentalResistance = 0.1f;
        SpiritualResistance = 0.1f;
        bIsShielded = false;
        ShieldStrength = 0.0f;
    }
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCombatStateChanged, ECombatState, OldState, ECombatState, NewState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnConsciousnessAttack, AActor*, Attacker, AActor*, Target, FConsciousnessAttack, Attack);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnConsciousnessDefense, AActor*, Defender, float, DamageBlocked);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UTranspersonalCombatSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UTranspersonalCombatSystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Combat State Management
    UFUNCTION(BlueprintCallable, Category = "Transpersonal Combat")
    void SetCombatState(ECombatState NewState);

    UFUNCTION(BlueprintPure, Category = "Transpersonal Combat")
    ECombatState GetCombatState() const { return CurrentCombatState; }

    // Attack System
    UFUNCTION(BlueprintCallable, Category = "Transpersonal Combat")
    bool ExecuteConsciousnessAttack(AActor* Target, FConsciousnessAttack Attack);

    UFUNCTION(BlueprintCallable, Category = "Transpersonal Combat")
    void AddAvailableAttack(FConsciousnessAttack NewAttack);

    UFUNCTION(BlueprintPure, Category = "Transpersonal Combat")
    TArray<FConsciousnessAttack> GetAvailableAttacks() const { return AvailableAttacks; }

    // Defense System
    UFUNCTION(BlueprintCallable, Category = "Transpersonal Combat")
    float CalculateDamageReduction(EConsciousnessAttackType AttackType);

    UFUNCTION(BlueprintCallable, Category = "Transpersonal Combat")
    void ActivateConsciousnessShield(float Strength, float Duration);

    UFUNCTION(BlueprintCallable, Category = "Transpersonal Combat")
    void DeactivateConsciousnessShield();

    // Consciousness Integration
    UFUNCTION(BlueprintCallable, Category = "Transpersonal Combat")
    void UpdateCombatBasedOnConsciousness(float ConsciousnessLevel);

    UFUNCTION(BlueprintCallable, Category = "Transpersonal Combat")
    bool CanUseTranscendentAbilities() const;

    // Events
    UPROPERTY(BlueprintAssignable, Category = "Transpersonal Combat")
    FOnCombatStateChanged OnCombatStateChanged;

    UPROPERTY(BlueprintAssignable, Category = "Transpersonal Combat")
    FOnConsciousnessAttack OnConsciousnessAttack;

    UPROPERTY(BlueprintAssignable, Category = "Transpersonal Combat")
    FOnConsciousnessDefense OnConsciousnessDefense;

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat State")
    ECombatState CurrentCombatState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat State")
    float ConsciousnessLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attacks")
    TArray<FConsciousnessAttack> AvailableAttacks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Defense")
    FDefensiveState DefensiveState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Settings")
    float TranscendentThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Settings")
    float ShadowThreshold;

    // Cooldown tracking
    UPROPERTY()
    TMap<FString, float> AttackCooldowns;

    // Shield timer
    UPROPERTY()
    FTimerHandle ShieldTimerHandle;

private:
    void UpdateAttackCooldowns(float DeltaTime);
    void OnShieldExpired();
    float CalculateConsciousnessModifier(EConsciousnessAttackType AttackType);
};