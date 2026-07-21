#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "Engine/World.h"
#include "QA_VFXTestSuite.generated.h"

/**
 * QA Test Suite for VFX System Validation
 * Tests Niagara effects, particle systems, and prehistoric VFX integration
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AQA_VFXTestSuite : public AActor
{
    GENERATED_BODY()

public:
    AQA_VFXTestSuite();

protected:
    virtual void BeginPlay() override;

    // VFX Test Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "QA VFX Testing")
    class UNiagaraComponent* FireTestComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "QA VFX Testing")
    class UNiagaraComponent* DustTestComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "QA VFX Testing")
    class UNiagaraComponent* BloodTestComponent;

    // Test Assets
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA VFX Assets")
    class UNiagaraSystem* FireSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA VFX Assets")
    class UNiagaraSystem* DustSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA VFX Assets")
    class UNiagaraSystem* BloodSystem;

public:
    // Test Functions
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "QA VFX Testing")
    void RunFireEffectTest();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "QA VFX Testing")
    void RunDustEffectTest();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "QA VFX Testing")
    void RunBloodEffectTest();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "QA VFX Testing")
    void RunAllVFXTests();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "QA VFX Testing")
    void ValidateVFXPerformance();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "QA VFX Testing")
    void TestVFXLODSystem();

    // Validation Results
    UPROPERTY(BlueprintReadOnly, Category = "QA Results")
    bool bFireTestPassed;

    UPROPERTY(BlueprintReadOnly, Category = "QA Results")
    bool bDustTestPassed;

    UPROPERTY(BlueprintReadOnly, Category = "QA Results")
    bool bBloodTestPassed;

    UPROPERTY(BlueprintReadOnly, Category = "QA Results")
    bool bPerformanceTestPassed;

    UPROPERTY(BlueprintReadOnly, Category = "QA Results")
    float LastFrameTime;

    UPROPERTY(BlueprintReadOnly, Category = "QA Results")
    int32 ActiveParticleCount;

private:
    void LogTestResult(const FString& TestName, bool bPassed, const FString& Details = TEXT(""));
    void MeasurePerformanceImpact();
    bool ValidateNiagaraSystem(class UNiagaraSystem* System, const FString& SystemName);
};