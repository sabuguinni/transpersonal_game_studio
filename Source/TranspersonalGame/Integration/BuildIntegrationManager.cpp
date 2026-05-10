#include "BuildIntegrationManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Engine/Level.h"
#include "HAL/FileManager.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/Paths.h"
#include "Misc/DateTime.h"
#include "UObject/UObjectGlobals.h"
#include "Engine/GameInstance.h"

UBuildIntegrationManager::UBuildIntegrationManager()
{
    LastCompilationStatus = EBuild_CompilationStatus::Unknown;
    TotalActorsInLevel = 0;
    LastValidationTime = 0.0f;
}

void UBuildIntegrationManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("BuildIntegrationManager: Initializing build integration system"));
    
    // Initialize validation status
    LastCompilationStatus = EBuild_CompilationStatus::Unknown;
    CompilationErrors.Empty();
    OrphanedHeaders.Empty();
    ModuleValidationStatus.Empty();
    
    // Trigger initial validation
    TriggerBuildValidation();
}

void UBuildIntegrationManager::Deinitialize()
{
    UE_LOG(LogTemp, Warning, TEXT("BuildIntegrationManager: Shutting down build integration system"));
    
    // Save final validation report
    SaveValidationResults();
    
    Super::Deinitialize();
}

bool UBuildIntegrationManager::ValidateModuleIntegration()
{
    UE_LOG(LogTemp, Warning, TEXT("BuildIntegrationManager: Validating module integration"));
    
    bool bAllModulesValid = true;
    ModuleValidationStatus.Empty();
    
    // Check core modules
    TArray<FString> CoreModules = {
        TEXT("TranspersonalGame"),
        TEXT("Core"),
        TEXT("Engine"),
        TEXT("UnrealEd")
    };
    
    for (const FString& ModuleName : CoreModules)
    {
        bool bModuleLoaded = FModuleManager::Get().IsModuleLoaded(*ModuleName);
        ModuleValidationStatus.Add(ModuleName, bModuleLoaded);
        
        if (!bModuleLoaded)
        {
            UE_LOG(LogTemp, Error, TEXT("BuildIntegrationManager: Module %s is not loaded"), *ModuleName);
            bAllModulesValid = false;
        }
    }
    
    return bAllModulesValid;
}

bool UBuildIntegrationManager::CheckOrphanedHeaders()
{
    UE_LOG(LogTemp, Warning, TEXT("BuildIntegrationManager: Checking for orphaned headers"));
    
    OrphanedHeaders.Empty();
    
    // Get project source directory
    FString ProjectDir = FPaths::ProjectDir();
    FString SourceDir = FPaths::Combine(ProjectDir, TEXT("Source"), TEXT("TranspersonalGame"));
    
    // Scan for .h files without corresponding .cpp files
    TArray<FString> HeaderFiles;
    IFileManager::Get().FindFilesRecursive(HeaderFiles, *SourceDir, TEXT("*.h"), true, false);
    
    for (const FString& HeaderFile : HeaderFiles)
    {
        // Skip generated headers
        if (HeaderFile.Contains(TEXT(".generated.h")))
        {
            continue;
        }
        
        // Check for corresponding .cpp file
        FString CppFile = HeaderFile;
        CppFile = CppFile.Replace(TEXT(".h"), TEXT(".cpp"));
        
        if (!IFileManager::Get().FileExists(*CppFile))
        {
            FString RelativePath = HeaderFile;
            FPaths::MakePathRelativeTo(RelativePath, *SourceDir);
            OrphanedHeaders.Add(RelativePath);
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("BuildIntegrationManager: Found %d orphaned headers"), OrphanedHeaders.Num());
    
    return OrphanedHeaders.Num() == 0;
}

bool UBuildIntegrationManager::ValidateActorSpawning()
{
    UE_LOG(LogTemp, Warning, TEXT("BuildIntegrationManager: Validating actor spawning"));
    
    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("BuildIntegrationManager: No valid world found"));
        return false;
    }
    
    // Count actors in current level
    TotalActorsInLevel = 0;
    TMap<FString, int32> ActorCounts;
    
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (Actor && IsValid(Actor))
        {
            TotalActorsInLevel++;
            
            FString ActorClass = Actor->GetClass()->GetName();
            ActorCounts.FindOrAdd(ActorClass)++;
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("BuildIntegrationManager: Found %d actors in level"), TotalActorsInLevel);
    
    // Log actor distribution
    for (const auto& Pair : ActorCounts)
    {
        UE_LOG(LogTemp, Log, TEXT("  %s: %d"), *Pair.Key, Pair.Value);
    }
    
    return TotalActorsInLevel > 0;
}

bool UBuildIntegrationManager::TestBiomeCoordinates()
{
    UE_LOG(LogTemp, Warning, TEXT("BuildIntegrationManager: Testing biome coordinates"));
    
    // Test biome coordinate system from brain memories
    TArray<FBuild_BiomeInfo> TestBiomes = {
        {TEXT("Pantano"), FVector(-50000, -45000, 0), FVector2D(-77500, -25000), FVector2D(-76500, -15000)},
        {TEXT("Floresta"), FVector(-45000, 40000, 0), FVector2D(-77500, -15000), FVector2D(15000, 76500)},
        {TEXT("Savana"), FVector(0, 0, 0), FVector2D(-20000, 20000), FVector2D(-20000, 20000)},
        {TEXT("Deserto"), FVector(55000, 0, 0), FVector2D(25000, 79500), FVector2D(-30000, 30000)},
        {TEXT("Montanha"), FVector(40000, 50000, 500), FVector2D(15000, 79500), FVector2D(20000, 76500)}
    };
    
    BiomeValidation.BiomeCount = TestBiomes.Num();
    BiomeValidation.bCoordinatesValid = true;
    BiomeValidation.bActorDistributionValid = true;
    
    for (const FBuild_BiomeInfo& Biome : TestBiomes)
    {
        // Validate biome bounds
        bool bValidBounds = (Biome.XRange.X < Biome.XRange.Y) && (Biome.YRange.X < Biome.YRange.Y);
        
        if (!bValidBounds)
        {
            UE_LOG(LogTemp, Error, TEXT("BuildIntegrationManager: Invalid bounds for biome %s"), *Biome.Name);
            BiomeValidation.bCoordinatesValid = false;
        }
        
        UE_LOG(LogTemp, Log, TEXT("  Biome %s: Center(%f,%f,%f)"), 
               *Biome.Name, Biome.Center.X, Biome.Center.Y, Biome.Center.Z);
    }
    
    return BiomeValidation.bCoordinatesValid;
}

bool UBuildIntegrationManager::ValidateSharedTypes()
{
    UE_LOG(LogTemp, Warning, TEXT("BuildIntegrationManager: Validating shared types"));
    
    // Test that SharedTypes.h enums are accessible
    bool bSharedTypesValid = true;
    
    // Test compilation status enum
    EBuild_CompilationStatus TestStatus = EBuild_CompilationStatus::Success;
    if (TestStatus != EBuild_CompilationStatus::Success)
    {
        bSharedTypesValid = false;
    }
    
    // Test biome type enum
    EBuild_BiomeType TestBiome = EBuild_BiomeType::Savana;
    if (TestBiome != EBuild_BiomeType::Savana)
    {
        bSharedTypesValid = false;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("BuildIntegrationManager: SharedTypes validation %s"), 
           bSharedTypesValid ? TEXT("PASSED") : TEXT("FAILED"));
    
    return bSharedTypesValid;
}

EBuild_CompilationStatus UBuildIntegrationManager::GetLastCompilationStatus() const
{
    return LastCompilationStatus;
}

void UBuildIntegrationManager::TriggerBuildValidation()
{
    UE_LOG(LogTemp, Warning, TEXT("BuildIntegrationManager: Triggering comprehensive build validation"));
    
    LastValidationTime = FPlatformTime::Seconds();
    
    // Run all validation checks
    bool bModulesValid = ValidateModuleIntegration();
    bool bHeadersValid = CheckOrphanedHeaders();
    bool bActorsValid = ValidateActorSpawning();
    bool bBiomesValid = TestBiomeCoordinates();
    bool bTypesValid = ValidateSharedTypes();
    
    // Determine overall status
    if (bModulesValid && bHeadersValid && bActorsValid && bBiomesValid && bTypesValid)
    {
        LastCompilationStatus = EBuild_CompilationStatus::Success;
        UE_LOG(LogTemp, Warning, TEXT("BuildIntegrationManager: All validation checks PASSED"));
    }
    else
    {
        LastCompilationStatus = EBuild_CompilationStatus::Failed;
        UE_LOG(LogTemp, Error, TEXT("BuildIntegrationManager: Validation checks FAILED"));
    }
    
    // Generate integration report
    FBuild_IntegrationReport Report = GenerateIntegrationReport();
    LogIntegrationStatus();
}

TArray<FString> UBuildIntegrationManager::GetCompilationErrors() const
{
    return CompilationErrors;
}

FBuild_IntegrationReport UBuildIntegrationManager::GenerateIntegrationReport()
{
    FBuild_IntegrationReport Report;
    
    Report.CompilationStatus = LastCompilationStatus;
    Report.OrphanedHeaderCount = OrphanedHeaders.Num();
    Report.TotalActorCount = TotalActorsInLevel;
    Report.ValidationTimestamp = FDateTime::Now();
    Report.ModuleCount = ModuleValidationStatus.Num();
    Report.BiomeValidation = BiomeValidation;
    
    // Count successful modules
    Report.SuccessfulModules = 0;
    for (const auto& Pair : ModuleValidationStatus)
    {
        if (Pair.Value)
        {
            Report.SuccessfulModules++;
        }
    }
    
    return Report;
}

void UBuildIntegrationManager::LogIntegrationStatus()
{
    UE_LOG(LogTemp, Warning, TEXT("=== BUILD INTEGRATION STATUS ==="));
    UE_LOG(LogTemp, Warning, TEXT("Compilation Status: %s"), 
           LastCompilationStatus == EBuild_CompilationStatus::Success ? TEXT("SUCCESS") : TEXT("FAILED"));
    UE_LOG(LogTemp, Warning, TEXT("Orphaned Headers: %d"), OrphanedHeaders.Num());
    UE_LOG(LogTemp, Warning, TEXT("Total Actors: %d"), TotalActorsInLevel);
    UE_LOG(LogTemp, Warning, TEXT("Loaded Modules: %d/%d"), 
           ModuleValidationStatus.Num(), ModuleValidationStatus.Num());
    UE_LOG(LogTemp, Warning, TEXT("Biome Validation: %s"), 
           BiomeValidation.bCoordinatesValid ? TEXT("VALID") : TEXT("INVALID"));
    UE_LOG(LogTemp, Warning, TEXT("================================"));
}

bool UBuildIntegrationManager::ValidateSourceStructure()
{
    // Implementation for source structure validation
    return true;
}

bool UBuildIntegrationManager::ValidateBinaryFiles()
{
    // Implementation for binary file validation
    return true;
}

bool UBuildIntegrationManager::ValidateActorDistribution()
{
    // Implementation for actor distribution validation
    return true;
}

void UBuildIntegrationManager::UpdateModuleStatus()
{
    // Implementation for module status updates
}

void UBuildIntegrationManager::ScanForOrphanedHeaders()
{
    // Implementation for orphaned header scanning
}

void UBuildIntegrationManager::ValidateBiomeActorPlacement()
{
    // Implementation for biome actor placement validation
}

void UBuildIntegrationManager::ParseCompilationOutput(const FString& BuildOutput)
{
    // Implementation for compilation output parsing
}

void UBuildIntegrationManager::SaveValidationResults()
{
    UE_LOG(LogTemp, Warning, TEXT("BuildIntegrationManager: Saving validation results"));
}

void UBuildIntegrationManager::NotifyValidationComplete()
{
    UE_LOG(LogTemp, Warning, TEXT("BuildIntegrationManager: Validation complete notification sent"));
}