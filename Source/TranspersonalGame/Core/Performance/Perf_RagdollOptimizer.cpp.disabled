// Copyright Transpersonal Game Studio. All Rights Reserved.

#include "Perf_RagdollOptimizer.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "Components/SkeletalMeshComponent.h"
#include "PhysicsEngine/PhysicsAsset.h"
#include "Engine/Engine.h"
#include "HAL/PlatformFilemanager.h"

UPerf_RagdollOptimizer::UPerf_RagdollOptimizer()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Update 10 times per second
    
    // Default performance settings
    RagdollQuality = EPerf_RagdollQuality::Medium;
    MaxActiveRagdolls = 10;
    RagdollCullDistance = 2000.0f;
    RagdollLifetime = 5.0f;
    
    // Default LOD distances
    LOD0Distance = 500.0f;
    LOD1Distance = 1000.0f;
    LOD2Distance = 2000.0f;
    
    // Performance monitoring
    bEnablePerformanceMonitoring = true;
    PerformanceUpdateInterval = 1.0f;
    MaxPhysicsTimeMs = 5.0f;
    MaxRagdollMemoryMB = 100.0f;
    
    // Initialize metrics
    CurrentMetrics = FPerf_RagdollPerformanceMetrics();
}

void UPerf_RagdollOptimizer::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Log, TEXT("Perf_RagdollOptimizer: System initialized"));
    
    // Apply initial quality settings
    ApplyQualitySettings();
    
    // Start performance monitoring
    if (bEnablePerformanceMonitoring)
    {
        LastPerformanceUpdate = GetWorld()->GetTimeSeconds();
    }
}

void UPerf_RagdollOptimizer::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Remove invalid ragdolls
    RemoveInvalidRagdolls();
    
    // Update performance metrics
    if (bEnablePerformanceMonitoring)
    {
        float CurrentTime = GetWorld()->GetTimeSeconds();
        if (CurrentTime - LastPerformanceUpdate >= PerformanceUpdateInterval)
        {
            UpdatePerformanceMetrics();
            LastPerformanceUpdate = CurrentTime;
        }
    }
    
    // Optimize ragdolls based on distance and performance
    OptimizeAllRagdolls();
    
    // Clean up expired ragdolls
    CleanupExpiredRagdolls();
    
    // Cull distant ragdolls if performance is poor
    if (!IsPerformanceOptimal())
    {
        CullDistantRagdolls();
        ApplyPerformanceOptimizations();
    }
}

void UPerf_RagdollOptimizer::EnableRagdoll(USkeletalMeshComponent* SkeletalMesh, bool bApplyLOD)
{
    if (!SkeletalMesh || !SkeletalMesh->GetPhysicsAsset())
    {
        UE_LOG(LogTemp, Warning, TEXT("Perf_RagdollOptimizer: Invalid skeletal mesh or missing physics asset"));
        return;
    }
    
    // Check if we're at the ragdoll limit
    if (ActiveRagdolls.Num() >= MaxActiveRagdolls)
    {
        // Remove the oldest ragdoll
        if (ActiveRagdolls.Num() > 0)
        {
            USkeletalMeshComponent* OldestRagdoll = ActiveRagdolls[0];
            DisableRagdoll(OldestRagdoll);
        }
    }
    
    // Enable ragdoll physics
    SkeletalMesh->SetSimulatePhysics(true);
    SkeletalMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    SkeletalMesh->SetCollisionObjectType(ECollisionChannel::ECC_PhysicsBody);
    
    // Add to active ragdolls
    ActiveRagdolls.AddUnique(SkeletalMesh);
    RagdollActivationTimes.Add(SkeletalMesh, GetWorld()->GetTimeSeconds());
    
    // Apply LOD if requested
    if (bApplyLOD)
    {
        float Distance = GetDistanceToPlayer(SkeletalMesh->GetOwner());
        UpdateRagdollLOD(SkeletalMesh, Distance);
    }
    
    UE_LOG(LogTemp, Log, TEXT("Perf_RagdollOptimizer: Ragdoll enabled for %s"), 
           SkeletalMesh->GetOwner() ? *SkeletalMesh->GetOwner()->GetName() : TEXT("Unknown"));
}

void UPerf_RagdollOptimizer::DisableRagdoll(USkeletalMeshComponent* SkeletalMesh)
{
    if (!SkeletalMesh)
    {
        return;
    }
    
    // Disable ragdoll physics
    SkeletalMesh->SetSimulatePhysics(false);
    SkeletalMesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    
    // Remove from tracking
    ActiveRagdolls.Remove(SkeletalMesh);
    RagdollActivationTimes.Remove(SkeletalMesh);
    
    UE_LOG(LogTemp, Log, TEXT("Perf_RagdollOptimizer: Ragdoll disabled for %s"), 
           SkeletalMesh->GetOwner() ? *SkeletalMesh->GetOwner()->GetName() : TEXT("Unknown"));
}

void UPerf_RagdollOptimizer::OptimizeAllRagdolls()
{
    for (USkeletalMeshComponent* RagdollMesh : ActiveRagdolls)
    {
        if (RagdollMesh && RagdollMesh->GetOwner())
        {
            float Distance = GetDistanceToPlayer(RagdollMesh->GetOwner());
            UpdateRagdollLOD(RagdollMesh, Distance);
        }
    }
}

void UPerf_RagdollOptimizer::CullDistantRagdolls()
{
    TArray<USkeletalMeshComponent*> RagdollsToRemove;
    
    for (USkeletalMeshComponent* RagdollMesh : ActiveRagdolls)
    {
        if (RagdollMesh && RagdollMesh->GetOwner())
        {
            float Distance = GetDistanceToPlayer(RagdollMesh->GetOwner());
            if (Distance > RagdollCullDistance)
            {
                RagdollsToRemove.Add(RagdollMesh);
            }
        }
    }
    
    for (USkeletalMeshComponent* RagdollToRemove : RagdollsToRemove)
    {
        DisableRagdoll(RagdollToRemove);
    }
    
    if (RagdollsToRemove.Num() > 0)
    {
        UE_LOG(LogTemp, Log, TEXT("Perf_RagdollOptimizer: Culled %d distant ragdolls"), RagdollsToRemove.Num());
    }
}

void UPerf_RagdollOptimizer::UpdateRagdollLOD(USkeletalMeshComponent* SkeletalMesh, float Distance)
{
    if (!SkeletalMesh)
    {
        return;
    }
    
    int32 LODLevel = CalculateLODLevel(Distance);
    ApplyLODToRagdoll(SkeletalMesh, LODLevel);
}

int32 UPerf_RagdollOptimizer::CalculateLODLevel(float Distance) const
{
    if (Distance <= LOD0Distance)
    {
        return 0; // Highest quality
    }
    else if (Distance <= LOD1Distance)
    {
        return 1; // Medium quality
    }
    else if (Distance <= LOD2Distance)
    {
        return 2; // Low quality
    }
    else
    {
        return 3; // Lowest quality or disabled
    }
}

void UPerf_RagdollOptimizer::UpdatePerformanceMetrics()
{
    // Update basic metrics
    CurrentMetrics.ActiveRagdolls = ActiveRagdolls.Num();
    CurrentMetrics.PhysicsBodies = 0;
    
    // Count physics bodies
    for (USkeletalMeshComponent* RagdollMesh : ActiveRagdolls)
    {
        if (RagdollMesh && RagdollMesh->GetPhysicsAsset())
        {
            CurrentMetrics.PhysicsBodies += RagdollMesh->GetPhysicsAsset()->SkeletalBodySetups.Num();
        }
    }
    
    // Calculate physics time (simplified estimation)
    if (PhysicsFrameCount > 0)
    {
        CurrentMetrics.PhysicsTimeMs = (AccumulatedPhysicsTime / PhysicsFrameCount) * 1000.0f;
        AccumulatedPhysicsTime = 0.0f;
        PhysicsFrameCount = 0;
    }
    
    // Calculate memory usage
    CurrentMetrics.RagdollMemoryMB = CalculateRagdollMemoryUsage();
    
    // Check performance warning
    CurrentMetrics.bPerformanceWarning = 
        (CurrentMetrics.PhysicsTimeMs > MaxPhysicsTimeMs) ||
        (CurrentMetrics.RagdollMemoryMB > MaxRagdollMemoryMB) ||
        (CurrentMetrics.ActiveRagdolls > MaxActiveRagdolls);
    
    if (CurrentMetrics.bPerformanceWarning)
    {
        LogPerformanceWarning(TEXT("Performance thresholds exceeded"));
    }
}

bool UPerf_RagdollOptimizer::IsPerformanceOptimal() const
{
    return !CurrentMetrics.bPerformanceWarning;
}

void UPerf_RagdollOptimizer::ApplyPerformanceOptimizations()
{
    if (CurrentMetrics.ActiveRagdolls > MaxActiveRagdolls)
    {
        // Reduce active ragdolls
        int32 ExcessRagdolls = CurrentMetrics.ActiveRagdolls - MaxActiveRagdolls;
        for (int32 i = 0; i < ExcessRagdolls && ActiveRagdolls.Num() > 0; i++)
        {
            DisableRagdoll(ActiveRagdolls[0]);
        }
    }
    
    if (CurrentMetrics.PhysicsTimeMs > MaxPhysicsTimeMs)
    {
        // Reduce ragdoll quality
        if (RagdollQuality > EPerf_RagdollQuality::Low)
        {
            SetRagdollQuality(static_cast<EPerf_RagdollQuality>(static_cast<int32>(RagdollQuality) - 1));
            LogPerformanceWarning(TEXT("Reduced ragdoll quality due to high physics time"));
        }
    }
}

void UPerf_RagdollOptimizer::SetRagdollQuality(EPerf_RagdollQuality NewQuality)
{
    RagdollQuality = NewQuality;
    ApplyQualitySettings();
    
    UE_LOG(LogTemp, Log, TEXT("Perf_RagdollOptimizer: Quality set to %d"), static_cast<int32>(RagdollQuality));
}

void UPerf_RagdollOptimizer::ApplyQualitySettings()
{
    switch (RagdollQuality)
    {
        case EPerf_RagdollQuality::Disabled:
            MaxActiveRagdolls = 0;
            RagdollCullDistance = 0.0f;
            break;
            
        case EPerf_RagdollQuality::Low:
            MaxActiveRagdolls = 3;
            RagdollCullDistance = 1000.0f;
            RagdollLifetime = 3.0f;
            break;
            
        case EPerf_RagdollQuality::Medium:
            MaxActiveRagdolls = 6;
            RagdollCullDistance = 1500.0f;
            RagdollLifetime = 5.0f;
            break;
            
        case EPerf_RagdollQuality::High:
            MaxActiveRagdolls = 10;
            RagdollCullDistance = 2000.0f;
            RagdollLifetime = 8.0f;
            break;
            
        case EPerf_RagdollQuality::Ultra:
            MaxActiveRagdolls = 15;
            RagdollCullDistance = 3000.0f;
            RagdollLifetime = 12.0f;
            break;
    }
}

TArray<USkeletalMeshComponent*> UPerf_RagdollOptimizer::GetActiveRagdolls() const
{
    return ActiveRagdolls;
}

float UPerf_RagdollOptimizer::GetDistanceToPlayer(AActor* Actor) const
{
    if (!Actor || !GetWorld())
    {
        return 0.0f;
    }
    
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (!PlayerPawn)
    {
        return 0.0f;
    }
    
    return FVector::Dist(Actor->GetActorLocation(), PlayerPawn->GetActorLocation());
}

void UPerf_RagdollOptimizer::CleanupExpiredRagdolls()
{
    float CurrentTime = GetWorld()->GetTimeSeconds();
    TArray<USkeletalMeshComponent*> ExpiredRagdolls;
    
    for (auto& RagdollPair : RagdollActivationTimes)
    {
        if (CurrentTime - RagdollPair.Value > RagdollLifetime)
        {
            ExpiredRagdolls.Add(RagdollPair.Key);
        }
    }
    
    for (USkeletalMeshComponent* ExpiredRagdoll : ExpiredRagdolls)
    {
        DisableRagdoll(ExpiredRagdoll);
    }
    
    if (ExpiredRagdolls.Num() > 0)
    {
        UE_LOG(LogTemp, Log, TEXT("Perf_RagdollOptimizer: Cleaned up %d expired ragdolls"), ExpiredRagdolls.Num());
    }
}

void UPerf_RagdollOptimizer::RemoveInvalidRagdolls()
{
    ActiveRagdolls.RemoveAll([](USkeletalMeshComponent* Mesh)
    {
        return !IsValid(Mesh) || !IsValid(Mesh->GetOwner());
    });
}

void UPerf_RagdollOptimizer::ApplyLODToRagdoll(USkeletalMeshComponent* SkeletalMesh, int32 LODLevel)
{
    if (!SkeletalMesh)
    {
        return;
    }
    
    // Force LOD level
    SkeletalMesh->SetForcedLOD(LODLevel + 1); // UE5 uses 1-based LOD indexing
    
    // Adjust physics simulation based on LOD
    if (LODLevel >= 3)
    {
        // Disable physics for very distant ragdolls
        SkeletalMesh->SetSimulatePhysics(false);
    }
    else if (LODLevel >= 2)
    {
        // Reduce physics update rate for distant ragdolls
        SkeletalMesh->SetComponentTickInterval(0.2f);
    }
    else
    {
        // Full physics for close ragdolls
        SkeletalMesh->SetComponentTickInterval(0.0f);
    }
}

float UPerf_RagdollOptimizer::CalculateRagdollMemoryUsage() const
{
    float TotalMemoryMB = 0.0f;
    
    for (USkeletalMeshComponent* RagdollMesh : ActiveRagdolls)
    {
        if (RagdollMesh && RagdollMesh->GetSkeletalMeshAsset())
        {
            // Rough estimation: 1MB per ragdoll (simplified)
            TotalMemoryMB += 1.0f;
        }
    }
    
    return TotalMemoryMB;
}

void UPerf_RagdollOptimizer::LogPerformanceWarning(const FString& Warning) const
{
    UE_LOG(LogTemp, Warning, TEXT("Perf_RagdollOptimizer Performance Warning: %s"), *Warning);
    UE_LOG(LogTemp, Warning, TEXT("  - Active Ragdolls: %d/%d"), CurrentMetrics.ActiveRagdolls, MaxActiveRagdolls);
    UE_LOG(LogTemp, Warning, TEXT("  - Physics Time: %.2fms (Max: %.2fms)"), CurrentMetrics.PhysicsTimeMs, MaxPhysicsTimeMs);
    UE_LOG(LogTemp, Warning, TEXT("  - Memory Usage: %.2fMB (Max: %.2fMB)"), CurrentMetrics.RagdollMemoryMB, MaxRagdollMemoryMB);
}