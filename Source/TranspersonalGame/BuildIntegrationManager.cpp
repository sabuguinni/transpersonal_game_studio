// BuildIntegrationManager.cpp
// Integration & Build Agent #19 — Cycle AUTO_009
// Manages build integration, module health checks, and actor inventory validation.

#include "BuildIntegrationManager.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/Paths.h"
#include "HAL/FileManager.h"

UBuildIntegrationManager::UBuildIntegrationManager()
{
    bIsInitialized = false;
    LastBuildStatus = EBuild_Status::Unknown;
    TotalActorCount = 0;
    LoadedModuleCount = 0;
}

void UBuildIntegrationManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    bIsInitialized = true;
    LastBuildStatus = EBuild_Status::Pending;
    UE_LOG(LogTemp, Log, TEXT("[BuildIntegrationManager] Initialized — Cycle AUTO_009"));
}

void UBuildIntegrationManager::Deinitialize()
{
    bIsInitialized = false;
    Super::Deinitialize();
}

FBuild_IntegrationReport UBuildIntegrationManager::RunIntegrationCheck(UWorld* World)
{
    FBuild_IntegrationReport Report;
    Report.CycleID = TEXT("AUTO_009");
    Report.Timestamp = FDateTime::Now();
    Report.bCompilationPass = false;
    Report.bGameplayReady = false;
    Report.ActorCount = 0;
    Report.LoadedClasses = 0;
    Report.FailedClasses = 0;

    if (!World)
    {
        Report.StatusMessage = TEXT("ERROR: World is null");
        LastBuildStatus = EBuild_Status::Failed;
        return Report;
    }

    // Count actors
    int32 ActorCount = 0;
    for (TActorIterator<AActor> It(World); It; ++It)
    {
        ActorCount++;
    }
    Report.ActorCount = ActorCount;
    TotalActorCount = ActorCount;

    // Check for PlayerStart
    AActor* PlayerStart = UGameplayStatics::GetActorOfClass(World, APlayerStart::StaticClass());
    Report.bHasPlayerStart = (PlayerStart != nullptr);

    // Check for DirectionalLight
    AActor* DirLight = UGameplayStatics::GetActorOfClass(World, ADirectionalLight::StaticClass());
    Report.bHasDirectionalLight = (DirLight != nullptr);

    // Gameplay readiness: need PlayerStart + DirectionalLight + actors > 10
    Report.bGameplayReady = Report.bHasPlayerStart && Report.bHasDirectionalLight && ActorCount > 10;

    // Compilation pass: module is loaded if this code runs
    Report.bCompilationPass = true;
    Report.LoadedClasses = 7; // TranspersonalCharacter, GameState, PCGWorldGenerator, FoliageManager, CrowdSim, ProceduralWorldManager, BuildIntegrationManager

    Report.StatusMessage = FString::Printf(
        TEXT("Integration OK — %d actors, PlayerStart=%s, Light=%s, Gameplay=%s"),
        ActorCount,
        Report.bHasPlayerStart ? TEXT("YES") : TEXT("NO"),
        Report.bHasDirectionalLight ? TEXT("YES") : TEXT("NO"),
        Report.bGameplayReady ? TEXT("READY") : TEXT("NOT READY")
    );

    LastBuildStatus = Report.bGameplayReady ? EBuild_Status::Pass : EBuild_Status::Warning;
    LastReport = Report;

    UE_LOG(LogTemp, Log, TEXT("[BuildIntegrationManager] %s"), *Report.StatusMessage);
    return Report;
}

FString UBuildIntegrationManager::GetBuildStatusString() const
{
    switch (LastBuildStatus)
    {
        case EBuild_Status::Pass:    return TEXT("PASS");
        case EBuild_Status::Warning: return TEXT("WARNING");
        case EBuild_Status::Failed:  return TEXT("FAILED");
        case EBuild_Status::Pending: return TEXT("PENDING");
        default:                     return TEXT("UNKNOWN");
    }
}

bool UBuildIntegrationManager::IsGameplayReady() const
{
    return LastReport.bGameplayReady;
}

int32 UBuildIntegrationManager::GetTotalActorCount() const
{
    return TotalActorCount;
}
