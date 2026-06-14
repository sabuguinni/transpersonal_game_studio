#include "Core_RagdollSystem.h"
#include "GameFramework/Character.h"
#include "Components/CapsuleComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"

UCore_RagdollSystem::UCore_RagdollSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = true;
    
    CurrentState = ECore_RagdollState::Disabled;
    StateTimer = 0.0f;
    RagdollActiveTime = 0.0f;
    
    // Default settings
    RagdollSettings.TransitionTime = 0.2f;
    RagdollSettings.RecoveryTime = 1.0f;
    RagdollSettings.MinImpulseThreshold = 500.0f;
    RagdollSettings.MaxRagdollDuration = 10.0f;
    RagdollSettings.bAutoRecover = true;
}

void UCore_RagdollSystem::BeginPlay()
{
    Super::BeginPlay();
    
    // Cache skeletal mesh component
    if (AActor* Owner = GetOwner())
    {
        SkeletalMeshComp = Owner->FindComponentByClass<USkeletalMeshComponent>();
        if (SkeletalMeshComp.IsValid())
        {
            CachedAnimInstance = SkeletalMeshComp->GetAnimInstance();
        }
    }
    
    if (!IsValidForRagdoll())
    {
        UE_LOG(LogTemp, Warning, TEXT("Core_RagdollSystem: Invalid setup on %s"), GetOwner() ? *GetOwner()->GetName() : TEXT("Unknown"));
    }
}

void UCore_RagdollSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    UpdateRagdollState(DeltaTime);
}

void UCore_RagdollSystem::EnableRagdoll(bool bImmediate)
{
    if (!IsValidForRagdoll() || CurrentState == ECore_RagdollState::Active)
    {
        return;
    }
    
    CachePreRagdollState();
    
    if (bImmediate)
    {
        TransitionToRagdoll();
        CurrentState = ECore_RagdollState::Active;
        StateTimer = 0.0f;
        RagdollActiveTime = 0.0f;
        OnRagdollStateChanged.Broadcast(CurrentState);
    }
    else
    {
        CurrentState = ECore_RagdollState::Transitioning;
        StateTimer = 0.0f;
        OnRagdollStateChanged.Broadcast(CurrentState);
    }
}

void UCore_RagdollSystem::DisableRagdoll(bool bImmediate)
{
    if (CurrentState == ECore_RagdollState::Disabled)
    {
        return;
    }
    
    if (bImmediate)
    {
        TransitionFromRagdoll();
        CurrentState = ECore_RagdollState::Disabled;
        StateTimer = 0.0f;
        OnRagdollStateChanged.Broadcast(CurrentState);
    }
    else if (CurrentState == ECore_RagdollState::Active)
    {
        StartRecovery();
    }
}

void UCore_RagdollSystem::ToggleRagdoll()
{
    if (IsRagdollActive())
    {
        DisableRagdoll();
    }
    else
    {
        EnableRagdoll();
    }
}

bool UCore_RagdollSystem::IsRagdollActive() const
{
    return CurrentState == ECore_RagdollState::Active || CurrentState == ECore_RagdollState::Transitioning;
}

void UCore_RagdollSystem::ApplyImpactForce(const FVector& ImpactLocation, const FVector& ImpactForce, FName BoneName)
{
    if (!IsValidForRagdoll())
    {
        return;
    }
    
    float ImpulseMagnitude = ImpactForce.Size();
    
    // Auto-enable ragdoll if impact is strong enough
    if (CurrentState == ECore_RagdollState::Disabled && ImpulseMagnitude >= RagdollSettings.MinImpulseThreshold)
    {
        EnableRagdoll(true);
    }
    
    // Apply force to physics body
    if (CurrentState == ECore_RagdollState::Active && SkeletalMeshComp.IsValid())
    {
        if (BoneName != NAME_None)
        {
            SkeletalMeshComp->AddImpulseAtLocation(ImpactForce, ImpactLocation, BoneName);
        }
        else
        {
            SkeletalMeshComp->AddImpulseAtLocation(ImpactForce, ImpactLocation);
        }
        
        OnRagdollImpact.Broadcast(ImpactLocation, ImpulseMagnitude);
    }
}

void UCore_RagdollSystem::ProcessDamageImpact(float DamageAmount, const FVector& ImpactLocation, const FVector& ImpactDirection)
{
    // Convert damage to impulse force
    float ImpulseMultiplier = FMath::Clamp(DamageAmount * 10.0f, 100.0f, 2000.0f);
    FVector ImpactForce = ImpactDirection.GetSafeNormal() * ImpulseMultiplier;
    
    ApplyImpactForce(ImpactLocation, ImpactForce);
}

void UCore_RagdollSystem::StartRecovery()
{
    if (CurrentState == ECore_RagdollState::Active)
    {
        CurrentState = ECore_RagdollState::Recovering;
        StateTimer = 0.0f;
        OnRagdollStateChanged.Broadcast(CurrentState);
    }
}

void UCore_RagdollSystem::ForceRecovery()
{
    DisableRagdoll(true);
}

void UCore_RagdollSystem::SetRagdollSettings(const FCore_RagdollSettings& NewSettings)
{
    RagdollSettings = NewSettings;
}

void UCore_RagdollSystem::UpdateRagdollState(float DeltaTime)
{
    StateTimer += DeltaTime;
    
    switch (CurrentState)
    {
        case ECore_RagdollState::Transitioning:
            if (StateTimer >= RagdollSettings.TransitionTime)
            {
                TransitionToRagdoll();
                CurrentState = ECore_RagdollState::Active;
                StateTimer = 0.0f;
                RagdollActiveTime = 0.0f;
                OnRagdollStateChanged.Broadcast(CurrentState);
            }
            break;
            
        case ECore_RagdollState::Active:
            RagdollActiveTime += DeltaTime;
            if (ShouldAutoRecover())
            {
                StartRecovery();
            }
            break;
            
        case ECore_RagdollState::Recovering:
            if (StateTimer >= RagdollSettings.RecoveryTime)
            {
                TransitionFromRagdoll();
                CurrentState = ECore_RagdollState::Disabled;
                StateTimer = 0.0f;
                OnRagdollStateChanged.Broadcast(CurrentState);
            }
            else
            {
                // Blend back to animation
                float BlendAlpha = StateTimer / RagdollSettings.RecoveryTime;
                BlendToRecoveryPose(BlendAlpha);
            }
            break;
            
        case ECore_RagdollState::Disabled:
        default:
            break;
    }
}

void UCore_RagdollSystem::TransitionToRagdoll()
{
    if (!SkeletalMeshComp.IsValid())
    {
        return;
    }
    
    // Enable physics simulation
    SkeletalMeshComp->SetSimulatePhysics(true);
    SkeletalMeshComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    
    // Disable capsule collision for character
    if (ACharacter* Character = Cast<ACharacter>(GetOwner()))
    {
        if (UCapsuleComponent* Capsule = Character->GetCapsuleComponent())
        {
            Capsule->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Core_RagdollSystem: Ragdoll enabled on %s"), *GetOwner()->GetName());
}

void UCore_RagdollSystem::TransitionFromRagdoll()
{
    if (!SkeletalMeshComp.IsValid())
    {
        return;
    }
    
    // Disable physics simulation
    SkeletalMeshComp->SetSimulatePhysics(false);
    SkeletalMeshComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    
    // Re-enable capsule collision for character
    if (ACharacter* Character = Cast<ACharacter>(GetOwner()))
    {
        if (UCapsuleComponent* Capsule = Character->GetCapsuleComponent())
        {
            Capsule->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        }
        
        // Restore character position
        Character->SetActorTransform(PreRagdollTransform);
    }
    
    // Restore animation
    if (CachedAnimInstance.IsValid())
    {
        SkeletalMeshComp->SetAnimInstanceClass(CachedAnimInstance->GetClass());
    }
    
    UE_LOG(LogTemp, Log, TEXT("Core_RagdollSystem: Ragdoll disabled on %s"), *GetOwner()->GetName());
}

void UCore_RagdollSystem::CachePreRagdollState()
{
    if (AActor* Owner = GetOwner())
    {
        PreRagdollTransform = Owner->GetActorTransform();
        
        if (UPrimitiveComponent* RootComp = Cast<UPrimitiveComponent>(Owner->GetRootComponent()))
        {
            PreRagdollVelocity = RootComp->GetPhysicsLinearVelocity();
        }
    }
    
    UpdateBoneTransforms();
}

void UCore_RagdollSystem::RestorePreRagdollState()
{
    if (AActor* Owner = GetOwner())
    {
        Owner->SetActorTransform(PreRagdollTransform);
    }
}

bool UCore_RagdollSystem::ShouldAutoRecover() const
{
    if (!RagdollSettings.bAutoRecover)
    {
        return false;
    }
    
    // Auto-recover after max duration
    if (RagdollActiveTime >= RagdollSettings.MaxRagdollDuration)
    {
        return true;
    }
    
    // Auto-recover if ragdoll has settled (low velocity)
    if (SkeletalMeshComp.IsValid() && RagdollActiveTime > 2.0f)
    {
        FVector Velocity = SkeletalMeshComp->GetPhysicsLinearVelocity();
        if (Velocity.Size() < 50.0f)
        {
            return true;
        }
    }
    
    return false;
}

void UCore_RagdollSystem::UpdateBoneTransforms()
{
    if (!SkeletalMeshComp.IsValid())
    {
        return;
    }
    
    BoneTransforms.Empty();
    
    const FReferenceSkeleton& RefSkeleton = SkeletalMeshComp->GetSkeletalMeshAsset()->GetRefSkeleton();
    int32 NumBones = RefSkeleton.GetNum();
    
    BoneTransforms.Reserve(NumBones);
    
    for (int32 BoneIndex = 0; BoneIndex < NumBones; ++BoneIndex)
    {
        FTransform BoneTransform = SkeletalMeshComp->GetBoneTransform(BoneIndex);
        BoneTransforms.Add(BoneTransform);
    }
}

void UCore_RagdollSystem::BlendToRecoveryPose(float BlendAlpha)
{
    // Implementation for blending from ragdoll back to animated pose
    // This would typically involve interpolating bone transforms
    if (SkeletalMeshComp.IsValid() && CachedAnimInstance.IsValid())
    {
        // Gradually reduce physics influence
        float PhysicsWeight = 1.0f - BlendAlpha;
        SkeletalMeshComp->SetAllBodiesPhysicsBlendWeight(PhysicsWeight);
    }
}

bool UCore_RagdollSystem::IsValidForRagdoll() const
{
    return SkeletalMeshComp.IsValid() && 
           SkeletalMeshComp->GetPhysicsAsset() != nullptr &&
           GetOwner() != nullptr;
}

USkeletalMeshComponent* UCore_RagdollSystem::GetSkeletalMeshComponent() const
{
    return SkeletalMeshComp.Get();
}