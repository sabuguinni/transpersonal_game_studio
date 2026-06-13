#include "Core_RagdollSystem.h"
#include "GameFramework/Character.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "Engine/World.h"
#include "TimerManager.h"

UCore_RagdollSystem::UCore_RagdollSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = true;
    
    // Initialize critical bones for ragdoll
    CriticalBones.Add(TEXT("pelvis"));
    CriticalBones.Add(TEXT("spine_01"));
    CriticalBones.Add(TEXT("spine_02"));
    CriticalBones.Add(TEXT("spine_03"));
    CriticalBones.Add(TEXT("head"));
    CriticalBones.Add(TEXT("upperarm_l"));
    CriticalBones.Add(TEXT("upperarm_r"));
    CriticalBones.Add(TEXT("thigh_l"));
    CriticalBones.Add(TEXT("thigh_r"));
    
    MinImpactForceForRagdoll = 500.0f;
    RagdollPhysicsBlend = 1.0f;
    bAutoRecovery = true;
    
    SkeletalMeshComp = nullptr;
    AnimInstance = nullptr;
}

void UCore_RagdollSystem::BeginPlay()
{
    Super::BeginPlay();
    InitializeRagdollSystem();
}

void UCore_RagdollSystem::InitializeRagdollSystem()
{
    AActor* Owner = GetOwner();
    if (!Owner)
    {
        UE_LOG(LogTemp, Warning, TEXT("Core_RagdollSystem: No owner found"));
        return;
    }

    // Find skeletal mesh component
    SkeletalMeshComp = Owner->FindComponentByClass<USkeletalMeshComponent>();
    if (!SkeletalMeshComp)
    {
        UE_LOG(LogTemp, Warning, TEXT("Core_RagdollSystem: No skeletal mesh component found on %s"), *Owner->GetName());
        return;
    }

    AnimInstance = SkeletalMeshComp->GetAnimInstance();
    if (!AnimInstance)
    {
        UE_LOG(LogTemp, Warning, TEXT("Core_RagdollSystem: No anim instance found on %s"), *Owner->GetName());
        return;
    }

    // Ensure physics asset is available
    if (!SkeletalMeshComp->GetPhysicsAsset())
    {
        UE_LOG(LogTemp, Warning, TEXT("Core_RagdollSystem: No physics asset found on skeletal mesh"));
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("Core_RagdollSystem: Successfully initialized on %s"), *Owner->GetName());
}

void UCore_RagdollSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (RagdollState.bIsRagdollActive)
    {
        UpdateRagdollPhysics(DeltaTime);
        
        RagdollState.RagdollDuration += DeltaTime;
        
        // Auto-recovery check
        if (bAutoRecovery && ShouldAutoRecover())
        {
            DeactivateRagdoll();
        }
    }
}

void UCore_RagdollSystem::ActivateRagdoll(const FVector& ImpactForce, const FVector& ImpactLocation)
{
    if (!SkeletalMeshComp || RagdollState.bIsRagdollActive)
    {
        return;
    }

    // Check if impact force is sufficient
    if (ImpactForce.Size() < MinImpactForceForRagdoll)
    {
        UE_LOG(LogTemp, Log, TEXT("Core_RagdollSystem: Impact force %f below threshold %f"), 
               ImpactForce.Size(), MinImpactForceForRagdoll);
        return;
    }

    // Store impact data
    RagdollState.ImpactForce = ImpactForce;
    RagdollState.ImpactLocation = ImpactLocation;
    RagdollState.bIsRagdollActive = true;
    RagdollState.RagdollDuration = 0.0f;

    // Disable character movement if it's a character
    if (ACharacter* Character = Cast<ACharacter>(GetOwner()))
    {
        Character->GetCharacterMovement()->DisableMovement();
    }

    // Enable physics simulation
    SkeletalMeshComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    SkeletalMeshComp->SetSimulatePhysics(true);
    SkeletalMeshComp->WakeAllRigidBodies();
    
    // Apply impact force
    ApplyImpactForce();

    UE_LOG(LogTemp, Log, TEXT("Core_RagdollSystem: Ragdoll activated on %s with force %s"), 
           *GetOwner()->GetName(), *ImpactForce.ToString());
}

void UCore_RagdollSystem::DeactivateRagdoll()
{
    if (!SkeletalMeshComp || !RagdollState.bIsRagdollActive)
    {
        return;
    }

    RagdollState.bIsRagdollActive = false;

    // Disable physics simulation
    SkeletalMeshComp->SetSimulatePhysics(false);
    SkeletalMeshComp->PutAllRigidBodiesToSleep();

    // Re-enable character movement if it's a character
    if (ACharacter* Character = Cast<ACharacter>(GetOwner()))
    {
        Character->GetCharacterMovement()->SetDefaultMovementMode();
    }

    // Blend back to animation
    BlendToRecovery();

    UE_LOG(LogTemp, Log, TEXT("Core_RagdollSystem: Ragdoll deactivated on %s after %f seconds"), 
           *GetOwner()->GetName(), RagdollState.RagdollDuration);
}

void UCore_RagdollSystem::ForceRecovery()
{
    if (RagdollState.bIsRagdollActive)
    {
        DeactivateRagdoll();
    }
}

void UCore_RagdollSystem::UpdateRagdollPhysics(float DeltaTime)
{
    if (!SkeletalMeshComp)
    {
        return;
    }

    // Apply damping to prevent excessive spinning
    const float DampingFactor = 0.95f;
    
    for (const FName& BoneName : CriticalBones)
    {
        if (SkeletalMeshComp->GetBoneIndex(BoneName) != INDEX_NONE)
        {
            FVector CurrentVelocity = SkeletalMeshComp->GetPhysicsLinearVelocityAtLocation(
                SkeletalMeshComp->GetBoneLocation(BoneName));
            
            if (CurrentVelocity.Size() > 100.0f)
            {
                SkeletalMeshComp->SetPhysicsLinearVelocity(
                    CurrentVelocity * DampingFactor, false, BoneName);
            }
        }
    }
}

void UCore_RagdollSystem::BlendToRecovery()
{
    if (!AnimInstance)
    {
        return;
    }

    // Reset collision to character capsule
    SkeletalMeshComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    
    // Smooth transition back to animated pose
    // This would typically involve custom animation blueprint logic
    // For now, we just ensure the mesh is properly positioned
    
    if (AActor* Owner = GetOwner())
    {
        FVector CurrentLocation = SkeletalMeshComp->GetComponentLocation();
        FRotator CurrentRotation = SkeletalMeshComp->GetComponentRotation();
        
        // Gradually blend back to owner transform
        Owner->SetActorLocation(CurrentLocation);
        Owner->SetActorRotation(FRotator(0.0f, CurrentRotation.Yaw, 0.0f));
    }
}

bool UCore_RagdollSystem::ShouldAutoRecover() const
{
    // Auto-recover if ragdoll has been active for max time
    if (RagdollState.RagdollDuration >= RagdollState.MaxRagdollTime)
    {
        return true;
    }

    // Auto-recover if character has come to rest
    if (SkeletalMeshComp)
    {
        FVector Velocity = SkeletalMeshComp->GetPhysicsLinearVelocity();
        if (Velocity.Size() < 50.0f && RagdollState.RagdollDuration > 2.0f)
        {
            return true;
        }
    }

    return false;
}

void UCore_RagdollSystem::ApplyImpactForce()
{
    if (!SkeletalMeshComp || RagdollState.ImpactForce.IsZero())
    {
        return;
    }

    // Find the closest bone to impact location
    FName ClosestBone = TEXT("pelvis");
    float ClosestDistance = FLT_MAX;
    
    for (const FName& BoneName : CriticalBones)
    {
        if (SkeletalMeshComp->GetBoneIndex(BoneName) != INDEX_NONE)
        {
            FVector BoneLocation = SkeletalMeshComp->GetBoneLocation(BoneName);
            float Distance = FVector::Dist(BoneLocation, RagdollState.ImpactLocation);
            
            if (Distance < ClosestDistance)
            {
                ClosestDistance = Distance;
                ClosestBone = BoneName;
            }
        }
    }

    // Apply impulse to the closest bone
    SkeletalMeshComp->AddImpulseAtLocation(
        RagdollState.ImpactForce, 
        RagdollState.ImpactLocation, 
        ClosestBone
    );

    UE_LOG(LogTemp, Log, TEXT("Core_RagdollSystem: Applied impulse %s to bone %s"), 
           *RagdollState.ImpactForce.ToString(), *ClosestBone.ToString());
}