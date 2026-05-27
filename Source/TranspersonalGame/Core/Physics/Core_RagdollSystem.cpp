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
    CurrentBlendWeight = 0.0f;
    StateTimer = 0.0f;
    bWasRagdollActive = false;
    
    // Default settings for prehistoric creatures
    RagdollSettings.BlendInTime = 0.2f;
    RagdollSettings.BlendOutTime = 0.5f;
    RagdollSettings.RecoveryTime = 1.0f;
    RagdollSettings.MinImpulseThreshold = 500.0f;
    RagdollSettings.MaxRagdollTime = 10.0f;
    RagdollSettings.bAutoRecover = true;
    RagdollSettings.bPreserveVelocity = true;
}

void UCore_RagdollSystem::BeginPlay()
{
    Super::BeginPlay();
    
    // Find skeletal mesh component
    AActor* Owner = GetOwner();
    if (Owner)
    {
        SkeletalMeshComp = Owner->FindComponentByClass<USkeletalMeshComponent>();
        if (SkeletalMeshComp)
        {
            AnimInstance = SkeletalMeshComp->GetAnimInstance();
            SetupPhysicsAsset();
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("Core_RagdollSystem: No SkeletalMeshComponent found on %s"), *Owner->GetName());
        }
    }
}

void UCore_RagdollSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!SkeletalMeshComp)
        return;
        
    StateTimer += DeltaTime;
    
    switch (CurrentState)
    {
        case ECore_RagdollState::Blending:
            UpdateBlendWeight(DeltaTime);
            if (CurrentBlendWeight >= 1.0f)
            {
                HandleStateTransition(ECore_RagdollState::Active);
            }
            break;
            
        case ECore_RagdollState::Active:
            ProcessRagdollPhysics();
            if (RagdollSettings.bAutoRecover && StateTimer >= RagdollSettings.MaxRagdollTime)
            {
                RecoverFromRagdoll();
            }
            break;
            
        case ECore_RagdollState::Recovering:
            UpdateBlendWeight(DeltaTime);
            if (CurrentBlendWeight <= 0.0f)
            {
                HandleStateTransition(ECore_RagdollState::Disabled);
            }
            break;
    }
}

void UCore_RagdollSystem::ActivateRagdoll(bool bForceActivation)
{
    if (!SkeletalMeshComp || CurrentState == ECore_RagdollState::Active)
        return;
        
    if (!bForceActivation && CurrentState != ECore_RagdollState::Disabled)
        return;
        
    CacheAnimationState();
    
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
    
    HandleStateTransition(ECore_RagdollState::Blending);
    
    UE_LOG(LogTemp, Log, TEXT("Core_RagdollSystem: Activated ragdoll for %s"), *GetOwner()->GetName());
}

void UCore_RagdollSystem::DeactivateRagdoll()
{
    if (CurrentState == ECore_RagdollState::Disabled)
        return;
        
    RecoverFromRagdoll();
}

void UCore_RagdollSystem::RecoverFromRagdoll()
{
    if (CurrentState == ECore_RagdollState::Disabled || CurrentState == ECore_RagdollState::Recovering)
        return;
        
    HandleStateTransition(ECore_RagdollState::Recovering);
    
    UE_LOG(LogTemp, Log, TEXT("Core_RagdollSystem: Starting ragdoll recovery for %s"), *GetOwner()->GetName());
}

void UCore_RagdollSystem::ApplyImpulseToRagdoll(const FVector& Impulse, const FVector& Location, FName BoneName)
{
    if (!SkeletalMeshComp || CurrentState == ECore_RagdollState::Disabled)
        return;
        
    float ImpulseStrength = Impulse.Size();
    
    // Check if impulse should trigger ragdoll activation
    if (CurrentState == ECore_RagdollState::Disabled && ShouldActivateRagdoll(ImpulseStrength))
    {
        ActivateRagdoll(true);
    }
    
    // Apply impulse to specific bone or root
    if (BoneName != NAME_None)
    {
        SkeletalMeshComp->AddImpulseAtLocation(Impulse, Location, BoneName);
    }
    else
    {
        SkeletalMeshComp->AddImpulseAtLocation(Impulse, Location);
    }
    
    // Broadcast impact event
    OnRagdollImpact.Broadcast(Location, ImpulseStrength);
    
    UE_LOG(LogTemp, Log, TEXT("Core_RagdollSystem: Applied impulse %.2f to %s"), ImpulseStrength, *GetOwner()->GetName());
}

void UCore_RagdollSystem::SetRagdollSettings(const FCore_RagdollSettings& NewSettings)
{
    RagdollSettings = NewSettings;
}

void UCore_RagdollSystem::UpdateBlendWeight(float DeltaTime)
{
    switch (CurrentState)
    {
        case ECore_RagdollState::Blending:
            if (RagdollSettings.BlendInTime > 0.0f)
            {
                CurrentBlendWeight = FMath::Clamp(StateTimer / RagdollSettings.BlendInTime, 0.0f, 1.0f);
            }
            else
            {
                CurrentBlendWeight = 1.0f;
            }
            break;
            
        case ECore_RagdollState::Recovering:
            if (RagdollSettings.BlendOutTime > 0.0f)
            {
                CurrentBlendWeight = FMath::Clamp(1.0f - (StateTimer / RagdollSettings.BlendOutTime), 0.0f, 1.0f);
            }
            else
            {
                CurrentBlendWeight = 0.0f;
            }
            break;
    }
}

void UCore_RagdollSystem::HandleStateTransition(ECore_RagdollState NewState)
{
    if (CurrentState == NewState)
        return;
        
    ECore_RagdollState PreviousState = CurrentState;
    CurrentState = NewState;
    StateTimer = 0.0f;
    
    switch (NewState)
    {
        case ECore_RagdollState::Disabled:
            CleanupRagdoll();
            CurrentBlendWeight = 0.0f;
            break;
            
        case ECore_RagdollState::Blending:
            CurrentBlendWeight = 0.0f;
            break;
            
        case ECore_RagdollState::Active:
            CurrentBlendWeight = 1.0f;
            break;
            
        case ECore_RagdollState::Recovering:
            CurrentBlendWeight = 1.0f;
            break;
    }
    
    OnRagdollStateChanged.Broadcast(NewState);
    
    UE_LOG(LogTemp, Log, TEXT("Core_RagdollSystem: State transition %d -> %d for %s"), 
           (int32)PreviousState, (int32)NewState, *GetOwner()->GetName());
}

void UCore_RagdollSystem::CacheAnimationState()
{
    if (!SkeletalMeshComp)
        return;
        
    // Cache current velocity and rotation
    if (AActor* Owner = GetOwner())
    {
        LastKnownVelocity = Owner->GetVelocity();
        LastKnownRotation = Owner->GetActorRotation();
    }
    
    // Cache bone transforms if needed
    BoneTransforms.Reset();
    if (RagdollSettings.bPreserveVelocity)
    {
        // Store bone transforms for smooth transition
        const int32 NumBones = SkeletalMeshComp->GetNumBones();
        BoneTransforms.Reserve(NumBones);
        
        for (int32 BoneIndex = 0; BoneIndex < NumBones; ++BoneIndex)
        {
            BoneTransforms.Add(SkeletalMeshComp->GetBoneTransform(BoneIndex));
        }
    }
}

void UCore_RagdollSystem::RestoreAnimationState()
{
    if (!SkeletalMeshComp)
        return;
        
    // Restore animation control
    SkeletalMeshComp->SetSimulatePhysics(false);
    SkeletalMeshComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    
    // Re-enable capsule collision for character
    if (ACharacter* Character = Cast<ACharacter>(GetOwner()))
    {
        if (UCapsuleComponent* Capsule = Character->GetCapsuleComponent())
        {
            Capsule->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        }
    }
    
    // Apply preserved velocity if enabled
    if (RagdollSettings.bPreserveVelocity && GetOwner())
    {
        GetOwner()->SetActorRotation(LastKnownRotation);
    }
}

void UCore_RagdollSystem::ProcessRagdollPhysics()
{
    if (!SkeletalMeshComp)
        return;
        
    // Monitor ragdoll physics state
    // Check for stability, ground contact, etc.
    
    // Get root bone velocity to determine if ragdoll has settled
    FVector RootVelocity = SkeletalMeshComp->GetPhysicsLinearVelocity();
    float VelocityMagnitude = RootVelocity.Size();
    
    // Auto-recovery based on velocity threshold
    if (RagdollSettings.bAutoRecover && VelocityMagnitude < 50.0f && StateTimer > 2.0f)
    {
        RecoverFromRagdoll();
    }
}

bool UCore_RagdollSystem::ShouldActivateRagdoll(float ImpulseStrength) const
{
    return ImpulseStrength >= RagdollSettings.MinImpulseThreshold;
}

void UCore_RagdollSystem::SetupPhysicsAsset()
{
    if (!SkeletalMeshComp)
        return;
        
    // Ensure physics asset is properly configured
    UPhysicsAsset* PhysicsAsset = SkeletalMeshComp->GetPhysicsAsset();
    if (!PhysicsAsset)
    {
        UE_LOG(LogTemp, Warning, TEXT("Core_RagdollSystem: No PhysicsAsset found for %s"), *GetOwner()->GetName());
        return;
    }
    
    // Configure physics asset settings for prehistoric creatures
    // This would typically be done in the asset itself, but we can override here
    UE_LOG(LogTemp, Log, TEXT("Core_RagdollSystem: Physics asset configured for %s"), *GetOwner()->GetName());
}

void UCore_RagdollSystem::CleanupRagdoll()
{
    if (!SkeletalMeshComp)
        return;
        
    RestoreAnimationState();
    
    // Clear cached data
    BoneTransforms.Reset();
    LastKnownVelocity = FVector::ZeroVector;
    LastKnownRotation = FRotator::ZeroRotator;
    
    bWasRagdollActive = false;
}