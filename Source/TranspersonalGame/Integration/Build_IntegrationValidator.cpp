#include "Build_IntegrationValidator.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/GameStateBase.h"
#include "Kismet/GameplayStatics.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/FileHelper.h"
#include "Misc/DateTime.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"

UBuild_IntegrationValidator::UBuild_IntegrationValidator()
{
    PrimaryComponentTick.bCanEverTick = false;
    
    MaxAllowedActors = 8000;
    bAutoEnforceActorCap = true;
    LastValidationTime = 0.0f;
    
    // Initialize essential actor labels
    EssentialActorLabels = {
        TEXT("playerstart"),
        TEXT("directionallight"),
        TEXT("skylight"),
        TEXT("skyatmosphere"),
        TEXT("fog"),
        TEXT("trex"),
        TEXT("veloci"),
        TEXT("tricera"),
        TEXT("brachi"),
        TEXT("ankylo"),
        TEXT("parasauro"),
        TEXT("pachy"),
        TEXT("proto"),
        TEXT("tsinta")
    };
    
    // Initialize critical class paths
    CriticalClassPaths = {
        TEXT("/Script/TranspersonalGame.TranspersonalCharacter"),
        TEXT("/Script/TranspersonalGame.TranspersonalGameState"),
        TEXT("/Script/TranspersonalGame.PCGWorldGenerator"),
        TEXT("/Script/TranspersonalGame.FoliageManager"),
        TEXT("/Script/TranspersonalGame.CrowdSimulationManager"),
        TEXT("/Script/TranspersonalGame.ProceduralWorldManager")
    };
}

void UBuild_IntegrationValidator::BeginPlay()
{
    Super::BeginPlay();
    
    if (bAutoEnforceActorCap)
    {
        EnforceActorCap(MaxAllowedActors);
    }
}

TArray<FBuild_ValidationReport> UBuild_IntegrationValidator::RunFullValidationSuite()
{
    LastValidationResults.Empty();
    float StartTime = FPlatformTime::Seconds();
    
    // Run all validation tests
    LastValidationResults.Add(ValidateClassLoading());
    LastValidationResults.Add(ValidateCrossSystemIntegration());
    LastValidationResults.Add(ValidateActorSpawning());
    LastValidationResults.Add(ValidateComponentSystems());
    LastValidationResults.Add(ValidateGameplayFramework());
    
    LastValidationTime = FPlatformTime::Seconds() - StartTime;
    
    UE_LOG(LogTemp, Warning, TEXT("Build_IntegrationValidator: Full validation suite completed in %s"), 
           *FormatValidationTime(LastValidationTime));
    
    return LastValidationResults;
}

FBuild_ValidationReport UBuild_IntegrationValidator::ValidateClassLoading()
{
    FBuild_ValidationReport Report;
    Report.TestName = TEXT("Class Loading Validation");
    float StartTime = FPlatformTime::Seconds();
    
    int32 LoadedCount = 0;
    int32 TotalCount = CriticalClassPaths.Num();
    FString FailedClasses;
    
    for (const FString& ClassPath : CriticalClassPaths)
    {
        if (ValidateClassPath(ClassPath))
        {
            LoadedCount++;
        }
        else
        {
            FailedClasses += ClassPath + TEXT(", ");
        }
    }
    
    Report.ExecutionTime = FPlatformTime::Seconds() - StartTime;
    
    if (LoadedCount == TotalCount)
    {
        Report.Result = EBuild_ValidationResult::Success;
        Report.Message = FString::Printf(TEXT("All %d critical classes loaded successfully"), TotalCount);
    }
    else if (LoadedCount > TotalCount / 2)
    {
        Report.Result = EBuild_ValidationResult::Warning;
        Report.Message = FString::Printf(TEXT("%d/%d classes loaded. Failed: %s"), 
                                       LoadedCount, TotalCount, *FailedClasses);
    }
    else
    {
        Report.Result = EBuild_ValidationResult::Error;
        Report.Message = FString::Printf(TEXT("Critical failure: Only %d/%d classes loaded. Failed: %s"), 
                                       LoadedCount, TotalCount, *FailedClasses);
    }
    
    return Report;
}

FBuild_ValidationReport UBuild_IntegrationValidator::ValidateCrossSystemIntegration()
{
    FBuild_ValidationReport Report;
    Report.TestName = TEXT("Cross-System Integration");
    float StartTime = FPlatformTime::Seconds();
    
    int32 PassedTests = 0;
    int32 TotalTests = 3;
    
    // Test 1: World access
    UWorld* World = GetWorld();
    if (World)
    {
        PassedTests++;
    }
    
    // Test 2: GameState access
    AGameStateBase* GameState = UGameplayStatics::GetGameState(World);
    if (GameState)
    {
        PassedTests++;
    }
    
    // Test 3: Actor iteration
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);
    if (AllActors.Num() > 0)
    {
        PassedTests++;
    }
    
    Report.ExecutionTime = FPlatformTime::Seconds() - StartTime;
    
    if (PassedTests == TotalTests)
    {
        Report.Result = EBuild_ValidationResult::Success;
        Report.Message = FString::Printf(TEXT("All %d cross-system tests passed"), TotalTests);
    }
    else
    {
        Report.Result = EBuild_ValidationResult::Warning;
        Report.Message = FString::Printf(TEXT("%d/%d cross-system tests passed"), PassedTests, TotalTests);
    }
    
    return Report;
}

FBuild_ValidationReport UBuild_IntegrationValidator::ValidateActorSpawning()
{
    FBuild_ValidationReport Report;
    Report.TestName = TEXT("Actor Spawning Validation");
    float StartTime = FPlatformTime::Seconds();
    
    int32 SpawnedCount = 0;
    UWorld* World = GetWorld();
    
    if (!World)
    {
        Report.Result = EBuild_ValidationResult::Error;
        Report.Message = TEXT("No valid world context for spawning test");
        Report.ExecutionTime = FPlatformTime::Seconds() - StartTime;
        return Report;
    }
    
    // Test spawning a basic StaticMeshActor
    FVector TestLocation(5000.0f, 5000.0f, 200.0f);
    AStaticMeshActor* TestActor = World->SpawnActor<AStaticMeshActor>(TestLocation, FRotator::ZeroRotator);
    
    if (TestActor)
    {
        TestActor->SetActorLabel(TEXT("IntegrationTest_SpawnValidation"));
        SpawnedTestActors.Add(TestActor);
        SpawnedCount++;
    }
    
    Report.ExecutionTime = FPlatformTime::Seconds() - StartTime;
    
    if (SpawnedCount > 0)
    {
        Report.Result = EBuild_ValidationResult::Success;
        Report.Message = FString::Printf(TEXT("Actor spawning successful (%d test actors created)"), SpawnedCount);
    }
    else
    {
        Report.Result = EBuild_ValidationResult::Error;
        Report.Message = TEXT("Actor spawning failed - no test actors created");
    }
    
    return Report;
}

FBuild_ValidationReport UBuild_IntegrationValidator::ValidateComponentSystems()
{
    FBuild_ValidationReport Report;
    Report.TestName = TEXT("Component Systems Validation");
    float StartTime = FPlatformTime::Seconds();
    
    int32 ValidComponents = 0;
    AActor* Owner = GetOwner();
    
    if (Owner)
    {
        TArray<UActorComponent*> Components = Owner->GetComponents().Array();
        for (UActorComponent* Component : Components)
        {
            if (Component && IsValid(Component))
            {
                ValidComponents++;
            }
        }
    }
    
    Report.ExecutionTime = FPlatformTime::Seconds() - StartTime;
    
    if (ValidComponents > 0)
    {
        Report.Result = EBuild_ValidationResult::Success;
        Report.Message = FString::Printf(TEXT("Component system functional (%d valid components)"), ValidComponents);
    }
    else
    {
        Report.Result = EBuild_ValidationResult::Warning;
        Report.Message = TEXT("No valid components found for testing");
    }
    
    return Report;
}

FBuild_ValidationReport UBuild_IntegrationValidator::ValidateGameplayFramework()
{
    FBuild_ValidationReport Report;
    Report.TestName = TEXT("Gameplay Framework Validation");
    float StartTime = FPlatformTime::Seconds();
    
    int32 FrameworkTests = 0;
    UWorld* World = GetWorld();
    
    if (World)
    {
        // Test GameMode access
        AGameModeBase* GameMode = World->GetAuthGameMode();
        if (GameMode) FrameworkTests++;
        
        // Test GameState access
        AGameStateBase* GameState = World->GetGameState();
        if (GameState) FrameworkTests++;
        
        // Test PlayerController access (in editor, this might be null)
        APlayerController* PC = World->GetFirstPlayerController();
        if (PC) FrameworkTests++;
    }
    
    Report.ExecutionTime = FPlatformTime::Seconds() - StartTime;
    
    if (FrameworkTests >= 2)
    {
        Report.Result = EBuild_ValidationResult::Success;
        Report.Message = FString::Printf(TEXT("Gameplay framework operational (%d/3 systems active)"), FrameworkTests);
    }
    else if (FrameworkTests >= 1)
    {
        Report.Result = EBuild_ValidationResult::Warning;
        Report.Message = FString::Printf(TEXT("Partial gameplay framework (%d/3 systems active)"), FrameworkTests);
    }
    else
    {
        Report.Result = EBuild_ValidationResult::Error;
        Report.Message = TEXT("Gameplay framework not accessible");
    }
    
    return Report;
}

int32 UBuild_IntegrationValidator::GetCurrentActorCount()
{
    UWorld* World = GetWorld();
    if (!World) return 0;
    
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);
    return AllActors.Num();
}

bool UBuild_IntegrationValidator::EnforceActorCap(int32 MaxActors)
{
    UWorld* World = GetWorld();
    if (!World) return false;
    
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);
    
    if (AllActors.Num() <= MaxActors)
    {
        return true; // Already within limits
    }
    
    TArray<AActor*> NonEssentialActors = GetNonEssentialActors();
    int32 ActorsToRemove = AllActors.Num() - MaxActors;
    int32 RemovedCount = 0;
    
    for (int32 i = 0; i < FMath::Min(NonEssentialActors.Num(), ActorsToRemove); i++)
    {
        if (IsValid(NonEssentialActors[i]))
        {
            NonEssentialActors[i]->Destroy();
            RemovedCount++;
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Build_IntegrationValidator: Enforced actor cap - removed %d actors"), RemovedCount);
    return RemovedCount > 0;
}

TArray<AActor*> UBuild_IntegrationValidator::GetNonEssentialActors()
{
    TArray<AActor*> NonEssential;
    UWorld* World = GetWorld();
    if (!World) return NonEssential;
    
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);
    
    for (AActor* Actor : AllActors)
    {
        if (!IsValid(Actor)) continue;
        
        FString ActorLabel = Actor->GetActorLabel().ToLower();
        bool bIsEssential = false;
        
        for (const FString& EssentialLabel : EssentialActorLabels)
        {
            if (ActorLabel.Contains(EssentialLabel.ToLower()))
            {
                bIsEssential = true;
                break;
            }
        }
        
        if (!bIsEssential)
        {
            NonEssential.Add(Actor);
        }
    }
    
    return NonEssential;
}

FString UBuild_IntegrationValidator::GenerateBuildStatusReport()
{
    FString Report;
    FDateTime Now = FDateTime::Now();
    
    Report += FString::Printf(TEXT("=== BUILD INTEGRATION REPORT ===\n"));
    Report += FString::Printf(TEXT("Generated: %s\n"), *Now.ToString());
    Report += FString::Printf(TEXT("Validation Time: %s\n\n"), *FormatValidationTime(LastValidationTime));
    
    Report += FString::Printf(TEXT("ACTOR STATUS:\n"));
    Report += FString::Printf(TEXT("- Current Count: %d\n"), GetCurrentActorCount());
    Report += FString::Printf(TEXT("- Max Allowed: %d\n"), MaxAllowedActors);
    Report += FString::Printf(TEXT("- Auto Cap Enforcement: %s\n\n"), bAutoEnforceActorCap ? TEXT("Enabled") : TEXT("Disabled"));
    
    Report += FString::Printf(TEXT("VALIDATION RESULTS:\n"));
    for (const FBuild_ValidationReport& ValidationReport : LastValidationResults)
    {
        FString ResultText;
        switch (ValidationReport.Result)
        {
            case EBuild_ValidationResult::Success: ResultText = TEXT("PASS"); break;
            case EBuild_ValidationResult::Warning: ResultText = TEXT("WARN"); break;
            case EBuild_ValidationResult::Error: ResultText = TEXT("FAIL"); break;
            case EBuild_ValidationResult::Critical: ResultText = TEXT("CRIT"); break;
        }
        
        Report += FString::Printf(TEXT("- %s: %s (%s)\n"), 
                                *ValidationReport.TestName, 
                                *ResultText, 
                                *ValidationReport.Message);
    }
    
    return Report;
}

bool UBuild_IntegrationValidator::SaveValidationReport(const FString& ReportContent)
{
    FString FilePath = FPaths::ProjectLogDir() / TEXT("BuildValidation_") + 
                      FDateTime::Now().ToString(TEXT("%Y%m%d_%H%M%S")) + TEXT(".txt");
    
    return FFileHelper::SaveStringToFile(ReportContent, *FilePath);
}

bool UBuild_IntegrationValidator::ValidateClassPath(const FString& ClassPath)
{
    UClass* LoadedClass = LoadClass<UObject>(nullptr, *ClassPath);
    return LoadedClass != nullptr;
}

bool UBuild_IntegrationValidator::TestActorSpawn(UClass* ActorClass, const FVector& Location)
{
    if (!ActorClass) return false;
    
    UWorld* World = GetWorld();
    if (!World) return false;
    
    AActor* TestActor = World->SpawnActor(ActorClass, &Location);
    if (TestActor)
    {
        SpawnedTestActors.Add(TestActor);
        return true;
    }
    
    return false;
}

void UBuild_IntegrationValidator::CleanupTestActors()
{
    for (AActor* TestActor : SpawnedTestActors)
    {
        if (IsValid(TestActor))
        {
            TestActor->Destroy();
        }
    }
    SpawnedTestActors.Empty();
}

FString UBuild_IntegrationValidator::FormatValidationTime(float TimeInSeconds)
{
    if (TimeInSeconds < 1.0f)
    {
        return FString::Printf(TEXT("%.0fms"), TimeInSeconds * 1000.0f);
    }
    else
    {
        return FString::Printf(TEXT("%.2fs"), TimeInSeconds);
    }
}