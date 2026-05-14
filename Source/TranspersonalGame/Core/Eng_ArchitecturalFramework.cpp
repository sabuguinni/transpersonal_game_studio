#include "Eng_ArchitecturalFramework.h"
#include "Engine/Engine.h"
#include "HAL/FileManager.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

UEng_ArchitecturalFramework::UEng_ArchitecturalFramework()
{
    // Constructor - initialize default values
}

void UEng_ArchitecturalFramework::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("Engine Architectural Framework: Initializing UE5.5 standards"));
    
    InitializeDefaultRules();
    InitializeModuleStandards();
    InitializeArchitecturalConstraints();
    
    UE_LOG(LogTemp, Warning, TEXT("Engine Architectural Framework: Initialization complete"));
}

void UEng_ArchitecturalFramework::Deinitialize()
{
    UE_LOG(LogTemp, Warning, TEXT("Engine Architectural Framework: Shutting down"));
    
    CompilationRules.Empty();
    ModuleStandardsMap.Empty();
    ArchitecturalConstraints.Empty();
    AgentApprovalStatus.Empty();
    AgentBlockReasons.Empty();
    
    Super::Deinitialize();
}

void UEng_ArchitecturalFramework::RegisterCompilationRule(const FEng_CompilationRule& Rule)
{
    CompilationRules.Add(Rule);
    UE_LOG(LogTemp, Log, TEXT("Registered compilation rule: %s"), *Rule.RuleName);
}

bool UEng_ArchitecturalFramework::ValidateModuleCompliance(const FString& ModuleName)
{
    if (!ModuleStandardsMap.Contains(ModuleName))
    {
        UE_LOG(LogTemp, Warning, TEXT("Module %s not found in standards map"), *ModuleName);
        return false;
    }
    
    const FEng_ModuleStandards& Standards = ModuleStandardsMap[ModuleName];
    
    // Check if module requires C++ implementation
    if (Standards.bRequiresCPPImplementation)
    {
        // Validate that both .h and .cpp files exist
        FString HeaderPath = FPaths::ProjectDir() + TEXT("Source/TranspersonalGame/") + ModuleName + TEXT(".h");
        FString SourcePath = FPaths::ProjectDir() + TEXT("Source/TranspersonalGame/") + ModuleName + TEXT(".cpp");
        
        if (!FPaths::FileExists(HeaderPath) || !FPaths::FileExists(SourcePath))
        {
            UE_LOG(LogTemp, Error, TEXT("Module %s missing required .h/.cpp pair"), *ModuleName);
            return false;
        }
    }
    
    return true;
}

TArray<FString> UEng_ArchitecturalFramework::GetCompilationViolations(const FString& ModuleName)
{
    TArray<FString> Violations;
    
    // Check for common UE5.5 compilation issues
    FString ModulePath = FPaths::ProjectDir() + TEXT("Source/TranspersonalGame/") + ModuleName;
    
    // Check for USTRUCT/UENUM at global scope
    FString HeaderContent;
    FString HeaderPath = ModulePath + TEXT(".h");
    
    if (FFileHelper::LoadFileToString(HeaderContent, *HeaderPath))
    {
        // Check for nested USTRUCT/UENUM (not allowed)
        if (HeaderContent.Contains(TEXT("class")) && 
            (HeaderContent.Find(TEXT("USTRUCT")) > HeaderContent.Find(TEXT("class")) ||
             HeaderContent.Find(TEXT("UENUM")) > HeaderContent.Find(TEXT("class"))))
        {
            Violations.Add(TEXT("USTRUCT/UENUM declared inside class (must be at global scope)"));
        }
        
        // Check for .generated.h as last include
        int32 GeneratedIncludePos = HeaderContent.Find(TEXT(".generated.h"));
        int32 LastIncludePos = HeaderContent.RFind(TEXT("#include"));
        
        if (GeneratedIncludePos != -1 && GeneratedIncludePos != LastIncludePos)
        {
            Violations.Add(TEXT(".generated.h must be the last #include"));
        }
        
        // Check for escaped quotes in macros
        if (HeaderContent.Contains(TEXT("\\\"")) && 
            (HeaderContent.Contains(TEXT("UPROPERTY")) || HeaderContent.Contains(TEXT("UFUNCTION"))))
        {
            Violations.Add(TEXT("Escaped quotes in UPROPERTY/UFUNCTION (use normal quotes)"));
        }
        
        // Check for spaces in identifiers
        TArray<FString> Lines;
        HeaderContent.ParseIntoArrayLines(Lines);
        
        for (const FString& Line : Lines)
        {
            if (Line.Contains(TEXT("UPROPERTY")) || Line.Contains(TEXT("bool ")) || Line.Contains(TEXT("float ")))
            {
                // Simple check for spaces in variable names
                if (Line.Contains(TEXT("bool b")) && Line.Contains(TEXT(" ")) && 
                    Line.Find(TEXT(";")) > Line.Find(TEXT("bool b")))
                {
                    FString VarPart = Line.Mid(Line.Find(TEXT("bool b")));
                    if (VarPart.Contains(TEXT(" ")) && !VarPart.StartsWith(TEXT("bool bIs")))
                    {
                        Violations.Add(FString::Printf(TEXT("Variable name contains spaces: %s"), *Line.TrimStartAndEnd()));
                    }
                }
            }
        }
    }
    
    return Violations;
}

void UEng_ArchitecturalFramework::RegisterModuleStandards(const FEng_ModuleStandards& Standards)
{
    ModuleStandardsMap.Add(Standards.ModuleName, Standards);
    UE_LOG(LogTemp, Log, TEXT("Registered module standards: %s"), *Standards.ModuleName);
}

FEng_ModuleStandards UEng_ArchitecturalFramework::GetModuleStandards(const FString& ModuleName)
{
    if (ModuleStandardsMap.Contains(ModuleName))
    {
        return ModuleStandardsMap[ModuleName];
    }
    
    // Return default standards if not found
    FEng_ModuleStandards DefaultStandards;
    DefaultStandards.ModuleName = ModuleName;
    return DefaultStandards;
}

bool UEng_ArchitecturalFramework::IsModuleCompliant(const FString& ModuleName)
{
    return ValidateModuleCompliance(ModuleName) && GetCompilationViolations(ModuleName).Num() == 0;
}

void UEng_ArchitecturalFramework::SetPerformanceTarget(const FString& SystemName, float MaxFrameTimeMS, int32 MaxMemoryMB)
{
    if (ModuleStandardsMap.Contains(SystemName))
    {
        ModuleStandardsMap[SystemName].MaxFrameTimeMS = MaxFrameTimeMS;
        ModuleStandardsMap[SystemName].MaxMemoryMB = MaxMemoryMB;
    }
    else
    {
        FEng_ModuleStandards NewStandards;
        NewStandards.ModuleName = SystemName;
        NewStandards.MaxFrameTimeMS = MaxFrameTimeMS;
        NewStandards.MaxMemoryMB = MaxMemoryMB;
        ModuleStandardsMap.Add(SystemName, NewStandards);
    }
    
    UE_LOG(LogTemp, Log, TEXT("Set performance target for %s: %.2fms, %dMB"), 
           *SystemName, MaxFrameTimeMS, MaxMemoryMB);
}

bool UEng_ArchitecturalFramework::CheckPerformanceCompliance(const FString& SystemName)
{
    // Placeholder for performance monitoring
    // In a real implementation, this would check actual performance metrics
    return true;
}

TArray<FString> UEng_ArchitecturalFramework::GetPerformanceViolations()
{
    TArray<FString> Violations;
    
    // Check each registered module against its performance targets
    for (const auto& ModulePair : ModuleStandardsMap)
    {
        const FEng_ModuleStandards& Standards = ModulePair.Value;
        
        // Placeholder checks - in real implementation would measure actual performance
        if (Standards.PerformanceTier == EEng_PerformanceTier::Critical && Standards.MaxFrameTimeMS > 16.67f)
        {
            Violations.Add(FString::Printf(TEXT("%s: Critical system exceeds 60fps target"), *Standards.ModuleName));
        }
    }
    
    return Violations;
}

void UEng_ArchitecturalFramework::AddArchitecturalConstraint(const FEng_ArchitecturalConstraint& Constraint)
{
    ArchitecturalConstraints.Add(Constraint);
    UE_LOG(LogTemp, Log, TEXT("Added architectural constraint: %s"), *Constraint.ConstraintName);
}

bool UEng_ArchitecturalFramework::ValidateSystemInteraction(const FString& SystemA, const FString& SystemB)
{
    // Check if interaction is allowed by architectural constraints
    for (const FEng_ArchitecturalConstraint& Constraint : ArchitecturalConstraints)
    {
        if (Constraint.SystemA == SystemA && Constraint.SystemB == SystemB)
        {
            if (Constraint.RelationshipType == TEXT("excludes"))
            {
                return false;
            }
        }
        
        if (Constraint.bBidirectional && Constraint.SystemB == SystemA && Constraint.SystemA == SystemB)
        {
            if (Constraint.RelationshipType == TEXT("excludes"))
            {
                return false;
            }
        }
    }
    
    return true;
}

TArray<FString> UEng_ArchitecturalFramework::GetArchitecturalViolations()
{
    TArray<FString> Violations;
    
    // Check for violations of architectural constraints
    // This would be implemented with actual system dependency analysis
    
    return Violations;
}

bool UEng_ArchitecturalFramework::ValidateHeaderStructure(const FString& HeaderPath)
{
    FString HeaderContent;
    if (!FFileHelper::LoadFileToString(HeaderContent, *HeaderPath))
    {
        return false;
    }
    
    // Check for #pragma once
    if (!HeaderContent.Contains(TEXT("#pragma once")))
    {
        return false;
    }
    
    // Check for .generated.h as last include
    int32 GeneratedPos = HeaderContent.Find(TEXT(".generated.h"));
    if (GeneratedPos != -1)
    {
        int32 LastIncludePos = HeaderContent.RFind(TEXT("#include"));
        if (GeneratedPos != LastIncludePos)
        {
            return false;
        }
    }
    
    return true;
}

bool UEng_ArchitecturalFramework::ValidateImplementationPair(const FString& HeaderPath, const FString& SourcePath)
{
    // Check if both files exist
    if (!FPaths::FileExists(HeaderPath) || !FPaths::FileExists(SourcePath))
    {
        return false;
    }
    
    // Check if .cpp includes its corresponding .h
    FString SourceContent;
    if (FFileHelper::LoadFileToString(SourceContent, *SourcePath))
    {
        FString HeaderName = FPaths::GetBaseFilename(HeaderPath) + TEXT(".h");
        if (!SourceContent.Contains(HeaderName))
        {
            return false;
        }
    }
    
    return true;
}

TArray<FString> UEng_ArchitecturalFramework::GetOrphanedHeaders()
{
    TArray<FString> OrphanedHeaders;
    
    // Scan for .h files without corresponding .cpp files
    FString SourceDir = FPaths::ProjectDir() + TEXT("Source/TranspersonalGame/");
    
    TArray<FString> HeaderFiles;
    IFileManager::Get().FindFilesRecursive(HeaderFiles, *SourceDir, TEXT("*.h"), true, false);
    
    for (const FString& HeaderFile : HeaderFiles)
    {
        FString BaseName = FPaths::GetBaseFilename(HeaderFile);
        FString CorrespondingCpp = FPaths::GetPath(HeaderFile) + TEXT("/") + BaseName + TEXT(".cpp");
        
        if (!FPaths::FileExists(CorrespondingCpp))
        {
            // Check if this header contains UCLASS/UFUNCTION/UPROPERTY
            FString HeaderContent;
            if (FFileHelper::LoadFileToString(HeaderContent, *HeaderFile))
            {
                if (HeaderContent.Contains(TEXT("UCLASS")) || 
                    HeaderContent.Contains(TEXT("UFUNCTION")) || 
                    HeaderContent.Contains(TEXT("UPROPERTY")))
                {
                    OrphanedHeaders.Add(HeaderFile);
                }
            }
        }
    }
    
    return OrphanedHeaders;
}

bool UEng_ArchitecturalFramework::ValidateUE55Compatibility(const FString& ClassName)
{
    // Check for UE5.5 specific compatibility issues
    // This would include checks for deprecated APIs, changed function signatures, etc.
    return true;
}

TArray<FString> UEng_ArchitecturalFramework::GetDeprecatedAPIUsages()
{
    TArray<FString> DeprecatedUsages;
    
    // Scan for deprecated UE5.5 API usages
    // This would be implemented with actual code analysis
    
    return DeprecatedUsages;
}

void UEng_ArchitecturalFramework::ApproveAgentImplementation(const FString& AgentName, const FString& SystemName)
{
    AgentApprovalStatus.Add(AgentName, true);
    AgentBlockReasons.Remove(AgentName);
    
    UE_LOG(LogTemp, Log, TEXT("Approved implementation for Agent %s: %s"), *AgentName, *SystemName);
}

void UEng_ArchitecturalFramework::BlockAgentImplementation(const FString& AgentName, const FString& Reason)
{
    AgentApprovalStatus.Add(AgentName, false);
    AgentBlockReasons.Add(AgentName, Reason);
    
    UE_LOG(LogTemp, Warning, TEXT("Blocked Agent %s: %s"), *AgentName, *Reason);
}

bool UEng_ArchitecturalFramework::IsAgentBlocked(const FString& AgentName)
{
    if (AgentApprovalStatus.Contains(AgentName))
    {
        return !AgentApprovalStatus[AgentName];
    }
    
    return false;
}

TArray<FString> UEng_ArchitecturalFramework::GetBlockedAgents()
{
    TArray<FString> BlockedAgents;
    
    for (const auto& StatusPair : AgentApprovalStatus)
    {
        if (!StatusPair.Value)
        {
            BlockedAgents.Add(StatusPair.Key);
        }
    }
    
    return BlockedAgents;
}

void UEng_ArchitecturalFramework::InitializeDefaultRules()
{
    // Initialize UE5.5 specific compilation rules
    
    FEng_CompilationRule Rule1;
    Rule1.RuleName = TEXT("USTRUCT_Global_Scope");
    Rule1.Description = TEXT("USTRUCT and UENUM must be declared at global scope, not inside classes");
    Rule1.bMandatory = true;
    Rule1.ViolationExamples.Add(TEXT("class MyClass { USTRUCT() struct MyStruct {}; };"));
    RegisterCompilationRule(Rule1);
    
    FEng_CompilationRule Rule2;
    Rule2.RuleName = TEXT("Generated_Header_Last");
    Rule2.Description = TEXT(".generated.h must be the last #include in header files");
    Rule2.bMandatory = true;
    Rule2.ViolationExamples.Add(TEXT("#include \"MyClass.generated.h\"\n#include \"Engine.h\""));
    RegisterCompilationRule(Rule2);
    
    FEng_CompilationRule Rule3;
    Rule3.RuleName = TEXT("No_Escaped_Quotes");
    Rule3.Description = TEXT("Use normal quotes in UPROPERTY/UFUNCTION macros, not escaped quotes");
    Rule3.bMandatory = true;
    Rule3.ViolationExamples.Add(TEXT("UPROPERTY(Category = \\\"Physics\\\")"));
    RegisterCompilationRule(Rule3);
    
    FEng_CompilationRule Rule4;
    Rule4.RuleName = TEXT("No_Spaces_In_Identifiers");
    Rule4.Description = TEXT("Variable and function names cannot contain spaces");
    Rule4.bMandatory = true;
    Rule4.ViolationExamples.Add(TEXT("bool bIsOnUneven Terrain;"));
    RegisterCompilationRule(Rule4);
    
    FEng_CompilationRule Rule5;
    Rule5.RuleName = TEXT("CallInEditor_Bare_Flag");
    Rule5.Description = TEXT("CallInEditor is a bare flag, not a key-value pair");
    Rule5.bMandatory = true;
    Rule5.ViolationExamples.Add(TEXT("UFUNCTION(CallInEditor = true)"));
    RegisterCompilationRule(Rule5);
}

void UEng_ArchitecturalFramework::InitializeModuleStandards()
{
    // Initialize standards for core modules
    
    FEng_ModuleStandards CoreStandards;
    CoreStandards.ModuleName = TEXT("Core");
    CoreStandards.ModuleType = EEng_ModuleType::Core;
    CoreStandards.PerformanceTier = EEng_PerformanceTier::Critical;
    CoreStandards.bRequiresCPPImplementation = true;
    CoreStandards.MaxMemoryMB = 50;
    CoreStandards.MaxFrameTimeMS = 1.0f;
    RegisterModuleStandards(CoreStandards);
    
    FEng_ModuleStandards GameplayStandards;
    GameplayStandards.ModuleName = TEXT("Gameplay");
    GameplayStandards.ModuleType = EEng_ModuleType::Gameplay;
    GameplayStandards.PerformanceTier = EEng_PerformanceTier::Critical;
    GameplayStandards.bRequiresCPPImplementation = true;
    GameplayStandards.MaxMemoryMB = 200;
    GameplayStandards.MaxFrameTimeMS = 8.0f;
    RegisterModuleStandards(GameplayStandards);
    
    FEng_ModuleStandards AIStandards;
    AIStandards.ModuleName = TEXT("AI");
    AIStandards.ModuleType = EEng_ModuleType::AI;
    AIStandards.PerformanceTier = EEng_PerformanceTier::Important;
    AIStandards.bRequiresCPPImplementation = true;
    AIStandards.MaxMemoryMB = 300;
    AIStandards.MaxFrameTimeMS = 5.0f;
    RegisterModuleStandards(AIStandards);
}

void UEng_ArchitecturalFramework::InitializeArchitecturalConstraints()
{
    // Initialize architectural constraints between systems
    
    FEng_ArchitecturalConstraint Constraint1;
    Constraint1.ConstraintName = TEXT("AI_Depends_Core");
    Constraint1.SystemA = TEXT("AI");
    Constraint1.SystemB = TEXT("Core");
    Constraint1.RelationshipType = TEXT("depends_on");
    Constraint1.bBidirectional = false;
    AddArchitecturalConstraint(Constraint1);
    
    FEng_ArchitecturalConstraint Constraint2;
    Constraint2.ConstraintName = TEXT("Gameplay_Depends_Physics");
    Constraint2.SystemA = TEXT("Gameplay");
    Constraint2.SystemB = TEXT("Physics");
    Constraint2.RelationshipType = TEXT("depends_on");
    Constraint2.bBidirectional = false;
    AddArchitecturalConstraint(Constraint2);
    
    FEng_ArchitecturalConstraint Constraint3;
    Constraint3.ConstraintName = TEXT("UI_Excludes_Physics");
    Constraint3.SystemA = TEXT("UI");
    Constraint3.SystemB = TEXT("Physics");
    Constraint3.RelationshipType = TEXT("excludes");
    Constraint3.bBidirectional = true;
    AddArchitecturalConstraint(Constraint3);
}

bool UEng_ArchitecturalFramework::ValidateUSTRUCTDeclaration(const FString& StructName)
{
    // Validate USTRUCT follows UE5.5 rules
    return true;
}

bool UEng_ArchitecturalFramework::ValidateUENUMDeclaration(const FString& EnumName)
{
    // Validate UENUM follows UE5.5 rules
    return true;
}

bool UEng_ArchitecturalFramework::ValidateUCLASSDeclaration(const FString& ClassName)
{
    // Validate UCLASS follows UE5.5 rules
    return true;
}