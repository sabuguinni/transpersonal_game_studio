#include "EngineArchitectCore.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/DateTime.h"
#include "Stats/Stats.h"
#include "UObject/UObjectGlobals.h"

// Static instance for singleton pattern
UArchitectureComplianceManager* UArchitectureComplianceManager::Instance = nullptr;

UEngineArchitectCore::UEngineArchitectCore()
{
    bModuleIntegrityValid = false;
    bCompilationStatusClean = false;
    MemoryUsageThreshold = 85.0f;
    MaxActorCount = 20000;
    LastMemoryCheck = 0.0f;
    LastActorCount = 0;
    LastValidationTime = FDateTime::Now();

    // Initialize critical modules list
    CriticalModules.Add(TEXT("TranspersonalGame"));
    CriticalModules.Add(TEXT("Engine"));
    CriticalModules.Add(TEXT("Core"));
    CriticalModules.Add(TEXT("CoreUObject"));
}

void UEngineArchitectCore::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("Engine Architect Core System Initialized"));
    
    // Perform initial validation
    ValidateModuleIntegrity();
    CheckCompilationStatus();
    
    // Set up performance monitoring
    LastMemoryCheck = FPlatformTime::Seconds();
    
    UE_LOG(LogTemp, Warning, TEXT("Architecture validation complete - Module integrity: %s, Compilation: %s"), 
           bModuleIntegrityValid ? TEXT("VALID") : TEXT("INVALID"),
           bCompilationStatusClean ? TEXT("CLEAN") : TEXT("DIRTY"));
}

void UEngineArchitectCore::Deinitialize()
{
    UE_LOG(LogTemp, Warning, TEXT("Engine Architect Core System Shutting Down"));
    
    // Clean up any architectural violations
    CleanupOrphanedReferences();
    
    Super::Deinitialize();
}

bool UEngineArchitectCore::ValidateModuleIntegrity()
{
    ValidationErrors.Empty();
    bool bIntegrityValid = true;

    // Check class definitions
    if (!ValidateClassDefinitions())
    {
        bIntegrityValid = false;
        ValidationErrors.Add(TEXT("Class definition validation failed"));
    }

    // Check header includes
    if (!ValidateHeaderIncludes())
    {
        bIntegrityValid = false;
        ValidationErrors.Add(TEXT("Header include validation failed"));
    }

    // Check for circular dependencies
    if (!CheckForCircularDependencies())
    {
        bIntegrityValid = false;
        ValidationErrors.Add(TEXT("Circular dependency detected"));
    }

    bModuleIntegrityValid = bIntegrityValid;
    
    if (!bIntegrityValid)
    {
        UE_LOG(LogTemp, Error, TEXT("Module integrity validation FAILED"));
        for (const FString& Error : ValidationErrors)
        {
            UE_LOG(LogTemp, Error, TEXT("  - %s"), *Error);
        }
    }
    
    return bIntegrityValid;
}

bool UEngineArchitectCore::CheckCompilationStatus()
{
    // Check if we can load critical classes
    bool bCompilationClean = true;
    
    try
    {
        // Test loading core game classes
        UClass* GameModeClass = LoadClass<UObject>(nullptr, TEXT("/Script/TranspersonalGame.TranspersonalGameMode"));
        UClass* CharacterClass = LoadClass<UObject>(nullptr, TEXT("/Script/TranspersonalGame.TranspersonalCharacter"));
        
        if (!GameModeClass)
        {
            bCompilationClean = false;
            LogArchitecturalViolation(TEXT("TranspersonalGameMode class not found - compilation issue"));
        }
        
        if (!CharacterClass)
        {
            bCompilationClean = false;
            LogArchitecturalViolation(TEXT("TranspersonalCharacter class not found - compilation issue"));
        }
    }
    catch (...)
    {
        bCompilationClean = false;
        LogArchitecturalViolation(TEXT("Exception during class loading - critical compilation error"));
    }
    
    bCompilationStatusClean = bCompilationClean;
    return bCompilationClean;
}

void UEngineArchitectCore::EnforceArchitecturalRules()
{
    UE_LOG(LogTemp, Warning, TEXT("Enforcing architectural rules..."));
    
    // Rule 1: Memory usage threshold
    OptimizeMemoryUsage();
    
    // Rule 2: Actor count limits
    UWorld* World = GetWorld();
    if (World)
    {
        int32 ActorCount = World->GetActorCount();
        if (ActorCount > MaxActorCount)
        {
            LogArchitecturalViolation(FString::Printf(TEXT("Actor count (%d) exceeds maximum (%d)"), ActorCount, MaxActorCount));
        }
        LastActorCount = ActorCount;
    }
    
    // Rule 3: Module dependency validation
    ValidateModuleDependencies();
    
    UE_LOG(LogTemp, Warning, TEXT("Architectural rule enforcement complete"));
}

FString UEngineArchitectCore::GetSystemPerformanceReport()
{
    FString Report = TEXT("=== SYSTEM PERFORMANCE REPORT ===\n");
    
    // Memory usage
    FPlatformMemoryStats MemStats = FPlatformMemory::GetStats();
    float MemoryUsagePercent = (float)MemStats.UsedPhysical / (float)MemStats.TotalPhysical * 100.0f;
    Report += FString::Printf(TEXT("Memory Usage: %.1f%% (%llu MB / %llu MB)\n"), 
                             MemoryUsagePercent, 
                             MemStats.UsedPhysical / 1024 / 1024,
                             MemStats.TotalPhysical / 1024 / 1024);
    
    // Actor count
    UWorld* World = GetWorld();
    if (World)
    {
        int32 ActorCount = World->GetActorCount();
        Report += FString::Printf(TEXT("Actor Count: %d / %d\n"), ActorCount, MaxActorCount);
    }
    
    // Module status
    Report += FString::Printf(TEXT("Module Integrity: %s\n"), bModuleIntegrityValid ? TEXT("VALID") : TEXT("INVALID"));
    Report += FString::Printf(TEXT("Compilation Status: %s\n"), bCompilationStatusClean ? TEXT("CLEAN") : TEXT("DIRTY"));
    
    // Validation errors
    if (ValidationErrors.Num() > 0)
    {
        Report += TEXT("Validation Errors:\n");
        for (const FString& Error : ValidationErrors)
        {
            Report += FString::Printf(TEXT("  - %s\n"), *Error);
        }
    }
    
    Report += TEXT("=== END REPORT ===");
    
    return Report;
}

void UEngineArchitectCore::OptimizeMemoryUsage()
{
    // Force garbage collection if memory usage is high
    FPlatformMemoryStats MemStats = FPlatformMemory::GetStats();
    float MemoryUsagePercent = (float)MemStats.UsedPhysical / (float)MemStats.TotalPhysical * 100.0f;
    
    if (MemoryUsagePercent > MemoryUsageThreshold)
    {
        UE_LOG(LogTemp, Warning, TEXT("Memory usage high (%.1f%%), forcing garbage collection"), MemoryUsagePercent);
        CollectGarbage(GARBAGE_COLLECTION_KEEPFLAGS);
        
        LogArchitecturalViolation(FString::Printf(TEXT("Memory usage exceeded threshold: %.1f%%"), MemoryUsagePercent));
    }
}

TArray<FString> UEngineArchitectCore::GetModuleDependencies()
{
    TArray<FString> Dependencies;
    
    // Core dependencies for TranspersonalGame module
    Dependencies.Add(TEXT("Engine"));
    Dependencies.Add(TEXT("Core"));
    Dependencies.Add(TEXT("CoreUObject"));
    Dependencies.Add(TEXT("UnrealEd"));
    Dependencies.Add(TEXT("ToolMenus"));
    Dependencies.Add(TEXT("EditorStyle"));
    Dependencies.Add(TEXT("EditorWidgets"));
    Dependencies.Add(TEXT("GameplayTags"));
    Dependencies.Add(TEXT("NavigationSystem"));
    Dependencies.Add(TEXT("AIModule"));
    Dependencies.Add(TEXT("UMG"));
    Dependencies.Add(TEXT("Slate"));
    Dependencies.Add(TEXT("SlateCore"));
    Dependencies.Add(TEXT("PCG"));
    Dependencies.Add(TEXT("Niagara"));
    Dependencies.Add(TEXT("MetasoundEngine"));
    
    return Dependencies;
}

bool UEngineArchitectCore::ValidateModuleDependencies()
{
    TArray<FString> RequiredModules = GetModuleDependencies();
    bool bAllDependenciesValid = true;
    
    for (const FString& ModuleName : RequiredModules)
    {
        if (!FModuleManager::Get().IsModuleLoaded(*ModuleName))
        {
            LogArchitecturalViolation(FString::Printf(TEXT("Required module not loaded: %s"), *ModuleName));
            bAllDependenciesValid = false;
        }
    }
    
    return bAllDependenciesValid;
}

bool UEngineArchitectCore::ValidateClassDefinitions()
{
    // Basic validation - check if critical classes exist
    bool bValid = true;
    
    // Test core game classes
    if (!FindObject<UClass>(ANY_PACKAGE, TEXT("TranspersonalGameMode")))
    {
        bValid = false;
    }
    
    if (!FindObject<UClass>(ANY_PACKAGE, TEXT("TranspersonalCharacter")))
    {
        bValid = false;
    }
    
    return bValid;
}

bool UEngineArchitectCore::ValidateHeaderIncludes()
{
    // This would typically check for proper include hierarchies
    // For now, return true as a placeholder
    return true;
}

bool UEngineArchitectCore::CheckForCircularDependencies()
{
    // Placeholder for circular dependency detection
    // Would implement graph traversal to detect cycles
    return true;
}

void UEngineArchitectCore::LogArchitecturalViolation(const FString& Violation)
{
    UE_LOG(LogTemp, Error, TEXT("ARCHITECTURAL VIOLATION: %s"), *Violation);
    
    // Register with compliance manager if available
    UArchitectureComplianceManager* ComplianceManager = UArchitectureComplianceManager::GetInstance();
    if (ComplianceManager)
    {
        FEng_TechnicalDebtEntry DebtEntry;
        DebtEntry.ModuleName = TEXT("EngineArchitect");
        DebtEntry.DebtDescription = Violation;
        DebtEntry.Severity = EEng_DebtSeverity::High;
        DebtEntry.EstimatedFixTime = 2.0f;
        
        ComplianceManager->RegisterTechnicalDebt(DebtEntry);
    }
}

void UEngineArchitectCore::CleanupOrphanedReferences()
{
    // Force cleanup of any orphaned references
    CollectGarbage(GARBAGE_COLLECTION_KEEPFLAGS);
    
    UE_LOG(LogTemp, Warning, TEXT("Orphaned reference cleanup complete"));
}

// Architecture Enforcer Component Implementation
UArchitectureEnforcerComponent::UArchitectureEnforcerComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 5.0f; // Check every 5 seconds
    
    bEnforceNamingRules = true;
    bValidateHierarchy = true;
    PerformanceCheckInterval = 10.0f;
    LastPerformanceCheck = 0.0f;
}

void UArchitectureEnforcerComponent::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Warning, TEXT("Architecture Enforcer Component activated on: %s"), *GetOwner()->GetName());
    
    // Initial compliance check
    CheckOwnerCompliance();
}

void UArchitectureEnforcerComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    LastPerformanceCheck += DeltaTime;
    
    if (LastPerformanceCheck >= PerformanceCheckInterval)
    {
        CheckPerformanceCompliance();
        LastPerformanceCheck = 0.0f;
    }
}

void UArchitectureEnforcerComponent::EnforceNamingConventions()
{
    if (!bEnforceNamingRules) return;
    
    AActor* Owner = GetOwner();
    if (!Owner) return;
    
    FString ActorName = Owner->GetName();
    
    // Check naming conventions
    if (!ActorName.StartsWith(TEXT("BP_")) && !ActorName.StartsWith(TEXT("A")) && !ActorName.Contains(TEXT("_")))
    {
        RuleViolations.Add(FString::Printf(TEXT("Actor name '%s' doesn't follow naming conventions"), *ActorName));
        UE_LOG(LogTemp, Warning, TEXT("Naming violation: %s"), *ActorName);
    }
}

void UArchitectureEnforcerComponent::ValidateComponentHierarchy()
{
    if (!bValidateHierarchy) return;
    
    AActor* Owner = GetOwner();
    if (!Owner) return;
    
    TArray<UActorComponent*> Components = Owner->GetRootComponent()->GetAttachChildren();
    
    // Validate component hierarchy depth (shouldn't exceed 5 levels)
    int32 MaxDepth = 0;
    // Implementation would recursively check component hierarchy depth
    
    if (MaxDepth > 5)
    {
        RuleViolations.Add(TEXT("Component hierarchy too deep (>5 levels)"));
    }
}

bool UArchitectureEnforcerComponent::CheckPerformanceCompliance()
{
    AActor* Owner = GetOwner();
    if (!Owner) return false;
    
    bool bCompliant = true;
    
    // Check component count
    TArray<UActorComponent*> Components;
    Owner->GetComponents(Components);
    
    if (Components.Num() > 20)
    {
        RuleViolations.Add(FString::Printf(TEXT("Too many components: %d (max 20)"), Components.Num()));
        bCompliant = false;
    }
    
    // Check mesh complexity (if it's a mesh actor)
    UStaticMeshComponent* MeshComp = Owner->FindComponentByClass<UStaticMeshComponent>();
    if (MeshComp && MeshComp->GetStaticMesh())
    {
        int32 TriangleCount = MeshComp->GetStaticMesh()->GetNumTriangles(0);
        if (TriangleCount > 10000)
        {
            RuleViolations.Add(FString::Printf(TEXT("Mesh too complex: %d triangles (max 10000)"), TriangleCount));
            bCompliant = false;
        }
    }
    
    return bCompliant;
}

void UArchitectureEnforcerComponent::CheckOwnerCompliance()
{
    EnforceNamingConventions();
    ValidateComponentHierarchy();
    CheckPerformanceCompliance();
    
    if (RuleViolations.Num() > 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("Architecture violations found on %s:"), *GetOwner()->GetName());
        for (const FString& Violation : RuleViolations)
        {
            UE_LOG(LogTemp, Warning, TEXT("  - %s"), *Violation);
        }
    }
}

// Architecture Compliance Manager Implementation
UArchitectureComplianceManager::UArchitectureComplianceManager()
{
    InitializeComplianceRules();
}

UArchitectureComplianceManager* UArchitectureComplianceManager::GetInstance()
{
    if (!Instance)
    {
        Instance = NewObject<UArchitectureComplianceManager>();
        Instance->AddToRoot(); // Prevent garbage collection
    }
    return Instance;
}

void UArchitectureComplianceManager::RegisterTechnicalDebt(const FEng_TechnicalDebtEntry& DebtEntry)
{
    TechnicalDebtEntries.Add(DebtEntry);
    
    // Update violation count for module
    int32* ViolationCount = ModuleViolationCounts.Find(DebtEntry.ModuleName);
    if (ViolationCount)
    {
        (*ViolationCount)++;
    }
    else
    {
        ModuleViolationCounts.Add(DebtEntry.ModuleName, 1);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Technical debt registered: %s - %s"), *DebtEntry.ModuleName, *DebtEntry.DebtDescription);
}

TArray<FEng_TechnicalDebtEntry> UArchitectureComplianceManager::GetTechnicalDebtReport()
{
    return TechnicalDebtEntries;
}

void UArchitectureComplianceManager::ClearResolvedDebt(const FString& ModuleName)
{
    TechnicalDebtEntries.RemoveAll([&ModuleName](const FEng_TechnicalDebtEntry& Entry)
    {
        return Entry.ModuleName == ModuleName;
    });
    
    ModuleViolationCounts.Remove(ModuleName);
    
    UE_LOG(LogTemp, Warning, TEXT("Technical debt cleared for module: %s"), *ModuleName);
}

float UArchitectureComplianceManager::CalculateTotalDebtScore()
{
    float TotalScore = 0.0f;
    
    for (const FEng_TechnicalDebtEntry& Entry : TechnicalDebtEntries)
    {
        float SeverityMultiplier = 1.0f;
        switch (Entry.Severity)
        {
            case EEng_DebtSeverity::Low: SeverityMultiplier = 1.0f; break;
            case EEng_DebtSeverity::Medium: SeverityMultiplier = 2.0f; break;
            case EEng_DebtSeverity::High: SeverityMultiplier = 4.0f; break;
            case EEng_DebtSeverity::Critical: SeverityMultiplier = 8.0f; break;
        }
        
        TotalScore += Entry.EstimatedFixTime * SeverityMultiplier;
    }
    
    return TotalScore;
}

void UArchitectureComplianceManager::InitializeComplianceRules()
{
    UE_LOG(LogTemp, Warning, TEXT("Architecture Compliance Manager initialized"));
    
    // Initialize with any default compliance rules
    ModuleViolationCounts.Empty();
    TechnicalDebtEntries.Empty();
}