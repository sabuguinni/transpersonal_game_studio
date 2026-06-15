#include "BuildValidationSystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/GameModeBase.h"
#include "GameFramework/PlayerController.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/DateTime.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/SkeletalMeshActor.h"
#include "BuildValidationSystem.generated.h"

void UBuildValidationSystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    bValidationInProgress = false;
    LastValidationTime = 0.0f;
    ValidationReports.Empty();
    
    UE_LOG(LogTemp, Warning, TEXT("BuildValidationSystem initialized"));
    
    // Schedule initial validation after 5 seconds
    if (UWorld* World = GetWorld())
    {
        FTimerHandle TimerHandle;
        World->GetTimerManager().SetTimer(TimerHandle, this, &UBuildValidationSystem::RunFullValidationSuite, 5.0f, false);
    }
}

void UBuildValidationSystem::Deinitialize()
{
    ValidationReports.Empty();
    Super::Deinitialize();
}

void UBuildValidationSystem::RunFullValidationSuite()
{
    if (bValidationInProgress)
    {
        UE_LOG(LogTemp, Warning, TEXT("Validation already in progress, skipping"));
        return;
    }
    
    bValidationInProgress = true;
    ValidationReports.Empty();
    
    float StartTime = FPlatformTime::Seconds();
    
    UE_LOG(LogTemp, Warning, TEXT("Starting full validation suite"));
    
    // Core validation tests
    ValidateActorCounts();
    ValidateSystemIntegrity();
    ValidatePerformanceMetrics();
    ValidateWorldState();
    ValidateGameSystems();
    ValidateMemoryUsage();
    ValidateDinosaurPopulation();
    ValidatePlayerSystems();
    ValidateEnvironmentSystems();
    ValidateAudioSystems();
    ValidateRenderingSystems();
    ValidateNetworkingSystems();
    ValidateInputSystems();
    
    float TotalTime = FPlatformTime::Seconds() - StartTime;
    LastValidationTime = TotalTime;
    bValidationInProgress = false;
    
    UE_LOG(LogTemp, Warning, TEXT("Validation suite completed in %.2f seconds"), TotalTime);
    
    // Log summary
    int32 PassCount = 0, WarningCount = 0, FailCount = 0, CriticalCount = 0;
    for (const FBuild_ValidationReport& Report : ValidationReports)
    {
        switch (Report.Result)
        {
            case EBuild_ValidationResult::Pass: PassCount++; break;
            case EBuild_ValidationResult::Warning: WarningCount++; break;
            case EBuild_ValidationResult::Fail: FailCount++; break;
            case EBuild_ValidationResult::Critical: CriticalCount++; break;
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Validation Summary: %d Pass, %d Warning, %d Fail, %d Critical"), 
           PassCount, WarningCount, FailCount, CriticalCount);
}

void UBuildValidationSystem::ValidateActorCounts()
{
    float StartTime = FPlatformTime::Seconds();
    
    UWorld* World = GetWorld();
    if (!World)
    {
        AddValidationReport(TEXT("Actor Count"), EBuild_ValidationResult::Critical, 
                          TEXT("No valid world found"), FPlatformTime::Seconds() - StartTime);
        return;
    }
    
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);
    
    int32 ActorCount = AllActors.Num();
    EBuild_ValidationResult Result = EBuild_ValidationResult::Pass;
    FString Message = FString::Printf(TEXT("Total actors: %d"), ActorCount);
    
    if (ActorCount > 8000)
    {
        Result = EBuild_ValidationResult::Critical;
        Message += TEXT(" - EXCEEDS CAP LIMIT");
    }
    else if (ActorCount > 6000)
    {
        Result = EBuild_ValidationResult::Warning;
        Message += TEXT(" - Approaching cap limit");
    }
    
    AddValidationReport(TEXT("Actor Count"), Result, Message, FPlatformTime::Seconds() - StartTime);
}

void UBuildValidationSystem::ValidateSystemIntegrity()
{
    float StartTime = FPlatformTime::Seconds();
    
    UWorld* World = GetWorld();
    if (!World)
    {
        AddValidationReport(TEXT("System Integrity"), EBuild_ValidationResult::Critical, 
                          TEXT("No valid world"), FPlatformTime::Seconds() - StartTime);
        return;
    }
    
    // Check game mode
    AGameModeBase* GameMode = World->GetAuthGameMode();
    if (!GameMode)
    {
        AddValidationReport(TEXT("System Integrity"), EBuild_ValidationResult::Fail, 
                          TEXT("No GameMode found"), FPlatformTime::Seconds() - StartTime);
        return;
    }
    
    // Check player controller
    APlayerController* PC = World->GetFirstPlayerController();
    if (!PC)
    {
        AddValidationReport(TEXT("System Integrity"), EBuild_ValidationResult::Warning, 
                          TEXT("No PlayerController found"), FPlatformTime::Seconds() - StartTime);
        return;
    }
    
    AddValidationReport(TEXT("System Integrity"), EBuild_ValidationResult::Pass, 
                      TEXT("Core systems operational"), FPlatformTime::Seconds() - StartTime);
}

void UBuildValidationSystem::ValidatePerformanceMetrics()
{
    float StartTime = FPlatformTime::Seconds();
    
    // Get current FPS
    float DeltaTime = GetWorld()->GetDeltaSeconds();
    float CurrentFPS = (DeltaTime > 0.0f) ? (1.0f / DeltaTime) : 0.0f;
    
    EBuild_ValidationResult Result = EBuild_ValidationResult::Pass;
    FString Message = FString::Printf(TEXT("Current FPS: %.1f"), CurrentFPS);
    
    if (CurrentFPS < 15.0f)
    {
        Result = EBuild_ValidationResult::Critical;
        Message += TEXT(" - CRITICAL PERFORMANCE");
    }
    else if (CurrentFPS < 30.0f)
    {
        Result = EBuild_ValidationResult::Fail;
        Message += TEXT(" - Poor performance");
    }
    else if (CurrentFPS < 45.0f)
    {
        Result = EBuild_ValidationResult::Warning;
        Message += TEXT(" - Below target");
    }
    
    AddValidationReport(TEXT("Performance"), Result, Message, FPlatformTime::Seconds() - StartTime);
}

void UBuildValidationSystem::ValidateWorldState()
{
    float StartTime = FPlatformTime::Seconds();
    
    UWorld* World = GetWorld();
    if (!World)
    {
        AddValidationReport(TEXT("World State"), EBuild_ValidationResult::Critical, 
                          TEXT("Invalid world"), FPlatformTime::Seconds() - StartTime);
        return;
    }
    
    FString WorldName = World->GetName();
    bool bIsGameWorld = World->IsGameWorld();
    
    AddValidationReport(TEXT("World State"), EBuild_ValidationResult::Pass, 
                      FString::Printf(TEXT("World: %s, GameWorld: %s"), 
                      *WorldName, bIsGameWorld ? TEXT("Yes") : TEXT("No")), 
                      FPlatformTime::Seconds() - StartTime);
}

void UBuildValidationSystem::ValidateGameSystems()
{
    float StartTime = FPlatformTime::Seconds();
    
    // Check for essential game systems
    int32 SystemCount = 0;
    
    // This would check for custom subsystems when they exist
    // For now, just validate basic engine systems
    if (GEngine)
    {
        SystemCount++;
    }
    
    if (GetWorld())
    {
        SystemCount++;
    }
    
    EBuild_ValidationResult Result = (SystemCount >= 2) ? EBuild_ValidationResult::Pass : EBuild_ValidationResult::Warning;
    FString Message = FString::Printf(TEXT("Core systems active: %d"), SystemCount);
    
    AddValidationReport(TEXT("Game Systems"), Result, Message, FPlatformTime::Seconds() - StartTime);
}

void UBuildValidationSystem::ValidateMemoryUsage()
{
    float StartTime = FPlatformTime::Seconds();
    
    FPlatformMemoryStats MemStats = FPlatformMemory::GetStats();
    float UsedMemoryMB = MemStats.UsedPhysical / (1024.0f * 1024.0f);
    float AvailableMemoryMB = MemStats.AvailablePhysical / (1024.0f * 1024.0f);
    
    EBuild_ValidationResult Result = EBuild_ValidationResult::Pass;
    FString Message = FString::Printf(TEXT("Memory: %.1f MB used, %.1f MB available"), 
                                    UsedMemoryMB, AvailableMemoryMB);
    
    if (UsedMemoryMB > 6000.0f)
    {
        Result = EBuild_ValidationResult::Critical;
        Message += TEXT(" - HIGH MEMORY USAGE");
    }
    else if (UsedMemoryMB > 4000.0f)
    {
        Result = EBuild_ValidationResult::Warning;
        Message += TEXT(" - Elevated memory usage");
    }
    
    AddValidationReport(TEXT("Memory Usage"), Result, Message, FPlatformTime::Seconds() - StartTime);
}

void UBuildValidationSystem::ValidateDinosaurPopulation()
{
    float StartTime = FPlatformTime::Seconds();
    
    UWorld* World = GetWorld();
    if (!World)
    {
        AddValidationReport(TEXT("Dinosaur Population"), EBuild_ValidationResult::Critical, 
                          TEXT("No world"), FPlatformTime::Seconds() - StartTime);
        return;
    }
    
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);
    
    int32 DinosaurCount = 0;
    TArray<FString> DinoLabels = {TEXT("trex"), TEXT("veloci"), TEXT("tricera"), TEXT("brachi"), 
                                 TEXT("ankylo"), TEXT("parasauro"), TEXT("pachy"), TEXT("proto"), TEXT("tsinta")};
    
    for (AActor* Actor : AllActors)
    {
        if (Actor)
        {
            FString ActorLabel = Actor->GetActorLabel().ToLower();
            for (const FString& DinoLabel : DinoLabels)
            {
                if (ActorLabel.Contains(DinoLabel))
                {
                    DinosaurCount++;
                    break;
                }
            }
        }
    }
    
    EBuild_ValidationResult Result = EBuild_ValidationResult::Pass;
    FString Message = FString::Printf(TEXT("Dinosaurs: %d"), DinosaurCount);
    
    if (DinosaurCount > 150)
    {
        Result = EBuild_ValidationResult::Critical;
        Message += TEXT(" - EXCEEDS LIMIT");
    }
    else if (DinosaurCount > 120)
    {
        Result = EBuild_ValidationResult::Warning;
        Message += TEXT(" - Approaching limit");
    }
    
    AddValidationReport(TEXT("Dinosaur Population"), Result, Message, FPlatformTime::Seconds() - StartTime);
}

void UBuildValidationSystem::ValidatePlayerSystems()
{
    float StartTime = FPlatformTime::Seconds();
    
    UWorld* World = GetWorld();
    if (!World)
    {
        AddValidationReport(TEXT("Player Systems"), EBuild_ValidationResult::Critical, 
                          TEXT("No world"), FPlatformTime::Seconds() - StartTime);
        return;
    }
    
    APlayerController* PC = World->GetFirstPlayerController();
    if (!PC)
    {
        AddValidationReport(TEXT("Player Systems"), EBuild_ValidationResult::Fail, 
                          TEXT("No PlayerController"), FPlatformTime::Seconds() - StartTime);
        return;
    }
    
    APawn* PlayerPawn = PC->GetPawn();
    FString Message = PlayerPawn ? TEXT("Player pawn active") : TEXT("No player pawn");
    EBuild_ValidationResult Result = PlayerPawn ? EBuild_ValidationResult::Pass : EBuild_ValidationResult::Warning;
    
    AddValidationReport(TEXT("Player Systems"), Result, Message, FPlatformTime::Seconds() - StartTime);
}

void UBuildValidationSystem::ValidateEnvironmentSystems()
{
    float StartTime = FPlatformTime::Seconds();
    
    UWorld* World = GetWorld();
    if (!World)
    {
        AddValidationReport(TEXT("Environment Systems"), EBuild_ValidationResult::Critical, 
                          TEXT("No world"), FPlatformTime::Seconds() - StartTime);
        return;
    }
    
    // Count environment actors
    TArray<AActor*> StaticMeshActors;
    UGameplayStatics::GetAllActorsOfClass(World, AStaticMeshActor::StaticClass(), StaticMeshActors);
    
    int32 EnvironmentCount = StaticMeshActors.Num();
    FString Message = FString::Printf(TEXT("Environment actors: %d"), EnvironmentCount);
    
    EBuild_ValidationResult Result = (EnvironmentCount > 0) ? EBuild_ValidationResult::Pass : EBuild_ValidationResult::Warning;
    
    AddValidationReport(TEXT("Environment Systems"), Result, Message, FPlatformTime::Seconds() - StartTime);
}

void UBuildValidationSystem::ValidateAudioSystems()
{
    float StartTime = FPlatformTime::Seconds();
    
    // Basic audio system validation
    bool bAudioSystemActive = GEngine && GEngine->GetAudioDeviceManager();
    
    EBuild_ValidationResult Result = bAudioSystemActive ? EBuild_ValidationResult::Pass : EBuild_ValidationResult::Warning;
    FString Message = bAudioSystemActive ? TEXT("Audio system active") : TEXT("Audio system not found");
    
    AddValidationReport(TEXT("Audio Systems"), Result, Message, FPlatformTime::Seconds() - StartTime);
}

void UBuildValidationSystem::ValidateRenderingSystems()
{
    float StartTime = FPlatformTime::Seconds();
    
    // Basic rendering validation
    bool bRenderingActive = GEngine && GEngine->GetWorldContexts().Num() > 0;
    
    EBuild_ValidationResult Result = bRenderingActive ? EBuild_ValidationResult::Pass : EBuild_ValidationResult::Warning;
    FString Message = bRenderingActive ? TEXT("Rendering active") : TEXT("No rendering context");
    
    AddValidationReport(TEXT("Rendering Systems"), Result, Message, FPlatformTime::Seconds() - StartTime);
}

void UBuildValidationSystem::ValidateNetworkingSystems()
{
    float StartTime = FPlatformTime::Seconds();
    
    UWorld* World = GetWorld();
    bool bNetworkingReady = World && World->GetNetDriver() != nullptr;
    
    // For single player, this is expected to be null
    EBuild_ValidationResult Result = EBuild_ValidationResult::Pass;
    FString Message = bNetworkingReady ? TEXT("Network driver active") : TEXT("Single player mode");
    
    AddValidationReport(TEXT("Networking Systems"), Result, Message, FPlatformTime::Seconds() - StartTime);
}

void UBuildValidationSystem::ValidateInputSystems()
{
    float StartTime = FPlatformTime::Seconds();
    
    UWorld* World = GetWorld();
    if (!World)
    {
        AddValidationReport(TEXT("Input Systems"), EBuild_ValidationResult::Critical, 
                          TEXT("No world"), FPlatformTime::Seconds() - StartTime);
        return;
    }
    
    APlayerController* PC = World->GetFirstPlayerController();
    bool bInputActive = PC && PC->IsInputKeyDown(EKeys::AnyKey) == false; // Just check system exists
    
    EBuild_ValidationResult Result = PC ? EBuild_ValidationResult::Pass : EBuild_ValidationResult::Warning;
    FString Message = PC ? TEXT("Input system ready") : TEXT("No input controller");
    
    AddValidationReport(TEXT("Input Systems"), Result, Message, FPlatformTime::Seconds() - StartTime);
}

TArray<FBuild_ValidationReport> UBuildValidationSystem::GetValidationReports() const
{
    return ValidationReports;
}

bool UBuildValidationSystem::HasCriticalErrors() const
{
    for (const FBuild_ValidationReport& Report : ValidationReports)
    {
        if (Report.Result == EBuild_ValidationResult::Critical)
        {
            return true;
        }
    }
    return false;
}

void UBuildValidationSystem::AddValidationReport(const FString& TestName, EBuild_ValidationResult Result, 
                                               const FString& Message, float ExecutionTime)
{
    FBuild_ValidationReport Report;
    Report.TestName = TestName;
    Report.Result = Result;
    Report.Message = Message;
    Report.ExecutionTime = ExecutionTime;
    
    ValidationReports.Add(Report);
    
    // Log the result
    FString ResultString;
    switch (Result)
    {
        case EBuild_ValidationResult::Pass: ResultString = TEXT("PASS"); break;
        case EBuild_ValidationResult::Warning: ResultString = TEXT("WARN"); break;
        case EBuild_ValidationResult::Fail: ResultString = TEXT("FAIL"); break;
        case EBuild_ValidationResult::Critical: ResultString = TEXT("CRIT"); break;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("[VALIDATION] %s: %s - %s (%.3fs)"), 
           *ResultString, *TestName, *Message, ExecutionTime);
}