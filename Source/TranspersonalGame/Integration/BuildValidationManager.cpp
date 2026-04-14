#include "BuildValidationManager.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/Paths.h"
#include "Misc/DateTime.h"
#include "EditorLevelLibrary.h"
#include "UObject/UObjectGlobals.h"

ABuildValidationManager::ABuildValidationManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.bStartWithTickEnabled = true;
    
    InitializeExpectedClasses();
}

void ABuildValidationManager::BeginPlay()
{
    Super::BeginPlay();
    
    if (bAutoRunValidationOnBeginPlay)
    {
        // Run validation after a short delay to allow other systems to initialize
        FTimerHandle ValidationTimer;
        GetWorld()->GetTimerManager().SetTimer(ValidationTimer, [this]()
        {
            RunFullValidation();
        }, 2.0f, false);
    }
}

void ABuildValidationManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    if (!bValidationInProgress && ValidationInterval > 0.0f)
    {
        TimeSinceLastValidation += DeltaTime;
        
        if (TimeSinceLastValidation >= ValidationInterval)
        {
            RunFullValidation();
            TimeSinceLastValidation = 0.0f;
        }
    }
}

FBuild_ValidationReport ABuildValidationManager::RunFullValidation()
{
    if (bValidationInProgress)
    {
        UE_LOG(LogTemp, Warning, TEXT("Validation already in progress"));
        return LastValidationReport;
    }
    
    bValidationInProgress = true;
    float StartTime = FPlatformTime::Seconds();
    
    UE_LOG(LogTemp, Log, TEXT("=== Starting Full Build Validation ==="));
    
    // Initialize report
    FBuild_ValidationReport Report;
    Report.ValidationTimestamp = FDateTime::Now().ToString();
    Report.ClassesExpected = ExpectedClasses.Num();
    
    // Run validation tests
    bool bClassValidation = ValidateClassLoading();
    bool bBinaryValidation = ValidateBinaryFiles();
    bool bSourceValidation = ValidateSourceFiles();
    
    // Update report with results
    Report.ClassesLoaded = 0;
    Report.FailedClasses.Empty();
    
    for (const FString& ClassName : ExpectedClasses)
    {
        if (CheckClassExists(ClassName))
        {
            Report.ClassesLoaded++;
        }
        else
        {
            Report.FailedClasses.Add(ClassName);
        }
    }
    
    Report.BinaryFilesFound = CountBinaryFiles();
    Report.SourceFilesFound = CountSourceFiles();
    
    // Determine overall result
    if (Report.ClassesLoaded == Report.ClassesExpected && Report.BinaryFilesFound > 0)
    {
        Report.Result = EBuild_ValidationResult::Pass;
    }
    else if (Report.ClassesLoaded > Report.ClassesExpected / 2)
    {
        Report.Result = EBuild_ValidationResult::PartialPass;
    }
    else
    {
        Report.Result = EBuild_ValidationResult::Fail;
    }
    
    Report.ValidationDuration = FPlatformTime::Seconds() - StartTime;
    LastValidationReport = Report;
    
    LogValidationResults(Report);
    
    bValidationInProgress = false;
    
    UE_LOG(LogTemp, Log, TEXT("=== Build Validation Complete ==="));
    
    return Report;
}

bool ABuildValidationManager::ValidateClassLoading()
{
    UE_LOG(LogTemp, Log, TEXT("Validating class loading..."));
    
    int32 LoadedCount = 0;
    for (const FString& ClassName : ExpectedClasses)
    {
        if (CheckClassExists(ClassName))
        {
            LoadedCount++;
            UE_LOG(LogTemp, Log, TEXT("✓ Class loaded: %s"), *ClassName);
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("✗ Class failed to load: %s"), *ClassName);
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Class validation: %d/%d loaded"), LoadedCount, ExpectedClasses.Num());
    return LoadedCount == ExpectedClasses.Num();
}

bool ABuildValidationManager::ValidateBinaryFiles()
{
    UE_LOG(LogTemp, Log, TEXT("Validating binary files..."));
    
    int32 BinaryCount = CountBinaryFiles();
    bool bValid = BinaryCount > 0;
    
    UE_LOG(LogTemp, Log, TEXT("Binary validation: %d files found - %s"), 
           BinaryCount, bValid ? TEXT("PASS") : TEXT("FAIL"));
    
    return bValid;
}

bool ABuildValidationManager::ValidateSourceFiles()
{
    UE_LOG(LogTemp, Log, TEXT("Validating source files..."));
    
    int32 SourceCount = CountSourceFiles();
    bool bValid = SourceCount > 10; // Expect at least 10 source files
    
    UE_LOG(LogTemp, Log, TEXT("Source validation: %d files found - %s"), 
           SourceCount, bValid ? TEXT("PASS") : TEXT("FAIL"));
    
    return bValid;
}

void ABuildValidationManager::RunIntegrationTests()
{
    UE_LOG(LogTemp, Log, TEXT("Running integration tests..."));
    
    // Test 1: Spawn test actors
    int32 SpawnedActors = 0;
    for (const FString& ClassName : ExpectedClasses)
    {
        UClass* ActorClass = FindObject<UClass>(ANY_PACKAGE, *FString::Printf(TEXT("/Script/TranspersonalGame.%s"), *ClassName));
        if (ActorClass && ActorClass->IsChildOf(AActor::StaticClass()))
        {
            FVector SpawnLocation(SpawnedActors * 200.0f, 0.0f, 100.0f);
            AActor* SpawnedActor = GetWorld()->SpawnActor<AActor>(ActorClass, SpawnLocation, FRotator::ZeroRotator);
            if (SpawnedActor)
            {
                SpawnedActor->SetActorLabel(FString::Printf(TEXT("IntegrationTest_%s"), *ClassName));
                SpawnedActors++;
                UE_LOG(LogTemp, Log, TEXT("✓ Spawned integration test actor: %s"), *ClassName);
            }
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Integration tests: %d actors spawned"), SpawnedActors);
}

void ABuildValidationManager::GenerateValidationReport()
{
    UE_LOG(LogTemp, Log, TEXT("=== VALIDATION REPORT ==="));
    UE_LOG(LogTemp, Log, TEXT("Timestamp: %s"), *LastValidationReport.ValidationTimestamp);
    UE_LOG(LogTemp, Log, TEXT("Duration: %.2f seconds"), LastValidationReport.ValidationDuration);
    UE_LOG(LogTemp, Log, TEXT("Result: %s"), 
           LastValidationReport.Result == EBuild_ValidationResult::Pass ? TEXT("PASS") :
           LastValidationReport.Result == EBuild_ValidationResult::PartialPass ? TEXT("PARTIAL PASS") : TEXT("FAIL"));
    UE_LOG(LogTemp, Log, TEXT("Classes: %d/%d loaded"), LastValidationReport.ClassesLoaded, LastValidationReport.ClassesExpected);
    UE_LOG(LogTemp, Log, TEXT("Binaries: %d found"), LastValidationReport.BinaryFilesFound);
    UE_LOG(LogTemp, Log, TEXT("Sources: %d found"), LastValidationReport.SourceFilesFound);
    
    if (LastValidationReport.FailedClasses.Num() > 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("Failed classes:"));
        for (const FString& FailedClass : LastValidationReport.FailedClasses)
        {
            UE_LOG(LogTemp, Warning, TEXT("  - %s"), *FailedClass);
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("=== END REPORT ==="));
}

bool ABuildValidationManager::IsValidationPassing() const
{
    return LastValidationReport.Result == EBuild_ValidationResult::Pass ||
           LastValidationReport.Result == EBuild_ValidationResult::PartialPass;
}

void ABuildValidationManager::InitializeExpectedClasses()
{
    ExpectedClasses = {
        TEXT("TranspersonalGameState"),
        TEXT("TranspersonalCharacter"),
        TEXT("PCGWorldGenerator"),
        TEXT("FoliageManager"),
        TEXT("CrowdSimulationManager"),
        TEXT("ProceduralWorldManager"),
        TEXT("BuildIntegrationManager")
    };
}

bool ABuildValidationManager::CheckClassExists(const FString& ClassName)
{
    FString FullClassName = FString::Printf(TEXT("/Script/TranspersonalGame.%s"), *ClassName);
    UClass* FoundClass = FindObject<UClass>(ANY_PACKAGE, *FullClassName);
    return FoundClass != nullptr;
}

int32 ABuildValidationManager::CountBinaryFiles()
{
    FString ProjectDir = FPaths::ProjectDir();
    FString BinariesDir = FPaths::Combine(ProjectDir, TEXT("Binaries"), TEXT("Linux"));
    
    TArray<FString> BinaryFiles;
    IFileManager& FileManager = IFileManager::Get();
    FileManager.FindFiles(BinaryFiles, *FPaths::Combine(BinariesDir, TEXT("*.so")), true, false);
    
    return BinaryFiles.Num();
}

int32 ABuildValidationManager::CountSourceFiles()
{
    FString ProjectDir = FPaths::ProjectDir();
    FString SourceDir = FPaths::Combine(ProjectDir, TEXT("Source"), TEXT("TranspersonalGame"));
    
    TArray<FString> SourceFiles;
    IFileManager& FileManager = IFileManager::Get();
    FileManager.FindFilesRecursive(SourceFiles, *SourceDir, TEXT("*.cpp"), true, false);
    FileManager.FindFilesRecursive(SourceFiles, *SourceDir, TEXT("*.h"), true, false);
    
    return SourceFiles.Num();
}

void ABuildValidationManager::LogValidationResults(const FBuild_ValidationReport& Report)
{
    FString ResultString;
    switch (Report.Result)
    {
        case EBuild_ValidationResult::Pass:
            ResultString = TEXT("🟢 PASS");
            break;
        case EBuild_ValidationResult::PartialPass:
            ResultString = TEXT("🟡 PARTIAL PASS");
            break;
        case EBuild_ValidationResult::Fail:
            ResultString = TEXT("🔴 FAIL");
            break;
        default:
            ResultString = TEXT("⚪ NOT RUN");
            break;
    }
    
    UE_LOG(LogTemp, Log, TEXT("VALIDATION RESULT: %s"), *ResultString);
    UE_LOG(LogTemp, Log, TEXT("Classes: %d/%d, Binaries: %d, Sources: %d, Duration: %.2fs"),
           Report.ClassesLoaded, Report.ClassesExpected, Report.BinaryFilesFound, 
           Report.SourceFilesFound, Report.ValidationDuration);
}