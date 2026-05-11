#include "Perf_LODController.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "TranspersonalGame.h"

DEFINE_LOG_CATEGORY_STATIC(LogPerfLODController, Log, All);

UPerf_LODController::UPerf_LODController()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.5f; // Update every 500ms
    
    // Initialize LOD distance thresholds
    LODDistances.Add(0.0f);      // LOD 0: 0-500 units
    LODDistances.Add(500.0f);    // LOD 1: 500-1500 units  
    LODDistances.Add(1500.0f);   // LOD 2: 1500-3000 units
    LODDistances.Add(3000.0f);   // LOD 3: 3000+ units
    
    // Initialize performance-based LOD bias
    PerformanceLODBias = 0;
    TargetFrameTime = 16.67f; // 60 FPS
    MaxLODBias = 2;
    
    bAutoLODEnabled = true;
    bPerformanceBasedLOD = true;
    
    UE_LOG(LogPerfLODController, Log, TEXT("LOD Controller initialized"));
}

void UPerf_LODController::BeginPlay()
{
    Super::BeginPlay();
    
    // Find player pawn for distance calculations
    if (UWorld* World = GetWorld())
    {
        PlayerPawn = UGameplayStatics::GetPlayerPawn(World, 0);
    }
    
    // Initialize managed objects
    RefreshManagedObjects();
}

void UPerf_LODController::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!bAutoLODEnabled || !PlayerPawn)
        return;
        
    // Update performance-based LOD bias
    if (bPerformanceBasedLOD)
    {
        UpdatePerformanceLODBias();
    }
    
    // Update LOD for all managed objects
    UpdateManagedObjectsLOD();
    
    // Periodically refresh the list of managed objects
    static float RefreshTimer = 0.0f;
    RefreshTimer += DeltaTime;
    if (RefreshTimer >= 5.0f) // Refresh every 5 seconds
    {
        RefreshManagedObjects();
        RefreshTimer = 0.0f;
    }
}

void UPerf_LODController::RefreshManagedObjects()
{
    ManagedStaticMeshes.Empty();
    ManagedSkeletalMeshes.Empty();
    
    if (!GetWorld())
        return;
        
    // Find all static mesh components in the world
    for (TActorIterator<AActor> ActorIterator(GetWorld()); ActorIterator; ++ActorIterator)
    {
        AActor* Actor = *ActorIterator;
        if (!Actor || Actor->IsPendingKill())
            continue;
            
        // Get static mesh components
        TArray<UStaticMeshComponent*> StaticMeshComponents;
        Actor->GetComponents<UStaticMeshComponent>(StaticMeshComponents);
        
        for (UStaticMeshComponent* MeshComp : StaticMeshComponents)
        {
            if (MeshComp && MeshComp->GetStaticMesh())
            {
                FPerf_LODObject LODObject;
                LODObject.StaticMeshComponent = MeshComp;
                LODObject.SkeletalMeshComponent = nullptr;
                LODObject.CurrentLOD = 0;
                LODObject.TargetLOD = 0;
                LODObject.LastDistance = 0.0f;
                
                ManagedStaticMeshes.Add(LODObject);
            }
        }
        
        // Get skeletal mesh components
        TArray<USkeletalMeshComponent*> SkeletalMeshComponents;
        Actor->GetComponents<USkeletalMeshComponent>(SkeletalMeshComponents);
        
        for (USkeletalMeshComponent* MeshComp : SkeletalMeshComponents)
        {
            if (MeshComp && MeshComp->GetSkeletalMeshAsset())
            {
                FPerf_LODObject LODObject;
                LODObject.StaticMeshComponent = nullptr;
                LODObject.SkeletalMeshComponent = MeshComp;
                LODObject.CurrentLOD = 0;
                LODObject.TargetLOD = 0;
                LODObject.LastDistance = 0.0f;
                
                ManagedSkeletalMeshes.Add(LODObject);
            }
        }
    }
    
    UE_LOG(LogPerfLODController, Log, TEXT("Refreshed managed objects: %d static meshes, %d skeletal meshes"), 
        ManagedStaticMeshes.Num(), ManagedSkeletalMeshes.Num());
}

void UPerf_LODController::UpdateManagedObjectsLOD()
{
    if (!PlayerPawn)
        return;
        
    FVector PlayerLocation = PlayerPawn->GetActorLocation();
    
    // Update static meshes
    for (FPerf_LODObject& LODObject : ManagedStaticMeshes)
    {
        if (!LODObject.StaticMeshComponent || !IsValid(LODObject.StaticMeshComponent))
            continue;
            
        FVector ObjectLocation = LODObject.StaticMeshComponent->GetComponentLocation();
        float Distance = FVector::Dist(PlayerLocation, ObjectLocation);
        LODObject.LastDistance = Distance;
        
        // Calculate target LOD based on distance
        int32 TargetLOD = CalculateLODFromDistance(Distance);
        
        // Apply performance bias
        TargetLOD = FMath::Clamp(TargetLOD + PerformanceLODBias, 0, 3);
        
        if (TargetLOD != LODObject.CurrentLOD)
        {
            LODObject.StaticMeshComponent->SetForcedLodModel(TargetLOD + 1); // UE5 uses 1-based LOD indices
            LODObject.CurrentLOD = TargetLOD;
        }
    }
    
    // Update skeletal meshes
    for (FPerf_LODObject& LODObject : ManagedSkeletalMeshes)
    {
        if (!LODObject.SkeletalMeshComponent || !IsValid(LODObject.SkeletalMeshComponent))
            continue;
            
        FVector ObjectLocation = LODObject.SkeletalMeshComponent->GetComponentLocation();
        float Distance = FVector::Dist(PlayerLocation, ObjectLocation);
        LODObject.LastDistance = Distance;
        
        // Calculate target LOD based on distance
        int32 TargetLOD = CalculateLODFromDistance(Distance);
        
        // Apply performance bias
        TargetLOD = FMath::Clamp(TargetLOD + PerformanceLODBias, 0, 3);
        
        if (TargetLOD != LODObject.CurrentLOD)
        {
            LODObject.SkeletalMeshComponent->SetForcedLOD(TargetLOD + 1); // UE5 uses 1-based LOD indices
            LODObject.CurrentLOD = TargetLOD;
        }
    }
}

int32 UPerf_LODController::CalculateLODFromDistance(float Distance) const
{
    for (int32 i = LODDistances.Num() - 1; i >= 0; i--)
    {
        if (Distance >= LODDistances[i])
        {
            return i;
        }
    }
    return 0;
}

void UPerf_LODController::UpdatePerformanceLODBias()
{
    float CurrentFrameTime = FApp::GetDeltaTime() * 1000.0f; // Convert to ms
    
    // Adjust LOD bias based on frame time
    if (CurrentFrameTime > TargetFrameTime * 1.2f) // 20% over target
    {
        // Performance is poor, increase LOD bias (lower quality)
        PerformanceLODBias = FMath::Min(PerformanceLODBias + 1, MaxLODBias);
    }
    else if (CurrentFrameTime < TargetFrameTime * 0.8f) // 20% under target
    {
        // Performance is good, decrease LOD bias (higher quality)
        PerformanceLODBias = FMath::Max(PerformanceLODBias - 1, 0);
    }
}

void UPerf_LODController::SetLODDistances(const TArray<float>& NewLODDistances)
{
    if (NewLODDistances.Num() >= 2)
    {
        LODDistances = NewLODDistances;
        UE_LOG(LogPerfLODController, Log, TEXT("LOD distances updated"));
    }
    else
    {
        UE_LOG(LogPerfLODController, Warning, TEXT("LOD distances array must have at least 2 elements"));
    }
}

void UPerf_LODController::SetPerformanceTarget(float InTargetFrameTime)
{
    TargetFrameTime = FMath::Max(InTargetFrameTime, 8.33f); // Minimum 120 FPS
    UE_LOG(LogPerfLODController, Log, TEXT("Performance target set to %.2fms"), TargetFrameTime);
}

void UPerf_LODController::SetAutoLODEnabled(bool bEnabled)
{
    bAutoLODEnabled = bEnabled;
    UE_LOG(LogPerfLODController, Log, TEXT("Auto LOD %s"), bEnabled ? TEXT("enabled") : TEXT("disabled"));
}

void UPerf_LODController::SetPerformanceBasedLODEnabled(bool bEnabled)
{
    bPerformanceBasedLOD = bEnabled;
    if (!bEnabled)
    {
        PerformanceLODBias = 0; // Reset bias when disabled
    }
    UE_LOG(LogPerfLODController, Log, TEXT("Performance-based LOD %s"), bEnabled ? TEXT("enabled") : TEXT("disabled"));
}

void UPerf_LODController::ForceLODLevel(int32 LODLevel)
{
    LODLevel = FMath::Clamp(LODLevel, 0, 3);
    
    // Apply to all managed static meshes
    for (FPerf_LODObject& LODObject : ManagedStaticMeshes)
    {
        if (LODObject.StaticMeshComponent && IsValid(LODObject.StaticMeshComponent))
        {
            LODObject.StaticMeshComponent->SetForcedLodModel(LODLevel + 1);
            LODObject.CurrentLOD = LODLevel;
        }
    }
    
    // Apply to all managed skeletal meshes
    for (FPerf_LODObject& LODObject : ManagedSkeletalMeshes)
    {
        if (LODObject.SkeletalMeshComponent && IsValid(LODObject.SkeletalMeshComponent))
        {
            LODObject.SkeletalMeshComponent->SetForcedLOD(LODLevel + 1);
            LODObject.CurrentLOD = LODLevel;
        }
    }
    
    UE_LOG(LogPerfLODController, Log, TEXT("Forced all objects to LOD level %d"), LODLevel);
}

FPerf_LODStats UPerf_LODController::GetLODStats() const
{
    FPerf_LODStats Stats;
    Stats.TotalManagedObjects = ManagedStaticMeshes.Num() + ManagedSkeletalMeshes.Num();
    Stats.CurrentPerformanceBias = PerformanceLODBias;
    Stats.bAutoLODEnabled = bAutoLODEnabled;
    Stats.bPerformanceBasedLOD = bPerformanceBasedLOD;
    
    // Count objects per LOD level
    Stats.ObjectsPerLOD.Init(0, 4);
    
    for (const FPerf_LODObject& LODObject : ManagedStaticMeshes)
    {
        if (LODObject.CurrentLOD >= 0 && LODObject.CurrentLOD < 4)
        {
            Stats.ObjectsPerLOD[LODObject.CurrentLOD]++;
        }
    }
    
    for (const FPerf_LODObject& LODObject : ManagedSkeletalMeshes)
    {
        if (LODObject.CurrentLOD >= 0 && LODObject.CurrentLOD < 4)
        {
            Stats.ObjectsPerLOD[LODObject.CurrentLOD]++;
        }
    }
    
    return Stats;
}