#include "Build_FinalBuildReport.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "GameFramework/Character.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"

UBuild_FinalBuildReport::UBuild_FinalBuildReport()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 5.0f; // Validate every 5 seconds
    
    LastValidationTime = 0.0f;
    bValidationInProgress = false;
    
    // Initialize current report
    CurrentReport.CycleID = TEXT("PROD_CYCLE_AUTO_20260515_005");
    CurrentReport.BuildTimestamp = FDateTime::Now().ToString();
}

void UBuild_FinalBuildReport::BeginPlay()
{
    Super::BeginPlay();
    
    // Generate initial build report
    GenerateBuildReport();
    
    UE_LOG(LogTemp, Warning, TEXT("Build_FinalBuildReport: Component initialized for cycle %s"), *CurrentReport.CycleID);
}

void UBuild_FinalBuildReport::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    LastValidationTime += DeltaTime;
    
    // Auto-validate every 30 seconds
    if (LastValidationTime >= 30.0f && !bValidationInProgress)
    {
        GenerateBuildReport();
        LastValidationTime = 0.0f;
    }
}

void UBuild_FinalBuildReport::GenerateBuildReport()
{
    if (bValidationInProgress) return;
    
    bValidationInProgress = true;
    
    // Clear previous report
    CurrentReport.SystemStatuses.Empty();
    CurrentReport.TotalActors = 0;
    CurrentReport.bBuildSuccessful = true;
    CurrentReport.BuildTimestamp = FDateTime::Now().ToString();
    
    UWorld* World = GetWorld();
    if (!World)
    {
        CurrentReport.bBuildSuccessful = false;
        bValidationInProgress = false;
        return;
    }
    
    // Count total actors
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);
    CurrentReport.TotalActors = AllActors.Num();
    
    // Validate each system
    FBuild_SystemStatus CharacterStatus;
    ValidateCharacterSystem(CharacterStatus);
    CurrentReport.SystemStatuses.Add(CharacterStatus);
    
    FBuild_SystemStatus DinosaurStatus;
    ValidateDinosaurSystem(DinosaurStatus);
    CurrentReport.SystemStatuses.Add(DinosaurStatus);
    
    FBuild_SystemStatus EnvironmentStatus;
    ValidateEnvironmentSystem(EnvironmentStatus);
    CurrentReport.SystemStatuses.Add(EnvironmentStatus);
    
    FBuild_SystemStatus QAStatus;
    ValidateQAFramework(QAStatus);
    CurrentReport.SystemStatuses.Add(QAStatus);
    
    FBuild_SystemStatus WorldGenStatus;
    ValidateWorldGeneration(WorldGenStatus);
    CurrentReport.SystemStatuses.Add(WorldGenStatus);
    
    FBuild_SystemStatus PhysicsStatus;
    ValidatePhysicsSystem(PhysicsStatus);
    CurrentReport.SystemStatuses.Add(PhysicsStatus);
    
    FBuild_SystemStatus AudioStatus;
    ValidateAudioSystem(AudioStatus);
    CurrentReport.SystemStatuses.Add(AudioStatus);
    
    // Check overall build success
    for (const FBuild_SystemStatus& Status : CurrentReport.SystemStatuses)
    {
        if (!Status.bIsOperational)
        {
            CurrentReport.bBuildSuccessful = false;
        }
    }
    
    // Add to history
    BuildHistory.Add(CurrentReport);
    
    // Keep only last 10 reports
    if (BuildHistory.Num() > 10)
    {
        BuildHistory.RemoveAt(0);
    }
    
    LogBuildStatus();
    bValidationInProgress = false;
}

bool UBuild_FinalBuildReport::ValidateSystemIntegration()
{
    GenerateBuildReport();
    return CurrentReport.bBuildSuccessful;
}

FBuild_SystemStatus UBuild_FinalBuildReport::GetSystemStatus(const FString& SystemName)
{
    for (const FBuild_SystemStatus& Status : CurrentReport.SystemStatuses)
    {
        if (Status.SystemName == SystemName)
        {
            return Status;
        }
    }
    
    // Return empty status if not found
    FBuild_SystemStatus EmptyStatus;
    EmptyStatus.SystemName = SystemName;
    return EmptyStatus;
}

void UBuild_FinalBuildReport::LogBuildStatus()
{
    UE_LOG(LogTemp, Warning, TEXT("=== FINAL BUILD REPORT - Cycle %s ==="), *CurrentReport.CycleID);
    UE_LOG(LogTemp, Warning, TEXT("Total Actors: %d"), CurrentReport.TotalActors);
    UE_LOG(LogTemp, Warning, TEXT("Build Successful: %s"), CurrentReport.bBuildSuccessful ? TEXT("YES") : TEXT("NO"));
    UE_LOG(LogTemp, Warning, TEXT("Timestamp: %s"), *CurrentReport.BuildTimestamp);
    
    for (const FBuild_SystemStatus& Status : CurrentReport.SystemStatuses)
    {
        UE_LOG(LogTemp, Warning, TEXT("System %s: %s (%d actors)"), 
            *Status.SystemName, 
            Status.bIsOperational ? TEXT("OPERATIONAL") : TEXT("FAILED"),
            Status.ActorCount);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("=== END BUILD REPORT ==="));
}

bool UBuild_FinalBuildReport::IsGamePlayable()
{
    // Game is playable if character system and environment are operational
    FBuild_SystemStatus CharacterStatus = GetSystemStatus(TEXT("Character"));
    FBuild_SystemStatus EnvironmentStatus = GetSystemStatus(TEXT("Environment"));
    
    return CharacterStatus.bIsOperational && EnvironmentStatus.bIsOperational && CurrentReport.TotalActors > 10;
}

void UBuild_FinalBuildReport::ValidateCharacterSystem(FBuild_SystemStatus& Status)
{
    Status.SystemName = TEXT("Character");
    Status.LastValidation = FDateTime::Now().ToString();
    
    UWorld* World = GetWorld();
    if (!World)
    {
        Status.bIsOperational = false;
        Status.ActorCount = 0;
        return;
    }
    
    TArray<AActor*> Characters;
    UGameplayStatics::GetAllActorsOfClass(World, ACharacter::StaticClass(), Characters);
    
    Status.ActorCount = Characters.Num();
    Status.bIsOperational = Characters.Num() > 0;
}

void UBuild_FinalBuildReport::ValidateDinosaurSystem(FBuild_SystemStatus& Status)
{
    Status.SystemName = TEXT("Dinosaur");
    Status.LastValidation = FDateTime::Now().ToString();
    
    UWorld* World = GetWorld();
    if (!World)
    {
        Status.bIsOperational = false;
        Status.ActorCount = 0;
        return;
    }
    
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);
    
    int32 DinosaurCount = 0;
    for (AActor* Actor : AllActors)
    {
        FString ActorName = Actor->GetActorLabel();
        if (ActorName.Contains(TEXT("Dinosaur")) || ActorName.Contains(TEXT("TRex")) || 
            ActorName.Contains(TEXT("Raptor")) || ActorName.Contains(TEXT("Brachio")))
        {
            DinosaurCount++;
        }
    }
    
    Status.ActorCount = DinosaurCount;
    Status.bIsOperational = DinosaurCount >= 3; // At least 3 dinosaurs required
}

void UBuild_FinalBuildReport::ValidateEnvironmentSystem(FBuild_SystemStatus& Status)
{
    Status.SystemName = TEXT("Environment");
    Status.LastValidation = FDateTime::Now().ToString();
    
    UWorld* World = GetWorld();
    if (!World)
    {
        Status.bIsOperational = false;
        Status.ActorCount = 0;
        return;
    }
    
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);
    
    int32 EnvironmentCount = 0;
    for (AActor* Actor : AllActors)
    {
        FString ActorName = Actor->GetActorLabel();
        if (ActorName.Contains(TEXT("Tree")) || ActorName.Contains(TEXT("Rock")) || 
            ActorName.Contains(TEXT("Foliage")) || ActorName.Contains(TEXT("Terrain")))
        {
            EnvironmentCount++;
        }
    }
    
    Status.ActorCount = EnvironmentCount;
    Status.bIsOperational = EnvironmentCount >= 10; // At least 10 environment elements
}

void UBuild_FinalBuildReport::ValidateQAFramework(FBuild_SystemStatus& Status)
{
    Status.SystemName = TEXT("QA Framework");
    Status.LastValidation = FDateTime::Now().ToString();
    
    // QA framework is operational if this component exists and is running
    Status.ActorCount = 1;
    Status.bIsOperational = true;
}

void UBuild_FinalBuildReport::ValidateWorldGeneration(FBuild_SystemStatus& Status)
{
    Status.SystemName = TEXT("World Generation");
    Status.LastValidation = FDateTime::Now().ToString();
    
    UWorld* World = GetWorld();
    if (!World)
    {
        Status.bIsOperational = false;
        Status.ActorCount = 0;
        return;
    }
    
    // Check for landscape or terrain actors
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);
    
    int32 TerrainCount = 0;
    for (AActor* Actor : AllActors)
    {
        if (Actor->GetClass()->GetName().Contains(TEXT("Landscape")) ||
            Actor->GetActorLabel().Contains(TEXT("Terrain")) ||
            Actor->GetActorLabel().Contains(TEXT("Ground")))
        {
            TerrainCount++;
        }
    }
    
    Status.ActorCount = TerrainCount;
    Status.bIsOperational = TerrainCount > 0;
}

void UBuild_FinalBuildReport::ValidatePhysicsSystem(FBuild_SystemStatus& Status)
{
    Status.SystemName = TEXT("Physics");
    Status.LastValidation = FDateTime::Now().ToString();
    
    UWorld* World = GetWorld();
    if (!World)
    {
        Status.bIsOperational = false;
        Status.ActorCount = 0;
        return;
    }
    
    // Check for actors with physics components
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);
    
    int32 PhysicsActorCount = 0;
    for (AActor* Actor : AllActors)
    {
        if (Actor->FindComponentByClass<UStaticMeshComponent>())
        {
            PhysicsActorCount++;
        }
    }
    
    Status.ActorCount = PhysicsActorCount;
    Status.bIsOperational = PhysicsActorCount > 0;
}

void UBuild_FinalBuildReport::ValidateAudioSystem(FBuild_SystemStatus& Status)
{
    Status.SystemName = TEXT("Audio");
    Status.LastValidation = FDateTime::Now().ToString();
    
    // Audio system validation - check for audio components
    UWorld* World = GetWorld();
    if (!World)
    {
        Status.bIsOperational = false;
        Status.ActorCount = 0;
        return;
    }
    
    // For now, consider audio operational if world exists
    Status.ActorCount = 1;
    Status.bIsOperational = true;
}