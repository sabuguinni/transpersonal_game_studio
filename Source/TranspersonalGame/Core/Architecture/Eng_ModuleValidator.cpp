#include "Eng_ModuleValidator.h"
#include "Engine/Engine.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/Paths.h"

void UEng_ModuleValidator::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    ModuleValidationCache.Empty();
    OverallArchitectureScore = 0.0f;
    
    UE_LOG(LogTemp, Log, TEXT("Eng_ModuleValidator: Module validation subsystem initialized"));
}

void UEng_ModuleValidator::Deinitialize()
{
    ModuleValidationCache.Empty();
    
    Super::Deinitialize();
}

FEng_ModuleValidationResult UEng_ModuleValidator::ValidateModule(const FString& ModuleName)
{
    // Check cache first
    if (ModuleValidationCache.Contains(ModuleName))
    {
        return ModuleValidationCache[ModuleName];
    }
    
    FEng_ModuleValidationResult Result;
    Result.ModuleName = ModuleName;
    Result.bIsValid = true;
    Result.ValidationScore = 0.0f;
    
    // Validate module structure
    if (!ValidateModuleStructure(ModuleName, Result))
    {
        Result.bIsValid = false;
    }
    
    // Validate dependencies
    if (!ValidateModuleDependencies(ModuleName, Result))
    {
        Result.bIsValid = false;
    }
    
    // Validate performance characteristics
    if (!ValidateModulePerformance(ModuleName, Result))
    {
        Result.ValidationWarnings.Add(TEXT("Performance validation incomplete"));
    }
    
    // Calculate final score
    float BaseScore = Result.bIsValid ? 70.0f : 30.0f;
    float ErrorPenalty = Result.ValidationErrors.Num() * 5.0f;
    float WarningPenalty = Result.ValidationWarnings.Num() * 2.0f;
    
    Result.ValidationScore = FMath::Max(0.0f, BaseScore - ErrorPenalty - WarningPenalty);
    
    // Cache result
    ModuleValidationCache.Add(ModuleName, Result);
    
    UE_LOG(LogTemp, Log, TEXT("Eng_ModuleValidator: Module %s validated - Score: %.1f"), 
           *ModuleName, Result.ValidationScore);
    
    return Result;
}

TArray<FEng_ModuleValidationResult> UEng_ModuleValidator::ValidateAllModules()
{
    TArray<FEng_ModuleValidationResult> AllResults;
    
    // Core modules to validate
    TArray<FString> CoreModules = {
        TEXT("TranspersonalGame"),
        TEXT("Core"),
        TEXT("Architecture"),
        TEXT("WorldGeneration"),
        TEXT("Character"),
        TEXT("AI"),
        TEXT("Physics")
    };
    
    for (const FString& ModuleName : CoreModules)
    {
        FEng_ModuleValidationResult Result = ValidateModule(ModuleName);
        AllResults.Add(Result);
    }
    
    UpdateArchitectureScore();
    
    return AllResults;
}

bool UEng_ModuleValidator::IsModuleArchitectureCompliant(const FString& ModuleName)
{
    FEng_ModuleValidationResult Result = ValidateModule(ModuleName);
    return Result.bIsValid && Result.ValidationScore >= 75.0f;
}

void UEng_ModuleValidator::GenerateArchitectureReport()
{
    UE_LOG(LogTemp, Warning, TEXT("=== ARCHITECTURE VALIDATION REPORT ==="));
    
    TArray<FEng_ModuleValidationResult> Results = ValidateAllModules();
    
    int32 ValidModules = 0;
    float TotalScore = 0.0f;
    
    for (const FEng_ModuleValidationResult& Result : Results)
    {
        if (Result.bIsValid)
        {
            ValidModules++;
        }
        
        TotalScore += Result.ValidationScore;
        
        UE_LOG(LogTemp, Warning, TEXT("Module: %s | Valid: %s | Score: %.1f"), 
               *Result.ModuleName, 
               Result.bIsValid ? TEXT("YES") : TEXT("NO"), 
               Result.ValidationScore);
        
        for (const FString& Error : Result.ValidationErrors)
        {
            UE_LOG(LogTemp, Error, TEXT("  ERROR: %s"), *Error);
        }
        
        for (const FString& Warning : Result.ValidationWarnings)
        {
            UE_LOG(LogTemp, Warning, TEXT("  WARNING: %s"), *Warning);
        }
    }
    
    float AverageScore = Results.Num() > 0 ? TotalScore / Results.Num() : 0.0f;
    
    UE_LOG(LogTemp, Warning, TEXT("=== SUMMARY ==="));
    UE_LOG(LogTemp, Warning, TEXT("Valid Modules: %d/%d"), ValidModules, Results.Num());
    UE_LOG(LogTemp, Warning, TEXT("Average Score: %.1f/100"), AverageScore);
    UE_LOG(LogTemp, Warning, TEXT("Overall Architecture Score: %.1f/100"), OverallArchitectureScore);
    UE_LOG(LogTemp, Warning, TEXT("=== END REPORT ==="));
}

bool UEng_ModuleValidator::ValidateModuleStructure(const FString& ModuleName, FEng_ModuleValidationResult& Result)
{
    bool bStructureValid = true;
    
    // Check for required files (simplified validation)
    if (ModuleName == TEXT("TranspersonalGame"))
    {
        // Core module should have basic structure
        Result.ValidationScore += 20.0f;
    }
    else if (ModuleName == TEXT("Architecture"))
    {
        // Architecture module validation
        Result.ValidationScore += 25.0f;
    }
    else
    {
        // Generic module validation
        Result.ValidationScore += 15.0f;
        Result.ValidationWarnings.Add(FString::Printf(TEXT("Generic validation for module %s"), *ModuleName));
    }
    
    return bStructureValid;
}

bool UEng_ModuleValidator::ValidateModuleDependencies(const FString& ModuleName, FEng_ModuleValidationResult& Result)
{
    bool bDependenciesValid = true;
    
    // Validate that modules don't have circular dependencies
    // This is a simplified check - in a real implementation, we'd parse Build.cs files
    
    if (ModuleName == TEXT("Core") || ModuleName == TEXT("Architecture"))
    {
        // Core modules should have minimal dependencies
        Result.ValidationScore += 15.0f;
    }
    else
    {
        // Other modules can depend on core modules
        Result.ValidationScore += 10.0f;
    }
    
    return bDependenciesValid;
}

bool UEng_ModuleValidator::ValidateModulePerformance(const FString& ModuleName, FEng_ModuleValidationResult& Result)
{
    // Performance validation would check:
    // - Memory usage patterns
    // - CPU usage
    // - Rendering impact
    // - Network usage (if applicable)
    
    Result.ValidationScore += 10.0f;
    Result.ValidationWarnings.Add(TEXT("Performance validation is simplified in this implementation"));
    
    return true;
}

void UEng_ModuleValidator::UpdateArchitectureScore()
{
    float TotalScore = 0.0f;
    int32 ModuleCount = 0;
    
    for (const auto& CachePair : ModuleValidationCache)
    {
        TotalScore += CachePair.Value.ValidationScore;
        ModuleCount++;
    }
    
    OverallArchitectureScore = ModuleCount > 0 ? TotalScore / ModuleCount : 0.0f;
    
    UE_LOG(LogTemp, Log, TEXT("Eng_ModuleValidator: Overall architecture score updated to %.1f"), 
           OverallArchitectureScore);
}