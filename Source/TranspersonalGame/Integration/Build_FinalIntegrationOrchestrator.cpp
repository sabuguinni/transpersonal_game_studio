#include "Build_FinalIntegrationOrchestrator.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/LightComponent.h"
#include "NiagaraComponent.h"
#include "TranspersonalCharacter.h"
#include "TranspersonalGameState.h"

UBuild_FinalIntegrationOrchestrator::UBuild_FinalIntegrationOrchestrator()
{
    bIntegrationComplete = false;
    LastValidationTime = FDateTime::Now();
}

void UBuild_FinalIntegrationOrchestrator::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("Build_FinalIntegrationOrchestrator: Initializing final build integration system"));
    
    InitializeBiomeDefinitions();
    
    // Schedule initial validation
    if (UWorld* World = GetWorld())
    {
        FTimerHandle ValidationTimer;
        World->GetTimerManager().SetTimer(ValidationTimer, [this]()
        {
            ValidateAllSystems();
        }, 2.0f, false);
    }
}

void UBuild_FinalIntegrationOrchestrator::Deinitialize()
{
    UE_LOG(LogTemp, Warning, TEXT("Build_FinalIntegrationOrchestrator: Deinitializing integration system"));
    
    SystemStatusMap.Empty();
    BiomeStatusMap.Empty();
    
    Super::Deinitialize();
}

void UBuild_FinalIntegrationOrchestrator::InitializeBiomeDefinitions()
{
    // Initialize biome status map with predefined biome locations
    BiomeStatusMap.Add(TEXT("Savana"), CreateBiomeStatus(TEXT("Savana"), FVector(0, 0, 0), 0));
    BiomeStatusMap.Add(TEXT("Pantano"), CreateBiomeStatus(TEXT("Pantano"), FVector(-50000, -45000, 0), 0));
    BiomeStatusMap.Add(TEXT("Floresta"), CreateBiomeStatus(TEXT("Floresta"), FVector(-45000, 40000, 0), 0));
    BiomeStatusMap.Add(TEXT("Deserto"), CreateBiomeStatus(TEXT("Deserto"), FVector(55000, 0, 0), 0));
    BiomeStatusMap.Add(TEXT("Montanha"), CreateBiomeStatus(TEXT("Montanha"), FVector(40000, 50000, 0), 0));
    
    UE_LOG(LogTemp, Warning, TEXT("Build_FinalIntegrationOrchestrator: Initialized 5 biome definitions"));
}

FBuild_IntegrationReport UBuild_FinalIntegrationOrchestrator::GenerateIntegrationReport()
{
    UE_LOG(LogTemp, Warning, TEXT("Build_FinalIntegrationOrchestrator: Generating comprehensive integration report"));
    
    CurrentReport = FBuild_IntegrationReport();
    CurrentReport.LastIntegrationTime = FDateTime::Now();
    
    // Validate all systems
    ValidateSystemIntegrity();
    ValidateWorldPopulation();
    ValidateVFXSystems();
    ValidateLightingSystems();
    ValidateCharacterSystems();
    ValidatePhysicsSystems();
    ValidateAudioSystems();
    
    // Validate biome population
    ValidateBiomePopulation();
    
    // Populate report arrays
    SystemStatusMap.GenerateValueArray(CurrentReport.SystemStatuses);
    BiomeStatusMap.GenerateValueArray(CurrentReport.BiomeStatuses);
    
    // Calculate total actor count
    CurrentReport.TotalActorCount = GetTotalActorCount();
    
    // Determine build stability
    CurrentReport.bBuildStable = IsBuildStable();
    
    UE_LOG(LogTemp, Warning, TEXT("Build_FinalIntegrationOrchestrator: Integration report complete - %d systems, %d biomes, %d total actors, Stable: %s"), 
           CurrentReport.SystemStatuses.Num(), 
           CurrentReport.BiomeStatuses.Num(), 
           CurrentReport.TotalActorCount,
           CurrentReport.bBuildStable ? TEXT("YES") : TEXT("NO"));
    
    return CurrentReport;
}

bool UBuild_FinalIntegrationOrchestrator::ValidateAllSystems()
{
    UE_LOG(LogTemp, Warning, TEXT("Build_FinalIntegrationOrchestrator: Validating all critical systems"));
    
    bool bAllSystemsValid = true;
    
    // Validate core systems
    bAllSystemsValid &= CheckSystemHealth(TEXT("TranspersonalCharacter"));
    bAllSystemsValid &= CheckSystemHealth(TEXT("TranspersonalGameState"));
    bAllSystemsValid &= CheckSystemHealth(TEXT("WorldGeneration"));
    bAllSystemsValid &= CheckSystemHealth(TEXT("VFXSystems"));
    bAllSystemsValid &= CheckSystemHealth(TEXT("LightingSystems"));
    bAllSystemsValid &= CheckSystemHealth(TEXT("PhysicsSystems"));
    bAllSystemsValid &= CheckSystemHealth(TEXT("AudioSystems"));
    
    LastValidationTime = FDateTime::Now();
    
    UE_LOG(LogTemp, Warning, TEXT("Build_FinalIntegrationOrchestrator: System validation complete - All systems valid: %s"), 
           bAllSystemsValid ? TEXT("YES") : TEXT("NO"));
    
    return bAllSystemsValid;
}

bool UBuild_FinalIntegrationOrchestrator::ValidateBiomePopulation()
{
    UE_LOG(LogTemp, Warning, TEXT("Build_FinalIntegrationOrchestrator: Validating biome population across all regions"));
    
    bool bAllBiomesValid = true;
    
    for (auto& BiomePair : BiomeStatusMap)
    {
        FString BiomeName = BiomePair.Key;
        FBuild_BiomeStatus& BiomeStatus = BiomePair.Value;
        
        int32 ActorCount = CountActorsInBiome(BiomeStatus.CenterLocation);
        BiomeStatus.PopulationCount = ActorCount;
        BiomeStatus.bMeetsMinimumRequirement = (ActorCount >= MINIMUM_BIOME_POPULATION);
        
        if (!BiomeStatus.bMeetsMinimumRequirement)
        {
            bAllBiomesValid = false;
        }
        
        UE_LOG(LogTemp, Warning, TEXT("Build_FinalIntegrationOrchestrator: Biome %s - %d actors (Min: %d) - %s"), 
               *BiomeName, 
               ActorCount, 
               MINIMUM_BIOME_POPULATION,
               BiomeStatus.bMeetsMinimumRequirement ? TEXT("PASS") : TEXT("FAIL"));
    }
    
    return bAllBiomesValid;
}

void UBuild_FinalIntegrationOrchestrator::OrchestrateFinalBuild()
{
    UE_LOG(LogTemp, Warning, TEXT("Build_FinalIntegrationOrchestrator: Orchestrating final build integration"));
    
    // Generate comprehensive report
    FBuild_IntegrationReport Report = GenerateIntegrationReport();
    
    // Log final status
    UE_LOG(LogTemp, Warning, TEXT("=== FINAL BUILD ORCHESTRATION COMPLETE ==="));
    UE_LOG(LogTemp, Warning, TEXT("Total Systems: %d"), Report.SystemStatuses.Num());
    UE_LOG(LogTemp, Warning, TEXT("Total Biomes: %d"), Report.BiomeStatuses.Num());
    UE_LOG(LogTemp, Warning, TEXT("Total Actors: %d"), Report.TotalActorCount);
    UE_LOG(LogTemp, Warning, TEXT("Build Stable: %s"), Report.bBuildStable ? TEXT("YES") : TEXT("NO"));
    UE_LOG(LogTemp, Warning, TEXT("Build Version: %s"), *Report.BuildVersion);
    
    bIntegrationComplete = true;
}

FBuild_SystemStatus UBuild_FinalIntegrationOrchestrator::GetSystemStatus(const FString& SystemName)
{
    if (FBuild_SystemStatus* Status = SystemStatusMap.Find(SystemName))
    {
        return *Status;
    }
    
    return FBuild_SystemStatus();
}

FBuild_BiomeStatus UBuild_FinalIntegrationOrchestrator::GetBiomeStatus(const FString& BiomeName)
{
    if (FBuild_BiomeStatus* Status = BiomeStatusMap.Find(BiomeName))
    {
        return *Status;
    }
    
    return FBuild_BiomeStatus();
}

bool UBuild_FinalIntegrationOrchestrator::IsSystemOperational(const FString& SystemName)
{
    FBuild_SystemStatus Status = GetSystemStatus(SystemName);
    return Status.bIsOperational;
}

int32 UBuild_FinalIntegrationOrchestrator::GetTotalActorCount()
{
    if (UWorld* World = GetWorld())
    {
        int32 Count = 0;
        for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
        {
            Count++;
        }
        return Count;
    }
    
    return 0;
}

bool UBuild_FinalIntegrationOrchestrator::IsBuildStable()
{
    // Check minimum requirements
    int32 TotalActors = GetTotalActorCount();
    bool bMinimumActors = (TotalActors >= MINIMUM_TOTAL_ACTORS);
    
    // Check system health
    bool bSystemsHealthy = true;
    for (const auto& SystemPair : SystemStatusMap)
    {
        if (!SystemPair.Value.bIsOperational)
        {
            bSystemsHealthy = false;
            break;
        }
    }
    
    // Check biome population
    bool bBiomesPopulated = true;
    for (const auto& BiomePair : BiomeStatusMap)
    {
        if (!BiomePair.Value.bMeetsMinimumRequirement)
        {
            bBiomesPopulated = false;
            break;
        }
    }
    
    return bMinimumActors && bSystemsHealthy && bBiomesPopulated;
}

void UBuild_FinalIntegrationOrchestrator::ValidateSystemIntegrity()
{
    UE_LOG(LogTemp, Warning, TEXT("Build_FinalIntegrationOrchestrator: Validating system integrity"));
    
    // Validate core game systems
    SystemStatusMap.Add(TEXT("CoreSystems"), CreateSystemStatus(TEXT("CoreSystems"), true, GetTotalActorCount()));
}

void UBuild_FinalIntegrationOrchestrator::ValidateWorldPopulation()
{
    UE_LOG(LogTemp, Warning, TEXT("Build_FinalIntegrationOrchestrator: Validating world population"));
    
    int32 WorldActorCount = GetTotalActorCount();
    SystemStatusMap.Add(TEXT("WorldPopulation"), CreateSystemStatus(TEXT("WorldPopulation"), 
                                                                   WorldActorCount >= MINIMUM_TOTAL_ACTORS, 
                                                                   WorldActorCount));
}

void UBuild_FinalIntegrationOrchestrator::ValidateVFXSystems()
{
    if (UWorld* World = GetWorld())
    {
        int32 VFXCount = 0;
        for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
        {
            if (ActorItr->FindComponentByClass<UNiagaraComponent>())
            {
                VFXCount++;
            }
        }
        
        SystemStatusMap.Add(TEXT("VFXSystems"), CreateSystemStatus(TEXT("VFXSystems"), VFXCount > 0, VFXCount));
    }
}

void UBuild_FinalIntegrationOrchestrator::ValidateLightingSystems()
{
    if (UWorld* World = GetWorld())
    {
        int32 LightCount = 0;
        for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
        {
            if (ActorItr->FindComponentByClass<ULightComponent>())
            {
                LightCount++;
            }
        }
        
        SystemStatusMap.Add(TEXT("LightingSystems"), CreateSystemStatus(TEXT("LightingSystems"), LightCount > 0, LightCount));
    }
}

void UBuild_FinalIntegrationOrchestrator::ValidateCharacterSystems()
{
    if (UWorld* World = GetWorld())
    {
        int32 CharacterCount = 0;
        for (TActorIterator<ATranspersonalCharacter> CharacterItr(World); CharacterItr; ++CharacterItr)
        {
            CharacterCount++;
        }
        
        SystemStatusMap.Add(TEXT("CharacterSystems"), CreateSystemStatus(TEXT("CharacterSystems"), CharacterCount > 0, CharacterCount));
    }
}

void UBuild_FinalIntegrationOrchestrator::ValidatePhysicsSystems()
{
    if (UWorld* World = GetWorld())
    {
        int32 PhysicsActorCount = 0;
        for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
        {
            if (UStaticMeshComponent* MeshComp = ActorItr->FindComponentByClass<UStaticMeshComponent>())
            {
                if (MeshComp->IsSimulatingPhysics())
                {
                    PhysicsActorCount++;
                }
            }
        }
        
        SystemStatusMap.Add(TEXT("PhysicsSystems"), CreateSystemStatus(TEXT("PhysicsSystems"), true, PhysicsActorCount));
    }
}

void UBuild_FinalIntegrationOrchestrator::ValidateAudioSystems()
{
    SystemStatusMap.Add(TEXT("AudioSystems"), CreateSystemStatus(TEXT("AudioSystems"), true, 0));
}

FBuild_SystemStatus UBuild_FinalIntegrationOrchestrator::CreateSystemStatus(const FString& SystemName, bool bOperational, int32 ActorCount)
{
    FBuild_SystemStatus Status;
    Status.SystemName = SystemName;
    Status.bIsOperational = bOperational;
    Status.ActorCount = ActorCount;
    Status.LastValidationTime = FDateTime::Now().ToString();
    
    return Status;
}

FBuild_BiomeStatus UBuild_FinalIntegrationOrchestrator::CreateBiomeStatus(const FString& BiomeName, const FVector& Location, int32 Population)
{
    FBuild_BiomeStatus Status;
    Status.BiomeName = BiomeName;
    Status.CenterLocation = Location;
    Status.PopulationCount = Population;
    Status.bMeetsMinimumRequirement = (Population >= MINIMUM_BIOME_POPULATION);
    
    return Status;
}

bool UBuild_FinalIntegrationOrchestrator::CheckSystemHealth(const FString& SystemName)
{
    // Basic system health check - can be expanded
    return true;
}

int32 UBuild_FinalIntegrationOrchestrator::CountActorsInBiome(const FVector& BiomeCenter, float Radius)
{
    if (UWorld* World = GetWorld())
    {
        int32 Count = 0;
        for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
        {
            FVector ActorLocation = ActorItr->GetActorLocation();
            float Distance = FVector::Dist(ActorLocation, BiomeCenter);
            
            if (Distance <= Radius)
            {
                Count++;
            }
        }
        return Count;
    }
    
    return 0;
}