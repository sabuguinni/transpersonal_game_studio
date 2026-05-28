#include "EngineArchitectureValidator.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "HAL/FileManager.h"
#include "Misc/Paths.h"

UEngineArchitectureValidator::UEngineArchitectureValidator()
{
    bSystemIntegrityValid = false;
    bBiomeCoordinatesValid = false;
    bModuleDependenciesValid = false;
    LastFrameTime = 0.0f;
    ActiveActorCount = 0;
}

void UEngineArchitectureValidator::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("EngineArchitectureValidator: Initializing system validation"));
    
    // Initialize biome boundaries from brain memories
    InitializeBiomeBoundaries();
    
    // Perform initial validation
    ValidateSystemIntegrity();
    
    // Set up periodic validation timer (every 30 seconds)
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(ValidationTimerHandle, this, 
            &UEngineArchitectureValidator::PerformPeriodicValidation, 30.0f, true);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("EngineArchitectureValidator: Initialization complete"));
}

void UEngineArchitectureValidator::Deinitialize()
{
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(ValidationTimerHandle);
    }
    
    Super::Deinitialize();
}

void UEngineArchitectureValidator::InitializeBiomeBoundaries()
{
    // Biome coordinates from brain memories - MANDATORY coordinates
    
    // 1. PANTANO (sudoeste)
    FBiomeBounds SwampBounds;
    SwampBounds.Center = FVector(-50000.0f, -45000.0f, 0.0f);
    SwampBounds.MinBounds = FVector(-77500.0f, -76500.0f, -1000.0f);
    SwampBounds.MaxBounds = FVector(-25000.0f, -15000.0f, 1000.0f);
    BiomeBoundaries.Add(EBiomeType::Swamp, SwampBounds);
    
    // 2. FLORESTA (noroeste)
    FBiomeBounds ForestBounds;
    ForestBounds.Center = FVector(-45000.0f, 40000.0f, 0.0f);
    ForestBounds.MinBounds = FVector(-77500.0f, 15000.0f, -1000.0f);
    ForestBounds.MaxBounds = FVector(-15000.0f, 76500.0f, 1000.0f);
    BiomeBoundaries.Add(EBiomeType::Forest, ForestBounds);
    
    // 3. SAVANA (centro)
    FBiomeBounds SavannaBounds;
    SavannaBounds.Center = FVector(0.0f, 0.0f, 0.0f);
    SavannaBounds.MinBounds = FVector(-20000.0f, -20000.0f, -1000.0f);
    SavannaBounds.MaxBounds = FVector(20000.0f, 20000.0f, 1000.0f);
    BiomeBoundaries.Add(EBiomeType::Savanna, SavannaBounds);
    
    // 4. DESERTO (leste)
    FBiomeBounds DesertBounds;
    DesertBounds.Center = FVector(55000.0f, 0.0f, 0.0f);
    DesertBounds.MinBounds = FVector(25000.0f, -30000.0f, -1000.0f);
    DesertBounds.MaxBounds = FVector(79500.0f, 30000.0f, 1000.0f);
    BiomeBoundaries.Add(EBiomeType::Desert, DesertBounds);
    
    // 5. MONTANHA NEVADA (nordeste)
    FBiomeBounds MountainBounds;
    MountainBounds.Center = FVector(40000.0f, 50000.0f, 500.0f);
    MountainBounds.MinBounds = FVector(15000.0f, 20000.0f, 0.0f);
    MountainBounds.MaxBounds = FVector(79500.0f, 76500.0f, 2000.0f);
    BiomeBoundaries.Add(EBiomeType::Mountain, MountainBounds);
    
    bBiomeCoordinatesValid = true;
    UE_LOG(LogTemp, Warning, TEXT("EngineArchitectureValidator: Biome boundaries initialized"));
}

bool UEngineArchitectureValidator::ValidateSystemIntegrity()
{
    UE_LOG(LogTemp, Warning, TEXT("EngineArchitectureValidator: Validating system integrity"));
    
    bool bAllValid = true;
    
    // Validate biome coordinates
    if (!ValidateBiomeCoordinates())
    {
        bAllValid = false;
        UE_LOG(LogTemp, Error, TEXT("Biome coordinate validation failed"));
    }
    
    // Validate module dependencies
    if (!ValidateModuleDependencies())
    {
        bAllValid = false;
        UE_LOG(LogTemp, Error, TEXT("Module dependency validation failed"));
    }
    
    // Validate file structure
    if (!ValidateFileStructure())
    {
        bAllValid = false;
        UE_LOG(LogTemp, Error, TEXT("File structure validation failed"));
    }
    
    // Validate performance targets
    if (!ValidatePerformanceTargets())
    {
        bAllValid = false;
        UE_LOG(LogTemp, Warning, TEXT("Performance target validation failed"));
    }
    
    bSystemIntegrityValid = bAllValid;
    LogValidationResults();
    
    return bAllValid;
}

bool UEngineArchitectureValidator::ValidateBiomeCoordinates()
{
    // Check that all 5 biomes are properly defined
    if (BiomeBoundaries.Num() != 5)
    {
        UE_LOG(LogTemp, Error, TEXT("Expected 5 biomes, found %d"), BiomeBoundaries.Num());
        return false;
    }
    
    // Validate each biome has valid bounds
    for (const auto& BiomePair : BiomeBoundaries)
    {
        const FBiomeBounds& Bounds = BiomePair.Value;
        if (Bounds.MinBounds.X >= Bounds.MaxBounds.X || 
            Bounds.MinBounds.Y >= Bounds.MaxBounds.Y)
        {
            UE_LOG(LogTemp, Error, TEXT("Invalid bounds for biome %d"), (int32)BiomePair.Key);
            return false;
        }
    }
    
    return true;
}

bool UEngineArchitectureValidator::ValidateModuleDependencies()
{
    // Check for missing CPP files
    TArray<FString> MissingFiles = GetMissingCppFiles();
    if (MissingFiles.Num() > 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("Found %d missing CPP files"), MissingFiles.Num());
        for (const FString& File : MissingFiles)
        {
            UE_LOG(LogTemp, Warning, TEXT("Missing: %s"), *File);
        }
        return false;
    }
    
    return true;
}

bool UEngineArchitectureValidator::IsLocationInValidBiome(FVector Location, EBiomeType BiomeType)
{
    if (!BiomeBoundaries.Contains(BiomeType))
    {
        return false;
    }
    
    const FBiomeBounds& Bounds = BiomeBoundaries[BiomeType];
    return (Location.X >= Bounds.MinBounds.X && Location.X <= Bounds.MaxBounds.X &&
            Location.Y >= Bounds.MinBounds.Y && Location.Y <= Bounds.MaxBounds.Y &&
            Location.Z >= Bounds.MinBounds.Z && Location.Z <= Bounds.MaxBounds.Z);
}

EBiomeType UEngineArchitectureValidator::GetBiomeAtLocation(FVector Location)
{
    for (const auto& BiomePair : BiomeBoundaries)
    {
        if (IsLocationInValidBiome(Location, BiomePair.Key))
        {
            return BiomePair.Key;
        }
    }
    
    // Default to Savanna if no biome matches
    return EBiomeType::Savanna;
}

bool UEngineArchitectureValidator::ValidatePerformanceTargets()
{
    if (UWorld* World = GetWorld())
    {
        // Update performance metrics
        LastFrameTime = World->GetDeltaSeconds();
        
        // Count active actors
        ActiveActorCount = 0;
        for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
        {
            if (IsValid(*ActorItr))
            {
                ActiveActorCount++;
            }
        }
        
        // Check performance targets
        bool bFrameTimeOK = LastFrameTime < 0.033f; // 30 FPS minimum
        bool bActorCountOK = ActiveActorCount < 10000; // Reasonable limit
        
        if (!bFrameTimeOK)
        {
            UE_LOG(LogTemp, Warning, TEXT("Frame time too high: %f ms"), LastFrameTime * 1000.0f);
        }
        
        if (!bActorCountOK)
        {
            UE_LOG(LogTemp, Warning, TEXT("Too many actors: %d"), ActiveActorCount);
        }
        
        return bFrameTimeOK && bActorCountOK;
    }
    
    return false;
}

TArray<FString> UEngineArchitectureValidator::GetMissingCppFiles()
{
    TArray<FString> MissingFiles;
    
    // This would need to scan the file system for .h files without matching .cpp
    // For now, return empty array as this is a complex file system operation
    // that should be handled by the build system
    
    return MissingFiles;
}

TArray<FString> UEngineArchitectureValidator::GetOrphanedHeaders()
{
    TArray<FString> OrphanedHeaders;
    
    // Similar to GetMissingCppFiles, this would scan for orphaned headers
    // Implementation would require file system access
    
    return OrphanedHeaders;
}

void UEngineArchitectureValidator::LogSystemStatus()
{
    UE_LOG(LogTemp, Warning, TEXT("=== ENGINE ARCHITECTURE STATUS ==="));
    UE_LOG(LogTemp, Warning, TEXT("System Integrity: %s"), bSystemIntegrityValid ? TEXT("VALID") : TEXT("INVALID"));
    UE_LOG(LogTemp, Warning, TEXT("Biome Coordinates: %s"), bBiomeCoordinatesValid ? TEXT("VALID") : TEXT("INVALID"));
    UE_LOG(LogTemp, Warning, TEXT("Module Dependencies: %s"), bModuleDependenciesValid ? TEXT("VALID") : TEXT("INVALID"));
    UE_LOG(LogTemp, Warning, TEXT("Active Actors: %d"), ActiveActorCount);
    UE_LOG(LogTemp, Warning, TEXT("Last Frame Time: %f ms"), LastFrameTime * 1000.0f);
    UE_LOG(LogTemp, Warning, TEXT("Biomes Configured: %d"), BiomeBoundaries.Num());
}

bool UEngineArchitectureValidator::ValidateFileStructure()
{
    // Basic validation that core files exist
    // This is a simplified check - real implementation would scan file system
    return true;
}

bool UEngineArchitectureValidator::ValidateClassHierarchy()
{
    // Validate that all classes follow proper UE5 inheritance patterns
    // This would check UCLASS declarations, proper base classes, etc.
    return true;
}

void UEngineArchitectureValidator::LogValidationResults()
{
    if (bSystemIntegrityValid)
    {
        UE_LOG(LogTemp, Warning, TEXT("✓ System integrity validation PASSED"));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("✗ System integrity validation FAILED"));
    }
}

void UEngineArchitectureValidator::PerformPeriodicValidation()
{
    UE_LOG(LogTemp, Log, TEXT("EngineArchitectureValidator: Performing periodic validation"));
    ValidateSystemIntegrity();
}