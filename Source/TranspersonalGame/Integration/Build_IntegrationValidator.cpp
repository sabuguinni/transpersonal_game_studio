#include "Build_IntegrationValidator.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "UObject/ConstructorHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/GameInstance.h"
#include "Modules/ModuleManager.h"

UBuild_IntegrationValidator::UBuild_IntegrationValidator()
{
    PrimaryComponentTick.bCanEverTick = false;
    
    // Initialize validation state
    OverallStatus = EBuild_ValidationStatus::Unknown;
    ValidationProgress = 0.0f;
    TotalTests = 0;
    PassedTests = 0;
    FailedTests = 0;
    
    // Configuration defaults
    bAutoRunOnBeginPlay = false;
    bVerboseLogging = true;
    
    // Required modules for validation
    RequiredModules.Add(TEXT("TranspersonalGame"));
    RequiredModules.Add(TEXT("Engine"));
    RequiredModules.Add(TEXT("CoreUObject"));
    
    // Required classes for validation
    RequiredClasses.Add(TEXT("TranspersonalGameState"));
    RequiredClasses.Add(TEXT("TranspersonalCharacter"));
    RequiredClasses.Add(TEXT("PCGWorldGenerator"));
    RequiredClasses.Add(TEXT("FoliageManager"));
    RequiredClasses.Add(TEXT("CrowdSimulationManager"));
    RequiredClasses.Add(TEXT("ProceduralWorldManager"));
    RequiredClasses.Add(TEXT("BuildIntegrationManager"));
}

void UBuild_IntegrationValidator::BeginPlay()
{
    Super::BeginPlay();
    
    if (bAutoRunOnBeginPlay)
    {
        RunFullValidation();
    }
}

void UBuild_IntegrationValidator::RunFullValidation()
{
    if (bVerboseLogging)
    {
        UE_LOG(LogTemp, Warning, TEXT("=== BUILD INTEGRATION VALIDATION STARTED ==="));
    }
    
    ResetValidationState();
    OverallStatus = EBuild_ValidationStatus::Running;
    
    // Run all validation tests
    TArray<TPair<FString, TFunction<bool()>>> ValidationTests;
    
    ValidationTests.Add(MakeTuple(TEXT("Module Loading"), [this]() { return ValidateModuleLoading(); }));
    ValidationTests.Add(MakeTuple(TEXT("Class Registration"), [this]() { return ValidateClassRegistration(); }));
    ValidationTests.Add(MakeTuple(TEXT("Level Integrity"), [this]() { return ValidateLevelIntegrity(); }));
    ValidationTests.Add(MakeTuple(TEXT("Actor Spawning"), [this]() { return ValidateActorSpawning(); }));
    
    TotalTests = ValidationTests.Num();
    
    // Execute all tests
    for (const auto& Test : ValidationTests)
    {
        FBuild_ValidationResult Result = RunSingleTest(Test.Key, Test.Value);
        ValidationResults.Add(Result);
        
        if (Result.Status == EBuild_ValidationStatus::Pass)
        {
            PassedTests++;
        }
        else
        {
            FailedTests++;
        }
        
        UpdateValidationProgress();
        LogValidationResult(Result);
    }
    
    // Determine overall status
    if (FailedTests == 0)
    {
        OverallStatus = EBuild_ValidationStatus::Pass;
    }
    else if (PassedTests > FailedTests)
    {
        OverallStatus = EBuild_ValidationStatus::Fail;
    }
    else
    {
        OverallStatus = EBuild_ValidationStatus::Critical;
    }
    
    GenerateIntegrationReport();
    
    if (bVerboseLogging)
    {
        UE_LOG(LogTemp, Warning, TEXT("=== BUILD INTEGRATION VALIDATION COMPLETED ==="));
    }
}

bool UBuild_IntegrationValidator::ValidateModuleLoading()
{
    bool bAllModulesLoaded = true;
    ModuleStatuses.Empty();
    
    for (const FString& ModuleName : RequiredModules)
    {
        FBuild_ModuleStatus ModuleStatus;
        ModuleStatus.ModuleName = ModuleName;
        
        // Check if module is loaded
        if (FModuleManager::Get().IsModuleLoaded(*ModuleName))
        {
            ModuleStatus.bIsLoaded = true;
            
            if (bVerboseLogging)
            {
                UE_LOG(LogTemp, Log, TEXT("✓ Module loaded: %s"), *ModuleName);
            }
        }
        else
        {
            ModuleStatus.bIsLoaded = false;
            bAllModulesLoaded = false;
            
            if (bVerboseLogging)
            {
                UE_LOG(LogTemp, Warning, TEXT("✗ Module not loaded: %s"), *ModuleName);
            }
        }
        
        ModuleStatuses.Add(ModuleStatus);
    }
    
    return bAllModulesLoaded;
}

bool UBuild_IntegrationValidator::ValidateClassRegistration()
{
    bool bAllClassesRegistered = true;
    
    for (const FString& ClassName : RequiredClasses)
    {
        FString ClassPath = FString::Printf(TEXT("/Script/TranspersonalGame.%s"), *ClassName);
        UClass* LoadedClass = LoadClass<UObject>(nullptr, *ClassPath);
        
        if (LoadedClass)
        {
            if (bVerboseLogging)
            {
                UE_LOG(LogTemp, Log, TEXT("✓ Class registered: %s"), *ClassName);
            }
            
            // Update module status with loaded class
            for (FBuild_ModuleStatus& ModuleStatus : ModuleStatuses)
            {
                if (ModuleStatus.ModuleName == TEXT("TranspersonalGame"))
                {
                    ModuleStatus.LoadedClasses.Add(ClassName);
                    ModuleStatus.ClassCount++;
                    break;
                }
            }
        }
        else
        {
            bAllClassesRegistered = false;
            
            if (bVerboseLogging)
            {
                UE_LOG(LogTemp, Warning, TEXT("✗ Class not registered: %s"), *ClassName);
            }
            
            // Update module status with failed class
            for (FBuild_ModuleStatus& ModuleStatus : ModuleStatuses)
            {
                if (ModuleStatus.ModuleName == TEXT("TranspersonalGame"))
                {
                    ModuleStatus.FailedClasses.Add(ClassName);
                    break;
                }
            }
        }
    }
    
    return bAllClassesRegistered;
}

bool UBuild_IntegrationValidator::ValidateLevelIntegrity()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        if (bVerboseLogging)
        {
            UE_LOG(LogTemp, Error, TEXT("✗ No valid world found"));
        }
        return false;
    }
    
    // Count actors in the level
    int32 ActorCount = 0;
    for (TActorIterator<AActor> ActorIterator(World); ActorIterator; ++ActorIterator)
    {
        ActorCount++;
    }
    
    if (bVerboseLogging)
    {
        UE_LOG(LogTemp, Log, TEXT("✓ Level integrity check: %d actors found"), ActorCount);
    }
    
    // Basic integrity check - level should have at least some actors
    return ActorCount > 0;
}

bool UBuild_IntegrationValidator::ValidateActorSpawning()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return false;
    }
    
    // Try to spawn a test actor
    FVector SpawnLocation(1000.0f, 1000.0f, 200.0f);
    FRotator SpawnRotation = FRotator::ZeroRotator;
    
    AActor* TestActor = World->SpawnActor<AActor>(AActor::StaticClass(), SpawnLocation, SpawnRotation);
    
    if (TestActor)
    {
        TestActor->SetActorLabel(TEXT("ValidationTestActor"));
        
        if (bVerboseLogging)
        {
            UE_LOG(LogTemp, Log, TEXT("✓ Actor spawning test successful"));
        }
        
        // Clean up test actor
        TestActor->Destroy();
        return true;
    }
    else
    {
        if (bVerboseLogging)
        {
            UE_LOG(LogTemp, Warning, TEXT("✗ Actor spawning test failed"));
        }
        return false;
    }
}

void UBuild_IntegrationValidator::GenerateIntegrationReport()
{
    if (bVerboseLogging)
    {
        UE_LOG(LogTemp, Warning, TEXT("=== INTEGRATION VALIDATION REPORT ==="));
        UE_LOG(LogTemp, Warning, TEXT("Overall Status: %s"), 
            OverallStatus == EBuild_ValidationStatus::Pass ? TEXT("PASS") :
            OverallStatus == EBuild_ValidationStatus::Fail ? TEXT("FAIL") :
            OverallStatus == EBuild_ValidationStatus::Critical ? TEXT("CRITICAL") : TEXT("UNKNOWN"));
        
        UE_LOG(LogTemp, Warning, TEXT("Tests Passed: %d/%d"), PassedTests, TotalTests);
        UE_LOG(LogTemp, Warning, TEXT("Tests Failed: %d/%d"), FailedTests, TotalTests);
        UE_LOG(LogTemp, Warning, TEXT("Validation Progress: %.1f%%"), ValidationProgress);
        
        UE_LOG(LogTemp, Warning, TEXT("Module Status:"));
        for (const FBuild_ModuleStatus& ModuleStatus : ModuleStatuses)
        {
            UE_LOG(LogTemp, Warning, TEXT("  %s: %s (%d classes)"), 
                *ModuleStatus.ModuleName,
                ModuleStatus.bIsLoaded ? TEXT("LOADED") : TEXT("NOT LOADED"),
                ModuleStatus.ClassCount);
        }
        
        UE_LOG(LogTemp, Warning, TEXT("=== END INTEGRATION REPORT ==="));
    }
}

FBuild_ValidationResult UBuild_IntegrationValidator::RunSingleTest(const FString& TestName, TFunction<bool()> TestFunction)
{
    FBuild_ValidationResult Result;
    Result.TestName = TestName;
    
    double StartTime = FPlatformTime::Seconds();
    
    try
    {
        bool bTestPassed = TestFunction();
        Result.Status = bTestPassed ? EBuild_ValidationStatus::Pass : EBuild_ValidationStatus::Fail;
        
        if (!bTestPassed)
        {
            Result.ErrorMessage = FString::Printf(TEXT("Test '%s' failed validation"), *TestName);
        }
    }
    catch (...)
    {
        Result.Status = EBuild_ValidationStatus::Critical;
        Result.ErrorMessage = FString::Printf(TEXT("Test '%s' threw an exception"), *TestName);
    }
    
    double EndTime = FPlatformTime::Seconds();
    Result.ExecutionTime = static_cast<float>(EndTime - StartTime);
    
    return Result;
}

void UBuild_IntegrationValidator::LogValidationResult(const FBuild_ValidationResult& Result)
{
    if (bVerboseLogging)
    {
        FString StatusText = 
            Result.Status == EBuild_ValidationStatus::Pass ? TEXT("PASS") :
            Result.Status == EBuild_ValidationStatus::Fail ? TEXT("FAIL") :
            Result.Status == EBuild_ValidationStatus::Critical ? TEXT("CRITICAL") : TEXT("UNKNOWN");
        
        UE_LOG(LogTemp, Log, TEXT("Test '%s': %s (%.3fs)"), 
            *Result.TestName, *StatusText, Result.ExecutionTime);
        
        if (!Result.ErrorMessage.IsEmpty())
        {
            UE_LOG(LogTemp, Warning, TEXT("  Error: %s"), *Result.ErrorMessage);
        }
    }
}

void UBuild_IntegrationValidator::UpdateValidationProgress()
{
    if (TotalTests > 0)
    {
        ValidationProgress = (static_cast<float>(PassedTests + FailedTests) / static_cast<float>(TotalTests)) * 100.0f;
    }
}

void UBuild_IntegrationValidator::ResetValidationState()
{
    ValidationResults.Empty();
    ModuleStatuses.Empty();
    OverallStatus = EBuild_ValidationStatus::Unknown;
    ValidationProgress = 0.0f;
    TotalTests = 0;
    PassedTests = 0;
    FailedTests = 0;
}

// === INTEGRATION TEST ACTOR IMPLEMENTATION ===

ABuild_IntegrationTestActor::ABuild_IntegrationTestActor()
{
    PrimaryActorTick.bCanEverTick = false;
    
    // Create integration validator component
    IntegrationValidator = CreateDefaultSubobject<UBuild_IntegrationValidator>(TEXT("IntegrationValidator"));
    
    // Create test mesh component
    TestMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TestMesh"));
    RootComponent = TestMesh;
    
    // Try to load a basic cube mesh for testing
    static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMeshAsset(TEXT("/Engine/BasicShapes/Cube"));
    if (CubeMeshAsset.Succeeded())
    {
        TestMesh->SetStaticMesh(CubeMeshAsset.Object);
    }
}

void ABuild_IntegrationTestActor::BeginPlay()
{
    Super::BeginPlay();
    
    // Set actor label for identification
    SetActorLabel(TEXT("IntegrationTestActor"));
}

void ABuild_IntegrationTestActor::RunIntegrationTests()
{
    if (IntegrationValidator)
    {
        IntegrationValidator->RunFullValidation();
        
        // Fire blueprint event with validation status
        OnValidationComplete(IntegrationValidator->GetOverallStatus());
    }
}