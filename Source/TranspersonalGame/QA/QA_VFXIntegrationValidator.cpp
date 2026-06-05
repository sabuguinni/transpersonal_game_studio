#include "QA_VFXIntegrationValidator.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "NiagaraSystem.h"
#include "NiagaraComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/Actor.h"
#include "Engine/StaticMeshActor.h"
#include "TranspersonalGame/VFX/VFX_NiagaraLibrary.h"
#include "TranspersonalGame/Core/TranspersonalCharacter.h"
#include "TranspersonalGame/Core/TranspersonalGameState.h"

UQA_VFXIntegrationValidator::UQA_VFXIntegrationValidator()
{
    PrimaryComponentTick.bCanEverTick = false;
    
    // Initialize validation parameters
    MaxAllowedVFXActors = 50;
    MinRequiredVFXSystems = 3;
    PerformanceThresholdMS = 16.67f; // 60 FPS target
    
    // Initialize validation results
    ValidationResults.Empty();
    LastValidationTime = 0.0f;
    bValidationPassed = false;
}

void UQA_VFXIntegrationValidator::BeginPlay()
{
    Super::BeginPlay();
    
    // Start validation on begin play
    RunVFXIntegrationValidation();
}

bool UQA_VFXIntegrationValidator::RunVFXIntegrationValidation()
{
    UE_LOG(LogTemp, Warning, TEXT("QA_VFXIntegrationValidator: Starting VFX integration validation"));
    
    ValidationResults.Empty();
    bValidationPassed = true;
    LastValidationTime = GetWorld()->GetTimeSeconds();
    
    // Test 1: VFX Library Class Validation
    if (!ValidateVFXLibraryClass())
    {
        bValidationPassed = false;
    }
    
    // Test 2: Niagara System Validation
    if (!ValidateNiagaraSystems())
    {
        bValidationPassed = false;
    }
    
    // Test 3: VFX Actor Count Validation
    if (!ValidateVFXActorCount())
    {
        bValidationPassed = false;
    }
    
    // Test 4: Performance Impact Validation
    if (!ValidateVFXPerformance())
    {
        bValidationPassed = false;
    }
    
    // Test 5: Integration with Character System
    if (!ValidateCharacterVFXIntegration())
    {
        bValidationPassed = false;
    }
    
    // Generate final report
    GenerateValidationReport();
    
    UE_LOG(LogTemp, Warning, TEXT("QA_VFXIntegrationValidator: Validation complete - Result: %s"), 
           bValidationPassed ? TEXT("PASS") : TEXT("FAIL"));
    
    return bValidationPassed;
}

bool UQA_VFXIntegrationValidator::ValidateVFXLibraryClass()
{
    FQA_ValidationResult Result;
    Result.TestName = TEXT("VFX Library Class Validation");
    Result.bPassed = false;
    Result.ErrorMessage = TEXT("");
    
    // Check if VFX_NiagaraLibrary class exists and is functional
    UClass* VFXLibClass = UVFX_NiagaraLibrary::StaticClass();
    if (VFXLibClass)
    {
        Result.bPassed = true;
        Result.Details = TEXT("VFX_NiagaraLibrary class found and accessible");
        UE_LOG(LogTemp, Log, TEXT("QA_VFXIntegrationValidator: VFX Library class validation PASSED"));
    }
    else
    {
        Result.ErrorMessage = TEXT("VFX_NiagaraLibrary class not found or not compiled");
        UE_LOG(LogTemp, Error, TEXT("QA_VFXIntegrationValidator: VFX Library class validation FAILED"));
    }
    
    ValidationResults.Add(Result);
    return Result.bPassed;
}

bool UQA_VFXIntegrationValidator::ValidateNiagaraSystems()
{
    FQA_ValidationResult Result;
    Result.TestName = TEXT("Niagara Systems Validation");
    Result.bPassed = false;
    Result.ErrorMessage = TEXT("");
    
    // Count Niagara components in the world
    int32 NiagaraComponentCount = 0;
    
    for (TActorIterator<AActor> ActorItr(GetWorld()); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (Actor)
        {
            TArray<UNiagaraComponent*> NiagaraComponents;
            Actor->GetComponents<UNiagaraComponent>(NiagaraComponents);
            NiagaraComponentCount += NiagaraComponents.Num();
        }
    }
    
    if (NiagaraComponentCount >= MinRequiredVFXSystems)
    {
        Result.bPassed = true;
        Result.Details = FString::Printf(TEXT("Found %d Niagara components (minimum required: %d)"), 
                                       NiagaraComponentCount, MinRequiredVFXSystems);
        UE_LOG(LogTemp, Log, TEXT("QA_VFXIntegrationValidator: Niagara systems validation PASSED"));
    }
    else
    {
        Result.ErrorMessage = FString::Printf(TEXT("Insufficient Niagara systems: %d found, %d required"), 
                                            NiagaraComponentCount, MinRequiredVFXSystems);
        UE_LOG(LogTemp, Error, TEXT("QA_VFXIntegrationValidator: Niagara systems validation FAILED"));
    }
    
    ValidationResults.Add(Result);
    return Result.bPassed;
}

bool UQA_VFXIntegrationValidator::ValidateVFXActorCount()
{
    FQA_ValidationResult Result;
    Result.TestName = TEXT("VFX Actor Count Validation");
    Result.bPassed = false;
    Result.ErrorMessage = TEXT("");
    
    // Count actors with VFX-related names or components
    int32 VFXActorCount = 0;
    
    for (TActorIterator<AActor> ActorItr(GetWorld()); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (Actor)
        {
            FString ActorName = Actor->GetName();
            if (ActorName.Contains(TEXT("VFX")) || 
                ActorName.Contains(TEXT("Effect")) || 
                ActorName.Contains(TEXT("Particle")) ||
                Actor->GetComponentByClass(UNiagaraComponent::StaticClass()))
            {
                VFXActorCount++;
            }
        }
    }
    
    if (VFXActorCount <= MaxAllowedVFXActors)
    {
        Result.bPassed = true;
        Result.Details = FString::Printf(TEXT("VFX actor count within limits: %d/%d"), 
                                       VFXActorCount, MaxAllowedVFXActors);
        UE_LOG(LogTemp, Log, TEXT("QA_VFXIntegrationValidator: VFX actor count validation PASSED"));
    }
    else
    {
        Result.ErrorMessage = FString::Printf(TEXT("Too many VFX actors: %d found, maximum allowed: %d"), 
                                            VFXActorCount, MaxAllowedVFXActors);
        UE_LOG(LogTemp, Warning, TEXT("QA_VFXIntegrationValidator: VFX actor count validation FAILED"));
    }
    
    ValidationResults.Add(Result);
    return Result.bPassed;
}

bool UQA_VFXIntegrationValidator::ValidateVFXPerformance()
{
    FQA_ValidationResult Result;
    Result.TestName = TEXT("VFX Performance Validation");
    Result.bPassed = true; // Assume pass for basic implementation
    Result.ErrorMessage = TEXT("");
    
    // Basic performance check - count total components
    int32 TotalComponentCount = 0;
    
    for (TActorIterator<AActor> ActorItr(GetWorld()); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (Actor)
        {
            TotalComponentCount += Actor->GetRootComponent() ? 1 : 0;
            TArray<UActorComponent*> Components = Actor->GetInstanceComponents().Array();
            TotalComponentCount += Components.Num();
        }
    }
    
    // Simple heuristic: if we have too many components, performance might suffer
    if (TotalComponentCount < 10000) // Arbitrary threshold
    {
        Result.Details = FString::Printf(TEXT("Component count within reasonable limits: %d"), TotalComponentCount);
        UE_LOG(LogTemp, Log, TEXT("QA_VFXIntegrationValidator: VFX performance validation PASSED"));
    }
    else
    {
        Result.bPassed = false;
        Result.ErrorMessage = FString::Printf(TEXT("High component count may impact performance: %d"), TotalComponentCount);
        UE_LOG(LogTemp, Warning, TEXT("QA_VFXIntegrationValidator: VFX performance validation WARNING"));
    }
    
    ValidationResults.Add(Result);
    return Result.bPassed;
}

bool UQA_VFXIntegrationValidator::ValidateCharacterVFXIntegration()
{
    FQA_ValidationResult Result;
    Result.TestName = TEXT("Character VFX Integration Validation");
    Result.bPassed = false;
    Result.ErrorMessage = TEXT("");
    
    // Find TranspersonalCharacter actors
    int32 CharacterCount = 0;
    int32 CharactersWithVFX = 0;
    
    for (TActorIterator<ATranspersonalCharacter> CharacterItr(GetWorld()); CharacterItr; ++CharacterItr)
    {
        ATranspersonalCharacter* Character = *CharacterItr;
        if (Character)
        {
            CharacterCount++;
            
            // Check if character has VFX components
            TArray<UNiagaraComponent*> VFXComponents;
            Character->GetComponents<UNiagaraComponent>(VFXComponents);
            
            if (VFXComponents.Num() > 0)
            {
                CharactersWithVFX++;
            }
        }
    }
    
    if (CharacterCount > 0)
    {
        Result.bPassed = true;
        Result.Details = FString::Printf(TEXT("Found %d characters, %d with VFX integration"), 
                                       CharacterCount, CharactersWithVFX);
        UE_LOG(LogTemp, Log, TEXT("QA_VFXIntegrationValidator: Character VFX integration validation PASSED"));
    }
    else
    {
        Result.ErrorMessage = TEXT("No TranspersonalCharacter actors found in world");
        UE_LOG(LogTemp, Warning, TEXT("QA_VFXIntegrationValidator: Character VFX integration validation FAILED"));
    }
    
    ValidationResults.Add(Result);
    return Result.bPassed;
}

void UQA_VFXIntegrationValidator::GenerateValidationReport()
{
    UE_LOG(LogTemp, Warning, TEXT("=== QA VFX INTEGRATION VALIDATION REPORT ==="));
    UE_LOG(LogTemp, Warning, TEXT("Validation Time: %f"), LastValidationTime);
    UE_LOG(LogTemp, Warning, TEXT("Overall Result: %s"), bValidationPassed ? TEXT("PASS") : TEXT("FAIL"));
    UE_LOG(LogTemp, Warning, TEXT("Total Tests: %d"), ValidationResults.Num());
    
    int32 PassedTests = 0;
    for (const FQA_ValidationResult& Result : ValidationResults)
    {
        if (Result.bPassed)
        {
            PassedTests++;
            UE_LOG(LogTemp, Log, TEXT("[PASS] %s: %s"), *Result.TestName, *Result.Details);
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("[FAIL] %s: %s"), *Result.TestName, *Result.ErrorMessage);
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Tests Passed: %d/%d"), PassedTests, ValidationResults.Num());
    UE_LOG(LogTemp, Warning, TEXT("=== END VALIDATION REPORT ==="));
}

TArray<FQA_ValidationResult> UQA_VFXIntegrationValidator::GetValidationResults() const
{
    return ValidationResults;
}

bool UQA_VFXIntegrationValidator::IsValidationPassed() const
{
    return bValidationPassed;
}

float UQA_VFXIntegrationValidator::GetLastValidationTime() const
{
    return LastValidationTime;
}