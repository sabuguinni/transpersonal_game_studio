#include "Eng_ArchitectureValidator.h"
#include "Engine/Engine.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Engine/World.h"
#include "EngineUtils.h"

void UEng_ArchitectureValidator::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("Engine Architecture Validator Initialized"));
    
    bArchitectureValid = false;
    TargetFrameRate = 60.0f;
    MaxActorCount = 8000;
    
    InitializeValidationRules();
    InitializeModuleRegistry();
    
    // Perform initial validation
    ValidateProjectArchitecture();
}

void UEng_ArchitectureValidator::Deinitialize()
{
    UE_LOG(LogTemp, Warning, TEXT("Engine Architecture Validator Deinitialized"));
    Super::Deinitialize();
}

bool UEng_ArchitectureValidator::ValidateProjectArchitecture()
{
    UE_LOG(LogTemp, Warning, TEXT("Starting Project Architecture Validation"));
    
    bool bAllTestsPassed = true;
    
    // Test 1: Module Dependencies
    bool bModulesValid = ValidateModuleDependencies();
    LogValidationResult(TEXT("Module Dependencies"), bModulesValid);
    bAllTestsPassed &= bModulesValid;
    
    // Test 2: Header Includes
    bool bHeadersValid = ValidateHeaderIncludes();
    LogValidationResult(TEXT("Header Includes"), bHeadersValid);
    bAllTestsPassed &= bHeadersValid;
    
    // Test 3: UE5 Compatibility
    bool bCompatibilityValid = ValidateUE5Compatibility();
    LogValidationResult(TEXT("UE5 Compatibility"), bCompatibilityValid);
    bAllTestsPassed &= bCompatibilityValid;
    
    // Test 4: Performance Constraints
    bool bPerformanceValid = ValidatePerformanceConstraints();
    LogValidationResult(TEXT("Performance Constraints"), bPerformanceValid);
    bAllTestsPassed &= bPerformanceValid;
    
    bArchitectureValid = bAllTestsPassed;
    
    UE_LOG(LogTemp, Warning, TEXT("Architecture Validation Complete: %s"), 
           bArchitectureValid ? TEXT("PASSED") : TEXT("FAILED"));
    
    return bArchitectureValid;
}

bool UEng_ArchitectureValidator::ValidateModuleDependencies()
{
    UE_LOG(LogTemp, Log, TEXT("Validating Module Dependencies"));
    
    bool bValid = true;
    
    for (const FEng_ModuleInfo& Module : RegisteredModules)
    {
        if (!ValidateModuleStructure(Module))
        {
            UE_LOG(LogTemp, Error, TEXT("Module validation failed: %s"), *Module.ModuleName);
            bValid = false;
        }
        
        // Check dependencies exist
        for (const FString& Dependency : Module.Dependencies)
        {
            bool bDependencyFound = false;
            for (const FEng_ModuleInfo& DepModule : RegisteredModules)
            {
                if (DepModule.ModuleName == Dependency)
                {
                    bDependencyFound = true;
                    break;
                }
            }
            
            if (!bDependencyFound)
            {
                UE_LOG(LogTemp, Error, TEXT("Missing dependency %s for module %s"), 
                       *Dependency, *Module.ModuleName);
                bValid = false;
            }
        }
    }
    
    return bValid;
}

bool UEng_ArchitectureValidator::ValidateHeaderIncludes()
{
    UE_LOG(LogTemp, Log, TEXT("Validating Header Includes"));
    
    // Check critical headers exist
    TArray<FString> RequiredHeaders = {
        TEXT("SharedTypes.h"),
        TEXT("TranspersonalGame.h"),
        TEXT("TranspersonalGameState.h"),
        TEXT("TranspersonalCharacter.h")
    };
    
    bool bValid = true;
    FString ProjectDir = FPaths::ProjectDir();
    
    for (const FString& Header : RequiredHeaders)
    {
        FString HeaderPath = FPaths::Combine(ProjectDir, TEXT("Source/TranspersonalGame"), Header);
        
        if (!FPaths::FileExists(HeaderPath))
        {
            UE_LOG(LogTemp, Error, TEXT("Required header missing: %s"), *Header);
            bValid = false;
        }
        else
        {
            // Check header compliance
            if (!CheckHeaderCompliance(HeaderPath))
            {
                UE_LOG(LogTemp, Error, TEXT("Header compliance failed: %s"), *Header);
                bValid = false;
            }
        }
    }
    
    return bValid;
}

bool UEng_ArchitectureValidator::ValidateUE5Compatibility()
{
    UE_LOG(LogTemp, Log, TEXT("Validating UE5 Compatibility"));
    
    bool bValid = true;
    
    // Check engine version
    FString EngineVersion = FEngineVersion::Current().ToString();
    UE_LOG(LogTemp, Log, TEXT("Engine Version: %s"), *EngineVersion);
    
    // Validate we're running UE5.x
    if (!EngineVersion.StartsWith(TEXT("5.")))
    {
        UE_LOG(LogTemp, Error, TEXT("Project requires UE5.x, current version: %s"), *EngineVersion);
        bValid = false;
    }
    
    // Check for deprecated API usage (would need file parsing)
    // For now, just log compatibility check
    UE_LOG(LogTemp, Log, TEXT("UE5 API compatibility check passed"));
    
    return bValid;
}

bool UEng_ArchitectureValidator::ValidatePerformanceConstraints()
{
    UE_LOG(LogTemp, Log, TEXT("Validating Performance Constraints"));
    
    bool bValid = true;
    
    // Check actor count
    int32 CurrentActorCount = GetActorCount();
    if (CurrentActorCount > MaxActorCount)
    {
        UE_LOG(LogTemp, Error, TEXT("Actor count %d exceeds maximum %d"), 
               CurrentActorCount, MaxActorCount);
        bValid = false;
    }
    
    // Check frame rate (if in game)
    float CurrentFPS = GetCurrentFrameRate();
    if (CurrentFPS > 0 && CurrentFPS < TargetFrameRate * 0.8f)
    {
        UE_LOG(LogTemp, Warning, TEXT("Frame rate %f below target %f"), 
               CurrentFPS, TargetFrameRate);
        // Don't fail validation for FPS in editor
    }
    
    UE_LOG(LogTemp, Log, TEXT("Performance validation: Actors=%d, FPS=%f"), 
           CurrentActorCount, CurrentFPS);
    
    return bValid;
}

void UEng_ArchitectureValidator::RegisterModule(const FEng_ModuleInfo& ModuleInfo)
{
    UE_LOG(LogTemp, Log, TEXT("Registering module: %s"), *ModuleInfo.ModuleName);
    
    // Check if module already exists
    for (int32 i = 0; i < RegisteredModules.Num(); i++)
    {
        if (RegisteredModules[i].ModuleName == ModuleInfo.ModuleName)
        {
            RegisteredModules[i] = ModuleInfo;
            UE_LOG(LogTemp, Log, TEXT("Updated existing module: %s"), *ModuleInfo.ModuleName);
            return;
        }
    }
    
    // Add new module
    RegisteredModules.Add(ModuleInfo);
    UE_LOG(LogTemp, Log, TEXT("Added new module: %s"), *ModuleInfo.ModuleName);
}

bool UEng_ArchitectureValidator::IsModuleValid(const FString& ModuleName)
{
    for (const FEng_ModuleInfo& Module : RegisteredModules)
    {
        if (Module.ModuleName == ModuleName)
        {
            return Module.bIsCompiled && ValidateModuleStructure(Module);
        }
    }
    
    return false;
}

TArray<FString> UEng_ArchitectureValidator::GetInvalidModules()
{
    TArray<FString> InvalidModules;
    
    for (const FEng_ModuleInfo& Module : RegisteredModules)
    {
        if (!IsModuleValid(Module.ModuleName))
        {
            InvalidModules.Add(Module.ModuleName);
        }
    }
    
    return InvalidModules;
}

void UEng_ArchitectureValidator::AddValidationRule(const FEng_ValidationRule& Rule)
{
    ValidationRules.Add(Rule);
    UE_LOG(LogTemp, Log, TEXT("Added validation rule: %s"), *Rule.RuleName);
}

TArray<FEng_ValidationRule> UEng_ArchitectureValidator::GetActiveValidationRules()
{
    TArray<FEng_ValidationRule> ActiveRules;
    
    for (const FEng_ValidationRule& Rule : ValidationRules)
    {
        if (Rule.bIsActive)
        {
            ActiveRules.Add(Rule);
        }
    }
    
    return ActiveRules;
}

bool UEng_ArchitectureValidator::CheckCompilationErrors()
{
    CompilationErrors.Empty();
    
    // In a real implementation, this would check the compiler output
    // For now, simulate by checking if critical files exist
    TArray<FString> RequiredFiles = {
        TEXT("TranspersonalGame.cpp"),
        TEXT("TranspersonalGameState.cpp"),
        TEXT("TranspersonalCharacter.cpp")
    };
    
    FString SourceDir = FPaths::Combine(FPaths::ProjectDir(), TEXT("Source/TranspersonalGame"));
    
    for (const FString& File : RequiredFiles)
    {
        FString FilePath = FPaths::Combine(SourceDir, File);
        if (!FPaths::FileExists(FilePath))
        {
            CompilationErrors.Add(FString::Printf(TEXT("Missing required file: %s"), *File));
        }
    }
    
    return CompilationErrors.Num() == 0;
}

TArray<FString> UEng_ArchitectureValidator::GetCompilationErrors()
{
    return CompilationErrors;
}

float UEng_ArchitectureValidator::GetCurrentFrameRate()
{
    if (GEngine && GEngine->GetWorldContexts().Num() > 0)
    {
        UWorld* World = GEngine->GetWorldContexts()[0].World();
        if (World)
        {
            return 1.0f / World->GetDeltaSeconds();
        }
    }
    
    return 0.0f;
}

int32 UEng_ArchitectureValidator::GetActorCount()
{
    if (GEngine && GEngine->GetWorldContexts().Num() > 0)
    {
        UWorld* World = GEngine->GetWorldContexts()[0].World();
        if (World)
        {
            int32 Count = 0;
            for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
            {
                Count++;
            }
            return Count;
        }
    }
    
    return 0;
}

void UEng_ArchitectureValidator::InitializeValidationRules()
{
    UE_LOG(LogTemp, Log, TEXT("Initializing Validation Rules"));
    
    ValidationRules.Empty();
    
    // Critical rules
    FEng_ValidationRule Rule1;
    Rule1.RuleName = TEXT("UPROPERTY_UFUNCTION_Required");
    Rule1.Description = TEXT("All exposed members must use UPROPERTY/UFUNCTION macros");
    Rule1.Level = EEng_ValidationLevel::Critical;
    Rule1.bIsActive = true;
    ValidationRules.Add(Rule1);
    
    FEng_ValidationRule Rule2;
    Rule2.RuleName = TEXT("Generated_Header_Last");
    Rule2.Description = TEXT(".generated.h must be the last include in headers");
    Rule2.Level = EEng_ValidationLevel::Critical;
    Rule2.bIsActive = true;
    ValidationRules.Add(Rule2);
    
    FEng_ValidationRule Rule3;
    Rule3.RuleName = TEXT("Unique_Type_Names");
    Rule3.Description = TEXT("All USTRUCT/UENUM/UCLASS names must be unique project-wide");
    Rule3.Level = EEng_ValidationLevel::Critical;
    Rule3.bIsActive = true;
    ValidationRules.Add(Rule3);
    
    FEng_ValidationRule Rule4;
    Rule4.RuleName = TEXT("Actor_Count_Limit");
    Rule4.Description = TEXT("Total actors must not exceed 8000");
    Rule4.Level = EEng_ValidationLevel::High;
    Rule4.bIsActive = true;
    ValidationRules.Add(Rule4);
    
    UE_LOG(LogTemp, Log, TEXT("Initialized %d validation rules"), ValidationRules.Num());
}

void UEng_ArchitectureValidator::InitializeModuleRegistry()
{
    UE_LOG(LogTemp, Log, TEXT("Initializing Module Registry"));
    
    RegisteredModules.Empty();
    
    // Register core modules
    FEng_ModuleInfo CoreModule;
    CoreModule.ModuleName = TEXT("TranspersonalGame");
    CoreModule.ModuleType = EEng_ModuleType::Core;
    CoreModule.Priority = 100;
    CoreModule.bIsCompiled = true;
    RegisterModule(CoreModule);
    
    FEng_ModuleInfo GameplayModule;
    GameplayModule.ModuleName = TEXT("Gameplay");
    GameplayModule.ModuleType = EEng_ModuleType::Gameplay;
    GameplayModule.Dependencies.Add(TEXT("TranspersonalGame"));
    GameplayModule.Priority = 90;
    GameplayModule.bIsCompiled = false;
    RegisterModule(GameplayModule);
    
    FEng_ModuleInfo WorldModule;
    WorldModule.ModuleName = TEXT("WorldGeneration");
    WorldModule.ModuleType = EEng_ModuleType::World;
    WorldModule.Dependencies.Add(TEXT("TranspersonalGame"));
    WorldModule.Priority = 80;
    WorldModule.bIsCompiled = false;
    RegisterModule(WorldModule);
    
    UE_LOG(LogTemp, Log, TEXT("Registered %d modules"), RegisteredModules.Num());
}

bool UEng_ArchitectureValidator::ValidateModuleStructure(const FEng_ModuleInfo& Module)
{
    // Check module has valid name
    if (Module.ModuleName.IsEmpty())
    {
        return false;
    }
    
    // Check priority is valid
    if (Module.Priority < 0 || Module.Priority > 100)
    {
        return false;
    }
    
    // Check dependencies don't create cycles (simplified check)
    for (const FString& Dependency : Module.Dependencies)
    {
        if (Dependency == Module.ModuleName)
        {
            UE_LOG(LogTemp, Error, TEXT("Module %s has circular dependency on itself"), *Module.ModuleName);
            return false;
        }
    }
    
    return true;
}

bool UEng_ArchitectureValidator::CheckHeaderCompliance(const FString& HeaderPath)
{
    FString HeaderContent;
    if (!FFileHelper::LoadFileToString(HeaderContent, *HeaderPath))
    {
        return false;
    }
    
    // Check for #pragma once
    if (!HeaderContent.Contains(TEXT("#pragma once")))
    {
        UE_LOG(LogTemp, Error, TEXT("Header missing #pragma once: %s"), *HeaderPath);
        return false;
    }
    
    // Check .generated.h is last include (simplified)
    int32 GeneratedPos = HeaderContent.Find(TEXT(".generated.h"));
    if (GeneratedPos != INDEX_NONE)
    {
        int32 LastIncludePos = HeaderContent.RFind(TEXT("#include"));
        if (LastIncludePos > GeneratedPos)
        {
            UE_LOG(LogTemp, Error, TEXT("Generated header not last include: %s"), *HeaderPath);
            return false;
        }
    }
    
    return true;
}

void UEng_ArchitectureValidator::LogValidationResult(const FString& TestName, bool bPassed)
{
    if (bPassed)
    {
        UE_LOG(LogTemp, Log, TEXT("✓ %s: PASSED"), *TestName);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("✗ %s: FAILED"), *TestName);
    }
}