#include "QA_VFXIntegrationValidator.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "Engine/StaticMesh.h"
#include "Materials/Material.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/DirectionalLight.h"
#include "Components/DirectionalLightComponent.h"

UQA_VFXIntegrationValidator::UQA_VFXIntegrationValidator()
{
    PrimaryComponentTick.bCanEverTick = false;
    
    // Initialize validation parameters
    MaxAllowedParticleCount = 10000;
    MinRequiredFPS = 30.0f;
    ValidationTimeout = 30.0f;
    
    // Initialize validation results
    bValidationPassed = false;
    ValidationScore = 0.0f;
    LastValidationTime = 0.0f;
}

void UQA_VFXIntegrationValidator::BeginPlay()
{
    Super::BeginPlay();
    
    // Start validation process
    StartVFXValidation();
}

void UQA_VFXIntegrationValidator::StartVFXValidation()
{
    UE_LOG(LogTemp, Warning, TEXT("QA_VFXIntegrationValidator: Starting VFX validation process"));
    
    ValidationResults.Empty();
    ValidationScore = 0.0f;
    bValidationPassed = false;
    
    // Run validation tests
    ValidateVFXEffectManager();
    ValidateNiagaraSystems();
    ValidateParticlePerformance();
    ValidateLightingIntegration();
    ValidateEnvironmentalEffects();
    
    // Calculate final score
    CalculateFinalValidationScore();
    
    // Log results
    LogValidationResults();
}

void UQA_VFXIntegrationValidator::ValidateVFXEffectManager()
{
    FQA_ValidationResult Result;
    Result.TestName = TEXT("VFX Effect Manager Validation");
    Result.bPassed = false;
    Result.Score = 0.0f;
    Result.Details = TEXT("Testing VFX Effect Manager functionality");
    
    try
    {
        // Check if VFX Effect Manager class exists
        UClass* VFXManagerClass = FindObject<UClass>(ANY_PACKAGE, TEXT("VFX_EffectManager"));
        if (VFXManagerClass)
        {
            Result.Score += 25.0f;
            Result.Details += TEXT(" | VFX Manager class found");
            
            // Try to spawn VFX manager
            UWorld* World = GetWorld();
            if (World)
            {
                AActor* VFXManager = World->SpawnActor<AActor>(VFXManagerClass);
                if (VFXManager)
                {
                    Result.Score += 25.0f;
                    Result.Details += TEXT(" | VFX Manager spawned successfully");
                    
                    // Clean up
                    VFXManager->Destroy();
                }
                else
                {
                    Result.Details += TEXT(" | Failed to spawn VFX Manager");
                }
            }
        }
        else
        {
            Result.Details += TEXT(" | VFX Manager class not found");
        }
        
        Result.bPassed = Result.Score >= 25.0f;
    }
    catch (...)
    {
        Result.Details += TEXT(" | Exception during VFX Manager validation");
    }
    
    ValidationResults.Add(Result);
}

void UQA_VFXIntegrationValidator::ValidateNiagaraSystems()
{
    FQA_ValidationResult Result;
    Result.TestName = TEXT("Niagara Systems Validation");
    Result.bPassed = false;
    Result.Score = 0.0f;
    Result.Details = TEXT("Testing Niagara particle systems");
    
    try
    {
        // Check for Niagara components in the world
        UWorld* World = GetWorld();
        if (World)
        {
            TArray<AActor*> AllActors;
            UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);
            
            int32 NiagaraComponentCount = 0;
            for (AActor* Actor : AllActors)
            {
                if (Actor)
                {
                    TArray<UNiagaraComponent*> NiagaraComponents;
                    Actor->GetComponents<UNiagaraComponent>(NiagaraComponents);
                    NiagaraComponentCount += NiagaraComponents.Num();
                }
            }
            
            if (NiagaraComponentCount > 0)
            {
                Result.Score += 30.0f;
                Result.Details += FString::Printf(TEXT(" | Found %d Niagara components"), NiagaraComponentCount);
                
                if (NiagaraComponentCount >= 3)
                {
                    Result.Score += 20.0f;
                    Result.Details += TEXT(" | Sufficient Niagara systems present");
                }
            }
            else
            {
                Result.Details += TEXT(" | No Niagara components found");
            }
        }
        
        Result.bPassed = Result.Score >= 30.0f;
    }
    catch (...)
    {
        Result.Details += TEXT(" | Exception during Niagara validation");
    }
    
    ValidationResults.Add(Result);
}

void UQA_VFXIntegrationValidator::ValidateParticlePerformance()
{
    FQA_ValidationResult Result;
    Result.TestName = TEXT("Particle Performance Validation");
    Result.bPassed = false;
    Result.Score = 0.0f;
    Result.Details = TEXT("Testing particle system performance");
    
    try
    {
        // Get current FPS
        float CurrentFPS = 1.0f / GetWorld()->GetDeltaSeconds();
        
        if (CurrentFPS >= MinRequiredFPS)
        {
            Result.Score += 40.0f;
            Result.Details += FString::Printf(TEXT(" | FPS: %.1f (>= %.1f required)"), CurrentFPS, MinRequiredFPS);
            
            if (CurrentFPS >= 60.0f)
            {
                Result.Score += 10.0f;
                Result.Details += TEXT(" | Excellent performance");
            }
        }
        else
        {
            Result.Details += FString::Printf(TEXT(" | Low FPS: %.1f (< %.1f required)"), CurrentFPS, MinRequiredFPS);
        }
        
        Result.bPassed = Result.Score >= 40.0f;
    }
    catch (...)
    {
        Result.Details += TEXT(" | Exception during performance validation");
    }
    
    ValidationResults.Add(Result);
}

void UQA_VFXIntegrationValidator::ValidateLightingIntegration()
{
    FQA_ValidationResult Result;
    Result.TestName = TEXT("Lighting Integration Validation");
    Result.bPassed = false;
    Result.Score = 0.0f;
    Result.Details = TEXT("Testing lighting system integration");
    
    try
    {
        UWorld* World = GetWorld();
        if (World)
        {
            // Check for directional lights
            TArray<AActor*> DirectionalLights;
            UGameplayStatics::GetAllActorsOfClass(World, ADirectionalLight::StaticClass(), DirectionalLights);
            
            if (DirectionalLights.Num() > 0)
            {
                Result.Score += 25.0f;
                Result.Details += FString::Printf(TEXT(" | Found %d directional lights"), DirectionalLights.Num());
                
                // Check light properties
                for (AActor* LightActor : DirectionalLights)
                {
                    ADirectionalLight* DirLight = Cast<ADirectionalLight>(LightActor);
                    if (DirLight && DirLight->GetLightComponent())
                    {
                        float Intensity = DirLight->GetLightComponent()->Intensity;
                        if (Intensity > 0.0f)
                        {
                            Result.Score += 15.0f;
                            Result.Details += FString::Printf(TEXT(" | Light intensity: %.2f"), Intensity);
                            break;
                        }
                    }
                }
            }
            else
            {
                Result.Details += TEXT(" | No directional lights found");
            }
            
            // Check for sky atmosphere
            TArray<AActor*> SkyActors;
            UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), SkyActors);
            
            for (AActor* Actor : SkyActors)
            {
                if (Actor && Actor->GetName().Contains(TEXT("Sky")))
                {
                    Result.Score += 10.0f;
                    Result.Details += TEXT(" | Sky atmosphere detected");
                    break;
                }
            }
        }
        
        Result.bPassed = Result.Score >= 25.0f;
    }
    catch (...)
    {
        Result.Details += TEXT(" | Exception during lighting validation");
    }
    
    ValidationResults.Add(Result);
}

void UQA_VFXIntegrationValidator::ValidateEnvironmentalEffects()
{
    FQA_ValidationResult Result;
    Result.TestName = TEXT("Environmental Effects Validation");
    Result.bPassed = false;
    Result.Score = 0.0f;
    Result.Details = TEXT("Testing environmental VFX effects");
    
    try
    {
        UWorld* World = GetWorld();
        if (World)
        {
            // Check for static mesh actors (terrain, rocks, trees)
            TArray<AActor*> StaticMeshActors;
            UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), StaticMeshActors);
            
            int32 EnvironmentalActors = 0;
            for (AActor* Actor : StaticMeshActors)
            {
                if (Actor)
                {
                    UStaticMeshComponent* MeshComp = Actor->FindComponentByClass<UStaticMeshComponent>();
                    if (MeshComp && MeshComp->GetStaticMesh())
                    {
                        FString ActorName = Actor->GetName();
                        if (ActorName.Contains(TEXT("Tree")) || ActorName.Contains(TEXT("Rock")) || 
                            ActorName.Contains(TEXT("Terrain")) || ActorName.Contains(TEXT("Ground")))
                        {
                            EnvironmentalActors++;
                        }
                    }
                }
            }
            
            if (EnvironmentalActors >= 5)
            {
                Result.Score += 30.0f;
                Result.Details += FString::Printf(TEXT(" | Found %d environmental actors"), EnvironmentalActors);
                
                if (EnvironmentalActors >= 15)
                {
                    Result.Score += 20.0f;
                    Result.Details += TEXT(" | Rich environmental content");
                }
            }
            else
            {
                Result.Details += FString::Printf(TEXT(" | Only %d environmental actors found"), EnvironmentalActors);
            }
        }
        
        Result.bPassed = Result.Score >= 30.0f;
    }
    catch (...)
    {
        Result.Details += TEXT(" | Exception during environmental validation");
    }
    
    ValidationResults.Add(Result);
}

void UQA_VFXIntegrationValidator::CalculateFinalValidationScore()
{
    float TotalScore = 0.0f;
    int32 PassedTests = 0;
    
    for (const FQA_ValidationResult& Result : ValidationResults)
    {
        TotalScore += Result.Score;
        if (Result.bPassed)
        {
            PassedTests++;
        }
    }
    
    ValidationScore = ValidationResults.Num() > 0 ? TotalScore / ValidationResults.Num() : 0.0f;
    bValidationPassed = PassedTests >= (ValidationResults.Num() * 0.8f); // 80% pass rate required
    
    LastValidationTime = GetWorld()->GetTimeSeconds();
}

void UQA_VFXIntegrationValidator::LogValidationResults()
{
    UE_LOG(LogTemp, Warning, TEXT("=== QA VFX INTEGRATION VALIDATION RESULTS ==="));
    UE_LOG(LogTemp, Warning, TEXT("Overall Score: %.2f"), ValidationScore);
    UE_LOG(LogTemp, Warning, TEXT("Validation Passed: %s"), bValidationPassed ? TEXT("YES") : TEXT("NO"));
    
    for (const FQA_ValidationResult& Result : ValidationResults)
    {
        UE_LOG(LogTemp, Warning, TEXT("Test: %s | Score: %.2f | Passed: %s"), 
               *Result.TestName, Result.Score, Result.bPassed ? TEXT("YES") : TEXT("NO"));
        UE_LOG(LogTemp, Warning, TEXT("Details: %s"), *Result.Details);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("=== END VALIDATION RESULTS ==="));
}

TArray<FQA_ValidationResult> UQA_VFXIntegrationValidator::GetValidationResults() const
{
    return ValidationResults;
}

bool UQA_VFXIntegrationValidator::IsValidationPassed() const
{
    return bValidationPassed;
}

float UQA_VFXIntegrationValidator::GetValidationScore() const
{
    return ValidationScore;
}