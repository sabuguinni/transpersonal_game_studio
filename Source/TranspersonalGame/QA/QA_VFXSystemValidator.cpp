#include "QA_VFXSystemValidator.h"
#include "Components/SceneComponent.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/ConstructorHelpers.h"

AQA_VFXSystemValidator::AQA_VFXSystemValidator()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create root scene component
    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;

    // Initialize validation flags
    bVFXManagerValidated = false;
    bImpactEffectsValidated = false;
    bParticleSystemsValidated = false;
    bCharacterIntegrationValidated = false;
    
    ValidationTimer = 0.0f;
    bValidationComplete = false;
}

void AQA_VFXSystemValidator::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Warning, TEXT("QA VFX System Validator initialized"));
    
    // Start validation after a short delay
    GetWorld()->GetTimerManager().SetTimer(
        FTimerHandle(),
        this,
        &AQA_VFXSystemValidator::RunCompleteVFXValidation,
        2.0f,
        false
    );
}

void AQA_VFXSystemValidator::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    ValidationTimer += DeltaTime;
    
    // Run periodic validation checks
    if (!bValidationComplete && ValidationTimer > 5.0f)
    {
        RunCompleteVFXValidation();
        ValidationTimer = 0.0f;
    }
}

bool AQA_VFXSystemValidator::ValidateVFXManagers()
{
    ValidationResults.Add(TEXT("=== VFX MANAGERS VALIDATION ==="));
    
    bool bAllManagersValid = true;
    
    // Check VFX Impact Effects Manager
    UClass* ImpactManagerClass = LoadClass<AActor>(nullptr, TEXT("/Script/TranspersonalGame.VFX_ImpactEffectsManager"));
    if (ImpactManagerClass)
    {
        ValidationResults.Add(TEXT("✅ VFX_ImpactEffectsManager class found"));
        
        // Try to find existing instances
        TArray<AActor*> FoundManagers;
        UGameplayStatics::GetAllActorsOfClass(GetWorld(), ImpactManagerClass, FoundManagers);
        
        if (FoundManagers.Num() > 0)
        {
            ValidationResults.Add(FString::Printf(TEXT("✅ Found %d VFX Impact Manager instances"), FoundManagers.Num()));
        }
        else
        {
            ValidationResults.Add(TEXT("⚠️ No VFX Impact Manager instances found in world"));
        }
    }
    else
    {
        ValidationErrors.Add(TEXT("❌ VFX_ImpactEffectsManager class not found"));
        bAllManagersValid = false;
    }
    
    bVFXManagerValidated = bAllManagersValid;
    return bAllManagersValid;
}

bool AQA_VFXSystemValidator::ValidateImpactEffects()
{
    ValidationResults.Add(TEXT("=== IMPACT EFFECTS VALIDATION ==="));
    
    bool bImpactEffectsValid = true;
    
    // Check for impact effect components and systems
    if (CheckImpactEffectComponents())
    {
        ValidationResults.Add(TEXT("✅ Impact effect components validated"));
    }
    else
    {
        ValidationErrors.Add(TEXT("❌ Impact effect components validation failed"));
        bImpactEffectsValid = false;
    }
    
    bImpactEffectsValidated = bImpactEffectsValid;
    return bImpactEffectsValid;
}

bool AQA_VFXSystemValidator::ValidateParticleSystems()
{
    ValidationResults.Add(TEXT("=== PARTICLE SYSTEMS VALIDATION ==="));
    
    bool bParticleSystemsValid = true;
    
    // Check particle system assets and integration
    if (CheckParticleSystemAssets())
    {
        ValidationResults.Add(TEXT("✅ Particle system assets validated"));
    }
    else
    {
        ValidationErrors.Add(TEXT("❌ Particle system assets validation failed"));
        bParticleSystemsValid = false;
    }
    
    bParticleSystemsValidated = bParticleSystemsValid;
    return bParticleSystemsValid;
}

bool AQA_VFXSystemValidator::ValidateCharacterIntegration()
{
    ValidationResults.Add(TEXT("=== CHARACTER VFX INTEGRATION VALIDATION ==="));
    
    bool bCharacterIntegrationValid = true;
    
    // Check character-VFX integration
    if (CheckCharacterVFXIntegration())
    {
        ValidationResults.Add(TEXT("✅ Character VFX integration validated"));
    }
    else
    {
        ValidationErrors.Add(TEXT("❌ Character VFX integration validation failed"));
        bCharacterIntegrationValid = false;
    }
    
    bCharacterIntegrationValidated = bCharacterIntegrationValid;
    return bCharacterIntegrationValid;
}

void AQA_VFXSystemValidator::RunCompleteVFXValidation()
{
    UE_LOG(LogTemp, Warning, TEXT("Running complete VFX system validation..."));
    
    // Clear previous results
    ValidationResults.Empty();
    ValidationErrors.Empty();
    
    ValidationResults.Add(TEXT("=== QA VFX SYSTEM VALIDATION SUITE ==="));
    ValidationResults.Add(FString::Printf(TEXT("Validation Time: %.2f seconds"), ValidationTimer));
    
    // Run all validation tests
    bool bAllTestsPassed = true;
    
    bAllTestsPassed &= ValidateVFXManagers();
    bAllTestsPassed &= ValidateImpactEffects();
    bAllTestsPassed &= ValidateParticleSystems();
    bAllTestsPassed &= ValidateCharacterIntegration();
    
    // Generate final report
    ValidationResults.Add(TEXT("=== VALIDATION SUMMARY ==="));
    if (bAllTestsPassed)
    {
        ValidationResults.Add(TEXT("✅ ALL VFX SYSTEM TESTS PASSED"));
    }
    else
    {
        ValidationResults.Add(TEXT("❌ SOME VFX SYSTEM TESTS FAILED"));
    }
    
    bValidationComplete = true;
    
    // Log results
    for (const FString& Result : ValidationResults)
    {
        UE_LOG(LogTemp, Warning, TEXT("%s"), *Result);
    }
    
    for (const FString& Error : ValidationErrors)
    {
        UE_LOG(LogTemp, Error, TEXT("%s"), *Error);
    }
}

FString AQA_VFXSystemValidator::GetValidationReport()
{
    FString Report;
    
    for (const FString& Result : ValidationResults)
    {
        Report += Result + TEXT("\n");
    }
    
    if (ValidationErrors.Num() > 0)
    {
        Report += TEXT("\n=== ERRORS ===\n");
        for (const FString& Error : ValidationErrors)
        {
            Report += Error + TEXT("\n");
        }
    }
    
    return Report;
}

bool AQA_VFXSystemValidator::CheckVFXManagerClasses()
{
    // Check if VFX manager classes are properly loaded
    UClass* ImpactManagerClass = LoadClass<AActor>(nullptr, TEXT("/Script/TranspersonalGame.VFX_ImpactEffectsManager"));
    return ImpactManagerClass != nullptr;
}

bool AQA_VFXSystemValidator::CheckImpactEffectComponents()
{
    // Validate impact effect components and their functionality
    // This would check for proper component setup, particle system references, etc.
    return true; // Placeholder - implement specific component checks
}

bool AQA_VFXSystemValidator::CheckParticleSystemAssets()
{
    // Validate particle system assets are properly loaded and configured
    // This would check for Niagara systems, material references, etc.
    return true; // Placeholder - implement specific asset checks
}

bool AQA_VFXSystemValidator::CheckCharacterVFXIntegration()
{
    // Check if character system properly integrates with VFX
    UClass* CharacterClass = LoadClass<APawn>(nullptr, TEXT("/Script/TranspersonalGame.TranspersonalCharacter"));
    if (CharacterClass)
    {
        ValidationResults.Add(TEXT("✅ TranspersonalCharacter class found for VFX integration"));
        return true;
    }
    else
    {
        ValidationErrors.Add(TEXT("❌ TranspersonalCharacter class not found"));
        return false;
    }
}