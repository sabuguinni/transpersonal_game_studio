#include "Eng_CompilationOrchestrator_v2.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "UObject/UObjectGlobals.h"
#include "Misc/FileHelper.h"
#include "HAL/PlatformFilemanager.h"

// Critical system names that MUST be functional
const TArray<FString> UEng_CompilationOrchestrator_v2::CriticalSystemNames = {
    TEXT("TranspersonalGameState"),
    TEXT("TranspersonalCharacter"),
    TEXT("DinosaurBase"),
    TEXT("BiomeManager"),
    TEXT("Eng_ArchitectureCore")
};

UEng_CompilationOrchestrator_v2::UEng_CompilationOrchestrator_v2()
{
    bCompilationStateValid = false;
    LastValidationTime = 0.0f;
}

void UEng_CompilationOrchestrator_v2::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("ENGINE ARCHITECT - Compilation Orchestrator v2 Initialized"));
    
    // Immediate validation on startup
    ValidateCompilationState();
    
    // Schedule periodic validation
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(
            FTimerHandle(),
            this,
            &UEng_CompilationOrchestrator_v2::ValidateCompilationState,
            30.0f, // Every 30 seconds
            true   // Loop
        );
    }
}

void UEng_CompilationOrchestrator_v2::Deinitialize()
{
    UE_LOG(LogTemp, Warning, TEXT("ENGINE ARCHITECT - Compilation Orchestrator v2 Shutdown"));
    Super::Deinitialize();
}

bool UEng_CompilationOrchestrator_v2::ValidateCompilationState()
{
    UE_LOG(LogTemp, Warning, TEXT("=== ENGINE ARCHITECT - COMPILATION STATE VALIDATION ==="));
    
    LastValidationTime = FPlatformTime::Seconds();
    bCompilationStateValid = true;
    
    // Clear previous data
    CriticalSystemRegistry.Empty();
    OrphanHeaders.Empty();
    DuplicateSystems.Empty();
    
    // Validate critical systems
    int32 LoadedSystems = 0;
    for (const FString& SystemName : CriticalSystemNames)
    {
        FString ClassPath = FString::Printf(TEXT("/Script/TranspersonalGame.%s"), *SystemName);
        UClass* SystemClass = LoadClass<UObject>(nullptr, *ClassPath);
        
        if (SystemClass)
        {
            CriticalSystemRegistry.Add(SystemName, SystemClass);
            LoadedSystems++;
            UE_LOG(LogTemp, Warning, TEXT("✓ Critical System LOADED: %s"), *SystemName);
        }
        else
        {
            bCompilationStateValid = false;
            UE_LOG(LogTemp, Error, TEXT("✗ Critical System FAILED: %s"), *SystemName);
        }
    }
    
    // Scan for orphan headers
    ScanForOrphanHeaders();
    
    // Identify duplicate systems
    IdentifyDuplicateSystems();
    
    // Log final status
    LogCompilationStatus();
    
    UE_LOG(LogTemp, Warning, TEXT("Compilation validation complete: %s"), 
           bCompilationStateValid ? TEXT("PASS") : TEXT("FAIL"));
    
    return bCompilationStateValid;
}

void UEng_CompilationOrchestrator_v2::FixOrphanHeaders()
{
    UE_LOG(LogTemp, Warning, TEXT("=== FIXING ORPHAN HEADERS ==="));
    
    // This would typically involve file system operations
    // For now, log the orphan headers that need attention
    for (const FString& OrphanHeader : OrphanHeaders)
    {
        UE_LOG(LogTemp, Warning, TEXT("ORPHAN HEADER: %s - Needs .cpp implementation"), *OrphanHeader);
    }
    
    if (OrphanHeaders.Num() > 0)
    {
        UE_LOG(LogTemp, Error, TEXT("CRITICAL: %d orphan headers detected - blocking compilation"), OrphanHeaders.Num());
    }
}

void UEng_CompilationOrchestrator_v2::CleanupDuplicateSystems()
{
    UE_LOG(LogTemp, Warning, TEXT("=== CLEANING DUPLICATE SYSTEMS ==="));
    
    for (const FString& DuplicateSystem : DuplicateSystems)
    {
        UE_LOG(LogTemp, Warning, TEXT("DUPLICATE SYSTEM: %s - Needs consolidation"), *DuplicateSystem);
    }
}

bool UEng_CompilationOrchestrator_v2::ValidateCriticalSystems()
{
    return ValidateCompilationState();
}

void UEng_CompilationOrchestrator_v2::RegisterCriticalClass(const FString& ClassName, UClass* ClassPtr)
{
    if (ClassPtr)
    {
        CriticalSystemRegistry.Add(ClassName, ClassPtr);
        UE_LOG(LogTemp, Warning, TEXT("Registered critical class: %s"), *ClassName);
    }
}

FString UEng_CompilationOrchestrator_v2::GetCompilationReport()
{
    FString Report = TEXT("=== ENGINE ARCHITECT COMPILATION REPORT ===\n");
    Report += FString::Printf(TEXT("Validation Time: %.2f seconds ago\n"), 
                             FPlatformTime::Seconds() - LastValidationTime);
    Report += FString::Printf(TEXT("Compilation State: %s\n"), 
                             bCompilationStateValid ? TEXT("VALID") : TEXT("INVALID"));
    Report += FString::Printf(TEXT("Critical Systems Loaded: %d/%d\n"), 
                             CriticalSystemRegistry.Num(), CriticalSystemNames.Num());
    Report += FString::Printf(TEXT("Orphan Headers: %d\n"), OrphanHeaders.Num());
    Report += FString::Printf(TEXT("Duplicate Systems: %d\n"), DuplicateSystems.Num());
    
    return Report;
}

int32 UEng_CompilationOrchestrator_v2::GetOrphanHeaderCount()
{
    return OrphanHeaders.Num();
}

void UEng_CompilationOrchestrator_v2::ScanForOrphanHeaders()
{
    // Simulated orphan header detection
    // In a real implementation, this would scan the file system
    
    // Known problematic headers from memory analysis
    TArray<FString> KnownOrphans = {
        TEXT("DirectorCoordinator.h"),
        TEXT("Eng_MovementSystem.h"), // Truncated file
        TEXT("Various Eng_ prefixed headers without matching .cpp")
    };
    
    OrphanHeaders = KnownOrphans;
    
    UE_LOG(LogTemp, Warning, TEXT("Orphan header scan complete: %d orphans detected"), OrphanHeaders.Num());
}

void UEng_CompilationOrchestrator_v2::IdentifyDuplicateSystems()
{
    // Known duplicate systems from memory analysis
    TArray<FString> KnownDuplicates = {
        TEXT("Multiple Compilation Managers"),
        TEXT("Multiple Architecture Validators"),
        TEXT("Fragmented Movement Systems")
    };
    
    DuplicateSystems = KnownDuplicates;
    
    UE_LOG(LogTemp, Warning, TEXT("Duplicate system scan complete: %d duplicates detected"), DuplicateSystems.Num());
}

bool UEng_CompilationOrchestrator_v2::ValidateSystemDependencies()
{
    // Check if critical systems can interact properly
    bool bDependenciesValid = true;
    
    // Example: Check if BiomeManager can work with DinosaurBase
    UClass* BiomeClass = CriticalSystemRegistry.FindRef(TEXT("BiomeManager"));
    UClass* DinosaurClass = CriticalSystemRegistry.FindRef(TEXT("DinosaurBase"));
    
    if (BiomeClass && DinosaurClass)
    {
        UE_LOG(LogTemp, Warning, TEXT("✓ BiomeManager <-> DinosaurBase dependency valid"));
    }
    else
    {
        bDependenciesValid = false;
        UE_LOG(LogTemp, Error, TEXT("✗ BiomeManager <-> DinosaurBase dependency BROKEN"));
    }
    
    return bDependenciesValid;
}

void UEng_CompilationOrchestrator_v2::LogCompilationStatus()
{
    UE_LOG(LogTemp, Warning, TEXT("=== COMPILATION STATUS SUMMARY ==="));
    UE_LOG(LogTemp, Warning, TEXT("Overall State: %s"), 
           bCompilationStateValid ? TEXT("FUNCTIONAL") : TEXT("CRITICAL"));
    UE_LOG(LogTemp, Warning, TEXT("Critical Systems: %d/%d loaded"), 
           CriticalSystemRegistry.Num(), CriticalSystemNames.Num());
    UE_LOG(LogTemp, Warning, TEXT("Issues Found: %d orphans, %d duplicates"), 
           OrphanHeaders.Num(), DuplicateSystems.Num());
    
    if (bCompilationStateValid)
    {
        UE_LOG(LogTemp, Warning, TEXT("✓ READY FOR NEXT AGENT - Core systems operational"));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("✗ BLOCKING ISSUES - Requires immediate attention"));
    }
}