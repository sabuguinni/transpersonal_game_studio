#include "Eng_TechnicalArchitecture.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"
#include "Misc/DateTime.h"

UEng_TechnicalArchitecture::UEng_TechnicalArchitecture()
{
    MinPerformanceScore = 75.0f;
    MaxCompilationTime = 300.0f;
}

void UEng_TechnicalArchitecture::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("Engine Architect: Technical Architecture System Initializing..."));
    
    // Initialize architectural rules
    InitializeArchitecturalRules();
    
    // Scan existing modules
    ScanModuleDirectory(TEXT("Source/TranspersonalGame"));
    
    // Validate current state
    EnforceArchitecturalRules();
    
    UE_LOG(LogTemp, Warning, TEXT("Engine Architect: Technical Architecture System Ready"));
}

void UEng_TechnicalArchitecture::Deinitialize()
{
    UE_LOG(LogTemp, Warning, TEXT("Engine Architect: Technical Architecture System Shutting Down"));
    
    // Generate final compliance report
    FEng_CompilationReport FinalReport = GenerateCompilationReport();
    UE_LOG(LogTemp, Warning, TEXT("Final Compilation Status: %s"), 
           FinalReport.bCompilationSuccessful ? TEXT("SUCCESS") : TEXT("FAILED"));
    
    Super::Deinitialize();
}

bool UEng_TechnicalArchitecture::ValidateModuleCompliance(const FString& ModuleName)
{
    UE_LOG(LogTemp, Warning, TEXT("Engine Architect: Validating module compliance for %s"), *ModuleName);
    
    // Find or create module compliance record
    FEng_ModuleCompliance* Compliance = ModuleCompliance.FindByPredicate(
        [&ModuleName](const FEng_ModuleCompliance& Item) 
        { 
            return Item.ModuleName == ModuleName; 
        });
    
    if (!Compliance)
    {
        ModuleCompliance.Add(CreateModuleCompliance(ModuleName));
        Compliance = &ModuleCompliance.Last();
    }
    
    // Validate module structure
    bool bStructureValid = ValidateModuleStructure(ModuleName);
    
    // Check for .cpp implementation
    FString HeaderPath = FString::Printf(TEXT("Source/TranspersonalGame/%s"), *ModuleName);
    Compliance->bHasCppImplementation = CheckCppImplementation(HeaderPath);
    
    // Update compliance level
    if (bStructureValid && Compliance->bHasCppImplementation)
    {
        Compliance->ComplianceLevel = EEng_ComplianceLevel::Compliant;
        Compliance->Status = EEng_ModuleStatus::Implemented;
        Compliance->bPassesValidation = true;
    }
    else if (Compliance->bHasCppImplementation)
    {
        Compliance->ComplianceLevel = EEng_ComplianceLevel::Medium;
        Compliance->Status = EEng_ModuleStatus::Implemented;
    }
    else
    {
        Compliance->ComplianceLevel = EEng_ComplianceLevel::Critical;
        Compliance->Status = EEng_ModuleStatus::HeaderOnly;
        Compliance->MissingImplementations.Add(TEXT("Missing .cpp implementation"));
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Module %s compliance: %s"), 
           *ModuleName, 
           Compliance->bPassesValidation ? TEXT("PASS") : TEXT("FAIL"));
    
    return Compliance->bPassesValidation;
}

FEng_CompilationReport UEng_TechnicalArchitecture::GenerateCompilationReport()
{
    FEng_CompilationReport Report;
    Report.bCompilationSuccessful = true;
    Report.TotalErrors = 0;
    Report.TotalWarnings = 0;
    
    UE_LOG(LogTemp, Warning, TEXT("Engine Architect: Generating compilation report..."));
    
    // Check all modules for compliance
    for (const FEng_ModuleCompliance& Module : ModuleCompliance)
    {
        if (!Module.bHasCppImplementation)
        {
            Report.MissingCppFiles.Add(Module.ModuleName);
            Report.TotalErrors++;
            Report.bCompilationSuccessful = false;
        }
        
        if (Module.ComplianceLevel == EEng_ComplianceLevel::Critical)
        {
            Report.ErrorMessages.Add(FString::Printf(TEXT("Critical compliance issue in %s"), *Module.ModuleName));
            Report.TotalErrors++;
            Report.bCompilationSuccessful = false;
        }
        
        if (Module.PerformanceScore < MinPerformanceScore)
        {
            Report.WarningMessages.Add(FString::Printf(TEXT("Performance warning in %s: %.1f%%"), 
                                                       *Module.ModuleName, Module.PerformanceScore));
            Report.TotalWarnings++;
        }
    }
    
    // Calculate compilation time estimate
    Report.CompilationTime = ModuleCompliance.Num() * 15.0f; // 15 seconds per module estimate
    
    UE_LOG(LogTemp, Warning, TEXT("Compilation Report: %d errors, %d warnings, %d missing .cpp files"), 
           Report.TotalErrors, Report.TotalWarnings, Report.MissingCppFiles.Num());
    
    return Report;
}

TArray<FEng_ModuleCompliance> UEng_TechnicalArchitecture::GetAllModuleCompliance()
{
    return ModuleCompliance;
}

bool UEng_TechnicalArchitecture::EnforceArchitecturalRules()
{
    UE_LOG(LogTemp, Warning, TEXT("Engine Architect: Enforcing architectural rules..."));
    
    bool bAllRulesCompliant = true;
    
    for (const FEng_ArchitecturalRule& Rule : ArchitecturalRules)
    {
        if (Rule.bMandatory)
        {
            // Check rule compliance for affected modules
            for (const FString& ModuleName : Rule.AffectedModules)
            {
                if (!ValidateModuleCompliance(ModuleName))
                {
                    UE_LOG(LogTemp, Error, TEXT("Mandatory rule violation: %s in module %s"), 
                           *Rule.RuleName, *ModuleName);
                    bAllRulesCompliant = false;
                }
            }
        }
    }
    
    return bAllRulesCompliant;
}

bool UEng_TechnicalArchitecture::ValidateHeaderCppPairs()
{
    UE_LOG(LogTemp, Warning, TEXT("Engine Architect: Validating header-cpp pairs..."));
    
    TArray<FString> OrphanedHeaders = FindOrphanedHeaders();
    
    if (OrphanedHeaders.Num() > 0)
    {
        UE_LOG(LogTemp, Error, TEXT("Found %d orphaned headers:"), OrphanedHeaders.Num());
        for (const FString& Header : OrphanedHeaders)
        {
            UE_LOG(LogTemp, Error, TEXT("  - %s"), *Header);
        }
        return false;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("All headers have corresponding .cpp files"));
    return true;
}

TArray<FString> UEng_TechnicalArchitecture::FindOrphanedHeaders()
{
    TArray<FString> OrphanedHeaders;
    
    // This would normally scan the file system, but for now we'll use known problematic modules
    TArray<FString> KnownHeaders = {
        TEXT("Core/Eng_TechnicalArchitecture.h"),
        TEXT("Director/ProductionPipeline.h"),
        TEXT("SharedTypes.h")
    };
    
    for (const FString& Header : KnownHeaders)
    {
        FString CppPath = Header.Replace(TEXT(".h"), TEXT(".cpp"));
        
        // Check if corresponding .cpp exists in our module compliance
        bool bHasCpp = ModuleCompliance.ContainsByPredicate(
            [&CppPath](const FEng_ModuleCompliance& Module)
            {
                return Module.bHasCppImplementation && Module.ModuleName.Contains(CppPath);
            });
        
        if (!bHasCpp)
        {
            OrphanedHeaders.Add(Header);
        }
    }
    
    return OrphanedHeaders;
}

TArray<FString> UEng_TechnicalArchitecture::FindMissingImplementations()
{
    TArray<FString> MissingImplementations;
    
    for (const FEng_ModuleCompliance& Module : ModuleCompliance)
    {
        if (!Module.bHasCppImplementation)
        {
            MissingImplementations.Add(Module.ModuleName);
        }
    }
    
    return MissingImplementations;
}

float UEng_TechnicalArchitecture::CalculateModulePerformanceScore(const FString& ModuleName)
{
    // Base performance calculation
    float Score = 100.0f;
    
    // Find module compliance
    const FEng_ModuleCompliance* Compliance = ModuleCompliance.FindByPredicate(
        [&ModuleName](const FEng_ModuleCompliance& Item) 
        { 
            return Item.ModuleName == ModuleName; 
        });
    
    if (Compliance)
    {
        // Reduce score for missing implementations
        if (!Compliance->bHasCppImplementation)
        {
            Score -= 50.0f;
        }
        
        // Reduce score for compilation errors
        Score -= Compliance->CompilationErrors.Num() * 10.0f;
        
        // Reduce score for missing implementations
        Score -= Compliance->MissingImplementations.Num() * 5.0f;
        
        // Ensure minimum score
        Score = FMath::Max(Score, 0.0f);
    }
    
    return Score;
}

bool UEng_TechnicalArchitecture::ValidatePerformanceCompliance()
{
    bool bAllCompliant = true;
    
    for (FEng_ModuleCompliance& Module : ModuleCompliance)
    {
        Module.PerformanceScore = CalculateModulePerformanceScore(Module.ModuleName);
        
        if (Module.PerformanceScore < MinPerformanceScore)
        {
            bAllCompliant = false;
            UE_LOG(LogTemp, Warning, TEXT("Performance compliance failure: %s (%.1f%%)"), 
                   *Module.ModuleName, Module.PerformanceScore);
        }
    }
    
    return bAllCompliant;
}

bool UEng_TechnicalArchitecture::TestModuleIntegration(const FString& ModuleA, const FString& ModuleB)
{
    UE_LOG(LogTemp, Warning, TEXT("Engine Architect: Testing integration between %s and %s"), 
           *ModuleA, *ModuleB);
    
    // Basic integration test - check if both modules are compliant
    bool bModuleAValid = ValidateModuleCompliance(ModuleA);
    bool bModuleBValid = ValidateModuleCompliance(ModuleB);
    
    return bModuleAValid && bModuleBValid;
}

bool UEng_TechnicalArchitecture::ValidateSystemDependencies()
{
    UE_LOG(LogTemp, Warning, TEXT("Engine Architect: Validating system dependencies..."));
    
    // Check core dependencies
    TArray<FString> CoreModules = {
        TEXT("SharedTypes"),
        TEXT("TranspersonalGame"),
        TEXT("Core/Eng_TechnicalArchitecture")
    };
    
    bool bAllDependenciesValid = true;
    
    for (const FString& Module : CoreModules)
    {
        if (!ValidateModuleCompliance(Module))
        {
            bAllDependenciesValid = false;
            UE_LOG(LogTemp, Error, TEXT("Core dependency failure: %s"), *Module);
        }
    }
    
    return bAllDependenciesValid;
}

bool UEng_TechnicalArchitecture::RegisterAgentModule(int32 AgentNumber, const FString& ModuleName, EEng_ArchitecturalLayer Layer)
{
    UE_LOG(LogTemp, Warning, TEXT("Engine Architect: Registering Agent #%d module: %s"), 
           AgentNumber, *ModuleName);
    
    AgentModuleMap.Add(AgentNumber, ModuleName);
    
    // Create or update module compliance
    FEng_ModuleCompliance NewCompliance = CreateModuleCompliance(ModuleName);
    NewCompliance.Layer = Layer;
    
    ModuleCompliance.Add(NewCompliance);
    
    return true;
}

TArray<FString> UEng_TechnicalArchitecture::GetAgentDependencies(int32 AgentNumber)
{
    TArray<FString> Dependencies;
    
    // Agent dependency chain based on production pipeline
    switch (AgentNumber)
    {
        case 2: // Engine Architect
            Dependencies.Add(TEXT("SharedTypes"));
            Dependencies.Add(TEXT("TranspersonalGame"));
            break;
        case 3: // Core Systems
            Dependencies.Add(TEXT("Core/Eng_TechnicalArchitecture"));
            Dependencies.Add(TEXT("SharedTypes"));
            break;
        case 5: // World Generation
            Dependencies.Add(TEXT("Core"));
            Dependencies.Add(TEXT("Physics"));
            break;
        default:
            Dependencies.Add(TEXT("Core"));
            break;
    }
    
    return Dependencies;
}

bool UEng_TechnicalArchitecture::ValidateAgentCompliance(int32 AgentNumber)
{
    if (!AgentModuleMap.Contains(AgentNumber))
    {
        UE_LOG(LogTemp, Warning, TEXT("Agent #%d not registered"), AgentNumber);
        return false;
    }
    
    FString ModuleName = AgentModuleMap[AgentNumber];
    return ValidateModuleCompliance(ModuleName);
}

bool UEng_TechnicalArchitecture::ValidateModuleStructure(const FString& ModuleName)
{
    // Basic structure validation
    return !ModuleName.IsEmpty() && ModuleName.Len() > 3;
}

bool UEng_TechnicalArchitecture::CheckCppImplementation(const FString& HeaderPath)
{
    // For now, assume .cpp exists if module is in our compliance list
    // In a real implementation, this would check the file system
    return ModuleCompliance.ContainsByPredicate(
        [&HeaderPath](const FEng_ModuleCompliance& Module)
        {
            return Module.ModuleName.Contains(HeaderPath) && Module.bHasCppImplementation;
        });
}

void UEng_TechnicalArchitecture::InitializeArchitecturalRules()
{
    UE_LOG(LogTemp, Warning, TEXT("Engine Architect: Initializing architectural rules..."));
    
    // Rule 1: Every .h must have a .cpp
    FEng_ArchitecturalRule HeaderCppRule;
    HeaderCppRule.RuleName = TEXT("Header-Cpp Pairing");
    HeaderCppRule.Description = TEXT("Every header file must have a corresponding .cpp implementation");
    HeaderCppRule.Severity = EEng_ComplianceLevel::Critical;
    HeaderCppRule.bMandatory = true;
    HeaderCppRule.AffectedModules.Add(TEXT("All"));
    ArchitecturalRules.Add(HeaderCppRule);
    
    // Rule 2: Performance threshold compliance
    FEng_ArchitecturalRule PerformanceRule;
    PerformanceRule.RuleName = TEXT("Performance Compliance");
    PerformanceRule.Description = TEXT("All modules must maintain minimum performance score");
    PerformanceRule.Severity = EEng_ComplianceLevel::High;
    PerformanceRule.bMandatory = true;
    PerformanceRule.AffectedModules.Add(TEXT("All"));
    ArchitecturalRules.Add(PerformanceRule);
    
    // Rule 3: Shared types usage
    FEng_ArchitecturalRule SharedTypesRule;
    SharedTypesRule.RuleName = TEXT("Shared Types Usage");
    SharedTypesRule.Description = TEXT("All cross-module types must be defined in SharedTypes.h");
    SharedTypesRule.Severity = EEng_ComplianceLevel::High;
    SharedTypesRule.bMandatory = true;
    SharedTypesRule.AffectedModules.Add(TEXT("All"));
    ArchitecturalRules.Add(SharedTypesRule);
    
    UE_LOG(LogTemp, Warning, TEXT("Initialized %d architectural rules"), ArchitecturalRules.Num());
}

void UEng_TechnicalArchitecture::ScanModuleDirectory(const FString& DirectoryPath)
{
    UE_LOG(LogTemp, Warning, TEXT("Engine Architect: Scanning module directory: %s"), *DirectoryPath);
    
    // For now, manually add known modules
    TArray<FString> KnownModules = {
        TEXT("SharedTypes"),
        TEXT("TranspersonalGame"),
        TEXT("Core/Eng_TechnicalArchitecture"),
        TEXT("Director/ProductionPipeline"),
        TEXT("Core/BiomeManager"),
        TEXT("Core/AssetProductionManager"),
        TEXT("Core/BuildSystemManager")
    };
    
    for (const FString& ModuleName : KnownModules)
    {
        ModuleCompliance.Add(CreateModuleCompliance(ModuleName));
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Scanned %d modules"), ModuleCompliance.Num());
}

FEng_ModuleCompliance UEng_TechnicalArchitecture::CreateModuleCompliance(const FString& ModuleName)
{
    FEng_ModuleCompliance Compliance;
    Compliance.ModuleName = ModuleName;
    Compliance.Layer = EEng_ArchitecturalLayer::Core;
    Compliance.Status = EEng_ModuleStatus::NotStarted;
    Compliance.ComplianceLevel = EEng_ComplianceLevel::Critical;
    Compliance.bHasCppImplementation = false;
    Compliance.bPassesValidation = false;
    Compliance.PerformanceScore = 0.0f;
    
    return Compliance;
}

// Game Mode Implementation
AEng_ArchitecturalGameMode::AEng_ArchitecturalGameMode()
{
    PrimaryActorTick.bCanEverTick = true;
    bEnableRuntimeValidation = true;
    ValidationInterval = 30.0f;
    LastValidationTime = 0.0f;
}

void AEng_ArchitecturalGameMode::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Warning, TEXT("Engine Architect Game Mode: Starting architectural oversight"));
    
    // Get technical architecture subsystem
    TechnicalArchitecture = GetGameInstance()->GetSubsystem<UEng_TechnicalArchitecture>();
    
    if (TechnicalArchitecture)
    {
        UE_LOG(LogTemp, Warning, TEXT("Technical Architecture subsystem found"));
        LogArchitecturalStatus();
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Technical Architecture subsystem not found"));
    }
}

void AEng_ArchitecturalGameMode::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    if (bEnableRuntimeValidation && TechnicalArchitecture)
    {
        LastValidationTime += DeltaTime;
        
        if (LastValidationTime >= ValidationInterval)
        {
            PerformRuntimeValidation();
            LastValidationTime = 0.0f;
        }
    }
}

bool AEng_ArchitecturalGameMode::PerformRuntimeValidation()
{
    if (!TechnicalArchitecture)
    {
        return false;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Engine Architect: Performing runtime validation..."));
    
    // Validate system dependencies
    bool bDependenciesValid = TechnicalArchitecture->ValidateSystemDependencies();
    
    // Validate performance compliance
    bool bPerformanceValid = TechnicalArchitecture->ValidatePerformanceCompliance();
    
    // Log results
    UE_LOG(LogTemp, Warning, TEXT("Runtime validation - Dependencies: %s, Performance: %s"),
           bDependenciesValid ? TEXT("PASS") : TEXT("FAIL"),
           bPerformanceValid ? TEXT("PASS") : TEXT("FAIL"));
    
    return bDependenciesValid && bPerformanceValid;
}

void AEng_ArchitecturalGameMode::LogArchitecturalStatus()
{
    if (!TechnicalArchitecture)
    {
        return;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("=== ENGINE ARCHITECT STATUS REPORT ==="));
    
    // Generate compilation report
    FEng_CompilationReport Report = TechnicalArchitecture->GenerateCompilationReport();
    
    UE_LOG(LogTemp, Warning, TEXT("Compilation Status: %s"), 
           Report.bCompilationSuccessful ? TEXT("SUCCESS") : TEXT("FAILED"));
    UE_LOG(LogTemp, Warning, TEXT("Total Errors: %d"), Report.TotalErrors);
    UE_LOG(LogTemp, Warning, TEXT("Total Warnings: %d"), Report.TotalWarnings);
    UE_LOG(LogTemp, Warning, TEXT("Missing .cpp Files: %d"), Report.MissingCppFiles.Num());
    
    // Log module compliance
    TArray<FEng_ModuleCompliance> AllCompliance = TechnicalArchitecture->GetAllModuleCompliance();
    UE_LOG(LogTemp, Warning, TEXT("Total Modules: %d"), AllCompliance.Num());
    
    int32 CompliantModules = 0;
    for (const FEng_ModuleCompliance& Module : AllCompliance)
    {
        if (Module.bPassesValidation)
        {
            CompliantModules++;
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Compliant Modules: %d/%d"), CompliantModules, AllCompliance.Num());
    UE_LOG(LogTemp, Warning, TEXT("=== END STATUS REPORT ==="));
}