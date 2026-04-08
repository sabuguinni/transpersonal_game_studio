// CombatSystem.h
// Sistema de combate consciente que responde aos estados espirituais
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "../Consciousness/ConsciousnessTypes.h"
#include "CombatSystem.generated.h"

UENUM(BlueprintType)
enum class ECombatState : uint8
{
    Peaceful        UMETA(DisplayName = "Peaceful"),
    Defensive       UMETA(DisplayName = "Defensive"), 
    Aggressive      UMETA(DisplayName = "Aggressive"),
    Transcendent    UMETA(DisplayName = "Transcendent")
};

UENUM(BlueprintType)
enum class EAttackType : uint8
{
    Physical        UMETA(DisplayName = "Physical"),
    Energetic       UMETA(DisplayName = "Energetic"),
    Spiritual       UMETA(DisplayName = "Spiritual"),
    Transformative  UMETA(DisplayName = "Transformative")
};

USTRUCT(BlueprintType)
struct FCombatStats
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float PhysicalPower = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float SpiritualResonance = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float ConsciousnessLevel = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float KarmicBalance = 0.0f;

    FCombatStats()
    {
        PhysicalPower = 100.0f;
        SpiritualResonance = 50.0f;
        ConsciousnessLevel = 1.0f;
        KarmicBalance = 0.0f;
    }
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnCombatStateChanged, ECombatState, NewState, float, Intensity, FString, Reason);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FOnSpiritualCombat, EAttackType, AttackType, float, Damage, float, SpiritualImpact, bool, bTransformative);

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
    UFUNCTION(BlueprintCallable, Category = "Combat")
    void SetCombatState(ECombatState NewState, float Intensity = 1.0f);

    UFUNCTION(BlueprintPure, Category = "Combat")
    ECombatState GetCombatState() const { return CurrentCombatState; }

    UFUNCTION(BlueprintPure, Category = "Combat")
    float GetCombatIntensity() const { return CombatIntensity; }

    // Spiritual Combat
    UFUNCTION(BlueprintCallable, Category = "Combat")
    bool ExecuteSpiritualAttack(EAttackType AttackType, AActor* Target, float Power = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    float CalculateSpiritualDamage(EAttackType AttackType, float BasePower, EConsciousnessState AttackerState, EConsciousnessState DefenderState);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    bool CanEngageInCombat(AActor* Target) const;

    // Consciousness Integration
    UFUNCTION(BlueprintCallable, Category = "Combat")
    void UpdateCombatFromConsciousness(EConsciousnessState ConsciousnessState, float Level);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void ApplyKarmicConsequences(EAttackType AttackType, bool bHarmedInnocent, float Damage);

    // Combat Stats
    UFUNCTION(BlueprintPure, Category = "Combat")
    FCombatStats GetCombatStats() const { return CombatStats; }

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void ModifyCombatStats(const FCombatStats& StatModifiers);

    // Events
    UPROPERTY(BlueprintAssignable, Category = "Combat Events")
    FOnCombatStateChanged OnCombatStateChanged;

    UPROPERTY(BlueprintAssignable, Category = "Combat Events")
    FOnSpiritualCombat OnSpiritualCombat;

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat State")
    ECombatState CurrentCombatState;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat State")
    float CombatIntensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Stats")
    FCombatStats CombatStats;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Config")
    float ConsciousnessInfluence = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Config")
    float KarmicDecayRate = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Config")
    TMap<EConsciousnessState, float> ConsciousnessCombatModifiers;

private:
    void InitializeConsciousnessModifiers();
    float CalculateConsciousnessBonus(EConsciousnessState State) const;
    void ProcessKarmicBalance(float DeltaTime);
};