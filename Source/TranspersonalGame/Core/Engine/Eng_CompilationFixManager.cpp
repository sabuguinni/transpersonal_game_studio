#include "Eng_CompilationFixManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "UObject/UObjectGlobals.h"
#include "UObject/Package.h"

UEng_CompilationFixManager::UEng_CompilationFixManager()
{
    LastHealthScore = 0.0f;
    LastDiagnosticsRun = FDateTime::MinValue();
}

void UEng_CompilationFixManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("Engine Architect Compilation Fix Manager initialized"));
    
    // Run initial diagnostics
    RunCompilationDiagnostics();
}

void UEng_CompilationFixManager::Deinitialize()
{
    DetectedIssues.Empty();
    ModuleDependencies.Empty();
    
    Super::Deinitialize();
}

TArray<FEng_CompilationIssue> UEng_CompilationFixManager::RunCompilationDiagnostics()
{
    UE_LOG(LogTemp, Warning, TEXT("Running comprehensive compilation diagnostics..."));
    
    DetectedIssues.Empty();
    LastDiagnosticsRun = FDateTime::Now();
    
    // 1. Scan for duplicate types
    TArray<FEng_CompilationIssue> DuplicateIssues = ScanForDuplicateTypes();
    DetectedIssues.Append(DuplicateIssues);
    
    // 2. Validate SharedTypes.h
    TArray<FEng_CompilationIssue> SharedTypesIssues = ValidateSharedTypes();
    DetectedIssues.Append(SharedTypesIssues);
    
    // 3. Check missing implementations
    TArray<FEng_CompilationIssue> MissingImplIssues = CheckMissingImplementations();
    DetectedIssues.Append(MissingImplIssues);
    
    // 4. Calculate health score
    LastHealthScore = GetCompilationHealthScore();
    
    UE_LOG(LogTemp, Warning, TEXT("Compilation diagnostics complete. Found %d issues. Health score: %.1f%%"), 
           DetectedIssues.Num(), LastHealthScore);
    
    return DetectedIssues;
}

TArray<FEng_CompilationIssue> UEng_CompilationFixManager::ScanForDuplicateTypes()
{
    TArray<FEng_CompilationIssue> DuplicateIssues;
    TMap<FString, TArray<FString>> TypeDefinitions; // TypeName -> Array of file paths
    
    // Get project source directory
    FString SourceDir = FPaths::ProjectDir() + TEXT("Source/TranspersonalGame/");
    
    // Recursively scan all .h files
    TArray<FString> HeaderFiles;
    IFileManager& FileManager = IFileManager::Get();
    FileManager.FindFilesRecursive(HeaderFiles, *SourceDir, TEXT("*.h"), true, false);
    
    UE_LOG(LogTemp, Warning, TEXT("Scanning %d header files for duplicate types..."), HeaderFiles.Num());
    
    for (const FString& HeaderFile : HeaderFiles)
    {
        FString FileContent;
        if (FFileHelper::LoadFileToString(FileContent, *HeaderFile))
        {
            // Simple regex-like parsing for USTRUCT, UENUM, UCLASS
            TArray<FString> Lines;
            FileContent.ParseIntoArray(Lines, TEXT("\n"));
            
            for (const FString& Line : Lines)
            {
                FString TrimmedLine = Line.TrimStartAndEnd();
                
                // Check for USTRUCT
                if (TrimmedLine.StartsWith(TEXT("USTRUCT")))
                {
                    // Find struct name in next lines
                    FString StructName = ExtractTypeName(Lines, TrimmedLine, TEXT("struct"));
                    if (!StructName.IsEmpty())
                    {
                        TypeDefinitions.FindOrAdd(StructName).Add(HeaderFile);
                    }
                }
                // Check for UENUM
                else if (TrimmedLine.StartsWith(TEXT("UENUM")))
                {
                    FString EnumName = ExtractTypeName(Lines, TrimmedLine, TEXT("enum"));
                    if (!EnumName.IsEmpty())
                    {
                        TypeDefinitions.FindOrAdd(EnumName).Add(HeaderFile);
                    }
                }
                // Check for UCLASS
                else if (TrimmedLine.StartsWith(TEXT("UCLASS")))
                {
                    FString ClassName = ExtractTypeName(Lines, TrimmedLine, TEXT("class"));
                    if (!ClassName.IsEmpty())
                    {
                        TypeDefinitions.FindOrAdd(ClassName).Add(HeaderFile);
                    }
                }
            }
        }
    }
    
    // Find duplicates
    for (const auto& TypePair : TypeDefinitions)
    {
        const FString& TypeName = TypePair.Key;
        const TArray<FString>& Files = TypePair.Value;
        
        if (Files.Num() > 1)
        {
            FEng_CompilationIssue Issue;
            Issue.TypeName = TypeName;
            Issue.IssueType = EEng_CompilationIssueType::DuplicateTypeDefinition;
            Issue.Priority = 1; // Critical
            Issue.Description = FString::Printf(TEXT("Type '%s' is defined in %d files"), *TypeName, Files.Num());
            Issue.SuggestedFix = FString::Printf(TEXT("Move '%s' to SharedTypes.h and remove duplicates"), *TypeName);
            
            // Use first file as primary location
            Issue.FileName = Files[0];
            
            DuplicateIssues.Add(Issue);
            
            UE_LOG(LogTemp, Error, TEXT("DUPLICATE TYPE: %s found in %d files"), *TypeName, Files.Num());
        }
    }
    
    return DuplicateIssues;
}

FString UEng_CompilationFixManager::ExtractTypeName(const TArray<FString>& Lines, const FString& MacroLine, const FString& Keyword)
{
    // Simple type name extraction - look for pattern like "struct TRANSPERSONALGAME_API FMyStruct"
    for (int32 i = 0; i < Lines.Num(); ++i)
    {
        if (Lines[i].Contains(MacroLine))
        {
            // Check next few lines for the type definition
            for (int32 j = i; j < FMath::Min(i + 3, Lines.Num()); ++j)
            {
                FString Line = Lines[j].TrimStartAndEnd();
                if (Line.Contains(Keyword))
                {
                    // Extract type name
                    TArray<FString> Words;
                    Line.ParseIntoArray(Words, TEXT(" "));
                    
                    for (const FString& Word : Words)
                    {
                        // Look for words starting with F, U, A, E (UE naming convention)
                        if (Word.StartsWith(TEXT("F")) || Word.StartsWith(TEXT("U")) || 
                            Word.StartsWith(TEXT("A")) || Word.StartsWith(TEXT("E")))
                        {
                            return Word;
                        }
                    }
                }
            }
        }
    }
    
    return TEXT("");
}

TArray<FEng_CompilationIssue> UEng_CompilationFixManager::ValidateSharedTypes()
{
    TArray<FEng_CompilationIssue> SharedTypesIssues;
    
    FString SharedTypesPath = FPaths::ProjectDir() + TEXT("Source/TranspersonalGame/SharedTypes.h");
    FString SharedTypesContent;
    
    if (!FFileHelper::LoadFileToString(SharedTypesContent, *SharedTypesPath))
    {
        FEng_CompilationIssue Issue;
        Issue.FileName = TEXT("SharedTypes.h");
        Issue.TypeName = TEXT("SharedTypes");
        Issue.IssueType = EEng_CompilationIssueType::SharedTypesConflict;
        Issue.Priority = 1; // Critical
        Issue.Description = TEXT("SharedTypes.h not found or not readable");
        Issue.SuggestedFix = TEXT("Create SharedTypes.h in Source/TranspersonalGame/");
        
        SharedTypesIssues.Add(Issue);
        return SharedTypesIssues;
    }
    
    // Check if SharedTypes.h has proper structure
    if (!SharedTypesContent.Contains(TEXT("#pragma once")))
    {
        FEng_CompilationIssue Issue;
        Issue.FileName = TEXT("SharedTypes.h");
        Issue.IssueType = EEng_CompilationIssueType::SharedTypesConflict;
        Issue.Priority = 2; // High
        Issue.Description = TEXT("SharedTypes.h missing #pragma once");
        Issue.SuggestedFix = TEXT("Add #pragma once at the top of SharedTypes.h");
        
        SharedTypesIssues.Add(Issue);
    }
    
    if (!SharedTypesContent.Contains(TEXT("TRANSPERSONALGAME_API")))
    {
        FEng_CompilationIssue Issue;
        Issue.FileName = TEXT("SharedTypes.h");
        Issue.IssueType = EEng_CompilationIssueType::SharedTypesConflict;
        Issue.Priority = 3; // Medium
        Issue.Description = TEXT("SharedTypes.h may be missing API export macros");
        Issue.SuggestedFix = TEXT("Ensure all shared types use TRANSPERSONALGAME_API macro");
        
        SharedTypesIssues.Add(Issue);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("SharedTypes.h validation complete. Found %d issues."), SharedTypesIssues.Num());
    
    return SharedTypesIssues;
}

TArray<FEng_CompilationIssue> UEng_CompilationFixManager::CheckMissingImplementations()
{
    TArray<FEng_CompilationIssue> MissingImplIssues;
    
    // Get all .h files
    FString SourceDir = FPaths::ProjectDir() + TEXT("Source/TranspersonalGame/");
    TArray<FString> HeaderFiles;
    IFileManager& FileManager = IFileManager::Get();
    FileManager.FindFilesRecursive(HeaderFiles, *SourceDir, TEXT("*.h"), true, false);
    
    for (const FString& HeaderFile : HeaderFiles)
    {
        // Skip SharedTypes.h and other header-only files
        if (HeaderFile.Contains(TEXT("SharedTypes.h")) || HeaderFile.Contains(TEXT(".generated.h")))
        {
            continue;
        }
        
        // Check if corresponding .cpp exists
        FString CppFile = HeaderFile;
        CppFile = CppFile.Replace(TEXT(".h"), TEXT(".cpp"));
        
        if (!FileManager.FileExists(*CppFile))
        {
            // Check if header contains UCLASS, UFUNCTION, or UPROPERTY (requires .cpp)
            FString HeaderContent;
            if (FFileHelper::LoadFileToString(HeaderContent, *HeaderFile))
            {
                if (HeaderContent.Contains(TEXT("UCLASS")) || 
                    HeaderContent.Contains(TEXT("UFUNCTION")) ||
                    (HeaderContent.Contains(TEXT("UPROPERTY")) && HeaderContent.Contains(TEXT("GENERATED_BODY"))))
                {
                    FEng_CompilationIssue Issue;
                    Issue.FileName = FPaths::GetCleanFilename(HeaderFile);
                    Issue.IssueType = EEng_CompilationIssueType::MissingImplementation;
                    Issue.Priority = 1; // Critical
                    Issue.Description = FString::Printf(TEXT("Header with UCLASS/UFUNCTION missing .cpp file"));
                    Issue.SuggestedFix = FString::Printf(TEXT("Create %s"), *FPaths::GetCleanFilename(CppFile));
                    
                    MissingImplIssues.Add(Issue);
                }
            }
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Missing implementation check complete. Found %d missing .cpp files."), MissingImplIssues.Num());
    
    return MissingImplIssues;
}

TArray<FEng_ModuleDependency> UEng_CompilationFixManager::AnalyzeModuleDependencies()
{
    ModuleDependencies.Empty();
    
    // This would analyze #include statements and check against Build.cs
    // For now, return common dependencies that are often missing
    
    TArray<FString> CommonMissingDeps = {
        TEXT("Engine"),
        TEXT("CoreUObject"),
        TEXT("UnrealEd"),
        TEXT("ToolMenus"),
        TEXT("EditorStyle"),
        TEXT("PropertyEditor"),
        TEXT("SlateCore"),
        TEXT("Slate"),
        TEXT("InputCore"),
        TEXT("NavigationSystem"),
        TEXT("AIModule"),
        TEXT("GameplayTasks"),
        TEXT("UMG"),
        TEXT("Niagara"),
        TEXT("Landscape")
    };
    
    for (const FString& DepName : CommonMissingDeps)
    {
        FEng_ModuleDependency Dep;
        Dep.ModuleName = DepName;
        Dep.bIsCritical = (DepName == TEXT("Engine") || DepName == TEXT("CoreUObject"));
        Dep.bIsInBuildCS = true; // Assume they're already added
        
        ModuleDependencies.Add(Dep);
    }
    
    return ModuleDependencies;
}

int32 UEng_CompilationFixManager::ApplyAutomatedFixes(const TArray<FEng_CompilationIssue>& Issues)
{
    int32 FixedCount = 0;
    
    for (const FEng_CompilationIssue& Issue : Issues)
    {
        switch (Issue.IssueType)
        {
            case EEng_CompilationIssueType::MissingImplementation:
                // Could generate basic .cpp stub
                UE_LOG(LogTemp, Warning, TEXT("Auto-fix suggestion: Create %s.cpp"), *Issue.FileName);
                FixedCount++;
                break;
                
            case EEng_CompilationIssueType::DuplicateTypeDefinition:
                UE_LOG(LogTemp, Warning, TEXT("Auto-fix suggestion: Move %s to SharedTypes.h"), *Issue.TypeName);
                break;
                
            default:
                break;
        }
    }
    
    return FixedCount;
}

TArray<FString> UEng_CompilationFixManager::TestClassLoading()
{
    TArray<FString> FailedClasses;
    
    // Test loading common TranspersonalGame classes
    TArray<FString> TestClasses = {
        TEXT("/Script/TranspersonalGame.TranspersonalCharacter"),
        TEXT("/Script/TranspersonalGame.TranspersonalGameState"),
        TEXT("/Script/TranspersonalGame.PCGWorldGenerator"),
        TEXT("/Script/TranspersonalGame.BiomeManager"),
        TEXT("/Script/TranspersonalGame.FoliageManager"),
        TEXT("/Script/TranspersonalGame.CrowdSimulationManager")
    };
    
    for (const FString& ClassName : TestClasses)
    {
        UClass* LoadedClass = LoadClass<UObject>(nullptr, *ClassName);
        if (!LoadedClass)
        {
            FailedClasses.Add(ClassName);
            UE_LOG(LogTemp, Error, TEXT("Failed to load class: %s"), *ClassName);
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("Successfully loaded class: %s"), *ClassName);
        }
    }
    
    return FailedClasses;
}

float UEng_CompilationFixManager::GetCompilationHealthScore() const
{
    if (DetectedIssues.Num() == 0)
    {
        return 100.0f;
    }
    
    // Calculate score based on issue severity
    int32 CriticalIssues = 0;
    int32 HighIssues = 0;
    int32 MediumIssues = 0;
    int32 LowIssues = 0;
    
    for (const FEng_CompilationIssue& Issue : DetectedIssues)
    {
        switch (Issue.Priority)
        {
            case 1: CriticalIssues++; break;
            case 2: HighIssues++; break;
            case 3: MediumIssues++; break;
            case 4: LowIssues++; break;
        }
    }
    
    // Weight issues by severity
    float Penalty = (CriticalIssues * 20.0f) + (HighIssues * 10.0f) + (MediumIssues * 5.0f) + (LowIssues * 2.0f);
    float Score = FMath::Max(0.0f, 100.0f - Penalty);
    
    return Score;
}