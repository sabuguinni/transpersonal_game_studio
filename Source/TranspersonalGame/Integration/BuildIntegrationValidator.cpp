#include "BuildIntegrationValidator.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "TimerManager.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Misc/DateTime.h"
#include "EditorLevelLibrary.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "Components/SkyAtmosphereComponent.h"

ABuildIntegrationValidator::ABuildIntegrationValidator()
{
    PrimaryActorTick.bCanEverTick = false;
    
    bAutoValidateOnBeginPlay = true;
    ValidationInterval = 300.0f; // 5 minutes
    bCleanupDuplicatesAutomatically = false;
    
    // Initialize validation report
    LastValidationReport = FBuild_ValidationReport();
}

void ABuildIntegrationValidator::BeginPlay()
{
    Super::BeginPlay();
    
    if (bAutoValidateOnBeginPlay)
    {
        // Delay initial validation by 2 seconds to allow world to fully load
        GetWorld()->GetTimerManager().SetTimer(ValidationTimerHandle, 
            FTimerDelegate::CreateUObject(this, &ABuildIntegrationValidator::PerformPeriodicValidation),
            2.0f, false);
    }
}

FBuild_ValidationReport ABuildIntegrationValidator::ValidateProjectStructure()
{
    double StartTime = FPlatformTime::Seconds();
    ClearValidationIssues();
    
    UE_LOG(LogTemp, Warning, TEXT("=== BUILD INTEGRATION VALIDATION STARTED ==="));
    
    // 1. Detect orphan headers
    int32 OrphanCount = DetectOrphanHeaders();
    LastValidationReport.OrphanHeaders = OrphanCount;
    
    // 2. Detect duplicate actors
    int32 DuplicateCount = DetectDuplicateActors();
    LastValidationReport.DuplicateActors = DuplicateCount;
    
    // 3. Check misplaced files
    TArray<FString> MisplacedFiles = GetMisplacedFiles();
    for (const FString& File : MisplacedFiles)
    {
        AddValidationIssue(EBuild_ValidationResult::Warning, TEXT("File Structure"), 
                          FString::Printf(TEXT("File outside Source/ directory: %s"), *File), File);
    }
    
    // 4. Validate module dependencies
    bool bModulesValid = ValidateModuleDependencies();
    if (!bModulesValid)
    {
        AddValidationIssue(EBuild_ValidationResult::Error, TEXT("Module Dependencies"), 
                          TEXT("Module dependency validation failed"));
    }
    
    // 5. Count total files
    FString SourcePath = GetProjectSourcePath();
    TArray<FString> HeaderFiles = FindFilesRecursive(SourcePath, TEXT("*.h"));
    TArray<FString> CppFiles = FindFilesRecursive(SourcePath, TEXT("*.cpp"));
    
    LastValidationReport.TotalHeaderFiles = HeaderFiles.Num();
    LastValidationReport.TotalCppFiles = CppFiles.Num();
    
    // Calculate validation time
    double EndTime = FPlatformTime::Seconds();
    LastValidationReport.ValidationTime = EndTime - StartTime;
    LastValidationReport.Timestamp = FDateTime::Now();
    
    // Log summary
    LogValidationSummary();
    
    // Auto-cleanup if enabled
    if (bCleanupDuplicatesAutomatically && DuplicateCount > 0)
    {
        CleanupDuplicateActors();
    }
    
    UE_LOG(LogTemp, Warning, TEXT("=== BUILD INTEGRATION VALIDATION COMPLETED ==="));
    
    return LastValidationReport;
}

int32 ABuildIntegrationValidator::DetectOrphanHeaders()
{
    FString SourcePath = GetProjectSourcePath();
    TArray<FString> HeaderFiles = FindFilesRecursive(SourcePath, TEXT("*.h"));
    
    int32 OrphanCount = 0;
    
    for (const FString& HeaderFile : HeaderFiles)
    {
        FString CppFile = HeaderFile;
        CppFile = CppFile.Replace(TEXT(".h"), TEXT(".cpp"));
        
        if (!FPlatformFileManager::Get().GetPlatformFile().FileExists(*CppFile))
        {
            // Skip certain files that don't need .cpp counterparts
            FString FileName = FPaths::GetCleanFilename(HeaderFile);
            if (FileName.Contains(TEXT("generated")) || 
                FileName.Contains(TEXT("Types")) ||
                FileName.Equals(TEXT("TranspersonalGame.h")) ||
                FileName.Contains(TEXT("Module")))
            {
                continue;
            }
            
            OrphanCount++;
            FString RelativePath = HeaderFile;
            FPaths::MakePathRelativeTo(RelativePath, *SourcePath);
            
            AddValidationIssue(EBuild_ValidationResult::Warning, TEXT("Orphan Header"), 
                              FString::Printf(TEXT("Header without .cpp implementation: %s"), *RelativePath), 
                              RelativePath);
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Detected %d orphan headers"), OrphanCount);
    return OrphanCount;
}

int32 ABuildIntegrationValidator::DetectDuplicateActors()
{
    if (!GetWorld())
    {
        return 0;
    }
    
    TMap<FString, int32> ActorCounts;
    int32 TotalDuplicates = 0;
    
    // Get all actors in the level
    for (TActorIterator<AActor> ActorItr(GetWorld()); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (Actor)
        {
            FString ClassName = Actor->GetClass()->GetName();
            ActorCounts.FindOrAdd(ClassName)++;
        }
    }
    
    // Check for critical duplicates
    TArray<FString> CriticalTypes = {
        TEXT("DirectionalLight"),
        TEXT("SkyLight"), 
        TEXT("ExponentialHeightFog"),
        TEXT("SkyAtmosphere")
    };
    
    for (const FString& CriticalType : CriticalTypes)
    {
        if (int32* Count = ActorCounts.Find(CriticalType))
        {
            if (*Count > 1)
            {
                TotalDuplicates += (*Count - 1);
                AddValidationIssue(EBuild_ValidationResult::Error, TEXT("Duplicate Actors"), 
                                  FString::Printf(TEXT("Found %d instances of %s (should be 1)"), 
                                  *Count, *CriticalType));
            }
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Detected %d duplicate critical actors"), TotalDuplicates);
    return TotalDuplicates;
}

bool ABuildIntegrationValidator::CleanupDuplicateActors()
{
    if (!GetWorld())
    {
        return false;
    }
    
    int32 CleanedCount = 0;
    
    // Clean DirectionalLights
    TArray<ADirectionalLight*> DirectionalLights;
    for (TActorIterator<ADirectionalLight> ActorItr(GetWorld()); ActorItr; ++ActorItr)
    {
        DirectionalLights.Add(*ActorItr);
    }
    
    if (DirectionalLights.Num() > 1)
    {
        for (int32 i = 1; i < DirectionalLights.Num(); i++)
        {
            DirectionalLights[i]->Destroy();
            CleanedCount++;
        }
    }
    
    // Clean SkyLights
    TArray<ASkyLight*> SkyLights;
    for (TActorIterator<ASkyLight> ActorItr(GetWorld()); ActorItr; ++ActorItr)
    {
        SkyLights.Add(*ActorItr);
    }
    
    if (SkyLights.Num() > 1)
    {
        for (int32 i = 1; i < SkyLights.Num(); i++)
        {
            SkyLights[i]->Destroy();
            CleanedCount++;
        }
    }
    
    // Clean ExponentialHeightFog
    TArray<AExponentialHeightFog*> HeightFogs;
    for (TActorIterator<AExponentialHeightFog> ActorItr(GetWorld()); ActorItr; ++ActorItr)
    {
        HeightFogs.Add(*ActorItr);
    }
    
    if (HeightFogs.Num() > 1)
    {
        for (int32 i = 1; i < HeightFogs.Num(); i++)
        {
            HeightFogs[i]->Destroy();
            CleanedCount++;
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Cleaned up %d duplicate actors"), CleanedCount);
    
    // Save level after cleanup
    #if WITH_EDITOR
    if (CleanedCount > 0)
    {
        UEditorLevelLibrary::SaveCurrentLevel();
    }
    #endif
    
    return CleanedCount > 0;
}

TArray<FString> ABuildIntegrationValidator::GetMisplacedFiles()
{
    TArray<FString> MisplacedFiles;
    
    FString ProjectPath = FPaths::ProjectDir();
    
    // Check for files that should be in Source/ but aren't
    TArray<FString> CheckPaths = {
        FPaths::Combine(ProjectPath, TEXT("Audio/AdaptiveMusicController.cpp")),
        FPaths::Combine(ProjectPath, TEXT("Content/Lighting/LightingMasterController.cpp")),
        FPaths::Combine(ProjectPath, TEXT("Integration/BuildManager.cpp"))
    };
    
    for (const FString& CheckPath : CheckPaths)
    {
        if (FPlatformFileManager::Get().GetPlatformFile().FileExists(*CheckPath))
        {
            FString RelativePath = CheckPath;
            FPaths::MakePathRelativeTo(RelativePath, *ProjectPath);
            MisplacedFiles.Add(RelativePath);
        }
    }
    
    return MisplacedFiles;
}

bool ABuildIntegrationValidator::ValidateModuleDependencies()
{
    FString BuildFilePath = FPaths::Combine(GetProjectSourcePath(), TEXT("TranspersonalGame.Build.cs"));
    
    if (!FPlatformFileManager::Get().GetPlatformFile().FileExists(*BuildFilePath))
    {
        AddValidationIssue(EBuild_ValidationResult::Critical, TEXT("Module Dependencies"), 
                          TEXT("TranspersonalGame.Build.cs not found"));
        return false;
    }
    
    FString BuildFileContent;
    if (!FFileHelper::LoadFileToString(BuildFileContent, *BuildFilePath))
    {
        AddValidationIssue(EBuild_ValidationResult::Error, TEXT("Module Dependencies"), 
                          TEXT("Failed to read TranspersonalGame.Build.cs"));
        return false;
    }
    
    // Check for required dependencies
    TArray<FString> RequiredModules = {
        TEXT("Core"),
        TEXT("CoreUObject"),
        TEXT("Engine"),
        TEXT("UnrealEd"),
        TEXT("EditorStyle"),
        TEXT("EditorWidgets"),
        TEXT("ToolMenus")
    };
    
    for (const FString& Module : RequiredModules)
    {
        if (!BuildFileContent.Contains(Module))
        {
            AddValidationIssue(EBuild_ValidationResult::Warning, TEXT("Module Dependencies"), 
                              FString::Printf(TEXT("Missing module dependency: %s"), *Module));
        }
    }
    
    return true;
}

void ABuildIntegrationValidator::ExportValidationReport(const FString& FilePath)
{
    FString ReportContent;
    ReportContent += FString::Printf(TEXT("=== BUILD INTEGRATION VALIDATION REPORT ===\n"));
    ReportContent += FString::Printf(TEXT("Timestamp: %s\n"), *LastValidationReport.Timestamp.ToString());
    ReportContent += FString::Printf(TEXT("Validation Time: %.3f seconds\n"), LastValidationReport.ValidationTime);
    ReportContent += FString::Printf(TEXT("Total Header Files: %d\n"), LastValidationReport.TotalHeaderFiles);
    ReportContent += FString::Printf(TEXT("Total CPP Files: %d\n"), LastValidationReport.TotalCppFiles);
    ReportContent += FString::Printf(TEXT("Orphan Headers: %d\n"), LastValidationReport.OrphanHeaders);
    ReportContent += FString::Printf(TEXT("Duplicate Actors: %d\n"), LastValidationReport.DuplicateActors);
    ReportContent += TEXT("\n=== ISSUES ===\n");
    
    for (const FBuild_ValidationIssue& Issue : LastValidationReport.Issues)
    {
        FString SeverityStr;
        switch (Issue.Severity)
        {
            case EBuild_ValidationResult::Success: SeverityStr = TEXT("SUCCESS"); break;
            case EBuild_ValidationResult::Warning: SeverityStr = TEXT("WARNING"); break;
            case EBuild_ValidationResult::Error: SeverityStr = TEXT("ERROR"); break;
            case EBuild_ValidationResult::Critical: SeverityStr = TEXT("CRITICAL"); break;
        }
        
        ReportContent += FString::Printf(TEXT("[%s] %s: %s\n"), 
                                        *SeverityStr, *Issue.Category, *Issue.Description);
        if (!Issue.FilePath.IsEmpty())
        {
            ReportContent += FString::Printf(TEXT("  File: %s\n"), *Issue.FilePath);
        }
    }
    
    FFileHelper::SaveStringToFile(ReportContent, *FilePath);
    UE_LOG(LogTemp, Warning, TEXT("Validation report exported to: %s"), *FilePath);
}

void ABuildIntegrationValidator::LogValidationSummary()
{
    UE_LOG(LogTemp, Warning, TEXT("=== VALIDATION SUMMARY ==="));
    UE_LOG(LogTemp, Warning, TEXT("Header Files: %d"), LastValidationReport.TotalHeaderFiles);
    UE_LOG(LogTemp, Warning, TEXT("CPP Files: %d"), LastValidationReport.TotalCppFiles);
    UE_LOG(LogTemp, Warning, TEXT("Orphan Headers: %d"), LastValidationReport.OrphanHeaders);
    UE_LOG(LogTemp, Warning, TEXT("Duplicate Actors: %d"), LastValidationReport.DuplicateActors);
    UE_LOG(LogTemp, Warning, TEXT("Total Issues: %d"), LastValidationReport.Issues.Num());
    UE_LOG(LogTemp, Warning, TEXT("Validation Time: %.3f seconds"), LastValidationReport.ValidationTime);
}

void ABuildIntegrationValidator::SetAutoValidation(bool bEnabled, float IntervalSeconds)
{
    bAutoValidateOnBeginPlay = bEnabled;
    ValidationInterval = IntervalSeconds;
    
    if (GetWorld())
    {
        if (bEnabled)
        {
            GetWorld()->GetTimerManager().SetTimer(ValidationTimerHandle, 
                FTimerDelegate::CreateUObject(this, &ABuildIntegrationValidator::PerformPeriodicValidation),
                ValidationInterval, true);
        }
        else
        {
            GetWorld()->GetTimerManager().ClearTimer(ValidationTimerHandle);
        }
    }
}

void ABuildIntegrationValidator::PerformPeriodicValidation()
{
    ValidateProjectStructure();
    
    // Schedule next validation
    if (bAutoValidateOnBeginPlay && GetWorld())
    {
        GetWorld()->GetTimerManager().SetTimer(ValidationTimerHandle, 
            FTimerDelegate::CreateUObject(this, &ABuildIntegrationValidator::PerformPeriodicValidation),
            ValidationInterval, false);
    }
}

void ABuildIntegrationValidator::AddValidationIssue(EBuild_ValidationResult Severity, const FString& Category, 
                                                   const FString& Description, const FString& FilePath, int32 LineNumber)
{
    FBuild_ValidationIssue Issue;
    Issue.Severity = Severity;
    Issue.Category = Category;
    Issue.Description = Description;
    Issue.FilePath = FilePath;
    Issue.LineNumber = LineNumber;
    
    LastValidationReport.Issues.Add(Issue);
}

void ABuildIntegrationValidator::ClearValidationIssues()
{
    LastValidationReport.Issues.Empty();
}

FString ABuildIntegrationValidator::GetProjectSourcePath() const
{
    return FPaths::Combine(FPaths::ProjectDir(), TEXT("Source/TranspersonalGame"));
}

TArray<FString> ABuildIntegrationValidator::FindFilesRecursive(const FString& Directory, const FString& Extension) const
{
    TArray<FString> FoundFiles;
    
    IFileManager& FileManager = IFileManager::Get();
    FileManager.FindFilesRecursive(FoundFiles, *Directory, *Extension, true, false);
    
    return FoundFiles;
}