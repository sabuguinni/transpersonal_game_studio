#include "Perf_PhysicsPerformanceManager.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "GameFramework/Actor.h"
#include "Components/PrimitiveComponent.h"
#include "Components/StaticMeshComponent.h"
#include "PhysicsEngine/BodyInstance.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/StaticMeshActor.h"

UPerf_PhysicsPerformanceManager::UPerf_PhysicsPerformanceManager()
{
    CurrentQuality = EPerf_PhysicsQuality::Medium;
    LastUpdateTime = 0.0f;
    bPhysicsOptimizationEnabled = true;
}

void UPerf_PhysicsPerformanceManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Log, TEXT("Physics Performance Manager Initialized"));
    
    // Set default physics settings
    PhysicsSettings = FPerf_PhysicsSettings();
    CurrentMetrics = FPerf_PhysicsMetrics();
    
    // Start performance monitoring
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().SetTimer(
            FTimerHandle(),
            this,
            &UPerf_PhysicsPerformanceManager::UpdatePhysicsPerformance,
            1.0f, // Update every second
            true
        );
    }
}

void UPerf_PhysicsPerformanceManager::Deinitialize()
{
    TrackedPhysicsActors.Empty();
    Super::Deinitialize();
}

bool UPerf_PhysicsPerformanceManager::ShouldCreateSubsystem(UObject* Outer) const
{
    return true;
}

void UPerf_PhysicsPerformanceManager::UpdatePhysicsPerformance()
{
    if (!GetWorld() || !bPhysicsOptimizationEnabled)
    {
        return;
    }

    CollectPhysicsMetrics();
    
    // Auto-optimize if performance is poor
    if (!IsPhysicsPerformanceGood())
    {
        OptimizePhysicsActors();
    }
    
    LastUpdateTime = GetWorld()->GetTimeSeconds();
}

FPerf_PhysicsMetrics UPerf_PhysicsPerformanceManager::GetPhysicsMetrics() const
{
    return CurrentMetrics;
}

void UPerf_PhysicsPerformanceManager::SetPhysicsQuality(EPerf_PhysicsQuality Quality)
{
    CurrentQuality = Quality;
    ApplyPhysicsQualitySettings();
    
    UE_LOG(LogTemp, Log, TEXT("Physics Quality set to: %d"), (int32)Quality);
}

void UPerf_PhysicsPerformanceManager::OptimizePhysicsActors()
{
    if (!GetWorld())
    {
        return;
    }

    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), AllActors);
    
    int32 OptimizedCount = 0;
    
    for (AActor* Actor : AllActors)
    {
        if (Actor && Actor->GetRootComponent())
        {
            UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(Actor->GetRootComponent());
            if (PrimComp && PrimComp->IsSimulatingPhysics())
            {
                OptimizePhysicsActor(Actor);
                OptimizedCount++;
            }
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Optimized %d physics actors"), OptimizedCount);
}

void UPerf_PhysicsPerformanceManager::EnablePhysicsLOD(bool bEnable)
{
    PhysicsSettings.bEnablePhysicsLOD = bEnable;
    
    if (bEnable)
    {
        UpdatePhysicsActorLOD();
    }
}

void UPerf_PhysicsPerformanceManager::SetMaxActivePhysicsActors(int32 MaxActors)
{
    PhysicsSettings.MaxActivePhysicsActors = FMath::Max(MaxActors, 10);
    UE_LOG(LogTemp, Log, TEXT("Max active physics actors set to: %d"), PhysicsSettings.MaxActivePhysicsActors);
}

void UPerf_PhysicsPerformanceManager::AnalyzePhysicsPerformance()
{
    if (!GetWorld())
    {
        UE_LOG(LogTemp, Warning, TEXT("No world available for physics analysis"));
        return;
    }

    CollectPhysicsMetrics();
    
    UE_LOG(LogTemp, Log, TEXT("=== PHYSICS PERFORMANCE ANALYSIS ==="));
    UE_LOG(LogTemp, Log, TEXT("Active Physics Actors: %d"), CurrentMetrics.ActivePhysicsActors);
    UE_LOG(LogTemp, Log, TEXT("Sleeping Physics Actors: %d"), CurrentMetrics.SleepingPhysicsActors);
    UE_LOG(LogTemp, Log, TEXT("Physics Step Time: %.3f ms"), CurrentMetrics.PhysicsStepTime);
    UE_LOG(LogTemp, Log, TEXT("Collision Query Time: %.3f ms"), CurrentMetrics.CollisionQueryTime);
    UE_LOG(LogTemp, Log, TEXT("Total Collision Queries: %d"), CurrentMetrics.TotalCollisionQueries);
    UE_LOG(LogTemp, Log, TEXT("Physics Load: %.1f%%"), GetCurrentPhysicsLoad() * 100.0f);
    UE_LOG(LogTemp, Log, TEXT("Performance Good: %s"), IsPhysicsPerformanceGood() ? TEXT("YES") : TEXT("NO"));
}

void UPerf_PhysicsPerformanceManager::CullDistantPhysicsActors(float CullDistance)
{
    if (!GetWorld())
    {
        return;
    }

    APawn* PlayerPawn = GetWorld()->GetFirstPlayerController() ? GetWorld()->GetFirstPlayerController()->GetPawn() : nullptr;
    if (!PlayerPawn)
    {
        return;
    }

    FVector PlayerLocation = PlayerPawn->GetActorLocation();
    
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), AllActors);
    
    int32 CulledCount = 0;
    
    for (AActor* Actor : AllActors)
    {
        if (ShouldCullPhysicsActor(Actor, CullDistance))
        {
            UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(Actor->GetRootComponent());
            if (PrimComp && PrimComp->IsSimulatingPhysics())
            {
                PrimComp->SetSimulatePhysics(false);
                CulledCount++;
            }
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Culled %d distant physics actors"), CulledCount);
}

void UPerf_PhysicsPerformanceManager::PutPhysicsActorsToSleep(float SleepThreshold)
{
    if (!GetWorld())
    {
        return;
    }

    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), AllActors);
    
    int32 SleepCount = 0;
    
    for (AActor* Actor : AllActors)
    {
        if (Actor && Actor->GetRootComponent())
        {
            UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(Actor->GetRootComponent());
            if (PrimComp && PrimComp->IsSimulatingPhysics())
            {
                FVector Velocity = PrimComp->GetPhysicsLinearVelocity();
                if (Velocity.Size() < SleepThreshold)
                {
                    PrimComp->PutRigidBodyToSleep();
                    SleepCount++;
                }
            }
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Put %d physics actors to sleep"), SleepCount);
}

bool UPerf_PhysicsPerformanceManager::IsPhysicsPerformanceGood() const
{
    // Performance is good if:
    // - Active physics actors under limit
    // - Physics step time under threshold
    // - Collision query time reasonable
    
    bool bActiveActorsOK = CurrentMetrics.ActivePhysicsActors <= PhysicsSettings.MaxActivePhysicsActors;
    bool bStepTimeOK = CurrentMetrics.PhysicsStepTime < 8.0f; // Under 8ms
    bool bQueryTimeOK = CurrentMetrics.CollisionQueryTime < 2.0f; // Under 2ms
    
    return bActiveActorsOK && bStepTimeOK && bQueryTimeOK;
}

float UPerf_PhysicsPerformanceManager::GetCurrentPhysicsLoad() const
{
    // Calculate load based on active actors vs max allowed
    if (PhysicsSettings.MaxActivePhysicsActors <= 0)
    {
        return 0.0f;
    }
    
    float ActorLoad = (float)CurrentMetrics.ActivePhysicsActors / (float)PhysicsSettings.MaxActivePhysicsActors;
    float TimeLoad = CurrentMetrics.PhysicsStepTime / 16.67f; // Relative to 60fps budget
    
    return FMath::Max(ActorLoad, TimeLoad);
}

void UPerf_PhysicsPerformanceManager::CollectPhysicsMetrics()
{
    if (!GetWorld())
    {
        return;
    }

    CurrentMetrics = FPerf_PhysicsMetrics();
    
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), AllActors);
    
    for (AActor* Actor : AllActors)
    {
        if (Actor && Actor->GetRootComponent())
        {
            UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(Actor->GetRootComponent());
            if (PrimComp && PrimComp->IsSimulatingPhysics())
            {
                if (PrimComp->RigidBodyIsAwake())
                {
                    CurrentMetrics.ActivePhysicsActors++;
                }
                else
                {
                    CurrentMetrics.SleepingPhysicsActors++;
                }
            }
        }
    }
    
    // Estimate physics step time (simplified)
    CurrentMetrics.PhysicsStepTime = FMath::Max(1.0f, CurrentMetrics.ActivePhysicsActors * 0.01f);
    
    // Estimate collision query metrics
    CurrentMetrics.TotalCollisionQueries = CurrentMetrics.ActivePhysicsActors * 5; // Rough estimate
    CurrentMetrics.CollisionQueryTime = CurrentMetrics.TotalCollisionQueries * 0.001f;
}

void UPerf_PhysicsPerformanceManager::ApplyPhysicsQualitySettings()
{
    switch (CurrentQuality)
    {
        case EPerf_PhysicsQuality::Low:
            PhysicsSettings.MaxActivePhysicsActors = 100;
            PhysicsSettings.PhysicsTimeStep = 0.033333f; // 30fps
            PhysicsSettings.PhysicsLODDistance = 2000.0f;
            break;
            
        case EPerf_PhysicsQuality::Medium:
            PhysicsSettings.MaxActivePhysicsActors = 300;
            PhysicsSettings.PhysicsTimeStep = 0.016667f; // 60fps
            PhysicsSettings.PhysicsLODDistance = 5000.0f;
            break;
            
        case EPerf_PhysicsQuality::High:
            PhysicsSettings.MaxActivePhysicsActors = 500;
            PhysicsSettings.PhysicsTimeStep = 0.016667f; // 60fps
            PhysicsSettings.PhysicsLODDistance = 8000.0f;
            break;
            
        case EPerf_PhysicsQuality::Ultra:
            PhysicsSettings.MaxActivePhysicsActors = 1000;
            PhysicsSettings.PhysicsTimeStep = 0.008333f; // 120fps
            PhysicsSettings.PhysicsLODDistance = 15000.0f;
            break;
    }
}

void UPerf_PhysicsPerformanceManager::UpdatePhysicsActorLOD()
{
    if (!PhysicsSettings.bEnablePhysicsLOD || !GetWorld())
    {
        return;
    }

    APawn* PlayerPawn = GetWorld()->GetFirstPlayerController() ? GetWorld()->GetFirstPlayerController()->GetPawn() : nullptr;
    if (!PlayerPawn)
    {
        return;
    }

    FVector PlayerLocation = PlayerPawn->GetActorLocation();
    
    for (TWeakObjectPtr<AActor> WeakActor : TrackedPhysicsActors)
    {
        if (AActor* Actor = WeakActor.Get())
        {
            float Distance = FVector::Dist(Actor->GetActorLocation(), PlayerLocation);
            
            UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(Actor->GetRootComponent());
            if (PrimComp)
            {
                // Disable physics for distant objects
                bool bShouldSimulate = Distance < PhysicsSettings.PhysicsLODDistance;
                if (PrimComp->IsSimulatingPhysics() != bShouldSimulate)
                {
                    PrimComp->SetSimulatePhysics(bShouldSimulate);
                }
            }
        }
    }
}

void UPerf_PhysicsPerformanceManager::ManagePhysicsActorSleep()
{
    // Put slow-moving actors to sleep automatically
    PutPhysicsActorsToSleep(10.0f); // Sleep threshold: 10 units/second
}

bool UPerf_PhysicsPerformanceManager::ShouldCullPhysicsActor(AActor* Actor, float CullDistance) const
{
    if (!Actor || !GetWorld())
    {
        return false;
    }

    APawn* PlayerPawn = GetWorld()->GetFirstPlayerController() ? GetWorld()->GetFirstPlayerController()->GetPawn() : nullptr;
    if (!PlayerPawn)
    {
        return false;
    }

    float Distance = FVector::Dist(Actor->GetActorLocation(), PlayerPawn->GetActorLocation());
    return Distance > CullDistance;
}

void UPerf_PhysicsPerformanceManager::OptimizePhysicsActor(AActor* Actor)
{
    if (!Actor)
    {
        return;
    }

    UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(Actor->GetRootComponent());
    if (!PrimComp)
    {
        return;
    }

    // Optimize collision complexity for performance
    if (UStaticMeshComponent* MeshComp = Cast<UStaticMeshComponent>(PrimComp))
    {
        MeshComp->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
        MeshComp->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
        
        // Use simple collision for better performance
        MeshComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    }

    // Add to tracked actors if not already tracked
    TWeakObjectPtr<AActor> WeakActor(Actor);
    if (!TrackedPhysicsActors.Contains(WeakActor))
    {
        TrackedPhysicsActors.Add(WeakActor);
    }
}