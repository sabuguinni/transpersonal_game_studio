#include "Perf_StreamingManager.h"
#include "Engine/World.h"
#include "Engine/LevelStreaming.h"
#include "Engine/LevelStreamingDynamic.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

void UPerf_StreamingManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UpdateFrequency = 1.0f;
    bAutoManageStreaming = true;
    MaxConcurrentLoads = 3;
    
    // Initialize default streaming zones for the 5 biomes
    StreamingZones.Empty();
    
    FPerf_StreamingZone SavannaZone;
    SavannaZone.ZoneName = TEXT("Savanna");
    SavannaZone.CenterLocation = FVector(0.0f, 0.0f, 0.0f);
    SavannaZone.StreamingRadius = 15000.0f;
    SavannaZone.UnloadRadius = 20000.0f;
    StreamingZones.Add(SavannaZone);
    
    FPerf_StreamingZone SwampZone;
    SwampZone.ZoneName = TEXT("Swamp");
    SwampZone.CenterLocation = FVector(-50000.0f, -45000.0f, 0.0f);
    SwampZone.StreamingRadius = 15000.0f;
    SwampZone.UnloadRadius = 20000.0f;
    StreamingZones.Add(SwampZone);
    
    FPerf_StreamingZone ForestZone;
    ForestZone.ZoneName = TEXT("Forest");
    ForestZone.CenterLocation = FVector(-45000.0f, 40000.0f, 0.0f);
    ForestZone.StreamingRadius = 15000.0f;
    ForestZone.UnloadRadius = 20000.0f;
    StreamingZones.Add(ForestZone);
    
    FPerf_StreamingZone DesertZone;
    DesertZone.ZoneName = TEXT("Desert");
    DesertZone.CenterLocation = FVector(55000.0f, 0.0f, 0.0f);
    DesertZone.StreamingRadius = 15000.0f;
    DesertZone.UnloadRadius = 20000.0f;
    StreamingZones.Add(DesertZone);
    
    FPerf_StreamingZone MountainZone;
    MountainZone.ZoneName = TEXT("Mountain");
    MountainZone.CenterLocation = FVector(40000.0f, 50000.0f, 0.0f);
    MountainZone.StreamingRadius = 15000.0f;
    MountainZone.UnloadRadius = 20000.0f;
    StreamingZones.Add(MountainZone);

    // Start streaming update timer
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(StreamingUpdateTimer, this, &UPerf_StreamingManager::UpdateStreaming, UpdateFrequency, true);
    }
}

void UPerf_StreamingManager::Deinitialize()
{
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(StreamingUpdateTimer);
    }
    
    // Unload all streaming levels
    for (auto& LoadedLevel : LoadedLevels)
    {
        if (LoadedLevel.Value)
        {
            LoadedLevel.Value->SetShouldBeLoaded(false);
            LoadedLevel.Value->SetShouldBeVisible(false);
        }
    }
    
    LoadedLevels.Empty();
    Super::Deinitialize();
}

void UPerf_StreamingManager::UpdateStreamingForPlayer(APawn* PlayerPawn)
{
    if (!PlayerPawn || !bAutoManageStreaming)
    {
        return;
    }

    FVector PlayerLocation = PlayerPawn->GetActorLocation();
    
    for (FPerf_StreamingZone& Zone : StreamingZones)
    {
        bool bShouldLoad = ShouldLoadZone(Zone, PlayerLocation);
        bool bShouldUnload = ShouldUnloadZone(Zone, PlayerLocation);
        
        if (bShouldLoad && !Zone.bIsLoaded && !PendingLoads.Contains(Zone.ZoneName))
        {
            if (PendingLoads.Num() < MaxConcurrentLoads)
            {
                StartZoneLoad(Zone.ZoneName);
            }
        }
        else if (bShouldUnload && Zone.bIsLoaded && !PendingUnloads.Contains(Zone.ZoneName))
        {
            StartZoneUnload(Zone.ZoneName);
        }
    }
}

void UPerf_StreamingManager::LoadZone(const FString& ZoneName)
{
    FPerf_StreamingZone* Zone = StreamingZones.FindByPredicate([&ZoneName](const FPerf_StreamingZone& Z)
    {
        return Z.ZoneName == ZoneName;
    });
    
    if (!Zone || Zone->bIsLoaded)
    {
        return;
    }
    
    StartZoneLoad(ZoneName);
}

void UPerf_StreamingManager::UnloadZone(const FString& ZoneName)
{
    FPerf_StreamingZone* Zone = StreamingZones.FindByPredicate([&ZoneName](const FPerf_StreamingZone& Z)
    {
        return Z.ZoneName == ZoneName;
    });
    
    if (!Zone || !Zone->bIsLoaded)
    {
        return;
    }
    
    StartZoneUnload(ZoneName);
}

void UPerf_StreamingManager::SetZoneVisibility(const FString& ZoneName, bool bVisible)
{
    if (ULevelStreaming** FoundLevel = LoadedLevels.Find(ZoneName))
    {
        if (*FoundLevel)
        {
            (*FoundLevel)->SetShouldBeVisible(bVisible);
            
            // Update zone state
            FPerf_StreamingZone* Zone = StreamingZones.FindByPredicate([&ZoneName](const FPerf_StreamingZone& Z)
            {
                return Z.ZoneName == ZoneName;
            });
            
            if (Zone)
            {
                Zone->bIsVisible = bVisible;
            }
        }
    }
}

bool UPerf_StreamingManager::IsZoneLoaded(const FString& ZoneName)
{
    FPerf_StreamingZone* Zone = StreamingZones.FindByPredicate([&ZoneName](const FPerf_StreamingZone& Z)
    {
        return Z.ZoneName == ZoneName;
    });
    
    return Zone ? Zone->bIsLoaded : false;
}

void UPerf_StreamingManager::AddStreamingZone(const FPerf_StreamingZone& NewZone)
{
    // Check if zone already exists
    bool bExists = StreamingZones.ContainsByPredicate([&NewZone](const FPerf_StreamingZone& Z)
    {
        return Z.ZoneName == NewZone.ZoneName;
    });
    
    if (!bExists)
    {
        StreamingZones.Add(NewZone);
    }
}

void UPerf_StreamingManager::RemoveStreamingZone(const FString& ZoneName)
{
    // Unload zone first if it's loaded
    UnloadZone(ZoneName);
    
    // Remove from zones array
    StreamingZones.RemoveAll([&ZoneName](const FPerf_StreamingZone& Z)
    {
        return Z.ZoneName == ZoneName;
    });
}

void UPerf_StreamingManager::PreloadZonesAroundLocation(FVector Location, float Radius)
{
    for (const FPerf_StreamingZone& Zone : StreamingZones)
    {
        float Distance = FVector::Dist(Zone.CenterLocation, Location);
        if (Distance <= Radius && !Zone.bIsLoaded)
        {
            LoadZone(Zone.ZoneName);
        }
    }
}

TArray<FString> UPerf_StreamingManager::GetLoadedZones()
{
    TArray<FString> LoadedZoneNames;
    
    for (const FPerf_StreamingZone& Zone : StreamingZones)
    {
        if (Zone.bIsLoaded)
        {
            LoadedZoneNames.Add(Zone.ZoneName);
        }
    }
    
    return LoadedZoneNames;
}

void UPerf_StreamingManager::UpdateStreaming()
{
    if (!bAutoManageStreaming)
    {
        return;
    }
    
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    APlayerController* PC = World->GetFirstPlayerController();
    if (!PC || !PC->GetPawn())
    {
        return;
    }
    
    UpdateStreamingForPlayer(PC->GetPawn());
    ProcessPendingOperations();
}

void UPerf_StreamingManager::ProcessPendingOperations()
{
    // Process pending loads
    for (int32 i = PendingLoads.Num() - 1; i >= 0; i--)
    {
        const FString& ZoneName = PendingLoads[i];
        
        if (ULevelStreaming** FoundLevel = LoadedLevels.Find(ZoneName))
        {
            if (*FoundLevel && (*FoundLevel)->IsLevelLoaded())
            {
                // Mark zone as loaded
                FPerf_StreamingZone* Zone = StreamingZones.FindByPredicate([&ZoneName](const FPerf_StreamingZone& Z)
                {
                    return Z.ZoneName == ZoneName;
                });
                
                if (Zone)
                {
                    Zone->bIsLoaded = true;
                    Zone->bIsVisible = true;
                }
                
                PendingLoads.RemoveAt(i);
            }
        }
    }
    
    // Process pending unloads
    for (int32 i = PendingUnloads.Num() - 1; i >= 0; i--)
    {
        const FString& ZoneName = PendingUnloads[i];
        
        if (ULevelStreaming** FoundLevel = LoadedLevels.Find(ZoneName))
        {
            if (*FoundLevel && !(*FoundLevel)->IsLevelLoaded())
            {
                // Mark zone as unloaded
                FPerf_StreamingZone* Zone = StreamingZones.FindByPredicate([&ZoneName](const FPerf_StreamingZone& Z)
                {
                    return Z.ZoneName == ZoneName;
                });
                
                if (Zone)
                {
                    Zone->bIsLoaded = false;
                    Zone->bIsVisible = false;
                }
                
                LoadedLevels.Remove(ZoneName);
                PendingUnloads.RemoveAt(i);
            }
        }
    }
}

float UPerf_StreamingManager::CalculateDistanceToZone(const FPerf_StreamingZone& Zone, FVector PlayerLocation)
{
    return FVector::Dist(Zone.CenterLocation, PlayerLocation);
}

bool UPerf_StreamingManager::ShouldLoadZone(const FPerf_StreamingZone& Zone, FVector PlayerLocation)
{
    float Distance = CalculateDistanceToZone(Zone, PlayerLocation);
    return Distance <= Zone.StreamingRadius && !Zone.bIsLoaded;
}

bool UPerf_StreamingManager::ShouldUnloadZone(const FPerf_StreamingZone& Zone, FVector PlayerLocation)
{
    float Distance = CalculateDistanceToZone(Zone, PlayerLocation);
    return Distance > Zone.UnloadRadius && Zone.bIsLoaded;
}

void UPerf_StreamingManager::StartZoneLoad(const FString& ZoneName)
{
    if (PendingLoads.Contains(ZoneName))
    {
        return;
    }
    
    FPerf_StreamingZone* Zone = StreamingZones.FindByPredicate([&ZoneName](const FPerf_StreamingZone& Z)
    {
        return Z.ZoneName == ZoneName;
    });
    
    if (!Zone)
    {
        return;
    }
    
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    // Create streaming level if it doesn't exist
    if (!LoadedLevels.Contains(ZoneName))
    {
        FString LevelPath = FString::Printf(TEXT("/Game/Maps/Biomes/%s"), *ZoneName);
        ULevelStreaming* StreamingLevel = ULevelStreamingDynamic::LoadLevelInstance(World, LevelPath, Zone->CenterLocation, FRotator::ZeroRotator);
        
        if (StreamingLevel)
        {
            LoadedLevels.Add(ZoneName, StreamingLevel);
        }
    }
    
    if (ULevelStreaming** FoundLevel = LoadedLevels.Find(ZoneName))
    {
        if (*FoundLevel)
        {
            (*FoundLevel)->SetShouldBeLoaded(true);
            (*FoundLevel)->SetShouldBeVisible(true);
            PendingLoads.Add(ZoneName);
        }
    }
}

void UPerf_StreamingManager::StartZoneUnload(const FString& ZoneName)
{
    if (PendingUnloads.Contains(ZoneName))
    {
        return;
    }
    
    if (ULevelStreaming** FoundLevel = LoadedLevels.Find(ZoneName))
    {
        if (*FoundLevel)
        {
            (*FoundLevel)->SetShouldBeLoaded(false);
            (*FoundLevel)->SetShouldBeVisible(false);
            PendingUnloads.Add(ZoneName);
        }
    }
}