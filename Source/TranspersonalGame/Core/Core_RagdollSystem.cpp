// Copyright Transpersonal Game Studio. All Rights Reserved.

#include "Core_RagdollSystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Components/SkeletalMeshComponent.h"
#include "PhysicsEngine/PhysicsAsset.h"
#include "Animation/AnimInstance.h"
#include "Engine/SkeletalMesh.h"
#include "PhysicsEngine/BodyInstance.h"

UCore_RagdollSystem::UCore_RagdollSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // 10Hz for performance
    
    // Initialize ragdoll properties
    MaxRagdollDistance = 5000.0f;
    MinImpactForce = 500.0f;
    DefaultRagdollMass = 80.0f;
    RagdollLinearDamping = 0.1f;
    RagdollAngularDamping = 0.1f;
    MaxSimultaneousRagdolls = 10;
    CurrentRagdollCount = 0;
    RagdollPerformanceBudget = 2.0f; // 2ms budget
}

void UCore_RagdollSystem::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize ragdoll system
    ActiveRagdolls.Empty();
    RagdollActivationTimes.Empty();
    CurrentRagdollCount = 0;
    
    UE_LOG(LogTemp, Log, TEXT("Core_RagdollSystem: Initialized with max distance %.1f, max ragdolls %d"), 
           MaxRagdollDistance, MaxSimultaneousRagdolls);
}

void UCore_RagdollSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Performance monitoring
    double StartTime = FPlatformTime::Seconds();
    
    // Update ragdoll LODs based on distance
    FVector PlayerLocation = GetPlayerLocation();
    
    for (int32 i = ActiveRagdolls.Num() - 1; i >= 0; i--)
    {
        USkeletalMeshComponent* RagdollMesh = ActiveRagdolls[i];
        if (!IsValid(RagdollMesh) || !IsValid(RagdollMesh->GetOwner()))
        {
            ActiveRagdolls.RemoveAt(i);
            CurrentRagdollCount--;
            continue;
        }
        
        float Distance = FVector::Dist(PlayerLocation, RagdollMesh->GetOwner()->GetActorLocation());
        UpdateRagdollLOD(RagdollMesh, Distance);
        
        // Deactivate distant ragdolls
        if (Distance > MaxRagdollDistance)
        {
            DeactivateRagdoll(RagdollMesh, 0.5f);
        }
    }
    
    // Cleanup inactive ragdolls
    CleanupInactiveRagdolls();
    
    // Performance check
    double EndTime = FPlatformTime::Seconds();
    float ExecutionTime = (EndTime - StartTime) * 1000.0f; // Convert to ms
    
    if (ExecutionTime > RagdollPerformanceBudget)
    {
        UE_LOG(LogTemp, Warning, TEXT("Core_RagdollSystem: Performance budget exceeded: %.2fms > %.2fms"), 
               ExecutionTime, RagdollPerformanceBudget);
    }
}

void UCore_RagdollSystem::ActivateRagdoll(USkeletalMeshComponent* TargetMesh, bool bPreserveVelocity)
{
    if (!IsValid(TargetMesh) || !IsValid(TargetMesh->GetOwner()))
    {
        UE_LOG(LogTemp, Warning, TEXT("Core_RagdollSystem: Invalid target mesh for ragdoll activation"));
        return;
    }
    
    // Check performance limits
    if (CurrentRagdollCount >= MaxSimultaneousRagdolls)
    {
        UE_LOG(LogTemp, Warning, TEXT("Core_RagdollSystem: Max ragdoll limit reached (%d), skipping activation"), 
               MaxSimultaneousRagdolls);
        return;
    }
    
    // Check distance
    if (!ShouldActivateRagdoll(TargetMesh->GetOwner()))
    {
        return;
    }
    
    // Store current velocity if preserving
    FVector CurrentVelocity = FVector::ZeroVector;
    if (bPreserveVelocity && TargetMesh->GetOwner()->GetRootComponent())
    {
        if (UPrimitiveComponent* RootPrimitive = Cast<UPrimitiveComponent>(TargetMesh->GetOwner()->GetRootComponent()))
        {
            CurrentVelocity = RootPrimitive->GetPhysicsLinearVelocity();
        }
    }
    
    // Activate ragdoll physics
    InternalActivateRagdoll(TargetMesh);
    
    // Apply preserved velocity
    if (bPreserveVelocity && !CurrentVelocity.IsNearlyZero())
    {
        TargetMesh->SetPhysicsLinearVelocity(CurrentVelocity);
    }
    
    // Configure default physics properties
    ConfigureRagdollPhysics(TargetMesh, DefaultRagdollMass, RagdollLinearDamping, RagdollAngularDamping);
    
    // Track active ragdoll
    ActiveRagdolls.AddUnique(TargetMesh);
    RagdollActivationTimes.Add(TargetMesh, GetWorld()->GetTimeSeconds());
    CurrentRagdollCount++;
    
    UE_LOG(LogTemp, Log, TEXT("Core_RagdollSystem: Activated ragdoll for %s (Total: %d)"), 
           *TargetMesh->GetOwner()->GetName(), CurrentRagdollCount);
}

void UCore_RagdollSystem::DeactivateRagdoll(USkeletalMeshComponent* TargetMesh, float BlendTime)
{
    if (!IsValid(TargetMesh))
    {
        return;
    }
    
    // Deactivate ragdoll physics
    InternalDeactivateRagdoll(TargetMesh);
    
    // Remove from tracking
    ActiveRagdolls.Remove(TargetMesh);
    RagdollActivationTimes.Remove(TargetMesh);
    CurrentRagdollCount = FMath::Max(0, CurrentRagdollCount - 1);
    
    UE_LOG(LogTemp, Log, TEXT("Core_RagdollSystem: Deactivated ragdoll for %s (Total: %d)"), 
           *TargetMesh->GetOwner()->GetName(), CurrentRagdollCount);
}

void UCore_RagdollSystem::ApplyImpactForce(USkeletalMeshComponent* TargetMesh, FName BoneName, FVector Force, FVector Location)
{
    if (!IsValid(TargetMesh))
    {
        return;
    }
    
    // Check if force is significant enough
    if (Force.Size() < MinImpactForce)
    {
        return;
    }
    
    // Activate ragdoll if not already active
    if (!ActiveRagdolls.Contains(TargetMesh))
    {
        ActivateRagdoll(TargetMesh, true);
    }
    
    // Apply force to specific bone
    if (BoneName != NAME_None)
    {
        TargetMesh->AddImpulseAtLocation(Force, Location, BoneName);
    }
    else
    {
        TargetMesh->AddImpulseAtLocation(Force, Location);
    }
    
    UE_LOG(LogTemp, Log, TEXT("Core_RagdollSystem: Applied impact force %.1f to bone %s"), 
           Force.Size(), *BoneName.ToString());
}

void UCore_RagdollSystem::ConfigureRagdollPhysics(USkeletalMeshComponent* TargetMesh, float Mass, float LinearDamping, float AngularDamping)
{
    if (!IsValid(TargetMesh))
    {
        return;
    }
    
    // Configure physics properties for all bodies
    TArray<FName> BoneNames;
    TargetMesh->GetBoneNames(BoneNames);
    
    for (const FName& BoneName : BoneNames)
    {
        FBodyInstance* BodyInstance = TargetMesh->GetBodyInstance(BoneName);
        if (BodyInstance && BodyInstance->IsValidBodyInstance())
        {
            BodyInstance->SetMassOverride(Mass / BoneNames.Num()); // Distribute mass
            BodyInstance->LinearDamping = LinearDamping;
            BodyInstance->AngularDamping = AngularDamping;
            BodyInstance->UpdateMassProperties();
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Core_RagdollSystem: Configured physics - Mass: %.1f, LinearDamp: %.2f, AngularDamp: %.2f"), 
           Mass, LinearDamping, AngularDamping);
}

void UCore_RagdollSystem::SetBoneConstraints(USkeletalMeshComponent* TargetMesh, FName BoneName, bool bLockPosition, bool bLockRotation)
{
    if (!IsValid(TargetMesh))
    {
        return;
    }
    
    FBodyInstance* BodyInstance = TargetMesh->GetBodyInstance(BoneName);
    if (BodyInstance && BodyInstance->IsValidBodyInstance())
    {
        if (bLockPosition)
        {
            BodyInstance->SetDOFLock(EDOFMode::SixDOF);
        }
        
        if (bLockRotation)
        {
            BodyInstance->SetDOFLock(EDOFMode::None);
        }
    }
}

bool UCore_RagdollSystem::ShouldActivateRagdoll(AActor* TargetActor) const
{
    if (!IsValid(TargetActor))
    {
        return false;
    }
    
    FVector PlayerLocation = GetPlayerLocation();
    float Distance = FVector::Dist(PlayerLocation, TargetActor->GetActorLocation());
    
    return Distance <= MaxRagdollDistance;
}

void UCore_RagdollSystem::UpdateRagdollLOD(USkeletalMeshComponent* TargetMesh, float DistanceToPlayer)
{
    if (!IsValid(TargetMesh))
    {
        return;
    }
    
    // Adjust physics simulation quality based on distance
    if (DistanceToPlayer > MaxRagdollDistance * 0.8f)
    {
        // Far - reduce quality
        TargetMesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    }
    else if (DistanceToPlayer > MaxRagdollDistance * 0.5f)
    {
        // Medium - normal quality
        TargetMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    }
    else
    {
        // Close - full quality
        TargetMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    }
}

void UCore_RagdollSystem::InternalActivateRagdoll(USkeletalMeshComponent* TargetMesh)
{
    if (!IsValid(TargetMesh))
    {
        return;
    }
    
    // Set to ragdoll physics
    TargetMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    TargetMesh->SetSimulatePhysics(true);
    TargetMesh->WakeAllRigidBodies();
    TargetMesh->bBlendPhysics = true;
}

void UCore_RagdollSystem::InternalDeactivateRagdoll(USkeletalMeshComponent* TargetMesh)
{
    if (!IsValid(TargetMesh))
    {
        return;
    }
    
    // Return to animation
    TargetMesh->SetSimulatePhysics(false);
    TargetMesh->PutAllRigidBodiesToSleep();
    TargetMesh->bBlendPhysics = false;
    TargetMesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
}

void UCore_RagdollSystem::CleanupInactiveRagdolls()
{
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    for (int32 i = ActiveRagdolls.Num() - 1; i >= 0; i--)
    {
        USkeletalMeshComponent* RagdollMesh = ActiveRagdolls[i];
        
        if (!IsValid(RagdollMesh) || !IsValid(RagdollMesh->GetOwner()))
        {
            ActiveRagdolls.RemoveAt(i);
            CurrentRagdollCount--;
            continue;
        }
        
        // Check if ragdoll has been inactive for too long
        float* ActivationTime = RagdollActivationTimes.Find(RagdollMesh);
        if (ActivationTime && (CurrentTime - *ActivationTime) > 30.0f) // 30 second timeout
        {
            DeactivateRagdoll(RagdollMesh, 1.0f);
        }
    }
}

FVector UCore_RagdollSystem::GetPlayerLocation() const
{
    if (UWorld* World = GetWorld())
    {
        if (APlayerController* PC = World->GetFirstPlayerController())
        {
            if (APawn* PlayerPawn = PC->GetPawn())
            {
                return PlayerPawn->GetActorLocation();
            }
        }
    }
    
    return FVector::ZeroVector;
}