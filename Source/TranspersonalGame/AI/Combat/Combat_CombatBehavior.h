#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "SharedTypes.h"
#include "Combat_CombatBehavior.generated.h"

class ACombat_CombatManager;
class UCombat_TacticalAI;

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCombat_AttackPattern
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack Pattern")
    FString PatternName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack Pattern")
    float MinRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack Pattern")
    float MaxRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack Pattern")
    float Damage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack Pattern")
    float Cooldown;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack Pattern")
    float Accuracy;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack Pattern")
    ECombat_AttackType AttackType;

    FCombat_AttackPattern()
    {
        PatternName = "Basic Attack";
        MinRange = 0.0f;
        MaxRange = 200.0f;
        Damage = 10.0f;
        Cooldown = 2.0f;
        Accuracy = 0.8f;
        AttackType = ECombat_AttackType::Melee;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCombat_DefensePattern
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Defense Pattern")
    FString PatternName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Defense Pattern")
    float BlockChance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Defense Pattern")
    float DodgeChance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Defense Pattern")
    float CounterAttackChance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Defense Pattern")
    float StaminaCost;

    FCombat_DefensePattern()
    {
        PatternName = "Basic Defense";
        BlockChance = 0.3f;
        DodgeChance = 0.2f;
        CounterAttackChance = 0.1f;
        StaminaCost = 5.0f;
    }
};

UCLASS(ClassGroup=(Combat), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCombat_CombatBehavior : public UActorComponent
{
    GENERATED_BODY()

public:
    UCombat_CombatBehavior();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Settings")
    TArray<FCombat_AttackPattern> AttackPatterns;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Settings")
    TArray<FCombat_DefensePattern> DefensePatterns;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Settings")
    float CombatRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Settings")
    float AggressionLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Settings")
    float DefenseLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Settings")
    bool bCanInitiateCombat;

    UPROPERTY(BlueprintReadOnly, Category = "Combat State")
    AActor* CurrentTarget;

    UPROPERTY(BlueprintReadOnly, Category = "Combat State")
    bool bIsInCombat;

    UPROPERTY(BlueprintReadOnly, Category = "Combat State")
    float LastAttackTime;

    UPROPERTY(BlueprintReadOnly, Category = "Combat State")
    int32 CurrentAttackPatternIndex;

    UPROPERTY(BlueprintReadOnly, Category = "Combat State")
    int32 CurrentDefensePatternIndex;

private:
    ACombat_CombatManager* CombatManager;
    UCombat_TacticalAI* TacticalAI;
    float CombatUpdateInterval;
    float LastCombatUpdate;

public:
    UFUNCTION(BlueprintCallable, Category = "Combat Behavior")
    void EnterCombat(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Combat Behavior")
    void ExitCombat();

    UFUNCTION(BlueprintCallable, Category = "Combat Behavior")
    bool TryAttack();

    UFUNCTION(BlueprintCallable, Category = "Combat Behavior")
    bool TryDefend();

    UFUNCTION(BlueprintCallable, Category = "Combat Behavior")
    void SetCombatTarget(AActor* NewTarget);

    UFUNCTION(BlueprintCallable, Category = "Combat Behavior")
    void UpdateCombatBehavior();

    UFUNCTION(BlueprintCallable, Category = "Combat Behavior")
    bool IsInAttackRange() const;

    UFUNCTION(BlueprintCallable, Category = "Combat Behavior")
    bool CanAttack() const;

    UFUNCTION(BlueprintCallable, Category = "Combat Behavior")
    FCombat_AttackPattern GetCurrentAttackPattern() const;

    UFUNCTION(BlueprintCallable, Category = "Combat Behavior")
    FCombat_DefensePattern GetCurrentDefensePattern() const;

    UFUNCTION(BlueprintCallable, Category = "Combat Behavior")
    void SelectBestAttackPattern();

    UFUNCTION(BlueprintCallable, Category = "Combat Behavior")
    void SelectBestDefensePattern();

    UFUNCTION(BlueprintCallable, Category = "Combat Behavior")
    float CalculateAttackDamage() const;

    UFUNCTION(BlueprintCallable, Category = "Combat Behavior")
    bool ShouldRetreat() const;

    UFUNCTION(BlueprintCallable, Category = "Combat Behavior")
    bool ShouldPursue() const;

    UFUNCTION(BlueprintPure, Category = "Combat Behavior")
    bool IsInCombat() const { return bIsInCombat; }

    UFUNCTION(BlueprintPure, Category = "Combat Behavior")
    AActor* GetCombatTarget() const { return CurrentTarget; }

    UFUNCTION(BlueprintPure, Category = "Combat Behavior")
    float GetDistanceToTarget() const;

    UFUNCTION(BlueprintPure, Category = "Combat Behavior")
    bool HasValidTarget() const;

protected:
    UFUNCTION()
    void OnCombatRangeEntered(AActor* OtherActor);

    UFUNCTION()
    void OnCombatRangeExited(AActor* OtherActor);

private:
    void InitializeCombatPatterns();
    void FindCombatManager();
    void RegisterWithCombatManager();
    void UnregisterFromCombatManager();
    bool ValidateTarget() const;
    void ProcessCombatLogic();
    void UpdateCombatState();
};