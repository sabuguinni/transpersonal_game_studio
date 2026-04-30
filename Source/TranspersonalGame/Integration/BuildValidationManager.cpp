#include "BuildValidationManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Character.h"
#include "GameFramework/GameModeBase.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

UBuildValidationManager::UBuildValidationManager()
{
    bValidationInProgress = false;
    ValidationStartTime = 0.0f;
}

void UBuildValidationManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Log, TEXT("BuildValidationManager initialized"));
    
    // Clear any previous results
    ClearValidationResults();
    
    // Run initial quick validation
    RunQuickValidation();
}

void UBuildValidationManager::Deinitialize()
{
    ClearValidationResults();
    Super::Deinitialize();
}

void UBuildValidationManager::RunFullValidation()
{
    if (bValidationInProgress)
    {
        UE_LOG(LogTemp, Warning, TEXT("Validation already in progress"));
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("Starting full build validation"));
    
    bValidationInProgress = true;
    ValidationStartTime = FPlatformTime::Seconds();
    ClearValidationResults();

    // Run all validation tests
    ValidateCurrentLevel();
    ValidateCoreClasses();
    ValidateActorCounts();
    ValidateCharacterSystem();
    ValidateDinosaurSystem();
    ValidateEnvironmentSystem();
    ValidateGameModeSystem();
    ValidatePhysicsSystem();

    UpdateSystemHealth();
    bValidationInProgress = false;

    float TotalTime = FPlatformTime::Seconds() - ValidationStartTime;
    UE_LOG(LogTemp, Log, TEXT("Full validation completed in %.2f seconds"), TotalTime);
}

void UBuildValidationManager::RunQuickValidation()
{
    if (bValidationInProgress)
    {
        return;
    }

    bValidationInProgress = true;
    ValidationStartTime = FPlatformTime::Seconds();
    ClearValidationResults();

    // Run essential tests only
    ValidateCurrentLevel();
    ValidateCoreClasses();
    ValidateActorCounts();

    UpdateSystemHealth();
    bValidationInProgress = false;
}

FBuild_SystemHealth UBuildValidationManager::GetSystemHealth() const
{
    return CurrentSystemHealth;
}

TArray<FBuild_ValidationResult> UBuildValidationManager::GetValidationResults() const
{
    return ValidationResults;
}

bool UBuildValidationManager::IsSystemHealthy() const
{
    // System is healthy if no failed tests and core requirements met
    for (const FBuild_ValidationResult& Result : ValidationResults)
    {
        if (Result.Status == EBuild_ValidationStatus::Failed)
        {
            return false;
        }
    }

    return CurrentSystemHealth.bMinPlayableMapLoaded && 
           CurrentSystemHealth.bCoreClassesLoaded &&
           CurrentSystemHealth.TotalActors > 0;
}

void UBuildValidationManager::ValidateCurrentLevel()
{
    float StartTime = FPlatformTime::Seconds();
    
    UWorld* World = GetWorld();
    if (!World)
    {
        AddValidationResult(TEXT("Level Validation"), EBuild_ValidationStatus::Failed, 
                          TEXT("No world loaded"), FPlatformTime::Seconds() - StartTime);
        return;
    }

    FString LevelName = World->GetName();
    bool bIsMinPlayableMap = LevelName.Contains(TEXT("MinPlayableMap"));
    
    if (bIsMinPlayableMap)
    {
        AddValidationResult(TEXT("Level Validation"), EBuild_ValidationStatus::Passed, 
                          FString::Printf(TEXT("MinPlayableMap loaded: %s"), *LevelName), 
                          FPlatformTime::Seconds() - StartTime);
    }
    else
    {
        AddValidationResult(TEXT("Level Validation"), EBuild_ValidationStatus::Warning, 
                          FString::Printf(TEXT("Non-MinPlayableMap loaded: %s"), *LevelName), 
                          FPlatformTime::Seconds() - StartTime);
    }
}

void UBuildValidationManager::ValidateCoreClasses()
{
    float StartTime = FPlatformTime::Seconds();
    
    // Test core class loading
    UClass* CharacterClass = LoadClass<AActor>(nullptr, TEXT("/Script/TranspersonalGame.TranspersonalCharacter"));
    UClass* GameModeClass = LoadClass<AGameModeBase>(nullptr, TEXT("/Script/TranspersonalGame.TranspersonalGameMode"));
    
    bool bCoreClassesValid = (CharacterClass != nullptr) && (GameModeClass != nullptr);
    
    if (bCoreClassesValid)
    {
        AddValidationResult(TEXT("Core Classes"), EBuild_ValidationStatus::Passed, 
                          TEXT("TranspersonalCharacter and TranspersonalGameMode loaded successfully"), 
                          FPlatformTime::Seconds() - StartTime);
    }
    else
    {
        FString MissingClasses;
        if (!CharacterClass) MissingClasses += TEXT("TranspersonalCharacter ");
        if (!GameModeClass) MissingClasses += TEXT("TranspersonalGameMode ");
        
        AddValidationResult(TEXT("Core Classes"), EBuild_ValidationStatus::Failed, 
                          FString::Printf(TEXT("Missing classes: %s"), *MissingClasses), 
                          FPlatformTime::Seconds() - StartTime);
    }
}

void UBuildValidationManager::ValidateActorCounts()
{
    float StartTime = FPlatformTime::Seconds();
    
    UWorld* World = GetWorld();
    if (!World)
    {
        AddValidationResult(TEXT("Actor Count"), EBuild_ValidationStatus::Failed, 
                          TEXT("No world for actor validation"), FPlatformTime::Seconds() - StartTime);
        return;
    }

    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);
    
    int32 CharacterCount = 0;
    int32 DinosaurCount = 0;
    int32 EnvironmentCount = 0;
    
    for (AActor* Actor : AllActors)
    {
        if (!Actor) continue;
        
        FString ActorName = Actor->GetName();
        if (ActorName.Contains(TEXT("Character")) || ActorName.Contains(TEXT("Player")))
        {
            CharacterCount++;
        }
        else if (ActorName.Contains(TEXT("Dinosaur")) || ActorName.Contains(TEXT("TRex")) || 
                 ActorName.Contains(TEXT("Raptor")) || ActorName.Contains(TEXT("Brachio")))
        {
            DinosaurCount++;
        }
        else if (ActorName.Contains(TEXT("Tree")) || ActorName.Contains(TEXT("Rock")) || 
                 ActorName.Contains(TEXT("Terrain")) || ActorName.Contains(TEXT("Landscape")))
        {
            EnvironmentCount++;
        }
    }
    
    // Update system health
    CurrentSystemHealth.TotalActors = AllActors.Num();
    CurrentSystemHealth.CharacterActors = CharacterCount;
    CurrentSystemHealth.DinosaurActors = DinosaurCount;
    CurrentSystemHealth.EnvironmentActors = EnvironmentCount;
    
    if (AllActors.Num() > 10) // Minimum expected actors
    {
        AddValidationResult(TEXT("Actor Count"), EBuild_ValidationStatus::Passed, 
                          FString::Printf(TEXT("Total: %d, Characters: %d, Dinosaurs: %d, Environment: %d"), 
                                        AllActors.Num(), CharacterCount, DinosaurCount, EnvironmentCount), 
                          FPlatformTime::Seconds() - StartTime);
    }
    else
    {
        AddValidationResult(TEXT("Actor Count"), EBuild_ValidationStatus::Warning, 
                          FString::Printf(TEXT("Low actor count: %d (expected >10)"), AllActors.Num()), 
                          FPlatformTime::Seconds() - StartTime);
    }
}

void UBuildValidationManager::ValidateCharacterSystem()
{
    float StartTime = FPlatformTime::Seconds();
    
    UWorld* World = GetWorld();
    if (!World)
    {
        AddValidationResult(TEXT("Character System"), EBuild_ValidationStatus::Failed, 
                          TEXT("No world for character validation"), FPlatformTime::Seconds() - StartTime);
        return;
    }

    TArray<AActor*> Characters;
    UGameplayStatics::GetAllActorsOfClass(World, ACharacter::StaticClass(), Characters);
    
    if (Characters.Num() > 0)
    {
        AddValidationResult(TEXT("Character System"), EBuild_ValidationStatus::Passed, 
                          FString::Printf(TEXT("Found %d character actors"), Characters.Num()), 
                          FPlatformTime::Seconds() - StartTime);
    }
    else
    {
        AddValidationResult(TEXT("Character System"), EBuild_ValidationStatus::Warning, 
                          TEXT("No character actors found in level"), 
                          FPlatformTime::Seconds() - StartTime);
    }
}

void UBuildValidationManager::ValidateDinosaurSystem()
{
    float StartTime = FPlatformTime::Seconds();
    
    // For now, just check for actors with dinosaur names
    // Later this will check actual dinosaur AI classes
    if (CurrentSystemHealth.DinosaurActors > 0)
    {
        AddValidationResult(TEXT("Dinosaur System"), EBuild_ValidationStatus::Passed, 
                          FString::Printf(TEXT("Found %d dinosaur placeholder actors"), CurrentSystemHealth.DinosaurActors), 
                          FPlatformTime::Seconds() - StartTime);
    }
    else
    {
        AddValidationResult(TEXT("Dinosaur System"), EBuild_ValidationStatus::Warning, 
                          TEXT("No dinosaur actors found"), 
                          FPlatformTime::Seconds() - StartTime);
    }
}

void UBuildValidationManager::ValidateEnvironmentSystem()
{
    float StartTime = FPlatformTime::Seconds();
    
    if (CurrentSystemHealth.EnvironmentActors > 5)
    {
        AddValidationResult(TEXT("Environment System"), EBuild_ValidationStatus::Passed, 
                          FString::Printf(TEXT("Found %d environment actors"), CurrentSystemHealth.EnvironmentActors), 
                          FPlatformTime::Seconds() - StartTime);
    }
    else
    {
        AddValidationResult(TEXT("Environment System"), EBuild_ValidationStatus::Warning, 
                          FString::Printf(TEXT("Low environment actor count: %d"), CurrentSystemHealth.EnvironmentActors), 
                          FPlatformTime::Seconds() - StartTime);
    }
}

void UBuildValidationManager::ValidateGameModeSystem()
{
    float StartTime = FPlatformTime::Seconds();
    
    UWorld* World = GetWorld();
    if (!World)
    {
        AddValidationResult(TEXT("GameMode System"), EBuild_ValidationStatus::Failed, 
                          TEXT("No world for GameMode validation"), FPlatformTime::Seconds() - StartTime);
        return;
    }

    AGameModeBase* GameMode = World->GetAuthGameMode();
    if (GameMode)
    {
        FString GameModeName = GameMode->GetClass()->GetName();
        AddValidationResult(TEXT("GameMode System"), EBuild_ValidationStatus::Passed, 
                          FString::Printf(TEXT("GameMode active: %s"), *GameModeName), 
                          FPlatformTime::Seconds() - StartTime);
    }
    else
    {
        AddValidationResult(TEXT("GameMode System"), EBuild_ValidationStatus::Warning, 
                          TEXT("No GameMode found"), 
                          FPlatformTime::Seconds() - StartTime);
    }
}

void UBuildValidationManager::ValidatePhysicsSystem()
{
    float StartTime = FPlatformTime::Seconds();
    
    UWorld* World = GetWorld();
    if (!World)
    {
        AddValidationResult(TEXT("Physics System"), EBuild_ValidationStatus::Failed, 
                          TEXT("No world for physics validation"), FPlatformTime::Seconds() - StartTime);
        return;
    }

    // Basic physics validation - check if physics world exists
    if (World->GetPhysicsScene())
    {
        AddValidationResult(TEXT("Physics System"), EBuild_ValidationStatus::Passed, 
                          TEXT("Physics scene active"), 
                          FPlatformTime::Seconds() - StartTime);
    }
    else
    {
        AddValidationResult(TEXT("Physics System"), EBuild_ValidationStatus::Failed, 
                          TEXT("No physics scene found"), 
                          FPlatformTime::Seconds() - StartTime);
    }
}

void UBuildValidationManager::GenerateHealthReport()
{
    RunFullValidation();
    
    UE_LOG(LogTemp, Log, TEXT("=== BUILD HEALTH REPORT ==="));
    UE_LOG(LogTemp, Log, TEXT("System Healthy: %s"), IsSystemHealthy() ? TEXT("YES") : TEXT("NO"));
    UE_LOG(LogTemp, Log, TEXT("Total Actors: %d"), CurrentSystemHealth.TotalActors);
    UE_LOG(LogTemp, Log, TEXT("Character Actors: %d"), CurrentSystemHealth.CharacterActors);
    UE_LOG(LogTemp, Log, TEXT("Dinosaur Actors: %d"), CurrentSystemHealth.DinosaurActors);
    UE_LOG(LogTemp, Log, TEXT("Environment Actors: %d"), CurrentSystemHealth.EnvironmentActors);
    UE_LOG(LogTemp, Log, TEXT("MinPlayableMap Loaded: %s"), CurrentSystemHealth.bMinPlayableMapLoaded ? TEXT("YES") : TEXT("NO"));
    UE_LOG(LogTemp, Log, TEXT("Core Classes Loaded: %s"), CurrentSystemHealth.bCoreClassesLoaded ? TEXT("YES") : TEXT("NO"));
    
    UE_LOG(LogTemp, Log, TEXT("=== VALIDATION RESULTS ==="));
    for (const FBuild_ValidationResult& Result : ValidationResults)
    {
        FString StatusStr;
        switch (Result.Status)
        {
            case EBuild_ValidationStatus::Passed: StatusStr = TEXT("PASS"); break;
            case EBuild_ValidationStatus::Failed: StatusStr = TEXT("FAIL"); break;
            case EBuild_ValidationStatus::Warning: StatusStr = TEXT("WARN"); break;
            default: StatusStr = TEXT("UNKNOWN"); break;
        }
        
        UE_LOG(LogTemp, Log, TEXT("[%s] %s: %s (%.2fs)"), 
               *StatusStr, *Result.TestName, *Result.Message, Result.ExecutionTime);
    }
}

void UBuildValidationManager::AddValidationResult(const FString& TestName, EBuild_ValidationStatus Status, const FString& Message, float ExecutionTime)
{
    FBuild_ValidationResult Result;
    Result.TestName = TestName;
    Result.Status = Status;
    Result.Message = Message;
    Result.ExecutionTime = ExecutionTime;
    
    ValidationResults.Add(Result);
    
    // Log the result
    FString StatusStr;
    switch (Status)
    {
        case EBuild_ValidationStatus::Passed: StatusStr = TEXT("PASS"); break;
        case EBuild_ValidationStatus::Failed: StatusStr = TEXT("FAIL"); break;
        case EBuild_ValidationStatus::Warning: StatusStr = TEXT("WARN"); break;
        default: StatusStr = TEXT("UNKNOWN"); break;
    }
    
    UE_LOG(LogTemp, Log, TEXT("Validation [%s] %s: %s"), *StatusStr, *TestName, *Message);
}

void UBuildValidationManager::ClearValidationResults()
{
    ValidationResults.Empty();
    CurrentSystemHealth = FBuild_SystemHealth();
}

void UBuildValidationManager::UpdateSystemHealth()
{
    // Update boolean flags based on validation results
    CurrentSystemHealth.bMinPlayableMapLoaded = false;
    CurrentSystemHealth.bCoreClassesLoaded = false;
    
    for (const FBuild_ValidationResult& Result : ValidationResults)
    {
        if (Result.TestName == TEXT("Level Validation") && Result.Status == EBuild_ValidationStatus::Passed)
        {
            CurrentSystemHealth.bMinPlayableMapLoaded = true;
        }
        else if (Result.TestName == TEXT("Core Classes") && Result.Status == EBuild_ValidationStatus::Passed)
        {
            CurrentSystemHealth.bCoreClassesLoaded = true;
        }
    }
}