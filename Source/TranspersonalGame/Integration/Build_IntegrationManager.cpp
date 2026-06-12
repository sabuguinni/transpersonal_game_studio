#include "Build_IntegrationManager.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "TranspersonalGameState.h"
#include "TranspersonalCharacter.h"
#include "PCGWorldGenerator.h"
#include "FoliageManager.h"
#include "QA/QA_TestFramework.h"

UBuild_IntegrationManager::UBuild_IntegrationManager()
{
    LastValidationTime = 0.0f;
    bIntegrationValid = false;
}

void UBuild_IntegrationManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Log, TEXT("Build_IntegrationManager: Initialized"));
    
    // Initialize system status tracking
    CachedSystemStatuses.Empty();
    
    // Add core systems to track
    UpdateSystemStatus(TEXT("WorldGeneration"), EBuild_ValidationStatus::Unknown);
    UpdateSystemStatus(TEXT("CharacterSystem"), EBuild_ValidationStatus::Unknown);
    UpdateSystemStatus(TEXT("AISystem"), EBuild_ValidationStatus::Unknown);
    UpdateSystemStatus(TEXT("QAFramework"), EBuild_ValidationStatus::Unknown);
    UpdateSystemStatus(TEXT("FoliageSystem"), EBuild_ValidationStatus::Unknown);
}

void UBuild_IntegrationManager::Deinitialize()
{
    CachedSystemStatuses.Empty();
    Super::Deinitialize();
}

FBuild_IntegrationReport UBuild_IntegrationManager::GenerateIntegrationReport()
{
    FBuild_IntegrationReport Report;
    
    // Get current world
    UWorld* World = GetWorld();
    if (!World)
    {
        return Report;
    }
    
    // Count total actors
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);
    Report.TotalActorCount = AllActors.Num();
    
    // Validate systems and update statuses
    ValidateSystemIntegrity();
    Report.SystemStatuses = CachedSystemStatuses;
    
    // Count active systems
    Report.ActiveSystemCount = 0;
    for (const FBuild_SystemStatus& Status : CachedSystemStatuses)
    {
        if (Status.Status == EBuild_ValidationStatus::Valid)
        {
            Report.ActiveSystemCount++;
        }
    }
    
    // Calculate build time (mock for now)
    Report.BuildTime = FPlatformTime::Seconds() - LastValidationTime;
    
    // Determine if game is playable
    Report.bIsPlayable = IsGamePlayable();
    
    return Report;
}

bool UBuild_IntegrationManager::ValidateSystemIntegrity()
{
    LastValidationTime = FPlatformTime::Seconds();
    
    ValidateWorldGeneration();
    ValidateCharacterSystems();
    ValidateAISystems();
    ValidateQASystems();
    
    // Check if all critical systems are valid
    int32 ValidSystems = 0;
    int32 TotalSystems = CachedSystemStatuses.Num();
    
    for (const FBuild_SystemStatus& Status : CachedSystemStatuses)
    {
        if (Status.Status == EBuild_ValidationStatus::Valid)
        {
            ValidSystems++;
        }
    }
    
    bIntegrationValid = (ValidSystems >= TotalSystems * 0.8f); // 80% systems must be valid
    
    return bIntegrationValid;
}

void UBuild_IntegrationManager::EnforceActorCap(int32 MaxActors)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);
    
    if (AllActors.Num() <= MaxActors)
    {
        return;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Actor cap exceeded: %d/%d - Enforcing cleanup"), AllActors.Num(), MaxActors);
    
    // Keep essential actors
    TArray<AActor*> EssentialActors;
    TArray<AActor*> NonEssentialActors;
    
    for (AActor* Actor : AllActors)
    {
        if (!Actor)
        {
            continue;
        }
        
        FString ActorName = Actor->GetName().ToLower();
        
        // Keep player starts, lights, and dinosaurs
        if (ActorName.Contains(TEXT("playerstart")) ||
            ActorName.Contains(TEXT("light")) ||
            ActorName.Contains(TEXT("trex")) ||
            ActorName.Contains(TEXT("veloci")) ||
            ActorName.Contains(TEXT("brachi")) ||
            ActorName.Contains(TEXT("sky")))
        {
            EssentialActors.Add(Actor);
        }
        else
        {
            NonEssentialActors.Add(Actor);
        }
    }
    
    // Remove excess non-essential actors
    int32 ActorsToRemove = AllActors.Num() - MaxActors;
    for (int32 i = 0; i < FMath::Min(ActorsToRemove, NonEssentialActors.Num()); i++)
    {
        if (NonEssentialActors[i])
        {
            NonEssentialActors[i]->Destroy();
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Actor cap enforced: Removed %d actors"), ActorsToRemove);
}

TArray<FBuild_SystemStatus> UBuild_IntegrationManager::GetSystemStatuses()
{
    return CachedSystemStatuses;
}

bool UBuild_IntegrationManager::IsGamePlayable()
{
    // Game is playable if:
    // 1. Character system is valid
    // 2. World generation is valid
    // 3. At least 80% of systems are functional
    
    bool bCharacterValid = false;
    bool bWorldValid = false;
    int32 ValidSystems = 0;
    
    for (const FBuild_SystemStatus& Status : CachedSystemStatuses)
    {
        if (Status.Status == EBuild_ValidationStatus::Valid)
        {
            ValidSystems++;
            
            if (Status.SystemName == TEXT("CharacterSystem"))
            {
                bCharacterValid = true;
            }
            else if (Status.SystemName == TEXT("WorldGeneration"))
            {
                bWorldValid = true;
            }
        }
    }
    
    float ValidPercentage = CachedSystemStatuses.Num() > 0 ? 
        (float)ValidSystems / (float)CachedSystemStatuses.Num() : 0.0f;
    
    return bCharacterValid && bWorldValid && (ValidPercentage >= 0.8f);
}

void UBuild_IntegrationManager::RunFullIntegrationTest()
{
    UE_LOG(LogTemp, Log, TEXT("Running full integration test..."));
    
    ValidateSystemIntegrity();
    EnforceActorCap();
    
    FBuild_IntegrationReport Report = GenerateIntegrationReport();
    
    UE_LOG(LogTemp, Log, TEXT("Integration Test Complete:"));
    UE_LOG(LogTemp, Log, TEXT("- Total Actors: %d"), Report.TotalActorCount);
    UE_LOG(LogTemp, Log, TEXT("- Active Systems: %d/%d"), Report.ActiveSystemCount, Report.SystemStatuses.Num());
    UE_LOG(LogTemp, Log, TEXT("- Game Playable: %s"), Report.bIsPlayable ? TEXT("Yes") : TEXT("No"));
}

void UBuild_IntegrationManager::ValidateWorldGeneration()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        UpdateSystemStatus(TEXT("WorldGeneration"), EBuild_ValidationStatus::Invalid, TEXT("No world context"));
        return;
    }
    
    // Check for PCG World Generator
    TArray<AActor*> PCGActors;
    UGameplayStatics::GetAllActorsOfClass(World, APCGWorldGenerator::StaticClass(), PCGActors);
    
    if (PCGActors.Num() > 0)
    {
        UpdateSystemStatus(TEXT("WorldGeneration"), EBuild_ValidationStatus::Valid);
    }
    else
    {
        UpdateSystemStatus(TEXT("WorldGeneration"), EBuild_ValidationStatus::Invalid, TEXT("No PCG World Generator found"));
    }
}

void UBuild_IntegrationManager::ValidateCharacterSystems()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        UpdateSystemStatus(TEXT("CharacterSystem"), EBuild_ValidationStatus::Invalid, TEXT("No world context"));
        return;
    }
    
    // Check for TranspersonalCharacter
    TArray<AActor*> Characters;
    UGameplayStatics::GetAllActorsOfClass(World, ATranspersonalCharacter::StaticClass(), Characters);
    
    if (Characters.Num() > 0)
    {
        UpdateSystemStatus(TEXT("CharacterSystem"), EBuild_ValidationStatus::Valid);
    }
    else
    {
        UpdateSystemStatus(TEXT("CharacterSystem"), EBuild_ValidationStatus::Invalid, TEXT("No TranspersonalCharacter found"));
    }
}

void UBuild_IntegrationManager::ValidateAISystems()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        UpdateSystemStatus(TEXT("AISystem"), EBuild_ValidationStatus::Invalid, TEXT("No world context"));
        return;
    }
    
    // Check for AI-related actors (placeholder validation)
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);
    
    bool bHasAI = false;
    for (AActor* Actor : AllActors)
    {
        if (Actor && Actor->GetName().ToLower().Contains(TEXT("ai")))
        {
            bHasAI = true;
            break;
        }
    }
    
    if (bHasAI)
    {
        UpdateSystemStatus(TEXT("AISystem"), EBuild_ValidationStatus::Valid);
    }
    else
    {
        UpdateSystemStatus(TEXT("AISystem"), EBuild_ValidationStatus::Warning, TEXT("No AI actors detected"));
    }
}

void UBuild_IntegrationManager::ValidateQASystems()
{
    // Check if QA framework classes are available
    UClass* QAClass = UQA_TestFramework::StaticClass();
    
    if (QAClass)
    {
        UpdateSystemStatus(TEXT("QAFramework"), EBuild_ValidationStatus::Valid);
    }
    else
    {
        UpdateSystemStatus(TEXT("QAFramework"), EBuild_ValidationStatus::Invalid, TEXT("QA Framework not found"));
    }
}

void UBuild_IntegrationManager::UpdateSystemStatus(const FString& SystemName, EBuild_ValidationStatus Status, const FString& ErrorMessage)
{
    // Find existing status or create new one
    FBuild_SystemStatus* ExistingStatus = CachedSystemStatuses.FindByPredicate(
        [&SystemName](const FBuild_SystemStatus& Status)
        {
            return Status.SystemName == SystemName;
        });
    
    if (ExistingStatus)
    {
        ExistingStatus->Status = Status;
        ExistingStatus->ErrorMessage = ErrorMessage;
        ExistingStatus->LastCheckTime = FPlatformTime::Seconds();
    }
    else
    {
        FBuild_SystemStatus NewStatus;
        NewStatus.SystemName = SystemName;
        NewStatus.Status = Status;
        NewStatus.ErrorMessage = ErrorMessage;
        NewStatus.LastCheckTime = FPlatformTime::Seconds();
        CachedSystemStatuses.Add(NewStatus);
    }
}