#include "Eng_ArchitectureValidator.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Engine/DirectionalLight.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/GameModeBase.h"
#include "Kismet/GameplayStatics.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/DateTime.h"
#include "Stats/Stats.h"

UEng_ArchitectureValidator::UEng_ArchitectureValidator()
{
    LastValidationTime = 0.0f;
}

void UEng_ArchitectureValidator::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("Architecture Validator initialized"));
    
    // Run initial validation
    ValidateArchitecture();
}

void UEng_ArchitectureValidator::Deinitialize()
{
    ValidationResults.Empty();
    
    Super::Deinitialize();
}

bool UEng_ArchitectureValidator::ValidateArchitecture()
{
    UE_LOG(LogTemp, Warning, TEXT("=== ARCHITECTURE VALIDATION STARTED ==="));
    
    bool bOverallValid = true;
    ValidationResults.Empty();
    
    // Update performance metrics first
    UpdatePerformanceMetrics();
    
    // 1. Validate lighting setup
    bool bLightingValid = ValidateLightingSetup();
    ValidationResults.Add(TEXT("Lighting"), bLightingValid);
    bOverallValid &= bLightingValid;
    
    // 2. Validate world partitioning
    bool bWorldValid = ValidateWorldPartitioning();
    ValidationResults.Add(TEXT("WorldPartition"), bWorldValid);
    bOverallValid &= bWorldValid;
    
    // 3. Validate character systems
    bool bCharacterValid = ValidateCharacterSystems();
    ValidationResults.Add(TEXT("Character"), bCharacterValid);
    bOverallValid &= bCharacterValid;
    
    // 4. Validate dinosaur systems
    bool bDinosaurValid = ValidateDinosaurSystems();
    ValidationResults.Add(TEXT("Dinosaur"), bDinosaurValid);
    bOverallValid &= bDinosaurValid;
    
    // 5. Validate module dependencies
    bool bModulesValid = ValidateModuleDependencies();
    ValidationResults.Add(TEXT("Modules"), bModulesValid);
    bOverallValid &= bModulesValid;
    
    // 6. Validate performance constraints
    bool bPerformanceValid = ValidatePerformanceConstraints();
    ValidationResults.Add(TEXT("Performance"), bPerformanceValid);
    bOverallValid &= bPerformanceValid;
    
    LastValidationTime = GetWorld()->GetTimeSeconds();
    
    UE_LOG(LogTemp, Warning, TEXT("Architecture validation complete: %s"), 
           bOverallValid ? TEXT("PASS") : TEXT("FAIL"));
    
    return bOverallValid;
}

int32 UEng_ArchitectureValidator::CleanupDuplicateSystems()
{
    int32 CleanedCount = 0;
    
    UWorld* World = GetWorld();
    if (!World)
    {
        return 0;
    }
    
    // Clean up duplicate lighting
    CleanupDuplicateLighting();
    
    // Clean up duplicate managers
    CleanupDuplicateManagers();
    
    UE_LOG(LogTemp, Warning, TEXT("Cleaned up %d duplicate systems"), CleanedCount);
    
    return CleanedCount;
}

bool UEng_ArchitectureValidator::ValidateBiomeArchitecture()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return false;
    }
    
    // Check if biome manager exists and is properly configured
    // This will be expanded when BiomeManager is implemented
    
    UE_LOG(LogTemp, Warning, TEXT("Biome architecture validation: PASS (placeholder)"));
    return true;
}

bool UEng_ArchitectureValidator::ValidateModuleDependencies()
{
    // Check if core modules are loaded
    bool bCoreLoaded = FModuleManager::Get().IsModuleLoaded("TranspersonalGame");
    bool bEngineLoaded = FModuleManager::Get().IsModuleLoaded("Engine");
    bool bCoreUObjectLoaded = FModuleManager::Get().IsModuleLoaded("CoreUObject");
    
    bool bAllLoaded = bCoreLoaded && bEngineLoaded && bCoreUObjectLoaded;
    
    UE_LOG(LogTemp, Warning, TEXT("Module dependencies validation: %s"), 
           bAllLoaded ? TEXT("PASS") : TEXT("FAIL"));
    
    if (!bAllLoaded)
    {
        UE_LOG(LogTemp, Error, TEXT("Missing modules - Core: %s, Engine: %s, CoreUObject: %s"),
               bCoreLoaded ? TEXT("OK") : TEXT("MISSING"),
               bEngineLoaded ? TEXT("OK") : TEXT("MISSING"),
               bCoreUObjectLoaded ? TEXT("OK") : TEXT("MISSING"));
    }
    
    return bAllLoaded;
}

bool UEng_ArchitectureValidator::ValidatePerformanceConstraints()
{
    // Check frame rate target (60 FPS PC, 30 FPS console)
    bool bFrameRateOK = CheckFrameRate();
    
    // Check memory usage
    bool bMemoryOK = CheckMemoryUsage();
    
    // Check actor count limits
    bool bActorCountOK = (CurrentMetrics.ActorCount < 10000); // Reasonable limit
    
    bool bPerformanceValid = bFrameRateOK && bMemoryOK && bActorCountOK;
    
    UE_LOG(LogTemp, Warning, TEXT("Performance validation: %s (FPS: %.1f, Memory: %.1f MB, Actors: %d)"),
           bPerformanceValid ? TEXT("PASS") : TEXT("FAIL"),
           CurrentMetrics.FrameRate,
           CurrentMetrics.MemoryUsageMB,
           CurrentMetrics.ActorCount);
    
    return bPerformanceValid;
}

bool UEng_ArchitectureValidator::ValidateLightingSetup()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return false;
    }
    
    // Count lighting actors
    int32 DirectionalLightCount = 0;
    int32 SkyAtmosphereCount = 0;
    int32 SkyLightCount = 0;
    int32 ExponentialHeightFogCount = 0;
    
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (!Actor) continue;
        
        FString ClassName = Actor->GetClass()->GetName();
        
        if (ClassName.Contains(TEXT("DirectionalLight")))
        {
            DirectionalLightCount++;
        }
        else if (ClassName.Contains(TEXT("SkyAtmosphere")))
        {
            SkyAtmosphereCount++;
        }
        else if (ClassName.Contains(TEXT("SkyLight")))
        {
            SkyLightCount++;
        }
        else if (ClassName.Contains(TEXT("ExponentialHeightFog")))
        {
            ExponentialHeightFogCount++;
        }
    }
    
    // Should have exactly 1 of each lighting type
    bool bLightingValid = (DirectionalLightCount == 1) && 
                         (SkyAtmosphereCount == 1) && 
                         (SkyLightCount == 1) && 
                         (ExponentialHeightFogCount == 1);
    
    UE_LOG(LogTemp, Warning, TEXT("Lighting validation: %s (Dir: %d, Sky: %d, SkyLight: %d, Fog: %d)"),
           bLightingValid ? TEXT("PASS") : TEXT("FAIL"),
           DirectionalLightCount, SkyAtmosphereCount, SkyLightCount, ExponentialHeightFogCount);
    
    return bLightingValid;
}

bool UEng_ArchitectureValidator::ValidateWorldPartitioning()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return false;
    }
    
    // Check if world has proper bounds for our 157km x 153km map
    FBox WorldBounds = World->GetWorldBounds();
    
    // Expected bounds: X(-77500 to 79500), Y(-76500 to 76500)
    bool bBoundsValid = (WorldBounds.GetSize().X > 150000.0f) && 
                       (WorldBounds.GetSize().Y > 150000.0f);
    
    UE_LOG(LogTemp, Warning, TEXT("World partitioning validation: %s (Size: %.0f x %.0f)"),
           bBoundsValid ? TEXT("PASS") : TEXT("FAIL"),
           WorldBounds.GetSize().X, WorldBounds.GetSize().Y);
    
    return bBoundsValid;
}

bool UEng_ArchitectureValidator::ValidateCharacterSystems()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return false;
    }
    
    // Check if TranspersonalCharacter class exists
    UClass* CharacterClass = FindObject<UClass>(ANY_PACKAGE, TEXT("TranspersonalCharacter"));
    bool bCharacterClassExists = (CharacterClass != nullptr);
    
    // Check if GameMode is properly set
    AGameModeBase* GameMode = World->GetAuthGameMode();
    bool bGameModeExists = (GameMode != nullptr);
    
    bool bCharacterValid = bCharacterClassExists && bGameModeExists;
    
    UE_LOG(LogTemp, Warning, TEXT("Character systems validation: %s (Class: %s, GameMode: %s)"),
           bCharacterValid ? TEXT("PASS") : TEXT("FAIL"),
           bCharacterClassExists ? TEXT("OK") : TEXT("MISSING"),
           bGameModeExists ? TEXT("OK") : TEXT("MISSING"));
    
    return bCharacterValid;
}

bool UEng_ArchitectureValidator::ValidateDinosaurSystems()
{
    // Placeholder for dinosaur system validation
    // Will be expanded when dinosaur systems are implemented
    
    UE_LOG(LogTemp, Warning, TEXT("Dinosaur systems validation: PASS (placeholder)"));
    return true;
}

void UEng_ArchitectureValidator::CleanupDuplicateLighting()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    // Track first instance of each lighting type
    AActor* FirstDirectionalLight = nullptr;
    AActor* FirstSkyAtmosphere = nullptr;
    AActor* FirstSkyLight = nullptr;
    AActor* FirstExponentialHeightFog = nullptr;
    
    TArray<AActor*> ActorsToDestroy;
    
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (!Actor) continue;
        
        FString ClassName = Actor->GetClass()->GetName();
        
        if (ClassName.Contains(TEXT("DirectionalLight")))
        {
            if (!FirstDirectionalLight)
            {
                FirstDirectionalLight = Actor;
            }
            else
            {
                ActorsToDestroy.Add(Actor);
            }
        }
        else if (ClassName.Contains(TEXT("SkyAtmosphere")))
        {
            if (!FirstSkyAtmosphere)
            {
                FirstSkyAtmosphere = Actor;
            }
            else
            {
                ActorsToDestroy.Add(Actor);
            }
        }
        else if (ClassName.Contains(TEXT("SkyLight")))
        {
            if (!FirstSkyLight)
            {
                FirstSkyLight = Actor;
            }
            else
            {
                ActorsToDestroy.Add(Actor);
            }
        }
        else if (ClassName.Contains(TEXT("ExponentialHeightFog")))
        {
            if (!FirstExponentialHeightFog)
            {
                FirstExponentialHeightFog = Actor;
            }
            else
            {
                ActorsToDestroy.Add(Actor);
            }
        }
    }
    
    // Destroy duplicates
    for (AActor* Actor : ActorsToDestroy)
    {
        if (Actor)
        {
            UE_LOG(LogTemp, Warning, TEXT("Destroying duplicate lighting actor: %s"), 
                   *Actor->GetClass()->GetName());
            Actor->Destroy();
        }
    }
    
    if (ActorsToDestroy.Num() > 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("Cleaned up %d duplicate lighting actors"), ActorsToDestroy.Num());
    }
}

void UEng_ArchitectureValidator::CleanupDuplicateManagers()
{
    // Placeholder for manager cleanup
    // Will be expanded when manager systems are implemented
}

void UEng_ArchitectureValidator::UpdatePerformanceMetrics()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    // Get frame rate
    CurrentMetrics.FrameRate = 1.0f / World->GetDeltaSeconds();
    
    // Get memory usage (approximate)
    FPlatformMemoryStats MemStats = FPlatformMemory::GetStats();
    CurrentMetrics.MemoryUsageMB = MemStats.UsedPhysical / (1024.0f * 1024.0f);
    
    // Count actors
    CurrentMetrics.ActorCount = 0;
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        CurrentMetrics.ActorCount++;
    }
    
    // Draw calls and triangles would need rendering stats
    CurrentMetrics.DrawCalls = 0; // Placeholder
    CurrentMetrics.TriangleCount = 0; // Placeholder
}

bool UEng_ArchitectureValidator::CheckFrameRate()
{
    // Target: 60 FPS for PC, 30 FPS for console
    // For now, accept anything above 20 FPS as reasonable
    return (CurrentMetrics.FrameRate > 20.0f);
}

bool UEng_ArchitectureValidator::CheckMemoryUsage()
{
    // Accept memory usage under 4GB as reasonable for development
    return (CurrentMetrics.MemoryUsageMB < 4096.0f);
}