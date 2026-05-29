#include "Perf_PhysicsPerformanceManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PrimitiveComponent.h"
#include "GameFramework/Character.h"
#include "TimerManager.h"
#include "HAL/PlatformFilemanager.h"

UPerf_PhysicsPerformanceManager::UPerf_PhysicsPerformanceManager()
    : CurrentQuality(EPerf_PhysicsQuality::High)
    , TargetPhysicsFrameTime(16.67f) // 60fps target
    , MaxActiveRagdolls(10)
    , PhysicsLODDistance(5000.0f)
    , bEnablePhysicsOptimization(true)
{
}

void UPerf_PhysicsPerformanceManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Log, TEXT("Physics Performance Manager initialized"));
    
    // Set initial physics quality based on platform
    #if PLATFORM_DESKTOP
        SetPhysicsQuality(EPerf_PhysicsQuality::High);
    #else
        SetPhysicsQuality(EPerf_PhysicsQuality::Medium);
    #endif
    
    // Start performance monitoring timer
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(
            PerformanceUpdateTimer,
            this,
            &UPerf_PhysicsPerformanceManager::UpdatePhysicsPerformanceMetrics,
            1.0f, // Update every second
            true
        );
    }
}

void UPerf_PhysicsPerformanceManager::Deinitialize()
{
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(PerformanceUpdateTimer);
    }
    
    TrackedPhysicsActors.Empty();
    Super::Deinitialize();
}

void UPerf_PhysicsPerformanceManager::UpdatePhysicsPerformanceMetrics()
{
    if (!bEnablePhysicsOptimization)
        return;
        
    UWorld* World = GetWorld();
    if (!World)
        return;
    
    // Reset metrics
    CurrentMetrics = FPerf_PhysicsPerformanceMetrics();
    
    // Count active physics objects
    int32 RigidBodyCount = 0;
    int32 RagdollCount = 0;
    int32 ConstraintCount = 0;
    
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (!Actor || !IsValid(Actor))
            continue;
            
        // Check for physics components
        TArray<UPrimitiveComponent*> PrimitiveComponents;
        Actor->GetComponents<UPrimitiveComponent>(PrimitiveComponents);
        
        for (UPrimitiveComponent* PrimComp : PrimitiveComponents)
        {
            if (PrimComp && PrimComp->IsSimulatingPhysics())
            {
                RigidBodyCount++;
                
                // Check if it's a ragdoll (skeletal mesh with physics)
                if (USkeletalMeshComponent* SkelMesh = Cast<USkeletalMeshComponent>(PrimComp))
                {
                    if (SkelMesh->IsSimulatingPhysics())
                    {
                        RagdollCount++;
                    }
                }
            }
        }
    }
    
    // Update metrics
    CurrentMetrics.ActiveRigidBodies = RigidBodyCount;
    CurrentMetrics.ActiveRagdolls = RagdollCount;
    CurrentMetrics.PhysicsConstraints = ConstraintCount;
    CurrentMetrics.PhysicsFrameTime = FApp::GetDeltaTime() * 1000.0f; // Convert to ms
    
    // Monitor memory usage (simplified)
    CurrentMetrics.PhysicsMemoryUsage = static_cast<float>(RigidBodyCount * 1024); // Rough estimate
    
    // Auto-optimize if performance is poor
    if (CurrentMetrics.PhysicsFrameTime > TargetPhysicsFrameTime * 1.2f)
    {
        OptimizePhysicsTickRate();
        CullDistantPhysicsObjects();
    }
    
    // Log performance data
    UE_LOG(LogTemp, VeryVerbose, TEXT("Physics Performance: %d rigid bodies, %d ragdolls, %.2fms frame time"),
        CurrentMetrics.ActiveRigidBodies, CurrentMetrics.ActiveRagdolls, CurrentMetrics.PhysicsFrameTime);
}

void UPerf_PhysicsPerformanceManager::SetPhysicsQuality(EPerf_PhysicsQuality Quality)
{
    CurrentQuality = Quality;
    ApplyPhysicsQualitySettings();
    
    UE_LOG(LogTemp, Log, TEXT("Physics quality set to: %d"), static_cast<int32>(Quality));
}

void UPerf_PhysicsPerformanceManager::OptimizeRagdollPerformance()
{
    UWorld* World = GetWorld();
    if (!World)
        return;
        
    TArray<AActor*> RagdollActors;
    
    // Find all ragdoll actors
    for (TActorIterator<ACharacter> CharacterItr(World); CharacterItr; ++CharacterItr)
    {
        ACharacter* Character = *CharacterItr;
        if (!Character || !IsValid(Character))
            continue;
            
        USkeletalMeshComponent* SkelMesh = Character->GetMesh();
        if (SkelMesh && SkelMesh->IsSimulatingPhysics())
        {
            RagdollActors.Add(Character);
        }
    }
    
    // Disable excess ragdolls (keep closest to player)
    if (RagdollActors.Num() > MaxActiveRagdolls)
    {
        // Sort by distance to player (simplified - using world origin)
        RagdollActors.Sort([](const AActor& A, const AActor& B) {
            return A.GetActorLocation().SizeSquared() < B.GetActorLocation().SizeSquared();
        });
        
        // Disable distant ragdolls
        for (int32 i = MaxActiveRagdolls; i < RagdollActors.Num(); i++)
        {
            if (ACharacter* Character = Cast<ACharacter>(RagdollActors[i]))
            {
                if (USkeletalMeshComponent* SkelMesh = Character->GetMesh())
                {
                    SkelMesh->SetSimulatePhysics(false);
                    SkelMesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
                }
            }
        }
    }
}

void UPerf_PhysicsPerformanceManager::SetMaxActiveRagdolls(int32 MaxRagdolls)
{
    MaxActiveRagdolls = FMath::Max(1, MaxRagdolls);
    OptimizeRagdollPerformance();
}

void UPerf_PhysicsPerformanceManager::UpdatePhysicsLOD()
{
    CullDistantPhysicsObjects();
}

void UPerf_PhysicsPerformanceManager::SetPhysicsLODDistance(float Distance)
{
    PhysicsLODDistance = FMath::Max(1000.0f, Distance);
    UpdatePhysicsLOD();
}

bool UPerf_PhysicsPerformanceManager::IsPhysicsPerformanceTarget() const
{
    return CurrentMetrics.PhysicsFrameTime <= TargetPhysicsFrameTime;
}

void UPerf_PhysicsPerformanceManager::SetTargetPhysicsFrameTime(float TargetTime)
{
    TargetPhysicsFrameTime = FMath::Max(8.33f, TargetTime); // Minimum 120fps target
}

void UPerf_PhysicsPerformanceManager::ApplyPhysicsQualitySettings()
{
    UPhysicsSettings* PhysicsSettings = UPhysicsSettings::Get();
    if (!PhysicsSettings)
        return;
        
    switch (CurrentQuality)
    {
        case EPerf_PhysicsQuality::Ultra:
            MaxActiveRagdolls = 20;
            PhysicsLODDistance = 10000.0f;
            break;
            
        case EPerf_PhysicsQuality::High:
            MaxActiveRagdolls = 15;
            PhysicsLODDistance = 7500.0f;
            break;
            
        case EPerf_PhysicsQuality::Medium:
            MaxActiveRagdolls = 10;
            PhysicsLODDistance = 5000.0f;
            break;
            
        case EPerf_PhysicsQuality::Low:
            MaxActiveRagdolls = 5;
            PhysicsLODDistance = 3000.0f;
            break;
            
        case EPerf_PhysicsQuality::Minimal:
            MaxActiveRagdolls = 2;
            PhysicsLODDistance = 1500.0f;
            break;
    }
}

void UPerf_PhysicsPerformanceManager::CullDistantPhysicsObjects()
{
    UWorld* World = GetWorld();
    if (!World)
        return;
        
    // Get player location (simplified - using world origin for now)
    FVector PlayerLocation = FVector::ZeroVector;
    
    // Find player pawn
    if (APawn* PlayerPawn = World->GetFirstPlayerController()->GetPawn())
    {
        PlayerLocation = PlayerPawn->GetActorLocation();
    }
    
    // Disable physics for distant objects
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (!Actor || !IsValid(Actor))
            continue;
            
        float Distance = FVector::Dist(Actor->GetActorLocation(), PlayerLocation);
        
        if (Distance > PhysicsLODDistance)
        {
            TArray<UPrimitiveComponent*> PrimitiveComponents;
            Actor->GetComponents<UPrimitiveComponent>(PrimitiveComponents);
            
            for (UPrimitiveComponent* PrimComp : PrimitiveComponents)
            {
                if (PrimComp && PrimComp->IsSimulatingPhysics())
                {
                    // Temporarily disable physics for distant objects
                    PrimComp->SetSimulatePhysics(false);
                    
                    // Track for re-enabling when closer
                    TrackedPhysicsActors.AddUnique(Actor);
                }
            }
        }
        else
        {
            // Re-enable physics for objects that came back into range
            if (TrackedPhysicsActors.Contains(Actor))
            {
                TArray<UPrimitiveComponent*> PrimitiveComponents;
                Actor->GetComponents<UPrimitiveComponent>(PrimitiveComponents);
                
                for (UPrimitiveComponent* PrimComp : PrimitiveComponents)
                {
                    if (PrimComp)
                    {
                        PrimComp->SetSimulatePhysics(true);
                    }
                }
                
                TrackedPhysicsActors.Remove(Actor);
            }
        }
    }
}

void UPerf_PhysicsPerformanceManager::OptimizePhysicsTickRate()
{
    // Reduce physics tick rate when performance is poor
    if (CurrentMetrics.PhysicsFrameTime > TargetPhysicsFrameTime * 1.5f)
    {
        UE_LOG(LogTemp, Warning, TEXT("Physics performance poor, optimizing tick rate"));
        
        // This would typically involve adjusting physics substep settings
        // For now, just log the optimization attempt
    }
}

void UPerf_PhysicsPerformanceManager::MonitorPhysicsMemory()
{
    // Monitor physics memory usage
    // This is a simplified implementation
    float EstimatedMemory = CurrentMetrics.ActiveRigidBodies * 1024.0f; // Rough estimate
    CurrentMetrics.PhysicsMemoryUsage = EstimatedMemory;
    
    if (EstimatedMemory > 100 * 1024 * 1024) // 100MB threshold
    {
        UE_LOG(LogTemp, Warning, TEXT("High physics memory usage: %.2f MB"), EstimatedMemory / (1024.0f * 1024.0f));
    }
}