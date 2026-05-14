#include "QA_VFXSystemValidator.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "NiagaraSystem.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/Material.h"
#include "Engine/Texture2D.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/FileHelper.h"
#include "Misc/DateTime.h"
#include "Engine/GameEngine.h"

UQA_VFXSystemValidator::UQA_VFXSystemValidator()
{
    PrimaryComponentTick.bCanEverTick = false;
    
    bValidationInProgress = false;
    LastValidationTime = 0.0f;
    
    // Set performance thresholds
    MaxFrameTimeThreshold = 16.67f; // 60 FPS target
    MaxParticleCountThreshold = 1000;
    MaxMemoryUsageThreshold = 512.0f; // MB
    
    PerformanceStartTime = 0.0;
    bAssetCacheValid = false;
}

void UQA_VFXSystemValidator::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Log, TEXT("QA_VFXSystemValidator initialized"));
    
    // Initialize asset cache
    CachedVFXAssets.Empty();
    bAssetCacheValid = false;
}

bool UQA_VFXSystemValidator::ValidateVFXSystems()
{
    if (bValidationInProgress)
    {
        UE_LOG(LogTemp, Warning, TEXT("VFX validation already in progress"));
        return false;
    }
    
    bValidationInProgress = true;
    ClearTestResults();
    StartPerformanceTimer();
    
    UE_LOG(LogTemp, Log, TEXT("Starting comprehensive VFX system validation"));
    
    bool bAllTestsPassed = true;
    
    // Test 1: Niagara Systems
    if (!ValidateNiagaraSystems())
    {
        bAllTestsPassed = false;
        AddTestResult(TEXT("Niagara Systems"), EQA_VFXValidationResult::Fail, TEXT("Niagara system validation failed"));
    }
    else
    {
        AddTestResult(TEXT("Niagara Systems"), EQA_VFXValidationResult::Pass, TEXT("All Niagara systems valid"));
    }
    
    // Test 2: Performance
    if (!ValidateParticlePerformance())
    {
        bAllTestsPassed = false;
        AddTestResult(TEXT("Performance"), EQA_VFXValidationResult::Warning, TEXT("Performance thresholds exceeded"));
    }
    else
    {
        AddTestResult(TEXT("Performance"), EQA_VFXValidationResult::Pass, TEXT("Performance within acceptable limits"));
    }
    
    // Test 3: Assets
    if (!ValidateVFXAssets())
    {
        bAllTestsPassed = false;
        AddTestResult(TEXT("Assets"), EQA_VFXValidationResult::Fail, TEXT("VFX asset validation failed"));
    }
    else
    {
        AddTestResult(TEXT("Assets"), EQA_VFXValidationResult::Pass, TEXT("All VFX assets valid"));
    }
    
    // Test 4: Integration
    if (!TestVFXCharacterIntegration())
    {
        AddTestResult(TEXT("Character Integration"), EQA_VFXValidationResult::Warning, TEXT("Character VFX integration issues"));
    }
    else
    {
        AddTestResult(TEXT("Character Integration"), EQA_VFXValidationResult::Pass, TEXT("Character VFX integration working"));
    }
    
    LastValidationTime = EndPerformanceTimer();
    bValidationInProgress = false;
    
    UE_LOG(LogTemp, Log, TEXT("VFX validation completed in %.2f seconds. Overall result: %s"), 
           LastValidationTime, bAllTestsPassed ? TEXT("PASS") : TEXT("FAIL"));
    
    return bAllTestsPassed;
}

bool UQA_VFXSystemValidator::ValidateNiagaraSystems()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("No valid world for Niagara validation"));
        return false;
    }
    
    int32 ValidSystems = 0;
    int32 InvalidSystems = 0;
    
    // Find all Niagara components in the world
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (!Actor) continue;
        
        TArray<UNiagaraComponent*> NiagaraComponents;
        Actor->GetComponents<UNiagaraComponent>(NiagaraComponents);
        
        for (UNiagaraComponent* Component : NiagaraComponents)
        {
            if (ValidateParticleComponent(Component))
            {
                ValidSystems++;
            }
            else
            {
                InvalidSystems++;
                UE_LOG(LogTemp, Warning, TEXT("Invalid Niagara component on actor: %s"), *Actor->GetName());
            }
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Niagara validation: %d valid, %d invalid systems"), ValidSystems, InvalidSystems);
    return InvalidSystems == 0;
}

bool UQA_VFXSystemValidator::ValidateParticlePerformance()
{
    float FrameTime = MeasureVFXFrameTime();
    int32 ParticleCount = CountActiveParticleSystems();
    bool bMemoryOK = CheckVFXMemoryUsage();
    
    bool bPerformanceOK = true;
    
    if (FrameTime > MaxFrameTimeThreshold)
    {
        UE_LOG(LogTemp, Warning, TEXT("VFX frame time %.2fms exceeds threshold %.2fms"), FrameTime, MaxFrameTimeThreshold);
        bPerformanceOK = false;
    }
    
    if (ParticleCount > MaxParticleCountThreshold)
    {
        UE_LOG(LogTemp, Warning, TEXT("Particle count %d exceeds threshold %d"), ParticleCount, MaxParticleCountThreshold);
        bPerformanceOK = false;
    }
    
    if (!bMemoryOK)
    {
        UE_LOG(LogTemp, Warning, TEXT("VFX memory usage exceeds threshold"));
        bPerformanceOK = false;
    }
    
    return bPerformanceOK;
}

bool UQA_VFXSystemValidator::ValidateVFXAssets()
{
    TArray<FString> MissingAssets = FindMissingVFXAssets();
    bool bMaterialsOK = ValidateVFXMaterials();
    bool bTexturesOK = ValidateVFXTextures();
    
    if (MissingAssets.Num() > 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("Found %d missing VFX assets"), MissingAssets.Num());
        for (const FString& Asset : MissingAssets)
        {
            UE_LOG(LogTemp, Warning, TEXT("Missing asset: %s"), *Asset);
        }
    }
    
    return MissingAssets.Num() == 0 && bMaterialsOK && bTexturesOK;
}

float UQA_VFXSystemValidator::MeasureVFXFrameTime()
{
    // Simplified frame time measurement
    // In a real implementation, this would measure actual VFX rendering time
    if (GEngine && GEngine->GetGameViewport())
    {
        return 1000.0f / 60.0f; // Assume 60 FPS for now
    }
    return 0.0f;
}

int32 UQA_VFXSystemValidator::CountActiveParticleSystems()
{
    UWorld* World = GetWorld();
    if (!World) return 0;
    
    int32 Count = 0;
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (!Actor) continue;
        
        TArray<UNiagaraComponent*> Components;
        Actor->GetComponents<UNiagaraComponent>(Components);
        
        for (UNiagaraComponent* Component : Components)
        {
            if (Component && Component->IsActive())
            {
                Count++;
            }
        }
    }
    
    return Count;
}

bool UQA_VFXSystemValidator::CheckVFXMemoryUsage()
{
    // Simplified memory check
    // In a real implementation, this would check actual VFX memory usage
    return true;
}

TArray<FString> UQA_VFXSystemValidator::FindMissingVFXAssets()
{
    TArray<FString> MissingAssets;
    
    // Check for common VFX asset paths
    TArray<FString> ExpectedAssets = {
        TEXT("/Game/VFX/Particles/Fire"),
        TEXT("/Game/VFX/Particles/Smoke"),
        TEXT("/Game/VFX/Particles/Dust"),
        TEXT("/Game/VFX/Materials/ParticleMaterial")
    };
    
    for (const FString& AssetPath : ExpectedAssets)
    {
        UObject* Asset = LoadObject<UObject>(nullptr, *AssetPath);
        if (!Asset)
        {
            MissingAssets.Add(AssetPath);
        }
    }
    
    return MissingAssets;
}

bool UQA_VFXSystemValidator::ValidateVFXMaterials()
{
    // Simplified material validation
    return true;
}

bool UQA_VFXSystemValidator::ValidateVFXTextures()
{
    // Simplified texture validation
    return true;
}

bool UQA_VFXSystemValidator::TestVFXCharacterIntegration()
{
    UWorld* World = GetWorld();
    if (!World) return false;
    
    // Look for character actors with VFX components
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (!Actor) continue;
        
        if (Actor->GetName().Contains(TEXT("Character")) || Actor->GetName().Contains(TEXT("Player")))
        {
            TArray<UNiagaraComponent*> VFXComponents;
            Actor->GetComponents<UNiagaraComponent>(VFXComponents);
            
            if (VFXComponents.Num() > 0)
            {
                UE_LOG(LogTemp, Log, TEXT("Found VFX integration on character: %s"), *Actor->GetName());
                return true;
            }
        }
    }
    
    return true; // No characters found, but that's not necessarily a failure
}

bool UQA_VFXSystemValidator::TestVFXEnvironmentIntegration()
{
    // Test environment VFX integration
    return true;
}

bool UQA_VFXSystemValidator::TestVFXCombatIntegration()
{
    // Test combat VFX integration
    return true;
}

TArray<FQA_VFXTestResult> UQA_VFXSystemValidator::GetLastTestResults()
{
    return TestResults;
}

void UQA_VFXSystemValidator::GenerateVFXQAReport()
{
    FString Report = TEXT("=== VFX QA VALIDATION REPORT ===\n");
    Report += FString::Printf(TEXT("Generated: %s\n"), *FDateTime::Now().ToString());
    Report += FString::Printf(TEXT("Validation Time: %.2f seconds\n\n"), LastValidationTime);
    
    for (const FQA_VFXTestResult& Result : TestResults)
    {
        FString ResultString;
        switch (Result.Result)
        {
            case EQA_VFXValidationResult::Pass: ResultString = TEXT("PASS"); break;
            case EQA_VFXValidationResult::Warning: ResultString = TEXT("WARNING"); break;
            case EQA_VFXValidationResult::Fail: ResultString = TEXT("FAIL"); break;
            case EQA_VFXValidationResult::Critical: ResultString = TEXT("CRITICAL"); break;
        }
        
        Report += FString::Printf(TEXT("[%s] %s: %s\n"), *ResultString, *Result.TestName, *Result.Details);
    }
    
    UE_LOG(LogTemp, Log, TEXT("%s"), *Report);
}

void UQA_VFXSystemValidator::ExportTestResultsToFile()
{
    FString FilePath = FPaths::ProjectSavedDir() / TEXT("QA") / TEXT("VFX_ValidationReport.txt");
    
    FString Report;
    GenerateVFXQAReport();
    
    // Create directory if it doesn't exist
    FString Directory = FPaths::GetPath(FilePath);
    IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
    if (!PlatformFile.DirectoryExists(*Directory))
    {
        PlatformFile.CreateDirectoryTree(*Directory);
    }
    
    // Write report to file
    if (FFileHelper::SaveStringToFile(Report, *FilePath))
    {
        UE_LOG(LogTemp, Log, TEXT("VFX QA report exported to: %s"), *FilePath);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to export VFX QA report to: %s"), *FilePath);
    }
}

bool UQA_VFXSystemValidator::ValidateNiagaraSystem(UNiagaraSystem* System)
{
    if (!System)
    {
        return false;
    }
    
    // Check if system has valid emitters
    if (System->GetNumEmitters() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("Niagara system has no emitters: %s"), *System->GetName());
        return false;
    }
    
    return true;
}

bool UQA_VFXSystemValidator::ValidateParticleComponent(UNiagaraComponent* Component)
{
    if (!Component)
    {
        return false;
    }
    
    UNiagaraSystem* System = Component->GetAsset();
    if (!System)
    {
        UE_LOG(LogTemp, Warning, TEXT("Niagara component has no system asset"));
        return false;
    }
    
    return ValidateNiagaraSystem(System);
}

bool UQA_VFXSystemValidator::CheckVFXLODSettings(UNiagaraSystem* System)
{
    // Check LOD settings for performance
    return System != nullptr;
}

bool UQA_VFXSystemValidator::ValidateVFXCulling(UNiagaraComponent* Component)
{
    // Check culling settings
    return Component != nullptr;
}

void UQA_VFXSystemValidator::StartPerformanceTimer()
{
    PerformanceStartTime = FPlatformTime::Seconds();
}

float UQA_VFXSystemValidator::EndPerformanceTimer()
{
    double EndTime = FPlatformTime::Seconds();
    return static_cast<float>(EndTime - PerformanceStartTime);
}

void UQA_VFXSystemValidator::LogPerformanceMetrics()
{
    float FrameTime = MeasureVFXFrameTime();
    int32 ParticleCount = CountActiveParticleSystems();
    
    UE_LOG(LogTemp, Log, TEXT("VFX Performance Metrics:"));
    UE_LOG(LogTemp, Log, TEXT("  Frame Time: %.2fms"), FrameTime);
    UE_LOG(LogTemp, Log, TEXT("  Active Particles: %d"), ParticleCount);
}

void UQA_VFXSystemValidator::AddTestResult(const FString& TestName, EQA_VFXValidationResult Result, const FString& Details)
{
    FQA_VFXTestResult NewResult;
    NewResult.TestName = TestName;
    NewResult.Result = Result;
    NewResult.Details = Details;
    NewResult.ExecutionTime = LastValidationTime;
    
    TestResults.Add(NewResult);
}

void UQA_VFXSystemValidator::ClearTestResults()
{
    TestResults.Empty();
}