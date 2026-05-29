#include "Perf_PhysicsOptimizer.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "Components/SkeletalMeshComponent.h"
#include "PhysicsEngine/PhysicsAsset.h"
#include "PhysicsEngine/BodySetup.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"

UPerf_PhysicsOptimizer::UPerf_PhysicsOptimizer()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f;
    
    CurrentPhysicsLOD = EPerf_PhysicsLOD::Standard;
    ActiveRagdollCount = 0;
    LastOptimizationTime = 0.0f;
}

void UPerf_PhysicsOptimizer::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize physics settings
    PhysicsSettings.FullPhysicsDistance = 1000.0f;
    PhysicsSettings.StandardPhysicsDistance = 2500.0f;
    PhysicsSettings.SimplifiedPhysicsDistance = 5000.0f;
    PhysicsSettings.MaxActiveRagdolls = 8;
    PhysicsSettings.PhysicsUpdateRate = 60.0f;
    PhysicsSettings.bEnableDistanceCulling = true;
    
    UE_LOG(LogTemp, Warning, TEXT("Physics Optimizer initialized"));
}

void UPerf_PhysicsOptimizer::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    LastUpdateTime += DeltaTime;
    if (LastUpdateTime >= OptimizationUpdateInterval)
    {
        UpdateRagdollTracking();
        CleanupInvalidRagdolls();
        LimitActiveRagdolls();
        
        if (PhysicsSettings.bEnableDistanceCulling)
        {
            OptimizeAllPhysicsActors();
        }
        
        LastUpdateTime = 0.0f;
    }
}

void UPerf_PhysicsOptimizer::OptimizePhysicsForDistance(float PlayerDistance)
{
    EPerf_PhysicsLOD NewLOD = EPerf_PhysicsLOD::Disabled;
    
    if (PlayerDistance <= PhysicsSettings.FullPhysicsDistance)
    {
        NewLOD = EPerf_PhysicsLOD::Full;
    }
    else if (PlayerDistance <= PhysicsSettings.StandardPhysicsDistance)
    {
        NewLOD = EPerf_PhysicsLOD::Standard;
    }
    else if (PlayerDistance <= PhysicsSettings.SimplifiedPhysicsDistance)
    {
        NewLOD = EPerf_PhysicsLOD::Simplified;
    }
    
    if (NewLOD != CurrentPhysicsLOD)
    {
        SetPhysicsLOD(NewLOD);
    }
}

void UPerf_PhysicsOptimizer::SetPhysicsLOD(EPerf_PhysicsLOD NewLOD)
{
    CurrentPhysicsLOD = NewLOD;
    
    // Apply LOD to owner's skeletal mesh
    if (AActor* Owner = GetOwner())
    {
        if (USkeletalMeshComponent* SkeletalMesh = Owner->FindComponentByClass<USkeletalMeshComponent>())
        {
            ApplyPhysicsLOD(SkeletalMesh, NewLOD);
        }
    }
}

void UPerf_PhysicsOptimizer::OptimizeRagdollPhysics(USkeletalMeshComponent* SkeletalMesh, float Distance)
{
    if (!SkeletalMesh || !IsValid(SkeletalMesh))
    {
        return;
    }
    
    EPerf_PhysicsLOD LOD = EPerf_PhysicsLOD::Disabled;
    
    if (Distance <= PhysicsSettings.FullPhysicsDistance)
    {
        LOD = EPerf_PhysicsLOD::Full;
    }
    else if (Distance <= PhysicsSettings.StandardPhysicsDistance)
    {
        LOD = EPerf_PhysicsLOD::Standard;
    }
    else if (Distance <= PhysicsSettings.SimplifiedPhysicsDistance)
    {
        LOD = EPerf_PhysicsLOD::Simplified;
    }
    
    ApplyPhysicsLOD(SkeletalMesh, LOD);
}

void UPerf_PhysicsOptimizer::EnableRagdollOptimization(bool bEnable)
{
    PhysicsSettings.bEnableDistanceCulling = bEnable;
    
    if (!bEnable)
    {
        // Reset all tracked ragdolls to full physics
        for (auto& RagdollPtr : TrackedRagdolls)
        {
            if (RagdollPtr.IsValid())
            {
                ApplyPhysicsLOD(RagdollPtr.Get(), EPerf_PhysicsLOD::Full);
            }
        }
    }
}

float UPerf_PhysicsOptimizer::GetPhysicsPerformanceMetric() const
{
    // Simple performance metric based on active ragdolls and physics complexity
    float BaseMetric = 1.0f;
    float RagdollPenalty = ActiveRagdollCount * 0.1f;
    float LODBonus = 0.0f;
    
    switch (CurrentPhysicsLOD)
    {
        case EPerf_PhysicsLOD::Full:
            LODBonus = 0.0f;
            break;
        case EPerf_PhysicsLOD::Standard:
            LODBonus = 0.2f;
            break;
        case EPerf_PhysicsLOD::Simplified:
            LODBonus = 0.4f;
            break;
        case EPerf_PhysicsLOD::Disabled:
            LODBonus = 0.8f;
            break;
    }
    
    return FMath::Clamp(BaseMetric - RagdollPenalty + LODBonus, 0.0f, 1.0f);
}

int32 UPerf_PhysicsOptimizer::GetActiveRagdollCount() const
{
    return ActiveRagdollCount;
}

void UPerf_PhysicsOptimizer::OptimizeCollisionComplexity(float Distance)
{
    if (AActor* Owner = GetOwner())
    {
        if (USkeletalMeshComponent* SkeletalMesh = Owner->FindComponentByClass<USkeletalMeshComponent>())
        {
            ECollisionEnabled::Type CollisionType = ECollisionEnabled::QueryAndPhysics;
            
            if (Distance > PhysicsSettings.SimplifiedPhysicsDistance)
            {
                CollisionType = ECollisionEnabled::NoCollision;
            }
            else if (Distance > PhysicsSettings.StandardPhysicsDistance)
            {
                CollisionType = ECollisionEnabled::QueryOnly;
            }
            
            SkeletalMesh->SetCollisionEnabled(CollisionType);
        }
    }
}

void UPerf_PhysicsOptimizer::SetCollisionLOD(int32 LODLevel)
{
    if (AActor* Owner = GetOwner())
    {
        if (USkeletalMeshComponent* SkeletalMesh = Owner->FindComponentByClass<USkeletalMeshComponent>())
        {
            // Force specific LOD level for collision
            SkeletalMesh->SetForcedLOD(FMath::Clamp(LODLevel, 0, 3));
        }
    }
}

void UPerf_PhysicsOptimizer::OptimizeAllPhysicsActors()
{
    if (UWorld* World = GetWorld())
    {
        APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(World, 0);
        if (!PlayerPawn)
        {
            return;
        }
        
        FVector PlayerLocation = PlayerPawn->GetActorLocation();
        
        for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
        {
            AActor* Actor = *ActorItr;
            if (!Actor || Actor == PlayerPawn)
            {
                continue;
            }
            
            if (USkeletalMeshComponent* SkeletalMesh = Actor->FindComponentByClass<USkeletalMeshComponent>())
            {
                float Distance = FVector::Dist(PlayerLocation, Actor->GetActorLocation());
                OptimizeRagdollPhysics(SkeletalMesh, Distance);
            }
        }
    }
}

void UPerf_PhysicsOptimizer::UpdatePhysicsLODForAllActors()
{
    OptimizeAllPhysicsActors();
}

void UPerf_PhysicsOptimizer::SetPhysicsSettings(const FPerf_PhysicsSettings& NewSettings)
{
    PhysicsSettings = NewSettings;
    
    // Apply new settings immediately
    OptimizeAllPhysicsActors();
}

void UPerf_PhysicsOptimizer::UpdateRagdollTracking()
{
    if (UWorld* World = GetWorld())
    {
        TrackedRagdolls.Empty();
        ActiveRagdollCount = 0;
        
        for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
        {
            AActor* Actor = *ActorItr;
            if (!Actor)
            {
                continue;
            }
            
            if (USkeletalMeshComponent* SkeletalMesh = Actor->FindComponentByClass<USkeletalMeshComponent>())
            {
                if (SkeletalMesh->IsSimulatingPhysics())
                {
                    TrackedRagdolls.Add(SkeletalMesh);
                    ActiveRagdollCount++;
                }
            }
        }
    }
}

void UPerf_PhysicsOptimizer::ApplyPhysicsLOD(USkeletalMeshComponent* SkeletalMesh, EPerf_PhysicsLOD LODLevel)
{
    if (!SkeletalMesh || !IsValid(SkeletalMesh))
    {
        return;
    }
    
    switch (LODLevel)
    {
        case EPerf_PhysicsLOD::Full:
            SkeletalMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
            SkeletalMesh->SetForcedLOD(-1); // Use automatic LOD
            break;
            
        case EPerf_PhysicsLOD::Standard:
            SkeletalMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
            SkeletalMesh->SetForcedLOD(1);
            break;
            
        case EPerf_PhysicsLOD::Simplified:
            SkeletalMesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
            SkeletalMesh->SetForcedLOD(2);
            break;
            
        case EPerf_PhysicsLOD::Disabled:
            SkeletalMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
            SkeletalMesh->SetForcedLOD(3);
            break;
    }
}

float UPerf_PhysicsOptimizer::CalculateDistanceToPlayer(AActor* Actor)
{
    if (!Actor)
    {
        return 99999.0f;
    }
    
    if (UWorld* World = GetWorld())
    {
        if (APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(World, 0))
        {
            return FVector::Dist(PlayerPawn->GetActorLocation(), Actor->GetActorLocation());
        }
    }
    
    return 99999.0f;
}

void UPerf_PhysicsOptimizer::CleanupInvalidRagdolls()
{
    TrackedRagdolls.RemoveAll([](const TWeakObjectPtr<USkeletalMeshComponent>& Ptr)
    {
        return !Ptr.IsValid();
    });
    
    ActiveRagdollCount = TrackedRagdolls.Num();
}

void UPerf_PhysicsOptimizer::LimitActiveRagdolls()
{
    if (ActiveRagdollCount <= PhysicsSettings.MaxActiveRagdolls)
    {
        return;
    }
    
    // Sort ragdolls by distance to player
    if (UWorld* World = GetWorld())
    {
        if (APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(World, 0))
        {
            FVector PlayerLocation = PlayerPawn->GetActorLocation();
            
            TrackedRagdolls.Sort([PlayerLocation](const TWeakObjectPtr<USkeletalMeshComponent>& A, const TWeakObjectPtr<USkeletalMeshComponent>& B)
            {
                if (!A.IsValid() || !B.IsValid())
                {
                    return false;
                }
                
                float DistA = FVector::Dist(PlayerLocation, A->GetOwner()->GetActorLocation());
                float DistB = FVector::Dist(PlayerLocation, B->GetOwner()->GetActorLocation());
                return DistA < DistB;
            });
            
            // Disable physics for furthest ragdolls
            for (int32 i = PhysicsSettings.MaxActiveRagdolls; i < TrackedRagdolls.Num(); i++)
            {
                if (TrackedRagdolls[i].IsValid())
                {
                    TrackedRagdolls[i]->SetSimulatePhysics(false);
                }
            }
        }
    }
}