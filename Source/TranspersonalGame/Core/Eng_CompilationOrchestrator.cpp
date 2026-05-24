#include "Eng_CompilationOrchestrator.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "UObject/UObjectGlobals.h"
#include "UObject/Package.h"
#include "HAL/FileManager.h"
#include "Misc/Paths.h"

UEng_CompilationOrchestrator::UEng_CompilationOrchestrator()
{
    bIsCompilationClean = false;
    LastValidationTime = 0.0f;
}

void UEng_CompilationOrchestrator::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("Engine Architect - Compilation Orchestrator Initialized"));
    
    // Initialize module status tracking
    ModuleStatusMap.Empty();
    
    // Add core modules to track
    FEng_ModuleStatus CoreStatus;
    CoreStatus.ModuleName = TEXT("TranspersonalGame");
    CoreStatus.Status = EEng_CompilationStatus::Unknown;
    ModuleStatusMap.Add(TEXT("TranspersonalGame"), CoreStatus);
    
    // Perform initial validation
    ValidateAllModules();
}

void UEng_CompilationOrchestrator::Deinitialize()
{
    UE_LOG(LogTemp, Warning, TEXT("Engine Architect - Compilation Orchestrator Shutdown"));
    Super::Deinitialize();
}

void UEng_CompilationOrchestrator::ValidateAllModules()
{
    UE_LOG(LogTemp, Warning, TEXT("=== ENGINE ARCHITECT - MODULE VALIDATION START ==="));
    
    LastValidationTime = FPlatformTime::Seconds();
    bIsCompilationClean = true;
    
    // Validate core systems
    CheckCoreClasses();
    CheckPhysicsClasses();
    CheckBiomeClasses();
    CheckCharacterClasses();
    CheckDinosaurClasses();
    
    // Update overall status
    int32 TotalErrors = 0;
    for (const auto& ModulePair : ModuleStatusMap)
    {
        TotalErrors += ModulePair.Value.ErrorCount;
    }
    
    bIsCompilationClean = (TotalErrors == 0);
    
    UE_LOG(LogTemp, Warning, TEXT("=== MODULE VALIDATION COMPLETE - Clean: %s, Total Errors: %d ==="), 
           bIsCompilationClean ? TEXT("YES") : TEXT("NO"), TotalErrors);
}

bool UEng_CompilationOrchestrator::IsProjectCompilationClean() const
{
    return bIsCompilationClean;
}

void UEng_CompilationOrchestrator::ForceRecompileModule(const FString& ModuleName)
{
    UE_LOG(LogTemp, Warning, TEXT("Force recompile requested for module: %s"), *ModuleName);
    
    // Update module status to compiling
    if (FEng_ModuleStatus* Status = ModuleStatusMap.Find(ModuleName))
    {
        Status->Status = EEng_CompilationStatus::Compiling;
        Status->ErrorCount = 0;
        Status->WarningCount = 0;
        Status->LastError = TEXT("");
    }
}

TArray<FEng_ModuleStatus> UEng_CompilationOrchestrator::GetModuleStatusList() const
{
    TArray<FEng_ModuleStatus> StatusList;
    for (const auto& ModulePair : ModuleStatusMap)
    {
        StatusList.Add(ModulePair.Value);
    }
    return StatusList;
}

void UEng_CompilationOrchestrator::CleanOrphanedHeaders()
{
    UE_LOG(LogTemp, Warning, TEXT("=== CLEANING ORPHANED HEADERS ==="));
    
    // Get project source directory
    FString SourcePath = FPaths::ProjectDir() + TEXT("Source/TranspersonalGame/");
    
    TArray<FString> HeaderFiles;
    IFileManager::Get().FindFilesRecursive(HeaderFiles, *SourcePath, TEXT("*.h"), true, false);
    
    int32 OrphanedCount = 0;
    
    for (const FString& HeaderFile : HeaderFiles)
    {
        // Check if corresponding .cpp exists
        FString CppFile = HeaderFile;
        CppFile = CppFile.Replace(TEXT(".h"), TEXT(".cpp"));
        
        if (!IFileManager::Get().FileExists(*CppFile))
        {
            // Skip certain files that are header-only by design
            if (HeaderFile.Contains(TEXT("SharedTypes.h")) || 
                HeaderFile.Contains(TEXT(".generated.h")) ||
                HeaderFile.Contains(TEXT("ArchitecturalStandards.h")))
            {
                continue;
            }
            
            OrphanedCount++;
            UE_LOG(LogTemp, Warning, TEXT("ORPHANED HEADER: %s"), *HeaderFile);
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Found %d orphaned headers"), OrphanedCount);
}

void UEng_CompilationOrchestrator::ValidateCriticalClasses()
{
    UE_LOG(LogTemp, Warning, TEXT("=== VALIDATING CRITICAL CLASSES ==="));
    
    // Test critical class loading
    TArray<FString> CriticalClasses = {
        TEXT("/Script/TranspersonalGame.TranspersonalGameMode"),
        TEXT("/Script/TranspersonalGame.TranspersonalCharacter"),
        TEXT("/Script/TranspersonalGame.TranspersonalGameState"),
        TEXT("/Script/TranspersonalGame.BiomeManager"),
        TEXT("/Script/TranspersonalGame.DinosaurBase")
    };
    
    int32 LoadedCount = 0;
    for (const FString& ClassName : CriticalClasses)
    {
        UClass* LoadedClass = LoadClass<UObject>(nullptr, *ClassName);
        if (LoadedClass)
        {
            LoadedCount++;
            UE_LOG(LogTemp, Warning, TEXT("✓ Class loaded: %s"), *ClassName);
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("✗ Failed to load: %s"), *ClassName);
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Critical classes loaded: %d/%d"), LoadedCount, CriticalClasses.Num());
}

void UEng_CompilationOrchestrator::CheckCoreClasses()
{
    FEng_ModuleStatus& Status = ModuleStatusMap.FindOrAdd(TEXT("Core"));
    Status.ModuleName = TEXT("Core");
    Status.ErrorCount = 0;
    
    // Check for GameMode
    UClass* GameModeClass = LoadClass<UObject>(nullptr, TEXT("/Script/TranspersonalGame.TranspersonalGameMode"));
    if (!GameModeClass)
    {
        Status.ErrorCount++;
        Status.LastError = TEXT("TranspersonalGameMode not found");
    }
    
    // Check for Character
    UClass* CharacterClass = LoadClass<UObject>(nullptr, TEXT("/Script/TranspersonalGame.TranspersonalCharacter"));
    if (!CharacterClass)
    {
        Status.ErrorCount++;
        Status.LastError = TEXT("TranspersonalCharacter not found");
    }
    
    Status.Status = (Status.ErrorCount == 0) ? EEng_CompilationStatus::Success : EEng_CompilationStatus::Failed;
    LogCompilationStatus(TEXT("Core"), Status.Status, FString::Printf(TEXT("Errors: %d"), Status.ErrorCount));
}

void UEng_CompilationOrchestrator::CheckPhysicsClasses()
{
    FEng_ModuleStatus& Status = ModuleStatusMap.FindOrAdd(TEXT("Physics"));
    Status.ModuleName = TEXT("Physics");
    Status.ErrorCount = 0;
    
    // Check for Physics Manager
    UClass* PhysicsClass = LoadClass<UObject>(nullptr, TEXT("/Script/TranspersonalGame.Core_PhysicsManager"));
    if (!PhysicsClass)
    {
        Status.ErrorCount++;
        Status.LastError = TEXT("Core_PhysicsManager not found");
    }
    
    Status.Status = (Status.ErrorCount == 0) ? EEng_CompilationStatus::Success : EEng_CompilationStatus::Failed;
    LogCompilationStatus(TEXT("Physics"), Status.Status, FString::Printf(TEXT("Errors: %d"), Status.ErrorCount));
}

void UEng_CompilationOrchestrator::CheckBiomeClasses()
{
    FEng_ModuleStatus& Status = ModuleStatusMap.FindOrAdd(TEXT("Biome"));
    Status.ModuleName = TEXT("Biome");
    Status.ErrorCount = 0;
    
    // Check for Biome Manager
    UClass* BiomeClass = LoadClass<UObject>(nullptr, TEXT("/Script/TranspersonalGame.BiomeManager"));
    if (!BiomeClass)
    {
        Status.ErrorCount++;
        Status.LastError = TEXT("BiomeManager not found");
    }
    
    Status.Status = (Status.ErrorCount == 0) ? EEng_CompilationStatus::Success : EEng_CompilationStatus::Failed;
    LogCompilationStatus(TEXT("Biome"), Status.Status, FString::Printf(TEXT("Errors: %d"), Status.ErrorCount));
}

void UEng_CompilationOrchestrator::CheckCharacterClasses()
{
    FEng_ModuleStatus& Status = ModuleStatusMap.FindOrAdd(TEXT("Character"));
    Status.ModuleName = TEXT("Character");
    Status.ErrorCount = 0;
    
    // Check for Character classes
    UClass* CharacterClass = LoadClass<UObject>(nullptr, TEXT("/Script/TranspersonalGame.TranspersonalCharacter"));
    if (!CharacterClass)
    {
        Status.ErrorCount++;
        Status.LastError = TEXT("TranspersonalCharacter not found");
    }
    
    Status.Status = (Status.ErrorCount == 0) ? EEng_CompilationStatus::Success : EEng_CompilationStatus::Failed;
    LogCompilationStatus(TEXT("Character"), Status.Status, FString::Printf(TEXT("Errors: %d"), Status.ErrorCount));
}

void UEng_CompilationOrchestrator::CheckDinosaurClasses()
{
    FEng_ModuleStatus& Status = ModuleStatusMap.FindOrAdd(TEXT("Dinosaur"));
    Status.ModuleName = TEXT("Dinosaur");
    Status.ErrorCount = 0;
    
    // Check for Dinosaur Base
    UClass* DinosaurClass = LoadClass<UObject>(nullptr, TEXT("/Script/TranspersonalGame.DinosaurBase"));
    if (!DinosaurClass)
    {
        Status.ErrorCount++;
        Status.LastError = TEXT("DinosaurBase not found");
    }
    
    Status.Status = (Status.ErrorCount == 0) ? EEng_CompilationStatus::Success : EEng_CompilationStatus::Failed;
    LogCompilationStatus(TEXT("Dinosaur"), Status.Status, FString::Printf(TEXT("Errors: %d"), Status.ErrorCount));
}

void UEng_CompilationOrchestrator::LogCompilationStatus(const FString& ModuleName, EEng_CompilationStatus Status, const FString& Details)
{
    FString StatusText;
    switch (Status)
    {
        case EEng_CompilationStatus::Success:
            StatusText = TEXT("SUCCESS");
            break;
        case EEng_CompilationStatus::Failed:
            StatusText = TEXT("FAILED");
            break;
        case EEng_CompilationStatus::Compiling:
            StatusText = TEXT("COMPILING");
            break;
        case EEng_CompilationStatus::Critical:
            StatusText = TEXT("CRITICAL");
            break;
        default:
            StatusText = TEXT("UNKNOWN");
            break;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("MODULE [%s]: %s - %s"), *ModuleName, *StatusText, *Details);
}