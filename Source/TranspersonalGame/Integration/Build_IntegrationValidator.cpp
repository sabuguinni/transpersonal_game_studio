#include "Build_IntegrationValidator.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Editor/EditorEngine.h"
#include "EditorLevelLibrary.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/Paths.h"
#include "UObject/UObjectGlobals.h"

UBuild_IntegrationValidator::UBuild_IntegrationValidator()
{
    ValidationStartTime = 0.0f;
    InitializeKnownModules();
    InitializeCriticalClasses();
}

void UBuild_IntegrationValidator::InitializeKnownModules()
{
    KnownModules.Empty();
    KnownModules.Add(TEXT("TranspersonalGame"));
    KnownModules.Add(TEXT("Core"));
    KnownModules.Add(TEXT("Engine"));
    KnownModules.Add(TEXT("UnrealEd"));
}

void UBuild_IntegrationValidator::InitializeCriticalClasses()
{
    CriticalClasses.Empty();
    CriticalClasses.Add(TEXT("TranspersonalCharacter"));
    CriticalClasses.Add(TEXT("TranspersonalGameState"));
    CriticalClasses.Add(TEXT("PCGWorldGenerator"));
    CriticalClasses.Add(TEXT("FoliageManager"));
    CriticalClasses.Add(TEXT("CrowdSimulationManager"));
    CriticalClasses.Add(TEXT("ProceduralWorldManager"));
    CriticalClasses.Add(TEXT("BuildIntegrationManager"));
}

FBuild_IntegrationReport UBuild_IntegrationValidator::ValidateFullIntegration()
{
    ValidationStartTime = FPlatformTime::Seconds();
    
    FBuild_IntegrationReport Report;
    Report.ValidationTimestamp = FDateTime::Now();
    
    UE_LOG(LogTemp, Warning, TEXT("=== STARTING FULL INTEGRATION VALIDATION ==="));
    
    // Validate TranspersonalGame module
    TArray<FString> TranspersonalClasses;
    for (const FString& ClassName : CriticalClasses)
    {
        TranspersonalClasses.Add(ClassName);
    }
    
    FBuild_ModuleStatus ModuleStatus = ValidateModule(TEXT("TranspersonalGame"), TranspersonalClasses);
    Report.ModuleStatuses.Add(ModuleStatus);
    
    // Count actors in level
    Report.TotalActorsInLevel = CountLevelActors(false);
    Report.CustomActorsInLevel = CountLevelActors(true);
    
    // Count compiled binaries
    Report.BinaryFilesFound = CountCompiledBinaries();
    
    // Determine overall status
    Report.OverallStatus = DetermineOverallStatus(Report.ModuleStatuses);
    
    // Calculate total validation time
    Report.TotalValidationTime = FPlatformTime::Seconds() - ValidationStartTime;
    
    // Cache the report
    LastValidationReport = Report;
    
    UE_LOG(LogTemp, Warning, TEXT("=== INTEGRATION VALIDATION COMPLETE ==="));
    UE_LOG(LogTemp, Warning, TEXT("Overall Status: %s"), 
           *UEnum::GetValueAsString(Report.OverallStatus));
    UE_LOG(LogTemp, Warning, TEXT("Validation Time: %.2f seconds"), Report.TotalValidationTime);
    
    return Report;
}

FBuild_ModuleStatus UBuild_IntegrationValidator::ValidateModule(const FString& ModuleName, const TArray<FString>& ClassNames)
{
    float ModuleStartTime = FPlatformTime::Seconds();
    
    FBuild_ModuleStatus Status;
    Status.ModuleName = ModuleName;
    Status.Status = EBuild_IntegrationStatus::Validating;
    
    UE_LOG(LogTemp, Log, TEXT("Validating module: %s"), *ModuleName);
    
    // Test each class
    for (const FString& ClassName : ClassNames)
    {
        FString ErrorMessage;
        if (ValidateClassLoading(ClassName, ErrorMessage))
        {
            Status.ClassesLoaded++;
            UE_LOG(LogTemp, Log, TEXT("  ✓ %s loaded successfully"), *ClassName);
        }
        else
        {
            Status.ClassesFailed++;
            Status.ErrorMessage += FString::Printf(TEXT("%s: %s; "), *ClassName, *ErrorMessage);
            UE_LOG(LogTemp, Warning, TEXT("  ✗ %s failed: %s"), *ClassName, *ErrorMessage);
        }
    }
    
    // Determine module status
    if (Status.ClassesFailed == 0)
    {
        Status.Status = EBuild_IntegrationStatus::Success;
    }
    else if (Status.ClassesLoaded > Status.ClassesFailed)
    {
        Status.Status = EBuild_IntegrationStatus::Warning;
    }
    else
    {
        Status.Status = EBuild_IntegrationStatus::Error;
    }
    
    Status.ValidationTime = FPlatformTime::Seconds() - ModuleStartTime;
    
    return Status;
}

bool UBuild_IntegrationValidator::ValidateClassLoading(const FString& ClassName, FString& OutErrorMessage)
{
    try
    {
        FString ClassPath = FString::Printf(TEXT("/Script/TranspersonalGame.%s"), *ClassName);
        UClass* LoadedClass = LoadClass<UObject>(nullptr, *ClassPath);
        
        if (LoadedClass)
        {
            OutErrorMessage = TEXT("Success");
            return true;
        }
        else
        {
            OutErrorMessage = TEXT("Class not found or failed to load");
            return false;
        }
    }
    catch (...)
    {
        OutErrorMessage = TEXT("Exception during class loading");
        return false;
    }
}

int32 UBuild_IntegrationValidator::CountLevelActors(bool bCustomActorsOnly)
{
    if (!GEditor || !GEditor->GetEditorWorldContext().World())
    {
        UE_LOG(LogTemp, Warning, TEXT("No valid editor world found"));
        return 0;
    }
    
    UWorld* World = GEditor->GetEditorWorldContext().World();
    TArray<AActor*> AllActors;
    UEditorLevelLibrary::GetAllLevelActors(AllActors);
    
    if (!bCustomActorsOnly)
    {
        return AllActors.Num();
    }
    
    // Count only custom TranspersonalGame actors
    int32 CustomCount = 0;
    for (AActor* Actor : AllActors)
    {
        if (Actor)
        {
            FString ClassName = Actor->GetClass()->GetName();
            if (ClassName.Contains(TEXT("Transpersonal")) || 
                ClassName.Contains(TEXT("PCG")) || 
                ClassName.Contains(TEXT("Foliage")) ||
                ClassName.Contains(TEXT("Crowd")))
            {
                CustomCount++;
            }
        }
    }
    
    return CustomCount;
}

int32 UBuild_IntegrationValidator::CountCompiledBinaries()
{
    FString ProjectDir = FPaths::ProjectDir();
    FString BinariesDir = FPaths::Combine(ProjectDir, TEXT("Binaries"), TEXT("Linux"));
    
    TArray<FString> FoundFiles;
    IFileManager& FileManager = IFileManager::Get();
    FileManager.FindFiles(FoundFiles, *FPaths::Combine(BinariesDir, TEXT("*.so")), true, false);
    
    UE_LOG(LogTemp, Log, TEXT("Found %d binary files in %s"), FoundFiles.Num(), *BinariesDir);
    
    return FoundFiles.Num();
}

TArray<FString> UBuild_IntegrationValidator::GetFailedClasses()
{
    TArray<FString> FailedClasses;
    
    for (const FString& ClassName : CriticalClasses)
    {
        FString ErrorMessage;
        if (!ValidateClassLoading(ClassName, ErrorMessage))
        {
            FailedClasses.Add(ClassName);
        }
    }
    
    return FailedClasses;
}

TArray<FString> UBuild_IntegrationValidator::GetLoadedClasses()
{
    TArray<FString> LoadedClasses;
    
    for (const FString& ClassName : CriticalClasses)
    {
        FString ErrorMessage;
        if (ValidateClassLoading(ClassName, ErrorMessage))
        {
            LoadedClasses.Add(ClassName);
        }
    }
    
    return LoadedClasses;
}

FString UBuild_IntegrationValidator::GenerateIntegrationReport(const FBuild_IntegrationReport& Report)
{
    FString ReportText;
    ReportText += TEXT("=== INTEGRATION VALIDATION REPORT ===\n");
    ReportText += FString::Printf(TEXT("Timestamp: %s\n"), *Report.ValidationTimestamp.ToString());
    ReportText += FString::Printf(TEXT("Overall Status: %s\n"), *UEnum::GetValueAsString(Report.OverallStatus));
    ReportText += FString::Printf(TEXT("Validation Time: %.2f seconds\n\n"), Report.TotalValidationTime);
    
    ReportText += TEXT("=== LEVEL STATISTICS ===\n");
    ReportText += FString::Printf(TEXT("Total Actors: %d\n"), Report.TotalActorsInLevel);
    ReportText += FString::Printf(TEXT("Custom Actors: %d\n"), Report.CustomActorsInLevel);
    ReportText += FString::Printf(TEXT("Binary Files: %d\n\n"), Report.BinaryFilesFound);
    
    ReportText += TEXT("=== MODULE STATUS ===\n");
    for (const FBuild_ModuleStatus& ModuleStatus : Report.ModuleStatuses)
    {
        ReportText += FString::Printf(TEXT("Module: %s\n"), *ModuleStatus.ModuleName);
        ReportText += FString::Printf(TEXT("  Status: %s\n"), *UEnum::GetValueAsString(ModuleStatus.Status));
        ReportText += FString::Printf(TEXT("  Classes Loaded: %d\n"), ModuleStatus.ClassesLoaded);
        ReportText += FString::Printf(TEXT("  Classes Failed: %d\n"), ModuleStatus.ClassesFailed);
        ReportText += FString::Printf(TEXT("  Validation Time: %.2f seconds\n"), ModuleStatus.ValidationTime);
        
        if (!ModuleStatus.ErrorMessage.IsEmpty())
        {
            ReportText += FString::Printf(TEXT("  Errors: %s\n"), *ModuleStatus.ErrorMessage);
        }
        ReportText += TEXT("\n");
    }
    
    return ReportText;
}

bool UBuild_IntegrationValidator::FixCommonIntegrationIssues()
{
    UE_LOG(LogTemp, Warning, TEXT("Attempting to fix common integration issues..."));
    
    bool bFixesApplied = false;
    
    // Try to refresh the editor
    if (GEditor)
    {
        GEditor->RedrawAllViewports();
        bFixesApplied = true;
    }
    
    // Force garbage collection
    CollectGarbage(GARBAGE_COLLECTION_KEEPFLAGS);
    bFixesApplied = true;
    
    UE_LOG(LogTemp, Warning, TEXT("Integration fixes applied: %s"), bFixesApplied ? TEXT("Yes") : TEXT("No"));
    
    return bFixesApplied;
}

bool UBuild_IntegrationValidator::RunIntegrationTests()
{
    UE_LOG(LogTemp, Warning, TEXT("Running integration tests..."));
    
    bool bAllTestsPassed = true;
    
    // Test 1: Actor spawning
    if (!TestActorSpawning())
    {
        bAllTestsPassed = false;
        UE_LOG(LogTemp, Error, TEXT("Actor spawning test FAILED"));
    }
    else
    {
        UE_LOG(LogTemp, Log, TEXT("Actor spawning test PASSED"));
    }
    
    // Test 2: System interactions
    if (!TestSystemInteractions())
    {
        bAllTestsPassed = false;
        UE_LOG(LogTemp, Error, TEXT("System interactions test FAILED"));
    }
    else
    {
        UE_LOG(LogTemp, Log, TEXT("System interactions test PASSED"));
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Integration tests result: %s"), 
           bAllTestsPassed ? TEXT("ALL PASSED") : TEXT("SOME FAILED"));
    
    return bAllTestsPassed;
}

bool UBuild_IntegrationValidator::TestActorSpawning()
{
    if (!GEditor || !GEditor->GetEditorWorldContext().World())
    {
        return false;
    }
    
    UWorld* World = GEditor->GetEditorWorldContext().World();
    
    try
    {
        // Test spawning a basic actor
        FVector TestLocation(0.0f, 0.0f, 200.0f);
        FRotator TestRotation = FRotator::ZeroRotator;
        
        AActor* TestActor = World->SpawnActor<AActor>(AActor::StaticClass(), TestLocation, TestRotation);
        
        if (TestActor)
        {
            TestActor->SetActorLabel(TEXT("IntegrationTest_Actor"));
            
            // Clean up
            TestActor->Destroy();
            return true;
        }
    }
    catch (...)
    {
        UE_LOG(LogTemp, Error, TEXT("Exception during actor spawning test"));
    }
    
    return false;
}

bool UBuild_IntegrationValidator::TestSystemInteractions()
{
    // Test basic system interactions
    bool bInteractionsWorking = true;
    
    // Test 1: Check if critical classes can be found
    for (const FString& ClassName : CriticalClasses)
    {
        FString ErrorMessage;
        if (!ValidateClassLoading(ClassName, ErrorMessage))
        {
            bInteractionsWorking = false;
            UE_LOG(LogTemp, Warning, TEXT("System interaction test failed for: %s"), *ClassName);
        }
    }
    
    return bInteractionsWorking;
}

EBuild_IntegrationStatus UBuild_IntegrationValidator::DetermineOverallStatus(const TArray<FBuild_ModuleStatus>& ModuleStatuses)
{
    if (ModuleStatuses.Num() == 0)
    {
        return EBuild_IntegrationStatus::Unknown;
    }
    
    bool bHasCritical = false;
    bool bHasError = false;
    bool bHasWarning = false;
    
    for (const FBuild_ModuleStatus& Status : ModuleStatuses)
    {
        switch (Status.Status)
        {
            case EBuild_IntegrationStatus::Critical:
                bHasCritical = true;
                break;
            case EBuild_IntegrationStatus::Error:
                bHasError = true;
                break;
            case EBuild_IntegrationStatus::Warning:
                bHasWarning = true;
                break;
            default:
                break;
        }
    }
    
    if (bHasCritical)
    {
        return EBuild_IntegrationStatus::Critical;
    }
    else if (bHasError)
    {
        return EBuild_IntegrationStatus::Error;
    }
    else if (bHasWarning)
    {
        return EBuild_IntegrationStatus::Warning;
    }
    else
    {
        return EBuild_IntegrationStatus::Success;
    }
}