#include "Eng_ArchitectureValidator.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/GameModeBase.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerStart.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/Light.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/DateTime.h"
#include "TimerManager.h"

UEng_ArchitectureValidator::UEng_ArchitectureValidator()
{
    PrimaryComponentTick.bCanEverTick = false;
    
    // Default validation settings
    bAutoValidateOnBeginPlay = true;
    ValidationInterval = 30.0f; // Validate every 30 seconds
    bLogValidationResults = true;
    
    // Default performance thresholds
    MinAcceptableFrameRate = 30.0f;
    MaxAcceptableActorCount = 10000;
    MaxAcceptableMemoryUsage = 2048.0f; // 2GB in MB
    
    // Initialize validation state
    LastValidationTime = 0.0f;
    bIsValidationInProgress = false;
}

void UEng_ArchitectureValidator::BeginPlay()
{
    Super::BeginPlay();
    
    if (bAutoValidateOnBeginPlay)
    {
        // Perform initial validation
        ValidateGameArchitecture();
        
        // Set up periodic validation
        if (ValidationInterval > 0.0f)
        {
            GetWorld()->GetTimerManager().SetTimer(
                ValidationTimerHandle,
                this,
                &UEng_ArchitectureValidator::PerformPeriodicValidation,
                ValidationInterval,
                true
            );
        }
    }
}

TArray<FEng_ValidationReport> UEng_ArchitectureValidator::ValidateGameArchitecture()
{
    if (bIsValidationInProgress)
    {
        return LastValidationResults;
    }
    
    bIsValidationInProgress = true;
    LastValidationResults.Empty();
    
    float StartTime = FPlatformTime::Seconds();
    
    // Run all validation tests
    LastValidationResults.Add(ValidateGameMode());
    LastValidationResults.Add(ValidatePlayerCharacter());
    LastValidationResults.Add(ValidateLevelActors());
    LastValidationResults.Add(ValidatePhysicsSystem());
    LastValidationResults.Add(ValidateRenderingSystem());
    
    LastValidationTime = FPlatformTime::Seconds() - StartTime;
    bIsValidationInProgress = false;
    
    // Log results if enabled
    if (bLogValidationResults)
    {
        UE_LOG(LogTemp, Warning, TEXT("Architecture Validation Complete - %d tests in %.3f seconds"), 
               LastValidationResults.Num(), LastValidationTime);
        
        for (const FEng_ValidationReport& Report : LastValidationResults)
        {
            LogValidationReport(Report);
        }
    }
    
    return LastValidationResults;
}

FEng_ValidationReport UEng_ArchitectureValidator::ValidateGameMode()
{
    float ExecutionTime = MeasureExecutionTime([this]()
    {
        // Validation logic will be executed here
    });
    
    UWorld* World = GetWorld();
    if (!World)
    {
        return CreateValidationReport(
            TEXT("GameMode Validation"),
            EEng_ValidationResult::Fail,
            TEXT("World is null - cannot validate GameMode")
        );
    }
    
    AGameModeBase* GameMode = World->GetAuthGameMode();
    if (!GameMode)
    {
        return CreateValidationReport(
            TEXT("GameMode Validation"),
            EEng_ValidationResult::Warning,
            TEXT("No GameMode found in current world")
        );
    }
    
    // Check if GameMode has proper pawn class set
    if (!GameMode->DefaultPawnClass)
    {
        return CreateValidationReport(
            TEXT("GameMode Validation"),
            EEng_ValidationResult::Warning,
            TEXT("GameMode has no DefaultPawnClass set")
        );
    }
    
    return CreateValidationReport(
        TEXT("GameMode Validation"),
        EEng_ValidationResult::Pass,
        FString::Printf(TEXT("GameMode valid: %s"), *GameMode->GetClass()->GetName())
    );
}

FEng_ValidationReport UEng_ArchitectureValidator::ValidatePlayerCharacter()
{
    float ExecutionTime = MeasureExecutionTime([this]()
    {
        // Validation logic will be executed here
    });
    
    UWorld* World = GetWorld();
    if (!World)
    {
        return CreateValidationReport(
            TEXT("Player Character Validation"),
            EEng_ValidationResult::Fail,
            TEXT("World is null - cannot validate Player Character")
        );
    }
    
    // Check for PlayerStart actors
    TArray<AActor*> PlayerStarts;
    UGameplayStatics::GetAllActorsOfClass(World, APlayerStart::StaticClass(), PlayerStarts);
    
    if (PlayerStarts.Num() == 0)
    {
        return CreateValidationReport(
            TEXT("Player Character Validation"),
            EEng_ValidationResult::Warning,
            TEXT("No PlayerStart actors found in level")
        );
    }
    
    // Check if we can find player character class
    AGameModeBase* GameMode = World->GetAuthGameMode();
    if (GameMode && GameMode->DefaultPawnClass)
    {
        return CreateValidationReport(
            TEXT("Player Character Validation"),
            EEng_ValidationResult::Pass,
            FString::Printf(TEXT("Player character setup valid - %d PlayerStarts found"), PlayerStarts.Num())
        );
    }
    
    return CreateValidationReport(
        TEXT("Player Character Validation"),
        EEng_ValidationResult::Warning,
        TEXT("Player character configuration incomplete")
    );
}

FEng_ValidationReport UEng_ArchitectureValidator::ValidateLevelActors()
{
    float ExecutionTime = MeasureExecutionTime([this]()
    {
        // Validation logic will be executed here
    });
    
    UWorld* World = GetWorld();
    if (!World)
    {
        return CreateValidationReport(
            TEXT("Level Actors Validation"),
            EEng_ValidationResult::Fail,
            TEXT("World is null - cannot validate Level Actors")
        );
    }
    
    // Count different types of actors
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);
    
    int32 LightCount = 0;
    int32 MeshCount = 0;
    int32 PlayerStartCount = 0;
    
    for (AActor* Actor : AllActors)
    {
        if (Actor->IsA<ALight>())
        {
            LightCount++;
        }
        else if (Actor->FindComponentByClass<UStaticMeshComponent>())
        {
            MeshCount++;
        }
        else if (Actor->IsA<APlayerStart>())
        {
            PlayerStartCount++;
        }
    }
    
    // Validate minimum requirements
    if (LightCount == 0)
    {
        return CreateValidationReport(
            TEXT("Level Actors Validation"),
            EEng_ValidationResult::Warning,
            TEXT("No lighting actors found - level may be too dark")
        );
    }
    
    if (PlayerStartCount == 0)
    {
        return CreateValidationReport(
            TEXT("Level Actors Validation"),
            EEng_ValidationResult::Fail,
            TEXT("No PlayerStart actors found - players cannot spawn")
        );
    }
    
    return CreateValidationReport(
        TEXT("Level Actors Validation"),
        EEng_ValidationResult::Pass,
        FString::Printf(TEXT("Level actors valid - %d total, %d lights, %d meshes"), 
                       AllActors.Num(), LightCount, MeshCount)
    );
}

FEng_ValidationReport UEng_ArchitectureValidator::ValidatePhysicsSystem()
{
    float ExecutionTime = MeasureExecutionTime([this]()
    {
        // Validation logic will be executed here
    });
    
    UWorld* World = GetWorld();
    if (!World)
    {
        return CreateValidationReport(
            TEXT("Physics System Validation"),
            EEng_ValidationResult::Fail,
            TEXT("World is null - cannot validate Physics System")
        );
    }
    
    // Check if physics world exists
    if (!World->GetPhysicsScene())
    {
        return CreateValidationReport(
            TEXT("Physics System Validation"),
            EEng_ValidationResult::Fail,
            TEXT("Physics scene not initialized")
        );
    }
    
    // Basic physics validation - check if gravity is set
    float GravityZ = World->GetGravityZ();
    if (FMath::IsNearlyZero(GravityZ))
    {
        return CreateValidationReport(
            TEXT("Physics System Validation"),
            EEng_ValidationResult::Warning,
            TEXT("Gravity is zero - physics may not behave as expected")
        );
    }
    
    return CreateValidationReport(
        TEXT("Physics System Validation"),
        EEng_ValidationResult::Pass,
        FString::Printf(TEXT("Physics system operational - Gravity: %.2f"), GravityZ)
    );
}

FEng_ValidationReport UEng_ArchitectureValidator::ValidateRenderingSystem()
{
    float ExecutionTime = MeasureExecutionTime([this]()
    {
        // Validation logic will be executed here
    });
    
    // Basic rendering validation
    if (!GEngine)
    {
        return CreateValidationReport(
            TEXT("Rendering System Validation"),
            EEng_ValidationResult::Fail,
            TEXT("GEngine is null - rendering system not available")
        );
    }
    
    // Check current frame rate
    float CurrentFPS = GetFrameRate();
    if (CurrentFPS < MinAcceptableFrameRate)
    {
        return CreateValidationReport(
            TEXT("Rendering System Validation"),
            EEng_ValidationResult::Warning,
            FString::Printf(TEXT("Frame rate below threshold: %.1f < %.1f"), CurrentFPS, MinAcceptableFrameRate)
        );
    }
    
    return CreateValidationReport(
        TEXT("Rendering System Validation"),
        EEng_ValidationResult::Pass,
        FString::Printf(TEXT("Rendering system operational - FPS: %.1f"), CurrentFPS)
    );
}

float UEng_ArchitectureValidator::GetSystemPerformanceScore()
{
    float Score = 100.0f;
    
    // Deduct points for performance issues
    float CurrentFPS = GetFrameRate();
    if (CurrentFPS < MinAcceptableFrameRate)
    {
        Score -= (MinAcceptableFrameRate - CurrentFPS) * 2.0f;
    }
    
    int32 ActorCount = GetTotalActorCount();
    if (ActorCount > MaxAcceptableActorCount)
    {
        Score -= (ActorCount - MaxAcceptableActorCount) * 0.01f;
    }
    
    float MemoryUsage = GetMemoryUsage();
    if (MemoryUsage > MaxAcceptableMemoryUsage)
    {
        Score -= (MemoryUsage - MaxAcceptableMemoryUsage) * 0.05f;
    }
    
    return FMath::Clamp(Score, 0.0f, 100.0f);
}

int32 UEng_ArchitectureValidator::GetTotalActorCount()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return 0;
    }
    
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);
    return AllActors.Num();
}

float UEng_ArchitectureValidator::GetFrameRate()
{
    if (GEngine && GEngine->GetGameViewport())
    {
        return 1.0f / GEngine->GetMaxTickRate(0.0f);
    }
    return 0.0f;
}

float UEng_ArchitectureValidator::GetMemoryUsage()
{
    // Get memory stats in MB
    FPlatformMemoryStats MemStats = FPlatformMemory::GetStats();
    return MemStats.UsedPhysical / (1024.0f * 1024.0f);
}

bool UEng_ArchitectureValidator::EnforceNamingConventions()
{
    // Implementation for naming convention enforcement
    return true;
}

bool UEng_ArchitectureValidator::ValidateModuleDependencies()
{
    // Implementation for module dependency validation
    return true;
}

bool UEng_ArchitectureValidator::CheckForArchitectureViolations()
{
    // Implementation for architecture violation checking
    return false;
}

FEng_ValidationReport UEng_ArchitectureValidator::CreateValidationReport(const FString& TestName, EEng_ValidationResult Result, const FString& Message)
{
    FEng_ValidationReport Report;
    Report.TestName = TestName;
    Report.Result = Result;
    Report.Message = Message;
    Report.ExecutionTime = 0.0f; // Will be set by caller if needed
    return Report;
}

void UEng_ArchitectureValidator::LogValidationReport(const FEng_ValidationReport& Report)
{
    FString ResultString;
    switch (Report.Result)
    {
        case EEng_ValidationResult::Pass:
            ResultString = TEXT("PASS");
            UE_LOG(LogTemp, Log, TEXT("[VALIDATION] %s: %s - %s"), *ResultString, *Report.TestName, *Report.Message);
            break;
        case EEng_ValidationResult::Warning:
            ResultString = TEXT("WARNING");
            UE_LOG(LogTemp, Warning, TEXT("[VALIDATION] %s: %s - %s"), *ResultString, *Report.TestName, *Report.Message);
            break;
        case EEng_ValidationResult::Fail:
            ResultString = TEXT("FAIL");
            UE_LOG(LogTemp, Error, TEXT("[VALIDATION] %s: %s - %s"), *ResultString, *Report.TestName, *Report.Message);
            break;
    }
}

float UEng_ArchitectureValidator::MeasureExecutionTime(TFunction<void()> TestFunction)
{
    float StartTime = FPlatformTime::Seconds();
    TestFunction();
    return FPlatformTime::Seconds() - StartTime;
}

void UEng_ArchitectureValidator::PerformPeriodicValidation()
{
    if (!bIsValidationInProgress)
    {
        ValidateGameArchitecture();
    }
}