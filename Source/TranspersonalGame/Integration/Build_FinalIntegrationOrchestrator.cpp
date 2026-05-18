#include "Build_FinalIntegrationOrchestrator.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/DateTime.h"

UBuild_FinalIntegrationOrchestrator::UBuild_FinalIntegrationOrchestrator()
{
    bValidationInProgress = false;
    LastValidationTime = 0.0f;
}

void UBuild_FinalIntegrationOrchestrator::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("Build_FinalIntegrationOrchestrator: Initializing comprehensive system validation"));
    
    InitializeBiomeData();
    
    // Start initial validation after 2 seconds
    FTimerHandle TimerHandle;
    GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &UBuild_FinalIntegrationOrchestrator::StartFullSystemValidation, 2.0f, false);
}

void UBuild_FinalIntegrationOrchestrator::Deinitialize()
{
    UE_LOG(LogTemp, Warning, TEXT("Build_FinalIntegrationOrchestrator: Deinitializing"));
    Super::Deinitialize();
}

void UBuild_FinalIntegrationOrchestrator::InitializeBiomeData()
{
    BiomeData.Empty();
    
    // Initialize biome data based on memory coordinates
    FBuild_BiomePopulationData SavanaBiome;
    SavanaBiome.BiomeName = TEXT("Savana");
    SavanaBiome.BiomeCenter = FVector(0.0f, 0.0f, 0.0f);
    SavanaBiome.RequiredActors = 500;
    BiomeData.Add(SavanaBiome);
    
    FBuild_BiomePopulationData PantanoBiome;
    PantanoBiome.BiomeName = TEXT("Pantano");
    PantanoBiome.BiomeCenter = FVector(-50000.0f, -45000.0f, 0.0f);
    PantanoBiome.RequiredActors = 500;
    BiomeData.Add(PantanoBiome);
    
    FBuild_BiomePopulationData FlorestaBiome;
    FlorestaBiome.BiomeName = TEXT("Floresta");
    FlorestaBiome.BiomeCenter = FVector(-45000.0f, 40000.0f, 0.0f);
    FlorestaBiome.RequiredActors = 500;
    BiomeData.Add(FlorestaBiome);
    
    FBuild_BiomePopulationData DesertoBiome;
    DesertoBiome.BiomeName = TEXT("Deserto");
    DesertoBiome.BiomeCenter = FVector(55000.0f, 0.0f, 0.0f);
    DesertoBiome.RequiredActors = 500;
    BiomeData.Add(DesertoBiome);
    
    FBuild_BiomePopulationData MontanhaBiome;
    MontanhaBiome.BiomeName = TEXT("Montanha");
    MontanhaBiome.BiomeCenter = FVector(40000.0f, 50000.0f, 0.0f);
    MontanhaBiome.RequiredActors = 500;
    BiomeData.Add(MontanhaBiome);
    
    UE_LOG(LogTemp, Warning, TEXT("Build_FinalIntegrationOrchestrator: Initialized %d biomes for validation"), BiomeData.Num());
}

void UBuild_FinalIntegrationOrchestrator::StartFullSystemValidation()
{
    if (bValidationInProgress)
    {
        UE_LOG(LogTemp, Warning, TEXT("Build_FinalIntegrationOrchestrator: Validation already in progress"));
        return;
    }
    
    bValidationInProgress = true;
    LastValidationTime = FPlatformTime::Seconds();
    
    UE_LOG(LogTemp, Warning, TEXT("Build_FinalIntegrationOrchestrator: Starting full system validation"));
    
    // Clear previous results
    SystemValidationResults.Empty();
    
    // Validate all systems
    ValidateAllBiomes();
    ValidateDinosaurSystems();
    ValidateCharacterSystems();
    ValidateWorldGeneration();
    
    bValidationInProgress = false;
    
    // Generate final report
    GenerateIntegrationReport();
}

void UBuild_FinalIntegrationOrchestrator::ValidateAllBiomes()
{
    UE_LOG(LogTemp, Warning, TEXT("Build_FinalIntegrationOrchestrator: Validating all biomes"));
    
    for (FBuild_BiomePopulationData& Biome : BiomeData)
    {
        ValidateActorsInBiome(Biome.BiomeName, Biome.BiomeCenter, 25000.0f);
    }
    
    FBuild_SystemValidationResult BiomeResult;
    BiomeResult.SystemName = TEXT("BiomePopulation");
    BiomeResult.Status = EBuild_IntegrationStatus::Success;
    BiomeResult.ActorCount = 0;
    
    for (const FBuild_BiomePopulationData& Biome : BiomeData)
    {
        BiomeResult.ActorCount += Biome.CurrentActors;
    }
    
    SystemValidationResults.Add(TEXT("BiomePopulation"), BiomeResult);
    LogValidationResult(TEXT("BiomePopulation"), EBuild_IntegrationStatus::Success, FString::Printf(TEXT("Total actors across biomes: %d"), BiomeResult.ActorCount));
}

void UBuild_FinalIntegrationOrchestrator::ValidateActorsInBiome(const FString& BiomeName, const FVector& Center, float Radius)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    int32 ActorCount = 0;
    
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (Actor && !Actor->IsPendingKill())
        {
            float Distance = FVector::Dist(Actor->GetActorLocation(), Center);
            if (Distance <= Radius)
            {
                ActorCount++;
            }
        }
    }
    
    // Update biome data
    for (FBuild_BiomePopulationData& Biome : BiomeData)
    {
        if (Biome.BiomeName == BiomeName)
        {
            Biome.CurrentActors = ActorCount;
            Biome.bIsPopulated = (ActorCount >= Biome.RequiredActors);
            break;
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Build_FinalIntegrationOrchestrator: Biome %s has %d actors (required: 500)"), *BiomeName, ActorCount);
}

void UBuild_FinalIntegrationOrchestrator::ValidateDinosaurSystems()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        LogValidationResult(TEXT("DinosaurSystems"), EBuild_IntegrationStatus::Failed, TEXT("No world available"));
        return;
    }
    
    int32 DinosaurCount = 0;
    
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (Actor && Actor->GetName().Contains(TEXT("Dinosaur")) || Actor->GetName().Contains(TEXT("TRex")) || Actor->GetName().Contains(TEXT("Raptor")))
        {
            DinosaurCount++;
        }
    }
    
    FBuild_SystemValidationResult DinosaurResult;
    DinosaurResult.SystemName = TEXT("DinosaurSystems");
    DinosaurResult.Status = (DinosaurCount > 0) ? EBuild_IntegrationStatus::Success : EBuild_IntegrationStatus::Failed;
    DinosaurResult.ActorCount = DinosaurCount;
    
    SystemValidationResults.Add(TEXT("DinosaurSystems"), DinosaurResult);
    LogValidationResult(TEXT("DinosaurSystems"), DinosaurResult.Status, FString::Printf(TEXT("Found %d dinosaur actors"), DinosaurCount));
}

void UBuild_FinalIntegrationOrchestrator::ValidateCharacterSystems()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        LogValidationResult(TEXT("CharacterSystems"), EBuild_IntegrationStatus::Failed, TEXT("No world available"));
        return;
    }
    
    // Check for player character
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(World, 0);
    bool bHasPlayerCharacter = (PlayerPawn != nullptr);
    
    FBuild_SystemValidationResult CharacterResult;
    CharacterResult.SystemName = TEXT("CharacterSystems");
    CharacterResult.Status = bHasPlayerCharacter ? EBuild_IntegrationStatus::Success : EBuild_IntegrationStatus::Failed;
    CharacterResult.ActorCount = bHasPlayerCharacter ? 1 : 0;
    
    SystemValidationResults.Add(TEXT("CharacterSystems"), CharacterResult);
    LogValidationResult(TEXT("CharacterSystems"), CharacterResult.Status, bHasPlayerCharacter ? TEXT("Player character found") : TEXT("No player character"));
}

void UBuild_FinalIntegrationOrchestrator::ValidateWorldGeneration()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        LogValidationResult(TEXT("WorldGeneration"), EBuild_IntegrationStatus::Failed, TEXT("No world available"));
        return;
    }
    
    int32 TotalActors = 0;
    int32 StaticMeshActors = 0;
    
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (Actor && !Actor->IsPendingKill())
        {
            TotalActors++;
            
            if (Actor->FindComponentByClass<UStaticMeshComponent>())
            {
                StaticMeshActors++;
            }
        }
    }
    
    FBuild_SystemValidationResult WorldGenResult;
    WorldGenResult.SystemName = TEXT("WorldGeneration");
    WorldGenResult.Status = (TotalActors > 100) ? EBuild_IntegrationStatus::Success : EBuild_IntegrationStatus::Failed;
    WorldGenResult.ActorCount = TotalActors;
    
    SystemValidationResults.Add(TEXT("WorldGeneration"), WorldGenResult);
    LogValidationResult(TEXT("WorldGeneration"), WorldGenResult.Status, FString::Printf(TEXT("Total: %d actors, StaticMesh: %d"), TotalActors, StaticMeshActors));
}

FBuild_SystemValidationResult UBuild_FinalIntegrationOrchestrator::GetSystemStatus(const FString& SystemName)
{
    if (SystemValidationResults.Contains(SystemName))
    {
        return SystemValidationResults[SystemName];
    }
    
    FBuild_SystemValidationResult EmptyResult;
    EmptyResult.SystemName = SystemName;
    EmptyResult.Status = EBuild_IntegrationStatus::Pending;
    return EmptyResult;
}

TArray<FBuild_BiomePopulationData> UBuild_FinalIntegrationOrchestrator::GetBiomePopulationStatus()
{
    return BiomeData;
}

bool UBuild_FinalIntegrationOrchestrator::IsSystemStable()
{
    if (bValidationInProgress)
    {
        return false;
    }
    
    for (const auto& ValidationPair : SystemValidationResults)
    {
        if (ValidationPair.Value.Status == EBuild_IntegrationStatus::Failed)
        {
            return false;
        }
    }
    
    return true;
}

void UBuild_FinalIntegrationOrchestrator::GenerateIntegrationReport()
{
    UE_LOG(LogTemp, Warning, TEXT("=== FINAL INTEGRATION REPORT ==="));
    UE_LOG(LogTemp, Warning, TEXT("Validation completed at: %s"), *FDateTime::Now().ToString());
    
    int32 PassedSystems = 0;
    int32 TotalSystems = SystemValidationResults.Num();
    
    for (const auto& ValidationPair : SystemValidationResults)
    {
        const FBuild_SystemValidationResult& Result = ValidationPair.Value;
        FString StatusText = (Result.Status == EBuild_IntegrationStatus::Success) ? TEXT("PASS") : TEXT("FAIL");
        
        if (Result.Status == EBuild_IntegrationStatus::Success)
        {
            PassedSystems++;
        }
        
        UE_LOG(LogTemp, Warning, TEXT("System: %s - Status: %s - Actors: %d"), *Result.SystemName, *StatusText, Result.ActorCount);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Overall Status: %d/%d systems passed"), PassedSystems, TotalSystems);
    UE_LOG(LogTemp, Warning, TEXT("System Stability: %s"), IsSystemStable() ? TEXT("STABLE") : TEXT("UNSTABLE"));
    
    // Log biome population status
    UE_LOG(LogTemp, Warning, TEXT("=== BIOME POPULATION STATUS ==="));
    for (const FBuild_BiomePopulationData& Biome : BiomeData)
    {
        FString PopulationStatus = Biome.bIsPopulated ? TEXT("POPULATED") : TEXT("UNDERPOPULATED");
        UE_LOG(LogTemp, Warning, TEXT("Biome: %s - Actors: %d/500 - Status: %s"), *Biome.BiomeName, Biome.CurrentActors, *PopulationStatus);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("=== END INTEGRATION REPORT ==="));
}

void UBuild_FinalIntegrationOrchestrator::LogValidationResult(const FString& SystemName, EBuild_IntegrationStatus Status, const FString& Message)
{
    FString StatusText;
    switch (Status)
    {
        case EBuild_IntegrationStatus::Success:
            StatusText = TEXT("SUCCESS");
            break;
        case EBuild_IntegrationStatus::Failed:
            StatusText = TEXT("FAILED");
            break;
        case EBuild_IntegrationStatus::InProgress:
            StatusText = TEXT("IN_PROGRESS");
            break;
        case EBuild_IntegrationStatus::Timeout:
            StatusText = TEXT("TIMEOUT");
            break;
        default:
            StatusText = TEXT("PENDING");
            break;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Build_FinalIntegrationOrchestrator: [%s] %s - %s"), *StatusText, *SystemName, *Message);
}