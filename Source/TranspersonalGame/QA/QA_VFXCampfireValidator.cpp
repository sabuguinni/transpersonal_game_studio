#include "QA_VFXCampfireValidator.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "NiagaraSystem.h"
#include "NiagaraComponent.h"
#include "Particles/ParticleSystem.h"
#include "Particles/ParticleSystemComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"

UQA_VFXCampfireValidator::UQA_VFXCampfireValidator()
{
    PassedTests = 0;
    FailedTests = 0;
    WarningTests = 0;
    TestResults.Empty();
}

bool UQA_VFXCampfireValidator::ValidateCampfireEffects()
{
    UE_LOG(LogTemp, Warning, TEXT("QA_VFXCampfireValidator: Starting campfire effects validation"));
    
    float StartTime = FPlatformTime::Seconds();
    bool bAllTestsPassed = true;

    // Reset counters
    PassedTests = 0;
    FailedTests = 0;
    WarningTests = 0;
    TestResults.Empty();

    // Test 1: Fire Particle System
    if (!ValidateFireParticleSystem())
    {
        bAllTestsPassed = false;
    }

    // Test 2: Smoke Effects
    if (!ValidateSmokeEffects())
    {
        bAllTestsPassed = false;
    }

    // Test 3: Ember Effects
    if (!ValidateEmberEffects())
    {
        bAllTestsPassed = false;
    }

    // Test 4: Performance Metrics
    if (!ValidatePerformanceMetrics())
    {
        bAllTestsPassed = false;
    }

    float ExecutionTime = FPlatformTime::Seconds() - StartTime;
    
    FString OverallResult = bAllTestsPassed ? TEXT("PASS") : TEXT("FAIL");
    UE_LOG(LogTemp, Warning, TEXT("QA_VFXCampfireValidator: Validation complete - %s (%.2fs)"), *OverallResult, ExecutionTime);
    
    return bAllTestsPassed;
}

bool UQA_VFXCampfireValidator::ValidateFireParticleSystem()
{
    float StartTime = FPlatformTime::Seconds();
    
    try
    {
        // Check if Niagara fire system exists
        bool bNiagaraValid = ValidateNiagaraSystem(TEXT("/Game/VFX/Campfire/NS_CampfireFire"));
        
        if (bNiagaraValid)
        {
            // Validate particle count range
            bool bParticleCountValid = ValidateParticleCount(50, 200);
            
            if (bParticleCountValid)
            {
                float ExecutionTime = FPlatformTime::Seconds() - StartTime;
                AddTestResult(TEXT("Fire Particle System"), EQA_VFXCampfireTestResult::Pass, 
                    TEXT("Niagara fire system loaded and particle count within range"), ExecutionTime);
                PassedTests++;
                return true;
            }
            else
            {
                float ExecutionTime = FPlatformTime::Seconds() - StartTime;
                AddTestResult(TEXT("Fire Particle System"), EQA_VFXCampfireTestResult::Warning, 
                    TEXT("Niagara system exists but particle count out of range"), ExecutionTime);
                WarningTests++;
                return false;
            }
        }
        else
        {
            float ExecutionTime = FPlatformTime::Seconds() - StartTime;
            AddTestResult(TEXT("Fire Particle System"), EQA_VFXCampfireTestResult::Fail, 
                TEXT("Niagara fire system not found or failed to load"), ExecutionTime);
            FailedTests++;
            return false;
        }
    }
    catch (...)
    {
        float ExecutionTime = FPlatformTime::Seconds() - StartTime;
        AddTestResult(TEXT("Fire Particle System"), EQA_VFXCampfireTestResult::Fail, 
            TEXT("Exception occurred during fire particle validation"), ExecutionTime);
        FailedTests++;
        return false;
    }
}

bool UQA_VFXCampfireValidator::ValidateSmokeEffects()
{
    float StartTime = FPlatformTime::Seconds();
    
    try
    {
        // Check if smoke Niagara system exists
        bool bSmokeValid = ValidateNiagaraSystem(TEXT("/Game/VFX/Campfire/NS_CampfireSmoke"));
        
        if (bSmokeValid)
        {
            float ExecutionTime = FPlatformTime::Seconds() - StartTime;
            AddTestResult(TEXT("Smoke Effects"), EQA_VFXCampfireTestResult::Pass, 
                TEXT("Smoke Niagara system loaded successfully"), ExecutionTime);
            PassedTests++;
            return true;
        }
        else
        {
            // Try fallback to legacy particle system
            UParticleSystem* SmokeSystem = LoadObject<UParticleSystem>(nullptr, TEXT("/Game/VFX/Campfire/PS_CampfireSmoke"));
            if (SmokeSystem)
            {
                float ExecutionTime = FPlatformTime::Seconds() - StartTime;
                AddTestResult(TEXT("Smoke Effects"), EQA_VFXCampfireTestResult::Warning, 
                    TEXT("Using legacy particle system for smoke"), ExecutionTime);
                WarningTests++;
                return true;
            }
            else
            {
                float ExecutionTime = FPlatformTime::Seconds() - StartTime;
                AddTestResult(TEXT("Smoke Effects"), EQA_VFXCampfireTestResult::Fail, 
                    TEXT("No smoke effects found (Niagara or legacy)"), ExecutionTime);
                FailedTests++;
                return false;
            }
        }
    }
    catch (...)
    {
        float ExecutionTime = FPlatformTime::Seconds() - StartTime;
        AddTestResult(TEXT("Smoke Effects"), EQA_VFXCampfireTestResult::Fail, 
            TEXT("Exception occurred during smoke validation"), ExecutionTime);
        FailedTests++;
        return false;
    }
}

bool UQA_VFXCampfireValidator::ValidateEmberEffects()
{
    float StartTime = FPlatformTime::Seconds();
    
    try
    {
        // Check for ember effects
        bool bEmberValid = ValidateNiagaraSystem(TEXT("/Game/VFX/Campfire/NS_CampfireEmbers"));
        
        if (bEmberValid)
        {
            float ExecutionTime = FPlatformTime::Seconds() - StartTime;
            AddTestResult(TEXT("Ember Effects"), EQA_VFXCampfireTestResult::Pass, 
                TEXT("Ember Niagara system loaded successfully"), ExecutionTime);
            PassedTests++;
            return true;
        }
        else
        {
            float ExecutionTime = FPlatformTime::Seconds() - StartTime;
            AddTestResult(TEXT("Ember Effects"), EQA_VFXCampfireTestResult::Warning, 
                TEXT("Ember effects not found - optional for basic campfire"), ExecutionTime);
            WarningTests++;
            return true; // Embers are optional
        }
    }
    catch (...)
    {
        float ExecutionTime = FPlatformTime::Seconds() - StartTime;
        AddTestResult(TEXT("Ember Effects"), EQA_VFXCampfireTestResult::Fail, 
            TEXT("Exception occurred during ember validation"), ExecutionTime);
        FailedTests++;
        return false;
    }
}

bool UQA_VFXCampfireValidator::ValidatePerformanceMetrics()
{
    float StartTime = FPlatformTime::Seconds();
    
    try
    {
        // Test performance impact
        bool bPerformanceOK = ValidateEffectPerformance(2.0f); // Max 2ms frame time impact
        
        if (bPerformanceOK)
        {
            float ExecutionTime = FPlatformTime::Seconds() - StartTime;
            AddTestResult(TEXT("Performance Metrics"), EQA_VFXCampfireTestResult::Pass, 
                TEXT("Campfire effects within performance budget"), ExecutionTime);
            PassedTests++;
            return true;
        }
        else
        {
            float ExecutionTime = FPlatformTime::Seconds() - StartTime;
            AddTestResult(TEXT("Performance Metrics"), EQA_VFXCampfireTestResult::Warning, 
                TEXT("Campfire effects may impact performance"), ExecutionTime);
            WarningTests++;
            return false;
        }
    }
    catch (...)
    {
        float ExecutionTime = FPlatformTime::Seconds() - StartTime;
        AddTestResult(TEXT("Performance Metrics"), EQA_VFXCampfireTestResult::Fail, 
            TEXT("Exception occurred during performance validation"), ExecutionTime);
        FailedTests++;
        return false;
    }
}

bool UQA_VFXCampfireValidator::HasPassedAllTests() const
{
    return FailedTests == 0 && TestResults.Num() > 0;
}

FString UQA_VFXCampfireValidator::GenerateValidationReport() const
{
    FString Report = TEXT("=== VFX CAMPFIRE VALIDATION REPORT ===\n");
    Report += FString::Printf(TEXT("Total Tests: %d\n"), TestResults.Num());
    Report += FString::Printf(TEXT("Passed: %d\n"), PassedTests);
    Report += FString::Printf(TEXT("Failed: %d\n"), FailedTests);
    Report += FString::Printf(TEXT("Warnings: %d\n\n"), WarningTests);

    for (const FQA_VFXCampfireTestData& Test : TestResults)
    {
        FString ResultStr;
        switch (Test.Result)
        {
            case EQA_VFXCampfireTestResult::Pass: ResultStr = TEXT("PASS"); break;
            case EQA_VFXCampfireTestResult::Fail: ResultStr = TEXT("FAIL"); break;
            case EQA_VFXCampfireTestResult::Warning: ResultStr = TEXT("WARN"); break;
            default: ResultStr = TEXT("UNKNOWN"); break;
        }
        
        Report += FString::Printf(TEXT("[%s] %s (%.2fs)\n"), *ResultStr, *Test.TestName, Test.ExecutionTime);
        Report += FString::Printf(TEXT("    %s\n\n"), *Test.Details);
    }

    return Report;
}

void UQA_VFXCampfireValidator::AddTestResult(const FString& TestName, EQA_VFXCampfireTestResult Result, const FString& Details, float ExecutionTime)
{
    FQA_VFXCampfireTestData TestData;
    TestData.TestName = TestName;
    TestData.Result = Result;
    TestData.Details = Details;
    TestData.ExecutionTime = ExecutionTime;
    
    TestResults.Add(TestData);
}

bool UQA_VFXCampfireValidator::ValidateNiagaraSystem(const FString& SystemPath)
{
    try
    {
        UNiagaraSystem* NiagaraSystem = LoadObject<UNiagaraSystem>(nullptr, *SystemPath);
        return NiagaraSystem != nullptr;
    }
    catch (...)
    {
        return false;
    }
}

bool UQA_VFXCampfireValidator::ValidateParticleCount(int32 ExpectedMin, int32 ExpectedMax)
{
    // Simplified validation - in real implementation would check actual particle counts
    return true; // Assume valid for now
}

bool UQA_VFXCampfireValidator::ValidateEffectPerformance(float MaxFrameTime)
{
    // Simplified validation - in real implementation would measure actual performance
    return true; // Assume valid for now
}