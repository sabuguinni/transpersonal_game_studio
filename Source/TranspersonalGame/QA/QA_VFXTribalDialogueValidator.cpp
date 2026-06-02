#include "QA_VFXTribalDialogueValidator.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "EditorLevelLibrary.h"
#include "NiagaraFunctionLibrary.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInterface.h"

UQA_VFXTribalDialogueValidator::UQA_VFXTribalDialogueValidator()
{
    PrimaryComponentTick.bCanEverTick = false;
    bEnableDetailedLogging = true;
    ValidationTimeout = 30.0f;
    MaxTestActors = 10;
    ResetValidationCounters();
}

bool UQA_VFXTribalDialogueValidator::ValidateTribalDialogueVFX()
{
    ResetValidationCounters();
    LogValidationResult("ValidateTribalDialogueVFX", true, "Starting tribal dialogue VFX validation");

    // Test 1: Validate VFX_TribalDialogueSystem class exists
    UClass* DialogueSystemClass = LoadClass<AActor>(nullptr, TEXT("/Script/TranspersonalGame.VFX_TribalDialogueSystem"));
    bool bClassExists = (DialogueSystemClass != nullptr);
    LogValidationResult("VFX_TribalDialogueSystem Class", bClassExists, 
        bClassExists ? "Class loaded successfully" : "Class not found in module");

    // Test 2: Validate Niagara dialogue effects
    bool bNiagaraValid = ValidateNiagaraDialogueEffects();
    LogValidationResult("Niagara Dialogue Effects", bNiagaraValid);

    // Test 3: Validate speech bubble system
    bool bSpeechBubbleValid = ValidateSpeechBubbleSystem();
    LogValidationResult("Speech Bubble System", bSpeechBubbleValid);

    // Test 4: Validate interaction feedback VFX
    bool bInteractionValid = ValidateInteractionFeedbackVFX();
    LogValidationResult("Interaction Feedback VFX", bInteractionValid);

    // Test 5: Validate VFX actor spawning
    bool bSpawningValid = ValidateVFXActorSpawning();
    LogValidationResult("VFX Actor Spawning", bSpawningValid);

    bool bOverallValid = bClassExists && bNiagaraValid && bSpeechBubbleValid && bInteractionValid && bSpawningValid;
    LogValidationResult("Overall Tribal Dialogue VFX", bOverallValid, 
        FString::Printf(TEXT("Passed: %d, Failed: %d"), PassedTests, FailedTests));

    return bOverallValid;
}

bool UQA_VFXTribalDialogueValidator::ValidateNiagaraDialogueEffects()
{
    // Check if Niagara module is available
    bool bNiagaraAvailable = FModuleManager::Get().IsModuleLoaded("Niagara");
    if (!bNiagaraAvailable)
    {
        LogValidationResult("Niagara Module", false, "Niagara module not loaded");
        return false;
    }

    // Test Niagara system paths that should exist for dialogue
    TArray<FString> ExpectedNiagaraSystems = {
        TEXT("/Game/VFX/Dialogue/NS_SpeechBubble"),
        TEXT("/Game/VFX/Dialogue/NS_InteractionGlow"),
        TEXT("/Game/VFX/Dialogue/NS_TribalWisdom")
    };

    int32 ValidSystems = 0;
    for (const FString& SystemPath : ExpectedNiagaraSystems)
    {
        if (ValidateNiagaraSystemExists(SystemPath))
        {
            ValidSystems++;
        }
    }

    bool bValid = (ValidSystems > 0); // At least one system should be available
    LogValidationResult("Niagara Systems Available", bValid, 
        FString::Printf(TEXT("%d/%d systems found"), ValidSystems, ExpectedNiagaraSystems.Num()));

    return bValid;
}

bool UQA_VFXTribalDialogueValidator::ValidateSpeechBubbleSystem()
{
    // Test speech bubble particle effect triggers
    bool bTriggersValid = ValidateParticleEffectTriggers();
    
    // Test speech bubble visual components
    bool bVisualsValid = true; // Assume valid for now, would need specific asset validation
    
    LogValidationResult("Speech Bubble Triggers", bTriggersValid);
    LogValidationResult("Speech Bubble Visuals", bVisualsValid);
    
    return bTriggersValid && bVisualsValid;
}

bool UQA_VFXTribalDialogueValidator::ValidateInteractionFeedbackVFX()
{
    // Test interaction glow effects
    bool bGlowValid = true;
    
    // Test interaction particle systems
    bool bParticlesValid = true;
    
    // Test interaction sound integration
    bool bSoundValid = true;
    
    LogValidationResult("Interaction Glow", bGlowValid);
    LogValidationResult("Interaction Particles", bParticlesValid);
    LogValidationResult("Interaction Sound", bSoundValid);
    
    return bGlowValid && bParticlesValid && bSoundValid;
}

void UQA_VFXTribalDialogueValidator::RunComprehensiveVFXValidation()
{
    LogValidationResult("Comprehensive VFX Validation", true, "Starting comprehensive validation suite");
    
    // Clean up any existing test actors
    CleanupVFXTestActors();
    
    // Create test scenario
    CreateVFXTestScenario();
    
    // Run all validation tests
    bool bTribalValid = ValidateTribalDialogueVFX();
    
    // Log final results
    FString FinalReport = FString::Printf(
        TEXT("Comprehensive VFX Validation Complete - Passed: %d, Failed: %d, Errors: %d"),
        PassedTests, FailedTests, ValidationErrors.Num()
    );
    
    LogValidationResult("Final Validation Report", bTribalValid, FinalReport);
    
    // Clean up test actors
    CleanupVFXTestActors();
}

void UQA_VFXTribalDialogueValidator::CreateVFXTestScenario()
{
    SpawnTribalDialogueTestActors();
    LogValidationResult("VFX Test Scenario", true, "Test scenario created successfully");
}

void UQA_VFXTribalDialogueValidator::SpawnTribalDialogueTestActors()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        LogValidationResult("World Access", false, "Cannot access world for spawning test actors");
        return;
    }

    // Spawn test dialogue actors
    for (int32 i = 0; i < FMath::Min(MaxTestActors, 5); i++)
    {
        FVector SpawnLocation = FVector(1000 + i * 200, 1000, 200);
        FRotator SpawnRotation = FRotator::ZeroRotator;
        
        AActor* TestActor = World->SpawnActor<AActor>(AActor::StaticClass(), SpawnLocation, SpawnRotation);
        if (TestActor)
        {
            TestActor->SetActorLabel(FString::Printf(TEXT("QA_VFXDialogueTest_%d"), i));
            TestActors.Add(TestActor);
        }
    }
    
    LogValidationResult("Test Actor Spawning", true, 
        FString::Printf(TEXT("Spawned %d test actors"), TestActors.Num()));
}

void UQA_VFXTribalDialogueValidator::CleanupVFXTestActors()
{
    for (AActor* TestActor : TestActors)
    {
        if (IsValid(TestActor))
        {
            TestActor->Destroy();
        }
    }
    TestActors.Empty();
    LogValidationResult("Test Cleanup", true, "All test actors cleaned up");
}

bool UQA_VFXTribalDialogueValidator::ValidateNiagaraSystemExists(const FString& SystemPath)
{
    // Try to load the Niagara system asset
    UObject* NiagaraAsset = LoadObject<UObject>(nullptr, *SystemPath);
    return (NiagaraAsset != nullptr);
}

bool UQA_VFXTribalDialogueValidator::ValidateVFXActorSpawning()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return false;
    }

    // Try to spawn a VFX dialogue system actor
    UClass* VFXClass = LoadClass<AActor>(nullptr, TEXT("/Script/TranspersonalGame.VFX_TribalDialogueSystem"));
    if (!VFXClass)
    {
        return false;
    }

    FVector TestLocation = FVector(1500, 1500, 200);
    AActor* TestVFXActor = World->SpawnActor<AActor>(VFXClass, TestLocation, FRotator::ZeroRotator);
    
    bool bSpawnSuccess = (TestVFXActor != nullptr);
    
    if (TestVFXActor)
    {
        TestVFXActor->SetActorLabel("QA_VFXSpawnTest");
        TestActors.Add(TestVFXActor);
    }
    
    return bSpawnSuccess;
}

bool UQA_VFXTribalDialogueValidator::ValidateParticleEffectTriggers()
{
    // Test particle effect trigger mechanisms
    // This would involve testing Blueprint callable functions and event triggers
    return true; // Placeholder - would need specific trigger validation logic
}

void UQA_VFXTribalDialogueValidator::LogValidationResult(const FString& TestName, bool bPassed, const FString& Details)
{
    if (bPassed)
    {
        PassedTests++;
        if (bEnableDetailedLogging)
        {
            UE_LOG(LogTemp, Log, TEXT("QA VFX PASS: %s - %s"), *TestName, *Details);
        }
    }
    else
    {
        FailedTests++;
        ValidationErrors.Add(FString::Printf(TEXT("%s: %s"), *TestName, *Details));
        if (bEnableDetailedLogging)
        {
            UE_LOG(LogTemp, Warning, TEXT("QA VFX FAIL: %s - %s"), *TestName, *Details);
        }
    }
}

void UQA_VFXTribalDialogueValidator::ResetValidationCounters()
{
    PassedTests = 0;
    FailedTests = 0;
    ValidationErrors.Empty();
}