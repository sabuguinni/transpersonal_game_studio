#include "DynamicLODSystemV43.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Camera/PlayerCameraManager.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Animation/SkeletalMeshActor.h"
#include "DrawDebugHelpers.h"
#include "HAL/IConsoleManager.h"

DEFINE_LOG_CATEGORY(LogDynamicLOD);

UDynamicLODSystemV43::UDynamicLODSystemV43()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Update every 100ms by default
    
    // Default settings
    CalculationMethod = ELODCalculationMethod::Hybrid;
    UpdateFrequency = 10.0f; // 10 updates per second
    PerformanceThreshold = 16.67f; // 60fps target
    bEnablePerformanceBasedLOD = true;
    bEnableDistanceCulling = true;
    GlobalCullDistanceScale = 1.0f;
    ScreenSizeThreshold = 0.01f; // 1% of screen
    
    CurrentLODBias = 0.0f;
    LastUpdateTime = 0.0f;
    AccumulatedDeltaTime = 0.0f;
    
    // Performance monitoring
    MaxFrameTimeHistory = 60; // 6 seconds at 10fps update rate
    AverageFrameTime = 16.67f;
    
    // Initialize statistics
    ResetLODStatistics();
}

void UDynamicLODSystemV43::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogDynamicLOD, Log, TEXT("Dynamic LOD System V43 initialized"));
    
    // Set initial tick interval based on update frequency
    PrimaryComponentTick.TickInterval = 1.0f / UpdateFrequency;
}

void UDynamicLODSystemV43::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    AccumulatedDeltaTime += DeltaTime;
    
    // Only update if enough time has passed
    if (!ShouldUpdateLOD(DeltaTime))
    {
        return;
    }
    
    SCOPE_CYCLE_COUNTER(STAT_LODCalculation);
    
    // Monitor performance if enabled
    if (bEnablePerformanceBasedLOD)
    {
        MonitorPerformance();
    }
    
    // Get viewer location
    FVector ViewerLocation = GetViewerLocation();
    
    // Update LOD for all registered actors
    UpdateAllActorsLOD(ViewerLocation);
    
    // Reset accumulated time
    AccumulatedDeltaTime = 0.0f;
    LastUpdateTime = GetWorld()->GetTimeSeconds();
}

void UDynamicLODSystemV43::UpdateLODForActor(AActor* Actor, const FVector& ViewerLocation)
{
    if (!Actor || !RegisteredActors.Contains(Actor))
    {
        return;
    }
    
    const FLODSettings& Settings = RegisteredActors[Actor];
    float Distance = FVector::Dist(Actor->GetActorLocation(), ViewerLocation);
    
    int32 NewLODLevel = 0;
    bool bShouldCull = false;
    
    // Calculate LOD based on selected method
    switch (CalculationMethod)
    {
        case ELODCalculationMethod::Distance:
            NewLODLevel = CalculateLODByDistance(Distance, Settings);
            break;
            
        case ELODCalculationMethod::ScreenSize:
        {
            float ScreenSize = CalculateScreenSize(Actor, ViewerLocation);
            NewLODLevel = CalculateLODByScreenSize(ScreenSize, Settings);
            break;
        }
        
        case ELODCalculationMethod::Performance:
            NewLODLevel = CalculateLODByPerformance(Settings);
            break;
            
        case ELODCalculationMethod::Hybrid:
        {
            float ScreenSize = CalculateScreenSize(Actor, ViewerLocation);
            NewLODLevel = CalculateHybridLOD(Distance, ScreenSize, Settings);
            break;
        }
    }
    
    // Check for distance culling
    if (bEnableDistanceCulling && Distance > Settings.CullDistance * GlobalCullDistanceScale)
    {
        bShouldCull = true;
    }
    
    // Apply LOD changes
    int32* CurrentLOD = CurrentLODLevels.Find(Actor);
    if (!CurrentLOD || *CurrentLOD != NewLODLevel || bShouldCull)
    {
        CurrentLODLevels.Add(Actor, NewLODLevel);
        
        // Apply to static mesh components
        TArray<UStaticMeshComponent*> StaticMeshComps;
        Actor->GetComponents<UStaticMeshComponent>(StaticMeshComps);
        
        for (UStaticMeshComponent* MeshComp : StaticMeshComps)
        {
            if (bShouldCull)
            {
                ApplyDistanceCulling(MeshComp, Distance, Settings.CullDistance * GlobalCullDistanceScale);
            }
            else
            {
                ApplyLODToStaticMesh(MeshComp, NewLODLevel);
            }
        }
        
        // Apply to skeletal mesh components
        TArray<USkeletalMeshComponent*> SkeletalMeshComps;
        Actor->GetComponents<USkeletalMeshComponent>(SkeletalMeshComps);
        
        for (USkeletalMeshComponent* MeshComp : SkeletalMeshComps)
        {
            if (bShouldCull)
            {
                ApplyDistanceCulling(MeshComp, Distance, Settings.CullDistance * GlobalCullDistanceScale);
            }
            else
            {
                ApplyLODToSkeletalMesh(MeshComp, NewLODLevel);
            }
        }
        
        UE_LOG(LogDynamicLOD, VeryVerbose, TEXT("Updated LOD for %s: LOD%d (Distance: %.1f)"), 
               *Actor->GetName(), NewLODLevel, Distance);
    }
}

void UDynamicLODSystemV43::UpdateAllActorsLOD(const FVector& ViewerLocation)
{
    // Reset statistics
    ResetLODStatistics();
    
    for (auto& ActorPair : RegisteredActors)
    {
        AActor* Actor = ActorPair.Key;
        if (IsValid(Actor))
        {
            UpdateLODForActor(Actor, ViewerLocation);
            
            // Update statistics
            int32* CurrentLOD = CurrentLODLevels.Find(Actor);
            if (CurrentLOD)
            {
                switch (*CurrentLOD)
                {
                    case 0: LOD0Count++; break;
                    case 1: LOD1Count++; break;
                    case 2: LOD2Count++; break;
                    case 3: LOD3Count++; break;
                    default: CulledCount++; break;
                }
            }
        }
        else
        {
            // Clean up invalid actors
            RegisteredActors.Remove(Actor);
            CurrentLODLevels.Remove(Actor);
            LastUpdateTimes.Remove(Actor);
        }
    }
}

void UDynamicLODSystemV43::RegisterActorForLOD(AActor* Actor, const FLODSettings& Settings)
{
    if (!Actor)
    {
        return;
    }
    
    RegisteredActors.Add(Actor, Settings);
    CurrentLODLevels.Add(Actor, 0); // Start with LOD0
    LastUpdateTimes.Add(Actor, GetWorld()->GetTimeSeconds());
    
    UE_LOG(LogDynamicLOD, Log, TEXT("Registered actor for LOD: %s"), *Actor->GetName());
}

void UDynamicLODSystemV43::UnregisterActorFromLOD(AActor* Actor)
{
    if (!Actor)
    {
        return;
    }
    
    RegisteredActors.Remove(Actor);
    CurrentLODLevels.Remove(Actor);
    LastUpdateTimes.Remove(Actor);
    
    UE_LOG(LogDynamicLOD, Log, TEXT("Unregistered actor from LOD: %s"), *Actor->GetName());
}

void UDynamicLODSystemV43::RegisterDinosaurForLOD(AActor* Dinosaur, const FString& DinosaurType)
{
    if (!Dinosaur)
    {
        return;
    }
    
    FLODSettings Settings;
    
    // Determine LOD settings based on dinosaur type
    if (DinosaurType.Contains(TEXT("Small")) || DinosaurType.Contains(TEXT("Compsognathus")))
    {
        Settings = DinosaurProfile.SmallDinosaur;
    }
    else if (DinosaurType.Contains(TEXT("Medium")) || DinosaurType.Contains(TEXT("Velociraptor")))
    {
        Settings = DinosaurProfile.MediumDinosaur;
    }
    else if (DinosaurType.Contains(TEXT("Large")) || DinosaurType.Contains(TEXT("Rex")) || DinosaurType.Contains(TEXT("Triceratops")))
    {
        Settings = DinosaurProfile.LargeDinosaur;
    }
    else if (DinosaurType.Contains(TEXT("Giant")) || DinosaurType.Contains(TEXT("Brontosaurus")))
    {
        Settings = DinosaurProfile.GiantDinosaur;
    }
    else
    {
        // Default to medium dinosaur settings
        Settings = DinosaurProfile.MediumDinosaur;
    }
    
    RegisterActorForLOD(Dinosaur, Settings);
    
    UE_LOG(LogDynamicLOD, Log, TEXT("Registered dinosaur for LOD: %s (Type: %s)"), 
           *Dinosaur->GetName(), *DinosaurType);
}

void UDynamicLODSystemV43::SetDinosaurLODProfile(const FDinosaurLODProfile& Profile)
{
    DinosaurProfile = Profile;
    
    UE_LOG(LogDynamicLOD, Log, TEXT("Updated dinosaur LOD profile"));
}

void UDynamicLODSystemV43::OptimizeLODForFrameRate(float TargetFrameTime)
{
    PerformanceThreshold = TargetFrameTime;
    
    float CurrentFrameTime = GetCurrentFrameTime();
    
    if (CurrentFrameTime > TargetFrameTime * 1.2f)
    {
        // Performance is poor, increase LOD bias
        CurrentLODBias = FMath::Min(CurrentLODBias + 0.5f, 3.0f);
        
        // Reduce cull distances
        GlobalCullDistanceScale = FMath::Max(GlobalCullDistanceScale * 0.9f, 0.5f);
        
        UE_LOG(LogDynamicLOD, Warning, TEXT("Performance optimization: Increased LOD bias to %.2f"), CurrentLODBias);
    }
    else if (CurrentFrameTime < TargetFrameTime * 0.8f)
    {
        // Performance is good, decrease LOD bias
        CurrentLODBias = FMath::Max(CurrentLODBias - 0.2f, 0.0f);
        
        // Increase cull distances
        GlobalCullDistanceScale = FMath::Min(GlobalCullDistanceScale * 1.05f, 1.5f);
        
        UE_LOG(LogDynamicLOD, Log, TEXT("Performance improvement: Decreased LOD bias to %.2f"), CurrentLODBias);
    }
}

int32 UDynamicLODSystemV43::CalculateLODByDistance(float Distance, const FLODSettings& Settings) const
{
    SCOPE_CYCLE_COUNTER(STAT_DistanceCulling);
    
    // Apply LOD bias
    float AdjustedDistance = Distance * (1.0f - CurrentLODBias * 0.2f);
    
    if (AdjustedDistance <= Settings.LOD0Distance)
        return 0;
    else if (AdjustedDistance <= Settings.LOD1Distance)
        return 1;
    else if (AdjustedDistance <= Settings.LOD2Distance)
        return 2;
    else if (AdjustedDistance <= Settings.LOD3Distance)
        return 3;
    else
        return 4; // Culled
}

int32 UDynamicLODSystemV43::CalculateLODByScreenSize(float ScreenSize, const FLODSettings& Settings) const
{
    SCOPE_CYCLE_COUNTER(STAT_ScreenSizeCalculation);
    
    float Threshold = Settings.ScreenSizeThreshold * (1.0f + CurrentLODBias * 0.5f);
    
    if (ScreenSize >= Threshold * 4.0f)
        return 0;
    else if (ScreenSize >= Threshold * 2.0f)
        return 1;
    else if (ScreenSize >= Threshold)
        return 2;
    else if (ScreenSize >= Threshold * 0.5f)
        return 3;
    else
        return 4; // Culled
}

int32 UDynamicLODSystemV43::CalculateLODByPerformance(const FLODSettings& Settings) const
{
    float CurrentFrameTime = GetCurrentFrameTime();
    
    if (CurrentFrameTime > PerformanceThreshold * 1.5f)
        return 3; // Force low LOD
    else if (CurrentFrameTime > PerformanceThreshold * 1.2f)
        return 2;
    else if (CurrentFrameTime > PerformanceThreshold * 1.1f)
        return 1;
    else
        return 0; // High quality
}

int32 UDynamicLODSystemV43::CalculateHybridLOD(float Distance, float ScreenSize, const FLODSettings& Settings) const
{
    int32 DistanceLOD = CalculateLODByDistance(Distance, Settings);
    int32 ScreenSizeLOD = CalculateLODByScreenSize(ScreenSize, Settings);
    int32 PerformanceLOD = bEnablePerformanceBasedLOD ? CalculateLODByPerformance(Settings) : 0;
    
    // Use the highest (worst) LOD level
    return FMath::Max3(DistanceLOD, ScreenSizeLOD, PerformanceLOD);
}

void UDynamicLODSystemV43::ApplyLODToStaticMesh(UStaticMeshComponent* MeshComp, int32 LODLevel)
{
    if (!MeshComp || !MeshComp->GetStaticMesh())
    {
        return;
    }
    
    // Ensure the mesh component is visible
    MeshComp->SetVisibility(true);
    
    // Set forced LOD level
    MeshComp->SetForcedLodModel(LODLevel + 1); // UE5 uses 1-based indexing for forced LOD
}

void UDynamicLODSystemV43::ApplyLODToSkeletalMesh(USkeletalMeshComponent* MeshComp, int32 LODLevel)
{
    if (!MeshComp || !MeshComp->GetSkeletalMeshAsset())
    {
        return;
    }
    
    // Ensure the mesh component is visible
    MeshComp->SetVisibility(true);
    
    // Set forced LOD level
    MeshComp->SetForcedLOD(LODLevel + 1); // UE5 uses 1-based indexing for forced LOD
}

void UDynamicLODSystemV43::ApplyDistanceCulling(UPrimitiveComponent* Component, float Distance, float CullDistance)
{
    if (!Component)
    {
        return;
    }
    
    bool bShouldBeVisible = Distance <= CullDistance;
    Component->SetVisibility(bShouldBeVisible);
    
    if (!bShouldBeVisible)
    {
        CulledCount++;
    }
}

float UDynamicLODSystemV43::CalculateScreenSize(AActor* Actor, const FVector& ViewerLocation) const
{
    if (!Actor)
    {
        return 0.0f;
    }
    
    // Get actor bounds
    FVector Origin, BoxExtent;
    Actor->GetActorBounds(false, Origin, BoxExtent);
    
    float Distance = FVector::Dist(Origin, ViewerLocation);
    if (Distance <= 0.0f)
    {
        return 1.0f; // Very close, full screen
    }
    
    // Calculate approximate screen size based on bounding sphere
    float BoundingRadius = BoxExtent.Size();
    float ScreenSize = (BoundingRadius / Distance) * 2.0f; // Simplified screen projection
    
    return FMath::Clamp(ScreenSize, 0.0f, 1.0f);
}

void UDynamicLODSystemV43::MonitorPerformance()
{
    float CurrentFrameTime = GetCurrentFrameTime();
    
    // Add to history
    FrameTimeHistory.Add(CurrentFrameTime);
    if (FrameTimeHistory.Num() > MaxFrameTimeHistory)
    {
        FrameTimeHistory.RemoveAt(0);
    }
    
    // Calculate average
    if (FrameTimeHistory.Num() > 0)
    {
        float Sum = 0.0f;
        for (float FrameTime : FrameTimeHistory)
        {
            Sum += FrameTime;
        }
        AverageFrameTime = Sum / FrameTimeHistory.Num();
    }
    
    // Adjust LOD based on performance
    AdjustLODBasedOnPerformance(CurrentFrameTime);
}

void UDynamicLODSystemV43::AdjustLODBasedOnPerformance(float CurrentFrameTime)
{
    if (CurrentFrameTime > PerformanceThreshold * 1.3f)
    {
        // Severe performance issues
        CurrentLODBias = FMath::Min(CurrentLODBias + 1.0f, 3.0f);
        GlobalCullDistanceScale = FMath::Max(GlobalCullDistanceScale * 0.8f, 0.3f);
        
        UE_LOG(LogDynamicLOD, Warning, TEXT("Severe performance drop detected: %.2f ms. Applied aggressive LOD bias: %.2f"), 
               CurrentFrameTime, CurrentLODBias);
    }
    else if (CurrentFrameTime > PerformanceThreshold * 1.1f)
    {
        // Minor performance issues
        CurrentLODBias = FMath::Min(CurrentLODBias + 0.3f, 2.0f);
        GlobalCullDistanceScale = FMath::Max(GlobalCullDistanceScale * 0.95f, 0.5f);
    }
    else if (CurrentFrameTime < PerformanceThreshold * 0.8f && CurrentLODBias > 0.0f)
    {
        // Performance is good, can reduce LOD bias
        CurrentLODBias = FMath::Max(CurrentLODBias - 0.1f, 0.0f);
        GlobalCullDistanceScale = FMath::Min(GlobalCullDistanceScale * 1.02f, 1.2f);
    }
}

FVector UDynamicLODSystemV43::GetViewerLocation() const
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return FVector::ZeroVector;
    }
    
    APlayerController* PC = World->GetFirstPlayerController();
    if (!PC || !PC->PlayerCameraManager)
    {
        return FVector::ZeroVector;
    }
    
    return PC->PlayerCameraManager->GetCameraLocation();
}

float UDynamicLODSystemV43::GetCurrentFrameTime() const
{
    return FPlatformTime::ToMilliseconds(GFrameTime);
}

bool UDynamicLODSystemV43::ShouldUpdateLOD(float DeltaTime) const
{
    return AccumulatedDeltaTime >= (1.0f / UpdateFrequency);
}

void UDynamicLODSystemV43::GetLODStatistics(int32& LOD0Count_Out, int32& LOD1Count_Out, int32& LOD2Count_Out, int32& LOD3Count_Out, int32& CulledCount_Out) const
{
    LOD0Count_Out = LOD0Count;
    LOD1Count_Out = LOD1Count;
    LOD2Count_Out = LOD2Count;
    LOD3Count_Out = LOD3Count;
    CulledCount_Out = CulledCount;
}

void UDynamicLODSystemV43::ResetLODStatistics()
{
    LOD0Count = 0;
    LOD1Count = 0;
    LOD2Count = 0;
    LOD3Count = 0;
    CulledCount = 0;
}

void UDynamicLODSystemV43::SetLODCalculationMethod(ELODCalculationMethod Method)
{
    CalculationMethod = Method;
    UE_LOG(LogDynamicLOD, Log, TEXT("LOD calculation method changed to: %d"), (int32)Method);
}

void UDynamicLODSystemV43::SetPerformanceBasedLOD(bool bEnabled)
{
    bEnablePerformanceBasedLOD = bEnabled;
    UE_LOG(LogDynamicLOD, Log, TEXT("Performance-based LOD %s"), bEnabled ? TEXT("enabled") : TEXT("disabled"));
}

void UDynamicLODSystemV43::SetLODBias(float Bias)
{
    CurrentLODBias = FMath::Clamp(Bias, 0.0f, 3.0f);
    UE_LOG(LogDynamicLOD, Log, TEXT("LOD bias set to: %.2f"), CurrentLODBias);
}

void UDynamicLODSystemV43::SetGlobalCullDistanceScale(float Scale)
{
    GlobalCullDistanceScale = FMath::Clamp(Scale, 0.1f, 2.0f);
    UE_LOG(LogDynamicLOD, Log, TEXT("Global cull distance scale set to: %.2f"), GlobalCullDistanceScale);
}

void UDynamicLODSystemV43::EnableDistanceCulling(bool bEnabled)
{
    bEnableDistanceCulling = bEnabled;
    UE_LOG(LogDynamicLOD, Log, TEXT("Distance culling %s"), bEnabled ? TEXT("enabled") : TEXT("disabled"));
}

void UDynamicLODSystemV43::SetScreenSizeThreshold(float Threshold)
{
    ScreenSizeThreshold = FMath::Clamp(Threshold, 0.001f, 0.1f);
    UE_LOG(LogDynamicLOD, Log, TEXT("Screen size threshold set to: %.4f"), ScreenSizeThreshold);
}