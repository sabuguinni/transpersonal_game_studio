#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Pawn.h"
#include "Components/ActorComponent.h"
#include "QA_CombatTestSuite.generated.h"

UENUM(BlueprintType)
enum class EQA_CombatTestType : uint8
{
    MeleeAttack = 0,
    RangedAttack = 1,
    DinosaurCombat = 2,
    DamageSystem = 3,
    CombatAI = 4,
    WeaponSystem = 5
};

USTRUCT(BlueprintType)
struct FQA_CombatTestResult
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Testing")
    EQA_CombatTestType TestType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Testing")
    bool bTestPassed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Testing")
    FString TestDetails;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Testing")
    float DamageDealt;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Testing")
    float TestDuration;

    FQA_CombatTestResult()
    {
        TestType = EQA_CombatTestType::MeleeAttack;
        bTestPassed = false;
        TestDetails = TEXT("");
        DamageDealt = 0.0f;
        TestDuration = 0.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UQA_CombatTestSuite : public UActorComponent
{
    GENERATED_BODY()

public:
    UQA_CombatTestSuite();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // === COMBAT TESTING METHODS ===
    UFUNCTION(BlueprintCallable, Category = "Combat Testing")
    void RunAllCombatTests();

    UFUNCTION(BlueprintCallable, Category = "Combat Testing")
    void TestMeleeAttackSystem();

    UFUNCTION(BlueprintCallable, Category = "Combat Testing")
    void TestRangedAttackSystem();

    UFUNCTION(BlueprintCallable, Category = "Combat Testing")
    void TestDinosaurCombatBehavior();

    UFUNCTION(BlueprintCallable, Category = "Combat Testing")
    void TestDamageCalculation();

    UFUNCTION(BlueprintCallable, Category = "Combat Testing")
    void TestCombatAI();

    UFUNCTION(BlueprintCallable, Category = "Combat Testing")
    void TestWeaponSystems();

    // === STRESS TESTING ===
    UFUNCTION(BlueprintCallable, Category = "Combat Testing")
    void RunCombatStressTest();

    UFUNCTION(BlueprintCallable, Category = "Combat Testing")
    void TestMultipleCombatants();

    // === RESULTS AND REPORTING ===
    UFUNCTION(BlueprintCallable, Category = "Combat Testing")
    TArray<FQA_CombatTestResult> GetCombatTestResults() const { return CombatTestResults; }

    UFUNCTION(BlueprintCallable, Category = "Combat Testing")
    void GenerateCombatTestReport();

    UFUNCTION(BlueprintCallable, Category = "Combat Testing")
    bool AreAllCombatTestsPassing() const;

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat Testing")
    TArray<FQA_CombatTestResult> CombatTestResults;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Testing")
    bool bAutoRunCombatTests;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Testing")
    float CombatTestInterval;

    // === TEST CONFIGURATION ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Testing")
    float BaseMeleeDamage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Testing")
    float BaseRangedDamage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Testing")
    int32 MaxCombatants;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Testing")
    float CombatTestTimeout;

private:
    void AddCombatTestResult(EQA_CombatTestType TestType, bool bPassed, const FString& Details, float Damage = 0.0f, float Duration = 0.0f);
    void LogCombatTestResult(const FQA_CombatTestResult& TestResult);
    float CombatTestStartTime;
};