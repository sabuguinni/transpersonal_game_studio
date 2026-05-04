#include "World_MassiveTerrainManager.h"
#include "Engine/World.h"
#include "Landscape.h"
#include "LandscapeProxy.h"
#include "LandscapeStreamingProxy.h"
#include "LandscapeInfo.h"
#include "LandscapeComponent.h"
#include "Engine/Engine.h"
#include "Engine/GameViewportClient.h"
#include "Kismet/GameplayStatics.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Materials/MaterialInterface.h"
#include "Materials/Material.h"

DEFINE_LOG_CATEGORY(LogMassiveTerrain);

UWorld_MassiveTerrainManager::UWorld_MassiveTerrainManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 1.0f;
    
    // Initialize 200km2 target parameters
    TargetLandscapeSize = 20000000.0f; // 200km in cm
    CurrentLandscapeSize = 0.0f;
    ExpansionProgress = 0.0f;
    
    // Streaming parameters
    StreamingDistance = 500000.0f; // 5km streaming radius
    LODDistance1 = 100000.0f; // 1km for LOD1
    LODDistance2 = 250000.0f; // 2.5km for LOD2
    
    // Performance limits
    MaxActiveProxies = 25;
    MaxComponentsPerFrame = 10;
    
    bIsExpanding = false;
    bStreamingEnabled = true;
    
    UE_LOG(LogMassiveTerrain, Warning, TEXT("MassiveTerrainManager initialized for 200km2 world"));
}

void UWorld_MassiveTerrainManager::BeginPlay()
{
    Super::BeginPlay();
    
    // Find existing landscape
    AnalyzeCurrentLandscape();
    
    // Initialize streaming system
    InitializeStreamingSystem();
    
    UE_LOG(LogMassiveTerrain, Warning, TEXT("MassiveTerrainManager BeginPlay - Current size: %.2f km2"), 
           (CurrentLandscapeSize * CurrentLandscapeSize) / 100000000.0f);
}

void UWorld_MassiveTerrainManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (bStreamingEnabled)
    {
        UpdateStreamingProxies();
    }
    
    if (bIsExpanding)
    {
        UpdateExpansionProgress(DeltaTime);
    }
}

void UWorld_MassiveTerrainManager::AnalyzeCurrentLandscape()
{
    UWorld* World = GetWorld();
    if (!World) return;
    
    // Find all landscape actors
    TArray<AActor*> FoundLandscapes;
    UGameplayStatics::GetAllActorsOfClass(World, ALandscape::StaticClass(), FoundLandscapes);
    
    if (FoundLandscapes.Num() > 0)
    {
        MainLandscape = Cast<ALandscape>(FoundLandscapes[0]);
        if (MainLandscape)
        {
            // Calculate current size
            FBox Bounds = MainLandscape->GetComponentsBoundingBox();
            FVector Size = Bounds.GetSize();
            CurrentLandscapeSize = FMath::Max(Size.X, Size.Y);
            
            UE_LOG(LogMassiveTerrain, Warning, TEXT("Found main landscape - Size: %.2f x %.2f UU"), 
                   Size.X, Size.Y);
        }
    }
    else
    {
        UE_LOG(LogMassiveTerrain, Error, TEXT("No landscape found in level!"));
    }
}

void UWorld_MassiveTerrainManager::InitializeStreamingSystem()
{
    if (!MainLandscape) return;
    
    // Create streaming proxy grid for 200km2
    int32 ProxyGridSize = 5; // 5x5 grid of proxies
    float ProxySize = TargetLandscapeSize / ProxyGridSize;
    
    StreamingProxies.Empty();
    
    for (int32 X = 0; X < ProxyGridSize; X++)
    {
        for (int32 Y = 0; Y < ProxyGridSize; Y++)
        {
            FWorld_StreamingProxy ProxyData;
            ProxyData.GridX = X;
            ProxyData.GridY = Y;
            ProxyData.WorldLocation = FVector(
                (X - ProxyGridSize/2) * ProxySize,
                (Y - ProxyGridSize/2) * ProxySize,
                0.0f
            );
            ProxyData.Size = ProxySize;
            ProxyData.bIsActive = false;
            ProxyData.LODLevel = 3; // Start with lowest LOD
            
            StreamingProxies.Add(ProxyData);
        }
    }
    
    UE_LOG(LogMassiveTerrain, Warning, TEXT("Initialized %d streaming proxies for massive terrain"), 
           StreamingProxies.Num());
}

void UWorld_MassiveTerrainManager::UpdateStreamingProxies()
{
    if (!MainLandscape) return;
    
    // Get player location
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (!PlayerPawn) return;
    
    FVector PlayerLocation = PlayerPawn->GetActorLocation();
    int32 ActiveProxies = 0;
    
    for (FWorld_StreamingProxy& Proxy : StreamingProxies)
    {
        float Distance = FVector::Dist2D(PlayerLocation, Proxy.WorldLocation);
        
        // Determine if proxy should be active
        bool bShouldBeActive = Distance < StreamingDistance;
        
        // Determine LOD level
        int32 NewLOD = 3;
        if (Distance < LODDistance1) NewLOD = 0;
        else if (Distance < LODDistance2) NewLOD = 1;
        else if (Distance < StreamingDistance) NewLOD = 2;
        
        // Update proxy state
        if (bShouldBeActive && !Proxy.bIsActive && ActiveProxies < MaxActiveProxies)
        {
            ActivateStreamingProxy(Proxy);
            ActiveProxies++;
        }
        else if (!bShouldBeActive && Proxy.bIsActive)
        {
            DeactivateStreamingProxy(Proxy);
        }
        else if (Proxy.bIsActive)
        {
            ActiveProxies++;
            if (Proxy.LODLevel != NewLOD)
            {
                UpdateProxyLOD(Proxy, NewLOD);
            }
        }
    }
}

void UWorld_MassiveTerrainManager::ActivateStreamingProxy(FWorld_StreamingProxy& Proxy)
{
    if (Proxy.bIsActive) return;
    
    // Create or activate landscape streaming proxy
    UWorld* World = GetWorld();
    if (!World || !MainLandscape) return;
    
    // For now, just mark as active - full implementation would create ALandscapeStreamingProxy
    Proxy.bIsActive = true;
    Proxy.LODLevel = 2; // Start with medium LOD
    
    UE_LOG(LogMassiveTerrain, Log, TEXT("Activated streaming proxy at (%d,%d) - Location: %s"), 
           Proxy.GridX, Proxy.GridY, *Proxy.WorldLocation.ToString());
}

void UWorld_MassiveTerrainManager::DeactivateStreamingProxy(FWorld_StreamingProxy& Proxy)
{
    if (!Proxy.bIsActive) return;
    
    Proxy.bIsActive = false;
    Proxy.LODLevel = 3;
    
    UE_LOG(LogMassiveTerrain, Log, TEXT("Deactivated streaming proxy at (%d,%d)"), 
           Proxy.GridX, Proxy.GridY);
}

void UWorld_MassiveTerrainManager::UpdateProxyLOD(FWorld_StreamingProxy& Proxy, int32 NewLOD)
{
    if (Proxy.LODLevel == NewLOD) return;
    
    Proxy.LODLevel = NewLOD;
    
    // Update visual representation based on LOD
    // LOD 0: Full detail, LOD 1: Medium, LOD 2: Low, LOD 3: Minimal/Disabled
}

void UWorld_MassiveTerrainManager::StartLandscapeExpansion()
{
    if (bIsExpanding) return;
    
    if (CurrentLandscapeSize >= TargetLandscapeSize)
    {
        UE_LOG(LogMassiveTerrain, Warning, TEXT("Landscape already at target size: %.2f"), CurrentLandscapeSize);
        return;
    }
    
    bIsExpanding = true;
    ExpansionProgress = 0.0f;
    ExpansionStartSize = CurrentLandscapeSize;
    
    UE_LOG(LogMassiveTerrain, Warning, TEXT("Starting landscape expansion from %.2f to %.2f"), 
           ExpansionStartSize, TargetLandscapeSize);
}

void UWorld_MassiveTerrainManager::UpdateExpansionProgress(float DeltaTime)
{
    if (!bIsExpanding) return;
    
    // Gradual expansion over time
    float ExpansionRate = 100000.0f; // 1km per second
    float SizeIncrease = ExpansionRate * DeltaTime;
    
    CurrentLandscapeSize = FMath::Min(CurrentLandscapeSize + SizeIncrease, TargetLandscapeSize);
    ExpansionProgress = (CurrentLandscapeSize - ExpansionStartSize) / (TargetLandscapeSize - ExpansionStartSize);
    
    if (CurrentLandscapeSize >= TargetLandscapeSize)
    {
        bIsExpanding = false;
        ExpansionProgress = 1.0f;
        UE_LOG(LogMassiveTerrain, Warning, TEXT("Landscape expansion completed! Size: %.2f km2"), 
               (CurrentLandscapeSize * CurrentLandscapeSize) / 100000000.0f);
    }
}

void UWorld_MassiveTerrainManager::SetStreamingEnabled(bool bEnabled)
{
    bStreamingEnabled = bEnabled;
    
    if (!bEnabled)
    {
        // Deactivate all proxies
        for (FWorld_StreamingProxy& Proxy : StreamingProxies)
        {
            if (Proxy.bIsActive)
            {
                DeactivateStreamingProxy(Proxy);
            }
        }
    }
    
    UE_LOG(LogMassiveTerrain, Warning, TEXT("Streaming %s"), bEnabled ? TEXT("ENABLED") : TEXT("DISABLED"));
}

float UWorld_MassiveTerrainManager::GetCurrentSizeKm2() const
{
    return (CurrentLandscapeSize * CurrentLandscapeSize) / 100000000.0f; // Convert cm2 to km2
}

int32 UWorld_MassiveTerrainManager::GetActiveProxyCount() const
{
    int32 Count = 0;
    for (const FWorld_StreamingProxy& Proxy : StreamingProxies)
    {
        if (Proxy.bIsActive) Count++;
    }
    return Count;
}

FString UWorld_MassiveTerrainManager::GetExpansionStatus() const
{
    if (bIsExpanding)
    {
        return FString::Printf(TEXT("Expanding: %.1f%% (%.2f/%.2f km2)"), 
                              ExpansionProgress * 100.0f, 
                              GetCurrentSizeKm2(), 
                              (TargetLandscapeSize * TargetLandscapeSize) / 100000000.0f);
    }
    else if (CurrentLandscapeSize >= TargetLandscapeSize)
    {
        return FString::Printf(TEXT("Complete: %.2f km2"), GetCurrentSizeKm2());
    }
    else
    {
        return FString::Printf(TEXT("Ready to expand: %.2f km2"), GetCurrentSizeKm2());
    }
}