#include "QA_VFXValidationFramework.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraSystem.h"
#include "NiagaraActor.h"
#include "NiagaraComponent.h"
#include "Materials/Material.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/DateTime.h"

AQA_VFXValidationFramework::AQA_VFXValidationFramework()
{
    PrimaryActorTick.bCanEverTick = false;
    
    // Set default values
    bAutoRunOnBeginPlay = false;
    bCleanupAfterTest = true;
    MaxAllowedParticleSystems = 50.0f;
    PerformanceThresholdMS = 16.67f;
    bTestInProgress = false;
}

void AQA_VFXValidationFramework::BeginPlay()
{
    Super::BeginPlay();
    
    if (bAutoRunOnBeginPlay)
    {
        // Delay the test to ensure all systems are initialized
        FTimerHandle TimerHandle;
        GetWorld()->GetTimerManager().SetTimer(TimerHandle, [this]()
        {
            RunFullVFXValidation();
        }, 2.0f, false);
    }
}

FQA_VFXTestResult AQA_VFXValidationFramework::RunFullVFXValidation()
{
    if (bTestInProgress)
    {
        UE_LOG(LogTemp, Warning, TEXT("VFX validation already in progress"));
        return LastTestResult;
    }

    bTestInProgress = true;
    TestStartTime = FPlatformTime::Seconds();
    CurrentErrors.Empty();
    CurrentWarnings.Empty();

    UE_LOG(LogTemp, Log, TEXT("=== Starting Full VFX Validation ==="));

    FQA_VFXTestResult Result;
    
    // Run all validation tests
    Result.bVFXManagerLoaded = ValidateVFXManager();
    Result.bCampfireSystemExists = ValidateParticleSystems();
    Result.bFireMaterialExists = ValidateVFXMaterials();
    Result.bCharacterIntegration = ValidateCharacterIntegration();
    Result.ParticleSystemCount = CountActiveParticleSystems();
    Result.bDustSystemExists = ValidateNiagaraSystem("/Game/VFX/Particles/NS_DinosaurFootstepDust");
    Result.bDustMaterialExists = ValidateMaterial("/Game/VFX/Materials/M_DustParticle");

    // Performance check
    bool bPerformanceOK = CheckVFXPerformance();
    if (!bPerformanceOK)
    {
        AddWarning("VFX Performance below threshold");
    }

    // Test spawning
    AActor* TestCampfire = SpawnTestCampfire(FVector(500, 0, 50));
    AActor* TestDust = SpawnTestDustImpact(FVector(1000, 0, 50));
    
    Result.TestActorsSpawned = 0;
    if (TestCampfire) Result.TestActorsSpawned++;
    if (TestDust) Result.TestActorsSpawned++;

    // Calculate validation time
    Result.ValidationTime = FPlatformTime::Seconds() - TestStartTime;
    
    // Copy error and warning messages
    Result.ErrorMessages = CurrentErrors;
    Result.WarningMessages = CurrentWarnings;

    // Store result
    LastTestResult = Result;

    // Log results
    LogTestResults(Result);

    // Generate report
    GenerateVFXReport();

    // Cleanup if requested
    if (bCleanupAfterTest)
    {
        FTimerHandle CleanupTimer;
        GetWorld()->GetTimerManager().SetTimer(CleanupTimer, [this]()
        {
            CleanupTestActors();
        }, 5.0f, false);
    }

    bTestInProgress = false;
    UE_LOG(LogTemp, Log, TEXT("=== VFX Validation Complete ==="));

    return Result;
}

bool AQA_VFXValidationFramework::ValidateVFXManager()
{
    UClass* VFXManagerClass = LoadClass<AActor>(nullptr, TEXT("/Script/TranspersonalGame.VFX_EffectManager"));
    if (!VFXManagerClass)
    {
        AddError("VFX_EffectManager class not found");
        return false;
    }

    UE_LOG(LogTemp, Log, TEXT("✓ VFX_EffectManager class loaded successfully"));
    return true;
}

bool AQA_VFXValidationFramework::ValidateParticleSystems()
{
    bool bCampfireValid = ValidateNiagaraSystem("/Game/VFX/Particles/NS_CampfireFlames");
    bool bDustValid = ValidateNiagaraSystem("/Game/VFX/Particles/NS_DinosaurFootstepDust");
    
    return bCampfireValid && bDustValid;
}

bool AQA_VFXValidationFramework::ValidateVFXMaterials()
{
    bool bFireMaterialValid = ValidateMaterial("/Game/VFX/Materials/M_FireParticle");
    bool bDustMaterialValid = ValidateMaterial("/Game/VFX/Materials/M_DustParticle");
    
    return bFireMaterialValid && bDustMaterialValid;
}

bool AQA_VFXValidationFramework::ValidateCharacterIntegration()
{
    UClass* CharacterClass = LoadClass<APawn>(nullptr, TEXT("/Script/TranspersonalGame.TranspersonalCharacter"));
    if (!CharacterClass)
    {
        AddError("TranspersonalCharacter class not found for VFX integration");
        return false;
    }

    UE_LOG(LogTemp, Log, TEXT("✓ Character class found for VFX integration"));
    return true;
}

int32 AQA_VFXValidationFramework::CountActiveParticleSystems()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        AddError("World not available for particle system count");
        return 0;
    }

    TArray<AActor*> NiagaraActors;
    UGameplayStatics::GetAllActorsOfClass(World, ANiagaraActor::StaticClass(), NiagaraActors);
    
    int32 ActiveCount = 0;
    for (AActor* Actor : NiagaraActors)
    {
        if (ANiagaraActor* NiagaraActor = Cast<ANiagaraActor>(Actor))
        {
            if (UNiagaraComponent* Component = NiagaraActor->GetNiagaraComponent())
            {
                if (Component->IsActive())
                {
                    ActiveCount++;
                }
            }
        }
    }

    UE_LOG(LogTemp, Log, TEXT("Active particle systems: %d"), ActiveCount);
    
    if (ActiveCount > MaxAllowedParticleSystems)
    {
        AddWarning(FString::Printf(TEXT("High particle system count: %d (max recommended: %.0f)"), 
                                   ActiveCount, MaxAllowedParticleSystems));
    }

    return ActiveCount;
}

bool AQA_VFXValidationFramework::CheckVFXPerformance()
{
    // Simple performance check - in a real implementation, this would measure frame time impact
    float CurrentFrameTime = GetWorld()->GetDeltaSeconds() * 1000.0f; // Convert to milliseconds
    
    if (CurrentFrameTime > PerformanceThresholdMS)
    {
        AddWarning(FString::Printf(TEXT("Frame time %.2fms exceeds threshold %.2fms"), 
                                   CurrentFrameTime, PerformanceThresholdMS));
        return false;
    }

    return true;
}

AActor* AQA_VFXValidationFramework::SpawnTestCampfire(FVector Location)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        AddError("World not available for campfire spawn test");
        return nullptr;
    }

    UClass* NiagaraActorClass = ANiagaraActor::StaticClass();
    if (!NiagaraActorClass)
    {
        AddError("NiagaraActor class not found");
        return nullptr;
    }

    AActor* CampfireActor = World->SpawnActor<AActor>(NiagaraActorClass, Location, FRotator::ZeroRotator);
    if (CampfireActor)
    {
        CampfireActor->SetActorLabel(TEXT("QA_TestCampfire"));
        SpawnedTestActors.Add(CampfireActor);
        
        // Try to set the Niagara system
        if (ANiagaraActor* NiagaraActor = Cast<ANiagaraActor>(CampfireActor))
        {
            UNiagaraSystem* CampfireSystem = LoadObject<UNiagaraSystem>(nullptr, TEXT("/Game/VFX/Particles/NS_CampfireFlames"));
            if (CampfireSystem && NiagaraActor->GetNiagaraComponent())
            {
                NiagaraActor->GetNiagaraComponent()->SetAsset(CampfireSystem);
                UE_LOG(LogTemp, Log, TEXT("✓ Test campfire spawned and configured"));
            }
        }
    }
    else
    {
        AddError("Failed to spawn test campfire actor");
    }

    return CampfireActor;
}

AActor* AQA_VFXValidationFramework::SpawnTestDustImpact(FVector Location)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        AddError("World not available for dust impact spawn test");
        return nullptr;
    }

    AActor* DustActor = World->SpawnActor<AActor>(ANiagaraActor::StaticClass(), Location, FRotator::ZeroRotator);
    if (DustActor)
    {
        DustActor->SetActorLabel(TEXT("QA_TestDustImpact"));
        SpawnedTestActors.Add(DustActor);
        
        if (ANiagaraActor* NiagaraActor = Cast<ANiagaraActor>(DustActor))
        {
            UNiagaraSystem* DustSystem = LoadObject<UNiagaraSystem>(nullptr, TEXT("/Game/VFX/Particles/NS_DinosaurFootstepDust"));
            if (DustSystem && NiagaraActor->GetNiagaraComponent())
            {
                NiagaraActor->GetNiagaraComponent()->SetAsset(DustSystem);
                UE_LOG(LogTemp, Log, TEXT("✓ Test dust impact spawned and configured"));
            }
        }
    }
    else
    {
        AddError("Failed to spawn test dust impact actor");
    }

    return DustActor;
}

void AQA_VFXValidationFramework::CleanupTestActors()
{
    for (AActor* Actor : SpawnedTestActors)
    {
        if (IsValid(Actor))
        {
            Actor->Destroy();
        }
    }
    
    SpawnedTestActors.Empty();
    UE_LOG(LogTemp, Log, TEXT("Test actors cleaned up"));
}

void AQA_VFXValidationFramework::GenerateVFXReport()
{
    FString ReportContent = TEXT("=== VFX VALIDATION REPORT ===\n");
    ReportContent += FString::Printf(TEXT("Validation Time: %.3f seconds\n"), LastTestResult.ValidationTime);
    ReportContent += FString::Printf(TEXT("VFX Manager Loaded: %s\n"), LastTestResult.bVFXManagerLoaded ? TEXT("PASS") : TEXT("FAIL"));
    ReportContent += FString::Printf(TEXT("Campfire System: %s\n"), LastTestResult.bCampfireSystemExists ? TEXT("PASS") : TEXT("FAIL"));
    ReportContent += FString::Printf(TEXT("Dust System: %s\n"), LastTestResult.bDustSystemExists ? TEXT("PASS") : TEXT("FAIL"));
    ReportContent += FString::Printf(TEXT("Fire Material: %s\n"), LastTestResult.bFireMaterialExists ? TEXT("PASS") : TEXT("FAIL"));
    ReportContent += FString::Printf(TEXT("Dust Material: %s\n"), LastTestResult.bDustMaterialExists ? TEXT("PASS") : TEXT("FAIL"));
    ReportContent += FString::Printf(TEXT("Character Integration: %s\n"), LastTestResult.bCharacterIntegration ? TEXT("PASS") : TEXT("FAIL"));
    ReportContent += FString::Printf(TEXT("Active Particle Systems: %d\n"), LastTestResult.ParticleSystemCount);
    ReportContent += FString::Printf(TEXT("Test Actors Spawned: %d\n"), LastTestResult.TestActorsSpawned);
    
    if (LastTestResult.ErrorMessages.Num() > 0)
    {
        ReportContent += TEXT("\nERRORS:\n");
        for (const FString& Error : LastTestResult.ErrorMessages)
        {
            ReportContent += FString::Printf(TEXT("- %s\n"), *Error);
        }
    }
    
    if (LastTestResult.WarningMessages.Num() > 0)
    {
        ReportContent += TEXT("\nWARNINGS:\n");
        for (const FString& Warning : LastTestResult.WarningMessages)
        {
            ReportContent += FString::Printf(TEXT("- %s\n"), *Warning);
        }
    }

    UE_LOG(LogTemp, Log, TEXT("%s"), *ReportContent);
}

void AQA_VFXValidationFramework::LogTestResults(const FQA_VFXTestResult& Results)
{
    UE_LOG(LogTemp, Log, TEXT("=== VFX TEST RESULTS ==="));
    UE_LOG(LogTemp, Log, TEXT("VFX Manager: %s"), Results.bVFXManagerLoaded ? TEXT("✓") : TEXT("✗"));
    UE_LOG(LogTemp, Log, TEXT("Campfire System: %s"), Results.bCampfireSystemExists ? TEXT("✓") : TEXT("✗"));
    UE_LOG(LogTemp, Log, TEXT("Dust System: %s"), Results.bDustSystemExists ? TEXT("✓") : TEXT("✗"));
    UE_LOG(LogTemp, Log, TEXT("Fire Material: %s"), Results.bFireMaterialExists ? TEXT("✓") : TEXT("✗"));
    UE_LOG(LogTemp, Log, TEXT("Dust Material: %s"), Results.bDustMaterialExists ? TEXT("✓") : TEXT("✗"));
    UE_LOG(LogTemp, Log, TEXT("Character Integration: %s"), Results.bCharacterIntegration ? TEXT("✓") : TEXT("✗"));
    UE_LOG(LogTemp, Log, TEXT("Particle Count: %d"), Results.ParticleSystemCount);
    UE_LOG(LogTemp, Log, TEXT("Test Actors: %d"), Results.TestActorsSpawned);
    UE_LOG(LogTemp, Log, TEXT("Validation Time: %.3fs"), Results.ValidationTime);
}

bool AQA_VFXValidationFramework::ValidateNiagaraSystem(const FString& SystemPath)
{
    UNiagaraSystem* System = LoadObject<UNiagaraSystem>(nullptr, *SystemPath);
    if (!System)
    {
        AddError(FString::Printf(TEXT("Niagara system not found: %s"), *SystemPath));
        return false;
    }

    UE_LOG(LogTemp, Log, TEXT("✓ Niagara system found: %s"), *SystemPath);
    return true;
}

bool AQA_VFXValidationFramework::ValidateMaterial(const FString& MaterialPath)
{
    UMaterial* Material = LoadObject<UMaterial>(nullptr, *MaterialPath);
    if (!Material)
    {
        AddError(FString::Printf(TEXT("Material not found: %s"), *MaterialPath));
        return false;
    }

    UE_LOG(LogTemp, Log, TEXT("✓ Material found: %s"), *MaterialPath);
    return true;
}

void AQA_VFXValidationFramework::AddError(const FString& ErrorMessage)
{
    CurrentErrors.Add(ErrorMessage);
    UE_LOG(LogTemp, Error, TEXT("VFX Validation Error: %s"), *ErrorMessage);
}

void AQA_VFXValidationFramework::AddWarning(const FString& WarningMessage)
{
    CurrentWarnings.Add(WarningMessage);
    UE_LOG(LogTemp, Warning, TEXT("VFX Validation Warning: %s"), *WarningMessage);
}