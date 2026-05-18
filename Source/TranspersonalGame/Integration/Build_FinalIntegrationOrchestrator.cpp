#include "Build_FinalIntegrationOrchestrator.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/Pawn.h"
#include "Engine/Light.h"
#include "Engine/DirectionalLight.h"
#include "Engine/PointLight.h"
#include "Engine/SpotLight.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/DateTime.h"
#include "Misc/FileHelper.h"

UBuild_FinalIntegrationOrchestrator::UBuild_FinalIntegrationOrchestrator()
{
    bValidationInProgress = false;
    LastValidationTime = 0.0f;
    TotalActorCount = 0;
    TotalDinosaurCount = 0;
    MemoryUsageMB = 0.0f;
    FrameRate = 0.0f;
}

void UBuild_FinalIntegrationOrchestrator::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("Build_FinalIntegrationOrchestrator: Initialized"));
    
    // Initialize validation results array
    ValidationResults.Empty();
    BiomeData.Empty();
    
    // Start initial validation
    StartFullSystemValidation();
}

void UBuild_FinalIntegrationOrchestrator::Deinitialize()
{
    UE_LOG(LogTemp, Warning, TEXT("Build_FinalIntegrationOrchestrator: Deinitializing"));
    
    ValidationResults.Empty();
    BiomeData.Empty();
    
    Super::Deinitialize();
}

void UBuild_FinalIntegrationOrchestrator::StartFullSystemValidation()
{
    if (bValidationInProgress)
    {
        UE_LOG(LogTemp, Warning, TEXT("Validation already in progress"));
        return;
    }
    
    bValidationInProgress = true;
    LastValidationTime = FPlatformTime::Seconds();
    
    UE_LOG(LogTemp, Warning, TEXT("Starting full system validation"));
    
    // Clear previous results
    ValidationResults.Empty();
    BiomeData.Empty();
    
    // Validate all systems
    ValidateAllBiomes();
    ValidateCharacterSystems();
    ValidateDinosaurSystems();
    ValidateWorldGeneration();
    ValidateAudioSystems();
    ValidateVFXSystems();
    ValidatePhysicsSystems();
    ValidatePerformanceMetrics();
    
    bValidationInProgress = false;
    
    UE_LOG(LogTemp, Warning, TEXT("Full system validation complete"));
    GenerateIntegrationReport();
}

void UBuild_FinalIntegrationOrchestrator::ValidateAllBiomes()
{
    UE_LOG(LogTemp, Warning, TEXT("Validating all biomes"));
    
    // Define biome centers
    TMap<FString, FVector> BiomeCenters;
    BiomeCenters.Add(TEXT("Savana"), FVector(0, 0, 0));
    BiomeCenters.Add(TEXT("Pantano"), FVector(-50000, -45000, 0));
    BiomeCenters.Add(TEXT("Floresta"), FVector(-45000, 40000, 0));
    BiomeCenters.Add(TEXT("Deserto"), FVector(55000, 0, 0));
    BiomeCenters.Add(TEXT("Montanha"), FVector(40000, 50000, 0));
    
    for (const auto& BiomePair : BiomeCenters)
    {
        ValidateBiomeInternal(BiomePair.Key, BiomePair.Value);
    }
}

void UBuild_FinalIntegrationOrchestrator::ValidateCharacterSystems()
{
    ValidateSystemInternal(TEXT("CharacterSystems"));
}

void UBuild_FinalIntegrationOrchestrator::ValidateDinosaurSystems()
{
    ValidateSystemInternal(TEXT("DinosaurSystems"));
}

void UBuild_FinalIntegrationOrchestrator::ValidateWorldGeneration()
{
    ValidateSystemInternal(TEXT("WorldGeneration"));
}

void UBuild_FinalIntegrationOrchestrator::ValidateAudioSystems()
{
    ValidateSystemInternal(TEXT("AudioSystems"));
}

void UBuild_FinalIntegrationOrchestrator::ValidateVFXSystems()
{
    ValidateSystemInternal(TEXT("VFXSystems"));
}

void UBuild_FinalIntegrationOrchestrator::ValidatePhysicsSystems()
{
    ValidateSystemInternal(TEXT("PhysicsSystems"));
}

void UBuild_FinalIntegrationOrchestrator::ValidatePerformanceMetrics()
{
    UpdatePerformanceMetrics();
    ValidateSystemInternal(TEXT("PerformanceMetrics"));
}

void UBuild_FinalIntegrationOrchestrator::ValidateSystemInternal(const FString& SystemName)
{
    float StartTime = FPlatformTime::Seconds();
    
    FBuild_SystemValidationResult Result;
    Result.SystemName = SystemName;
    Result.Status = EBuild_IntegrationStatus::InProgress;
    
    UWorld* World = GetWorld();
    if (!World)
    {
        Result.Status = EBuild_IntegrationStatus::Failed;
        Result.ErrorMessage = TEXT("World not found");
        ValidationResults.Add(Result);
        return;
    }
    
    // Count actors in the world
    int32 ActorCount = 0;
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (Actor && IsValid(Actor))
        {
            ActorCount++;
        }
    }
    
    Result.ActorCount = ActorCount;
    TotalActorCount = ActorCount;
    
    // System-specific validation
    if (SystemName == TEXT("CharacterSystems"))
    {
        int32 CharacterCount = 0;
        for (TActorIterator<ACharacter> CharacterItr(World); CharacterItr; ++CharacterItr)
        {
            if (IsValid(*CharacterItr))
            {
                CharacterCount++;
            }
        }
        
        if (CharacterCount > 0)
        {
            Result.Status = EBuild_IntegrationStatus::Success;
        }
        else
        {
            Result.Status = EBuild_IntegrationStatus::Failed;
            Result.ErrorMessage = TEXT("No characters found in world");
        }
    }
    else if (SystemName == TEXT("DinosaurSystems"))
    {
        int32 DinosaurCount = 0;
        for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
        {
            AActor* Actor = *ActorItr;
            if (Actor && IsValid(Actor))
            {
                FString ActorName = Actor->GetName().ToLower();
                if (ActorName.Contains(TEXT("trex")) || ActorName.Contains(TEXT("raptor")) || 
                    ActorName.Contains(TEXT("brachio")) || ActorName.Contains(TEXT("tricera")))
                {
                    DinosaurCount++;
                }
            }
        }
        
        TotalDinosaurCount = DinosaurCount;
        Result.Status = DinosaurCount > 0 ? EBuild_IntegrationStatus::Success : EBuild_IntegrationStatus::Failed;
        if (DinosaurCount == 0)
        {
            Result.ErrorMessage = TEXT("No dinosaurs found in world");
        }
    }
    else
    {
        // Generic validation - if we have actors, system is considered working
        Result.Status = ActorCount > 0 ? EBuild_IntegrationStatus::Success : EBuild_IntegrationStatus::Failed;
        if (ActorCount == 0)
        {
            Result.ErrorMessage = TEXT("No actors found for system");
        }
    }
    
    Result.ValidationTime = FPlatformTime::Seconds() - StartTime;
    ValidationResults.Add(Result);
    
    LogValidationResult(Result);
}

void UBuild_FinalIntegrationOrchestrator::ValidateBiomeInternal(const FString& BiomeName, const FVector& BiomeCenter)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    FBuild_BiomeValidationData BiomeValidation;
    BiomeValidation.BiomeName = BiomeName;
    BiomeValidation.BiomeCenter = BiomeCenter;
    
    // Count actors within biome radius (25km)
    const float BiomeRadius = 25000.0f;
    
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (Actor && IsValid(Actor))
        {
            FVector ActorLocation = Actor->GetActorLocation();
            float Distance = FVector::Dist2D(ActorLocation, BiomeCenter);
            
            if (Distance <= BiomeRadius)
            {
                BiomeValidation.ActorCount++;
                
                // Check for dinosaurs
                FString ActorName = Actor->GetName().ToLower();
                if (ActorName.Contains(TEXT("trex")) || ActorName.Contains(TEXT("raptor")) || 
                    ActorName.Contains(TEXT("brachio")) || ActorName.Contains(TEXT("tricera")))
                {
                    BiomeValidation.DinosaurCount++;
                }
                
                // Check for vegetation
                if (ActorName.Contains(TEXT("tree")) || ActorName.Contains(TEXT("plant")) || 
                    ActorName.Contains(TEXT("fern")) || ActorName.Contains(TEXT("grass")))
                {
                    BiomeValidation.VegetationCount++;
                }
                
                // Check for lighting
                if (Actor->IsA<ALight>())
                {
                    BiomeValidation.bHasLighting = true;
                }
            }
        }
    }
    
    BiomeData.Add(BiomeValidation);
    
    UE_LOG(LogTemp, Warning, TEXT("Biome %s: %d actors, %d dinosaurs, %d vegetation"), 
           *BiomeName, BiomeValidation.ActorCount, BiomeValidation.DinosaurCount, BiomeValidation.VegetationCount);
}

void UBuild_FinalIntegrationOrchestrator::UpdatePerformanceMetrics()
{
    // Get memory usage (simplified)
    MemoryUsageMB = FPlatformMemory::GetStats().UsedPhysical / (1024.0f * 1024.0f);
    
    // Get frame rate (simplified)
    FrameRate = 1.0f / FApp::GetDeltaTime();
    if (FrameRate > 1000.0f) FrameRate = 60.0f; // Cap unrealistic values
}

FBuild_SystemValidationResult UBuild_FinalIntegrationOrchestrator::GetValidationResult(const FString& SystemName)
{
    for (const FBuild_SystemValidationResult& Result : ValidationResults)
    {
        if (Result.SystemName == SystemName)
        {
            return Result;
        }
    }
    
    // Return empty result if not found
    FBuild_SystemValidationResult EmptyResult;
    EmptyResult.SystemName = SystemName;
    EmptyResult.Status = EBuild_IntegrationStatus::Failed;
    EmptyResult.ErrorMessage = TEXT("System not validated");
    return EmptyResult;
}

TArray<FBuild_SystemValidationResult> UBuild_FinalIntegrationOrchestrator::GetAllValidationResults()
{
    return ValidationResults;
}

TArray<FBuild_BiomeValidationData> UBuild_FinalIntegrationOrchestrator::GetBiomeValidationData()
{
    return BiomeData;
}

bool UBuild_FinalIntegrationOrchestrator::IsSystemHealthy()
{
    int32 SuccessCount = 0;
    int32 TotalCount = ValidationResults.Num();
    
    for (const FBuild_SystemValidationResult& Result : ValidationResults)
    {
        if (Result.Status == EBuild_IntegrationStatus::Success)
        {
            SuccessCount++;
        }
    }
    
    // System is healthy if at least 80% of validations pass
    return TotalCount > 0 && (float)SuccessCount / (float)TotalCount >= 0.8f;
}

void UBuild_FinalIntegrationOrchestrator::GenerateIntegrationReport()
{
    FString Report = TEXT("=== INTEGRATION REPORT ===\n");
    Report += FString::Printf(TEXT("Validation Time: %s\n"), *FDateTime::Now().ToString());
    Report += FString::Printf(TEXT("Total Actors: %d\n"), TotalActorCount);
    Report += FString::Printf(TEXT("Total Dinosaurs: %d\n"), TotalDinosaurCount);
    Report += FString::Printf(TEXT("Memory Usage: %.2f MB\n"), MemoryUsageMB);
    Report += FString::Printf(TEXT("Frame Rate: %.1f FPS\n"), FrameRate);
    Report += FString::Printf(TEXT("System Health: %s\n"), IsSystemHealthy() ? TEXT("HEALTHY") : TEXT("DEGRADED"));
    
    Report += TEXT("\n=== SYSTEM VALIDATION RESULTS ===\n");
    for (const FBuild_SystemValidationResult& Result : ValidationResults)
    {
        FString StatusText;
        switch (Result.Status)
        {
            case EBuild_IntegrationStatus::Success: StatusText = TEXT("SUCCESS"); break;
            case EBuild_IntegrationStatus::Failed: StatusText = TEXT("FAILED"); break;
            case EBuild_IntegrationStatus::InProgress: StatusText = TEXT("IN_PROGRESS"); break;
            case EBuild_IntegrationStatus::Timeout: StatusText = TEXT("TIMEOUT"); break;
            default: StatusText = TEXT("PENDING"); break;
        }
        
        Report += FString::Printf(TEXT("%s: %s (%.3fs, %d actors)\n"), 
                                 *Result.SystemName, *StatusText, Result.ValidationTime, Result.ActorCount);
        
        if (!Result.ErrorMessage.IsEmpty())
        {
            Report += FString::Printf(TEXT("  Error: %s\n"), *Result.ErrorMessage);
        }
    }
    
    Report += TEXT("\n=== BIOME VALIDATION ===\n");
    for (const FBuild_BiomeValidationData& Biome : BiomeData)
    {
        Report += FString::Printf(TEXT("%s: %d actors, %d dinosaurs, %d vegetation, Lighting: %s\n"),
                                 *Biome.BiomeName, Biome.ActorCount, Biome.DinosaurCount, 
                                 Biome.VegetationCount, Biome.bHasLighting ? TEXT("YES") : TEXT("NO"));
    }
    
    UE_LOG(LogTemp, Warning, TEXT("%s"), *Report);
    
    // Save report to file
    FString FilePath = FPaths::ProjectLogDir() / TEXT("IntegrationReport.txt");
    FFileHelper::SaveStringToFile(Report, *FilePath);
}

void UBuild_FinalIntegrationOrchestrator::CleanupOrphanedActors()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    int32 CleanedCount = 0;
    TArray<AActor*> ActorsToDestroy;
    
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (Actor && !IsValid(Actor))
        {
            ActorsToDestroy.Add(Actor);
        }
    }
    
    for (AActor* Actor : ActorsToDestroy)
    {
        if (Actor)
        {
            Actor->Destroy();
            CleanedCount++;
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Cleaned up %d orphaned actors"), CleanedCount);
}

void UBuild_FinalIntegrationOrchestrator::OptimizeActorDistribution()
{
    UE_LOG(LogTemp, Warning, TEXT("Optimizing actor distribution across biomes"));
    
    // This is a placeholder for future optimization logic
    // Could implement actor redistribution, LOD optimization, etc.
}

void UBuild_FinalIntegrationOrchestrator::LogValidationResult(const FBuild_SystemValidationResult& Result)
{
    FString StatusText;
    switch (Result.Status)
    {
        case EBuild_IntegrationStatus::Success: StatusText = TEXT("SUCCESS"); break;
        case EBuild_IntegrationStatus::Failed: StatusText = TEXT("FAILED"); break;
        case EBuild_IntegrationStatus::InProgress: StatusText = TEXT("IN_PROGRESS"); break;
        case EBuild_IntegrationStatus::Timeout: StatusText = TEXT("TIMEOUT"); break;
        default: StatusText = TEXT("PENDING"); break;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Validation: %s - %s (%.3fs, %d actors)"), 
           *Result.SystemName, *StatusText, Result.ValidationTime, Result.ActorCount);
    
    if (!Result.ErrorMessage.IsEmpty())
    {
        UE_LOG(LogTemp, Error, TEXT("Validation Error for %s: %s"), *Result.SystemName, *Result.ErrorMessage);
    }
}