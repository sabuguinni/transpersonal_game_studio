#include "Eng_CompilationRules.h"
#include "Engine/Engine.h"
#include "Misc/Paths.h"
#include "HAL/FileManager.h"

UEng_CompilationRules::UEng_CompilationRules()
{
    InitializeStandardRules();
}

void UEng_CompilationRules::InitializeStandardRules()
{
    CompilationRules.Empty();
    
    SetupCoreRules();
    SetupHeaderRules();
    SetupMacroRules();
    SetupTypeRules();
    SetupModuleRules();
    SetupNamingRules();
    SetupMemoryRules();
    SetupBlueprintRules();
    
    UE_LOG(LogTemp, Log, TEXT("Engine Architect: Initialized %d compilation rules"), CompilationRules.Num());
}

void UEng_CompilationRules::SetupCoreRules()
{
    // Rule 1: USTRUCT/UENUM at global scope only
    FEng_CompilationRule Rule1;
    Rule1.RuleID = TEXT("CORE_001");
    Rule1.RuleName = TEXT("Global Scope Types");
    Rule1.Description = TEXT("USTRUCT() and UENUM() must be declared at global scope, not nested inside classes");
    Rule1.Severity = EEng_CompilationSeverity::Error;
    Rule1.Category = EEng_RuleCategory::TypeDefinitions;
    Rule1.Example = TEXT("USTRUCT() struct FMyStruct { GENERATED_BODY() }; // CORRECT at global scope");
    Rule1.bIsEnforced = true;
    CompilationRules.Add(Rule1);

    // Rule 2: Unique type names
    FEng_CompilationRule Rule2;
    Rule2.RuleID = TEXT("CORE_002");
    Rule2.RuleName = TEXT("Unique Type Names");
    Rule2.Description = TEXT("Every USTRUCT, UENUM, UCLASS name must be unique across entire project");
    Rule2.Severity = EEng_CompilationSeverity::Critical;
    Rule2.Category = EEng_RuleCategory::NamingConventions;
    Rule2.Example = TEXT("Use agent prefix: FEng_MyStruct, EEng_MyEnum, UEng_MyClass");
    Rule2.bIsEnforced = true;
    CompilationRules.Add(Rule2);

    // Rule 3: Never redefine engine types
    FEng_CompilationRule Rule3;
    Rule3.RuleID = TEXT("CORE_003");
    Rule3.RuleName = TEXT("No Engine Type Redefinition");
    Rule3.Description = TEXT("Do not create types that already exist in UE5 Engine");
    Rule3.Severity = EEng_CompilationSeverity::Critical;
    Rule3.Category = EEng_RuleCategory::TypeDefinitions;
    Rule3.Example = TEXT("Use EEng_CollisionChannel instead of ECollisionChannel");
    Rule3.bIsEnforced = true;
    CompilationRules.Add(Rule3);
}

void UEng_CompilationRules::SetupHeaderRules()
{
    // Rule 4: .generated.h must be last include
    FEng_CompilationRule Rule4;
    Rule4.RuleID = TEXT("HEADER_001");
    Rule4.RuleName = TEXT("Generated Include Last");
    Rule4.Description = TEXT(".generated.h must be the last include in header files");
    Rule4.Severity = EEng_CompilationSeverity::Error;
    Rule4.Category = EEng_RuleCategory::HeaderStructure;
    Rule4.Example = TEXT("#include \"MyClass.generated.h\" // Must be last");
    Rule4.bIsEnforced = true;
    CompilationRules.Add(Rule4);

    // Rule 5: Pragma once
    FEng_CompilationRule Rule5;
    Rule5.RuleID = TEXT("HEADER_002");
    Rule5.RuleName = TEXT("Pragma Once Required");
    Rule5.Description = TEXT("All header files must start with #pragma once");
    Rule5.Severity = EEng_CompilationSeverity::Error;
    Rule5.Category = EEng_RuleCategory::HeaderStructure;
    Rule5.Example = TEXT("#pragma once // First line of header");
    Rule5.bIsEnforced = true;
    CompilationRules.Add(Rule5);
}

void UEng_CompilationRules::SetupMacroRules()
{
    // Rule 6: No escaped quotes in macros
    FEng_CompilationRule Rule6;
    Rule6.RuleID = TEXT("MACRO_001");
    Rule6.RuleName = TEXT("No Escaped Quotes");
    Rule6.Description = TEXT("Use normal quotes in UPROPERTY/UFUNCTION macros, not escaped quotes");
    Rule6.Severity = EEng_CompilationSeverity::Error;
    Rule6.Category = EEng_RuleCategory::MacroUsage;
    Rule6.Example = TEXT("Category = \"Physics\" // CORRECT, not \\\"Physics\\\"");
    Rule6.bIsEnforced = true;
    CompilationRules.Add(Rule6);

    // Rule 7: CallInEditor is bare flag
    FEng_CompilationRule Rule7;
    Rule7.RuleID = TEXT("MACRO_002");
    Rule7.RuleName = TEXT("CallInEditor Bare Flag");
    Rule7.Description = TEXT("CallInEditor should be used as bare flag, not with = true");
    Rule7.Severity = EEng_CompilationSeverity::Warning;
    Rule7.Category = EEng_RuleCategory::MacroUsage;
    Rule7.Example = TEXT("UFUNCTION(CallInEditor) // CORRECT, not CallInEditor = true");
    Rule7.bIsEnforced = true;
    CompilationRules.Add(Rule7);
}

void UEng_CompilationRules::SetupTypeRules()
{
    // Rule 8: One GENERATED_BODY per type
    FEng_CompilationRule Rule8;
    Rule8.RuleID = TEXT("TYPE_001");
    Rule8.RuleName = TEXT("Single Generated Body");
    Rule8.Description = TEXT("Only one GENERATED_BODY() per UCLASS/USTRUCT, none for plain C++ structs");
    Rule8.Severity = EEng_CompilationSeverity::Error;
    Rule8.Category = EEng_RuleCategory::TypeDefinitions;
    Rule8.Example = TEXT("USTRUCT() struct FMyStruct { GENERATED_BODY() }; // One only");
    Rule8.bIsEnforced = true;
    CompilationRules.Add(Rule8);

    // Rule 9: API export on public classes
    FEng_CompilationRule Rule9;
    Rule9.RuleID = TEXT("TYPE_002");
    Rule9.RuleName = TEXT("API Export Required");
    Rule9.Description = TEXT("Use TRANSPERSONALGAME_API on all exported classes");
    Rule9.Severity = EEng_CompilationSeverity::Error;
    Rule9.Category = EEng_RuleCategory::ModuleDependencies;
    Rule9.Example = TEXT("UCLASS() class TRANSPERSONALGAME_API UMyClass : public UObject");
    Rule9.bIsEnforced = true;
    CompilationRules.Add(Rule9);
}

void UEng_CompilationRules::SetupModuleRules()
{
    // Rule 10: Module dependencies in Build.cs
    FEng_CompilationRule Rule10;
    Rule10.RuleID = TEXT("MODULE_001");
    Rule10.RuleName = TEXT("Build.cs Dependencies");
    Rule10.Description = TEXT("Every cross-module include requires module in Build.cs PublicDependencyModules");
    Rule10.Severity = EEng_CompilationSeverity::Error;
    Rule10.Category = EEng_RuleCategory::ModuleDependencies;
    Rule10.Example = TEXT("PublicDependencyModules.AddRange(new string[] { \"Core\", \"CoreUObject\", \"Engine\" });");
    Rule10.bIsEnforced = true;
    CompilationRules.Add(Rule10);
}

void UEng_CompilationRules::SetupNamingRules()
{
    // Rule 11: No spaces in identifiers
    FEng_CompilationRule Rule11;
    Rule11.RuleID = TEXT("NAMING_001");
    Rule11.RuleName = TEXT("No Spaces in Names");
    Rule11.Description = TEXT("Variable and function names cannot contain spaces");
    Rule11.Severity = EEng_CompilationSeverity::Error;
    Rule11.Category = EEng_RuleCategory::NamingConventions;
    Rule11.Example = TEXT("bool bIsOnUnevenTerrain; // CORRECT, not 'bIsOnUneven Terrain'");
    Rule11.bIsEnforced = true;
    CompilationRules.Add(Rule11);

    // Rule 12: Agent prefixes
    FEng_CompilationRule Rule12;
    Rule12.RuleID = TEXT("NAMING_002");
    Rule12.RuleName = TEXT("Agent Prefixes Required");
    Rule12.Description = TEXT("Use agent-specific prefixes to prevent naming conflicts");
    Rule12.Severity = EEng_CompilationSeverity::Warning;
    Rule12.Category = EEng_RuleCategory::NamingConventions;
    Rule12.Example = TEXT("Engine Architect uses Eng_ prefix: UEng_CompilationRules");
    Rule12.bIsEnforced = true;
    CompilationRules.Add(Rule12);
}

void UEng_CompilationRules::SetupMemoryRules()
{
    // Rule 13: UPROPERTY for GC protection
    FEng_CompilationRule Rule13;
    Rule13.RuleID = TEXT("MEMORY_001");
    Rule13.RuleName = TEXT("UPROPERTY GC Protection");
    Rule13.Description = TEXT("Use UPROPERTY() on UObject pointers for garbage collection protection");
    Rule13.Severity = EEng_CompilationSeverity::Warning;
    Rule13.Category = EEng_RuleCategory::MemoryManagement;
    Rule13.Example = TEXT("UPROPERTY() UMyComponent* ComponentRef; // GC protected");
    Rule13.bIsEnforced = true;
    CompilationRules.Add(Rule13);

    // Rule 14: Forward declarations
    FEng_CompilationRule Rule14;
    Rule14.RuleID = TEXT("MEMORY_002");
    Rule14.RuleName = TEXT("Forward Declarations");
    Rule14.Description = TEXT("Use forward declarations for cross-module types without module dependency");
    Rule14.Severity = EEng_CompilationSeverity::Info;
    Rule14.Category = EEng_RuleCategory::HeaderStructure;
    Rule14.Example = TEXT("class UBiomeManager; // Forward declare, raw pointer only");
    Rule14.bIsEnforced = false;
    CompilationRules.Add(Rule14);
}

void UEng_CompilationRules::SetupBlueprintRules()
{
    // Rule 15: BlueprintReadOnly private access
    FEng_CompilationRule Rule15;
    Rule15.RuleID = TEXT("BLUEPRINT_001");
    Rule15.RuleName = TEXT("Private Access Meta");
    Rule15.Description = TEXT("Add AllowPrivateAccess meta for BlueprintReadOnly on private members");
    Rule15.Severity = EEng_CompilationSeverity::Warning;
    Rule15.Category = EEng_RuleCategory::BlueprintIntegration;
    Rule15.Example = TEXT("UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = \"true\"))");
    Rule15.bIsEnforced = true;
    CompilationRules.Add(Rule15);
}

FEng_ValidationResult UEng_CompilationRules::ValidateHeaderFile(const FString& HeaderContent)
{
    FEng_ValidationResult Result;
    Result.TotalRulesChecked = 0;
    Result.RulesPassed = 0;

    // Check pragma once
    if (!HeaderContent.StartsWith(TEXT("#pragma once")))
    {
        Result.Errors.Add(TEXT("HEADER_002: Missing #pragma once at start of file"));
    }
    else
    {
        Result.RulesPassed++;
    }
    Result.TotalRulesChecked++;

    // Check .generated.h include order
    if (!CheckGeneratedInclude(HeaderContent))
    {
        Result.Errors.Add(TEXT("HEADER_001: .generated.h must be the last include"));
    }
    else
    {
        Result.RulesPassed++;
    }
    Result.TotalRulesChecked++;

    // Check USTRUCT placement
    if (!CheckUSTRUCTPlacement(HeaderContent))
    {
        Result.Errors.Add(TEXT("CORE_001: USTRUCT/UENUM must be at global scope"));
    }
    else
    {
        Result.RulesPassed++;
    }
    Result.TotalRulesChecked++;

    // Check API export
    if (!ValidateAPIExportRule(HeaderContent))
    {
        Result.Warnings.Add(TEXT("TYPE_002: Consider adding TRANSPERSONALGAME_API to exported classes"));
    }
    else
    {
        Result.RulesPassed++;
    }
    Result.TotalRulesChecked++;

    Result.bIsValid = Result.Errors.Num() == 0;
    return Result;
}

FEng_ValidationResult UEng_CompilationRules::ValidateSourceFile(const FString& SourceContent)
{
    FEng_ValidationResult Result;
    Result.TotalRulesChecked = 0;
    Result.RulesPassed = 0;

    // Check header include
    if (!SourceContent.Contains(TEXT("#include \"")))
    {
        Result.Errors.Add(TEXT("Missing header include in source file"));
    }
    else
    {
        Result.RulesPassed++;
    }
    Result.TotalRulesChecked++;

    // Check for implementation completeness
    int32 UFunctionCount = 0;
    int32 ImplementationCount = 0;
    
    // Count UFUNCTION declarations (simplified check)
    FString SearchString = TEXT("UFUNCTION");
    int32 SearchIndex = 0;
    while ((SearchIndex = SourceContent.Find(SearchString, ESearchCase::IgnoreCase, ESearchDir::FromStart, SearchIndex)) != INDEX_NONE)
    {
        UFunctionCount++;
        SearchIndex += SearchString.Len();
    }

    // Count function implementations (simplified check for :: operator)
    SearchString = TEXT("::");
    SearchIndex = 0;
    while ((SearchIndex = SourceContent.Find(SearchString, ESearchCase::IgnoreCase, ESearchDir::FromStart, SearchIndex)) != INDEX_NONE)
    {
        ImplementationCount++;
        SearchIndex += SearchString.Len();
    }

    if (UFunctionCount > 0 && ImplementationCount == 0)
    {
        Result.Warnings.Add(TEXT("UFUNCTION declarations found but no implementations detected"));
    }
    else
    {
        Result.RulesPassed++;
    }
    Result.TotalRulesChecked++;

    Result.bIsValid = Result.Errors.Num() == 0;
    return Result;
}

FEng_ValidationResult UEng_CompilationRules::ValidateModuleDependencies(const TArray<FString>& Dependencies)
{
    FEng_ValidationResult Result;
    Result.TotalRulesChecked = 1;
    Result.RulesPassed = 0;

    // Check for core dependencies
    TArray<FString> RequiredModules = { TEXT("Core"), TEXT("CoreUObject"), TEXT("Engine") };
    bool bHasAllRequired = true;

    for (const FString& Required : RequiredModules)
    {
        if (!Dependencies.Contains(Required))
        {
            Result.Errors.Add(FString::Printf(TEXT("MODULE_001: Missing required module dependency: %s"), *Required));
            bHasAllRequired = false;
        }
    }

    if (bHasAllRequired)
    {
        Result.RulesPassed = 1;
    }

    Result.bIsValid = Result.Errors.Num() == 0;
    return Result;
}

bool UEng_CompilationRules::CheckUSTRUCTPlacement(const FString& FileContent)
{
    // Simplified check: USTRUCT should not appear inside class definitions
    TArray<FString> Lines;
    FileContent.ParseIntoArrayLines(Lines);

    bool bInsideClass = false;
    for (const FString& Line : Lines)
    {
        FString TrimmedLine = Line.TrimStartAndEnd();
        
        if (TrimmedLine.StartsWith(TEXT("class ")) || TrimmedLine.StartsWith(TEXT("UCLASS")))
        {
            bInsideClass = true;
        }
        else if (TrimmedLine.StartsWith(TEXT("};")) && bInsideClass)
        {
            bInsideClass = false;
        }
        else if (bInsideClass && (TrimmedLine.StartsWith(TEXT("USTRUCT")) || TrimmedLine.StartsWith(TEXT("UENUM"))))
        {
            return false; // Found USTRUCT/UENUM inside class
        }
    }
    
    return true;
}

bool UEng_CompilationRules::CheckGeneratedInclude(const FString& FileContent)
{
    // Find the last #include line and check if it's .generated.h
    TArray<FString> Lines;
    FileContent.ParseIntoArrayLines(Lines);

    int32 LastIncludeIndex = -1;
    for (int32 i = Lines.Num() - 1; i >= 0; i--)
    {
        if (Lines[i].TrimStartAndEnd().StartsWith(TEXT("#include")))
        {
            LastIncludeIndex = i;
            break;
        }
    }

    if (LastIncludeIndex >= 0)
    {
        return Lines[LastIncludeIndex].Contains(TEXT(".generated.h"));
    }

    return true; // No includes found, rule doesn't apply
}

bool UEng_CompilationRules::CheckNamingConventions(const FString& ClassName)
{
    // Check for spaces in class name
    if (ClassName.Contains(TEXT(" ")))
    {
        return false;
    }

    // Check for agent prefix (Eng_ for Engine Architect)
    if (ClassName.StartsWith(TEXT("UEng_")) || ClassName.StartsWith(TEXT("FEng_")) || ClassName.StartsWith(TEXT("EEng_")))
    {
        return true;
    }

    // Allow standard UE5 prefixes for base classes
    if (ClassName.StartsWith(TEXT("U")) || ClassName.StartsWith(TEXT("A")) || ClassName.StartsWith(TEXT("F")) || ClassName.StartsWith(TEXT("E")))
    {
        return true;
    }

    return false;
}

TArray<FString> UEng_CompilationRules::GetMandatoryIncludes()
{
    return {
        TEXT("#pragma once"),
        TEXT("#include \"CoreMinimal.h\""),
        TEXT("#include \"Engine/Engine.h\""),
        TEXT("#include \"UObject/NoExportTypes.h\"")
    };
}

TArray<FString> UEng_CompilationRules::GetForbiddenPatterns()
{
    return {
        TEXT("\\\""), // Escaped quotes
        TEXT("CallInEditor = true"), // Should be bare flag
        TEXT("find_class"), // Doesn't exist in UE5
        TEXT("SystemLibrary.get_project_name"), // Doesn't exist
        TEXT("#if 0") // Disables code compilation
    };
}

void UEng_CompilationRules::EnforceRule(const FString& RuleID, bool bEnforce)
{
    for (FEng_CompilationRule& Rule : CompilationRules)
    {
        if (Rule.RuleID == RuleID)
        {
            Rule.bIsEnforced = bEnforce;
            UE_LOG(LogTemp, Log, TEXT("Engine Architect: Rule %s enforcement set to %s"), *RuleID, bEnforce ? TEXT("true") : TEXT("false"));
            return;
        }
    }
    UE_LOG(LogTemp, Warning, TEXT("Engine Architect: Rule %s not found"), *RuleID);
}

bool UEng_CompilationRules::IsRuleEnforced(const FString& RuleID)
{
    for (const FEng_CompilationRule& Rule : CompilationRules)
    {
        if (Rule.RuleID == RuleID)
        {
            return Rule.bIsEnforced;
        }
    }
    return false;
}

FString UEng_CompilationRules::GenerateValidationReport(const FEng_ValidationResult& Result)
{
    FString Report = TEXT("=== ENGINE ARCHITECT VALIDATION REPORT ===\n");
    Report += FString::Printf(TEXT("Overall Status: %s\n"), Result.bIsValid ? TEXT("PASS") : TEXT("FAIL"));
    Report += FString::Printf(TEXT("Rules Checked: %d\n"), Result.TotalRulesChecked);
    Report += FString::Printf(TEXT("Rules Passed: %d\n"), Result.RulesPassed);
    Report += FString::Printf(TEXT("Success Rate: %.1f%%\n"), Result.TotalRulesChecked > 0 ? (float)Result.RulesPassed / Result.TotalRulesChecked * 100.0f : 0.0f);

    if (Result.Errors.Num() > 0)
    {
        Report += TEXT("\nERRORS:\n");
        for (const FString& Error : Result.Errors)
        {
            Report += FString::Printf(TEXT("  ✗ %s\n"), *Error);
        }
    }

    if (Result.Warnings.Num() > 0)
    {
        Report += TEXT("\nWARNINGS:\n");
        for (const FString& Warning : Result.Warnings)
        {
            Report += FString::Printf(TEXT("  ⚠ %s\n"), *Warning);
        }
    }

    if (Result.Info.Num() > 0)
    {
        Report += TEXT("\nINFO:\n");
        for (const FString& Info : Result.Info)
        {
            Report += FString::Printf(TEXT("  ℹ %s\n"), *Info);
        }
    }

    Report += TEXT("=== END VALIDATION REPORT ===");
    return Report;
}

void UEng_CompilationRules::LogValidationResult(const FEng_ValidationResult& Result)
{
    FString Report = GenerateValidationReport(Result);
    
    if (Result.bIsValid)
    {
        UE_LOG(LogTemp, Log, TEXT("%s"), *Report);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("%s"), *Report);
    }
}

bool UEng_CompilationRules::ValidateUSTRUCTRule(const FString& Content)
{
    return CheckUSTRUCTPlacement(Content);
}

bool UEng_CompilationRules::ValidateGeneratedBodyRule(const FString& Content)
{
    // Count GENERATED_BODY occurrences
    int32 Count = 0;
    int32 SearchIndex = 0;
    FString SearchString = TEXT("GENERATED_BODY");
    
    while ((SearchIndex = Content.Find(SearchString, ESearchCase::IgnoreCase, ESearchDir::FromStart, SearchIndex)) != INDEX_NONE)
    {
        Count++;
        SearchIndex += SearchString.Len();
    }
    
    // Should have exactly one GENERATED_BODY per UCLASS/USTRUCT
    return Count <= 1;
}

bool UEng_CompilationRules::ValidateIncludeOrderRule(const FString& Content)
{
    return CheckGeneratedInclude(Content);
}

bool UEng_CompilationRules::ValidateAPIExportRule(const FString& Content)
{
    // Check if UCLASS exists and has TRANSPERSONALGAME_API
    if (Content.Contains(TEXT("UCLASS")))
    {
        return Content.Contains(TEXT("TRANSPERSONALGAME_API"));
    }
    return true; // No UCLASS, rule doesn't apply
}

bool UEng_CompilationRules::ValidateForwardDeclarationRule(const FString& Content)
{
    // This is an informational rule, always passes
    return true;
}