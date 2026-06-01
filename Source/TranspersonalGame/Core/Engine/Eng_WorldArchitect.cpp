#include "Eng_WorldArchitect.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Engine/Engine.h"

UEng_WorldArchitect::UEng_WorldArchitect()
{
    bIsInitialized = false;
}

void UEng_WorldArchitect::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("UEng_WorldArchitect: Initializing World Architect subsystem"));
    
    CreateDefaultZones();
    bIsInitialized = true;
    
    UE_LOG(LogTemp, Warning, TEXT("UEng_WorldArchitect: Initialized with %d zones"), WorldZones.Num());
}

void UEng_WorldArchitect::Deinitialize()
{
    WorldZones.Empty();
    bIsInitialized = false;
    
    Super::Deinitialize();
}

void UEng_WorldArchitect::InitializeWorldZones()
{
    if (!bIsInitialized)
    {
        CreateDefaultZones();
        bIsInitialized = true;
    }
    
    UpdateZoneActorCounts();
}

void UEng_WorldArchitect::CreateDefaultZones()
{
    WorldZones.Empty();
    
    // Create 5 biome zones based on memory coordinates
    FEng_WorldZone SavannaZone;
    SavannaZone.ZoneName = TEXT("Savanna");
    SavannaZone.CenterLocation = FVector(0.0f, 0.0f, 100.0f);
    SavannaZone.BiomeType = EBiomeType::Savanna;
    SavannaZone.MaxActors = 1600; // 20% of 8000 total
    WorldZones.Add(SavannaZone);
    
    FEng_WorldZone SwampZone;
    SwampZone.ZoneName = TEXT("Swamp");
    SwampZone.CenterLocation = FVector(-50000.0f, -45000.0f, 100.0f);
    SwampZone.BiomeType = EBiomeType::Swamp;
    SwampZone.MaxActors = 1600;
    WorldZones.Add(SwampZone);
    
    FEng_WorldZone ForestZone;
    ForestZone.ZoneName = TEXT("Forest");
    ForestZone.CenterLocation = FVector(-45000.0f, 40000.0f, 100.0f);
    ForestZone.BiomeType = EBiomeType::Forest;
    ForestZone.MaxActors = 1600;
    WorldZones.Add(ForestZone);
    
    FEng_WorldZone DesertZone;
    DesertZone.ZoneName = TEXT("Desert");
    DesertZone.CenterLocation = FVector(55000.0f, 0.0f, 100.0f);
    DesertZone.BiomeType = EBiomeType::Desert;
    DesertZone.MaxActors = 1600;
    WorldZones.Add(DesertZone);
    
    FEng_WorldZone MountainZone;
    MountainZone.ZoneName = TEXT("Mountain");
    MountainZone.CenterLocation = FVector(40000.0f, 50000.0f, 100.0f);
    MountainZone.BiomeType = EBiomeType::Mountain;
    MountainZone.MaxActors = 1600;
    WorldZones.Add(MountainZone);
    
    UE_LOG(LogTemp, Warning, TEXT("UEng_WorldArchitect: Created %d default zones"), WorldZones.Num());
}

bool UEng_WorldArchitect::CanSpawnActorInZone(const FVector& Location, const FString& ActorType)
{
    FEng_WorldZone* Zone = FindZoneByLocation(Location);
    if (!Zone)
    {
        return false;
    }
    
    // Check zone limits
    if (Zone->CurrentActors >= Zone->MaxActors)
    {
        UE_LOG(LogTemp, Warning, TEXT("UEng_WorldArchitect: Zone %s at capacity (%d/%d)"), 
               *Zone->ZoneName, Zone->CurrentActors, Zone->MaxActors);
        return false;
    }
    
    // Check global limits
    int32 TotalActors = GetTotalActorCount();
    if (TotalActors >= PerformanceLimits.MaxTotalActors)
    {
        UE_LOG(LogTemp, Warning, TEXT("UEng_WorldArchitect: Global actor limit reached (%d/%d)"), 
               TotalActors, PerformanceLimits.MaxTotalActors);
        return false;
    }
    
    return true;
}

FEng_WorldZone UEng_WorldArchitect::GetZoneAtLocation(const FVector& Location)
{
    FEng_WorldZone* Zone = FindZoneByLocation(Location);
    if (Zone)
    {
        return *Zone;
    }
    
    // Return default zone if not found
    FEng_WorldZone DefaultZone;
    return DefaultZone;
}

void UEng_WorldArchitect::RegisterActorSpawned(const FVector& Location, const FString& ActorType)
{
    FEng_WorldZone* Zone = FindZoneByLocation(Location);
    if (Zone)
    {
        Zone->CurrentActors++;
        UE_LOG(LogTemp, Log, TEXT("UEng_WorldArchitect: Registered %s in %s (%d/%d)"), 
               *ActorType, *Zone->ZoneName, Zone->CurrentActors, Zone->MaxActors);
    }
}

void UEng_WorldArchitect::CleanupExcessActors()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    int32 TotalActors = GetTotalActorCount();
    if (TotalActors <= PerformanceLimits.MaxTotalActors)
    {
        return;
    }
    
    int32 ActorsToRemove = TotalActors - PerformanceLimits.MaxTotalActors;
    int32 RemovedCount = 0;
    
    UE_LOG(LogTemp, Warning, TEXT("UEng_WorldArchitect: Cleaning up %d excess actors"), ActorsToRemove);
    
    // Remove oldest non-essential actors first
    for (TActorIterator<AActor> ActorItr(World); ActorItr && RemovedCount < ActorsToRemove; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (!Actor || Actor->IsA<APawn>() || Actor->GetName().Contains(TEXT("PlayerStart")))
        {
            continue; // Skip important actors
        }
        
        Actor->Destroy();
        RemovedCount++;
    }
    
    UpdateZoneActorCounts();
    UE_LOG(LogTemp, Warning, TEXT("UEng_WorldArchitect: Removed %d actors"), RemovedCount);
}

int32 UEng_WorldArchitect::GetTotalActorCount()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return 0;
    }
    
    int32 Count = 0;
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        Count++;
    }
    
    return Count;
}

void UEng_WorldArchitect::UpdateZoneActorCounts()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    // Reset all zone counts
    for (FEng_WorldZone& Zone : WorldZones)
    {
        Zone.CurrentActors = 0;
    }
    
    // Count actors in each zone
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (!Actor)
        {
            continue;
        }
        
        FVector ActorLocation = Actor->GetActorLocation();
        FEng_WorldZone* Zone = FindZoneByLocation(ActorLocation);
        if (Zone)
        {
            Zone->CurrentActors++;
        }
    }
}

FEng_WorldZone* UEng_WorldArchitect::FindZoneByLocation(const FVector& Location)
{
    for (FEng_WorldZone& Zone : WorldZones)
    {
        float Distance = FVector::Dist2D(Location, Zone.CenterLocation);
        if (Distance <= Zone.Radius)
        {
            return &Zone;
        }
    }
    
    return nullptr;
}