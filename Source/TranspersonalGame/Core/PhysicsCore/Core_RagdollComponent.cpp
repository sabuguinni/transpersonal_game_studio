#include "Core_RagdollComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "PhysicsEngine/PhysicsAsset.h"
#include "Animation/AnimInstance.h"
#include "Components/CapsuleComponent.h"
#include "Engine/Engine.h"

UCore_RagdollComponent::UCore_RagdollComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = true;
    
    // Default settings
    RagdollSettings.BlendInTime = 0.2f;
    RagdollSettings.BlendOutTime = 0.5f;
    RagdollSettings.LinearDamping = 0.1f;
    RagdollSettings.AngularDamping = 0.1f;
    RagdollSettings.MaxLinearVelocity = 1000.0f;
    RagdollSettings.MaxAngularVelocity = 50.0f;
    RagdollSettings.RecoveryThreshold = 50.0f;
    RagdollSettings.bAutoRecover = true;
    
    CurrentState = ECore_RagdollState::Inactive;
    CurrentBlendWeight = 0.0f;
    StateTimer = 0.0f;
}

void UCore_RagdollComponent::BeginPlay()
{
    Super::BeginPlay();
    
    // Cache the target mesh component
    if (!TargetMeshComponent.IsValid())
    {
        if (ACharacter* Character = Cast<ACharacter>(GetOwner()))
        {
            TargetMeshComponent = Character->GetMesh();
        }
        else
        {
            TargetMeshComponent = GetOwner()->FindComponentByClass<USkeletalMeshComponent>();
        }
    }
    
    // Store initial position and rotation
    if (USkeletalMeshComponent* MeshComp = GetTargetMeshComponent())
    {
        LastKnownPosition = MeshComp->GetComponentLocation();
        LastKnownRotation = MeshComp->GetComponentRotation();
    }
}

void UCore_RagdollComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    StateTimer += DeltaTime;
    
    switch (CurrentState)
    {
        case ECore_RagdollState::Transitioning:
            UpdateRagdollTransition(DeltaTime);
            break;
            
        case ECore_RagdollState::Active:
            if (RagdollSettings.bAutoRecover)
            {
                UpdateRecoveryCheck();
            }
            break;
            
        case ECore_RagdollState::Recovering:
            UpdateRagdollTransition(DeltaTime);
            break;
            
        default:
            break;
    }
}

void UCore_RagdollComponent::ActivateRagdoll(bool bImmediate)
{
    USkeletalMeshComponent* MeshComp = GetTargetMeshComponent();
    if (!MeshComp || !MeshComp->GetPhysicsAsset())
    {
        UE_LOG(LogTemp, Warning, TEXT("Core_RagdollComponent: Cannot activate ragdoll - no mesh component or physics asset"));
        return;
    }
    
    if (CurrentState == ECore_RagdollState::Active)
    {
        return; // Already active
    }
    
    // Store current position before ragdoll activation
    LastKnownPosition = MeshComp->GetComponentLocation();
    LastKnownRotation = MeshComp->GetComponentRotation();
    
    if (bImmediate)
    {
        // Immediate activation
        MeshComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        MeshComp->SetSimulatePhysics(true);
        CurrentBlendWeight = 1.0f;
        SetRagdollState(ECore_RagdollState::Active);
    }
    else
    {
        // Gradual transition
        SetRagdollState(ECore_RagdollState::Transitioning);
        CurrentBlendWeight = 0.0f;
    }
    
    // Disable character movement if this is a character
    if (ACharacter* Character = Cast<ACharacter>(GetOwner()))
    {
        if (UCharacterMovementComponent* MovementComp = Character->GetCharacterMovement())
        {
            MovementComp->SetMovementMode(MOVE_None);
        }
        
        // Disable collision on capsule component
        if (UCapsuleComponent* CapsuleComp = Character->GetCapsuleComponent())
        {
            CapsuleComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        }
    }
    
    ApplyRagdollSettings();
}

void UCore_RagdollComponent::DeactivateRagdoll(bool bImmediate)
{
    if (CurrentState == ECore_RagdollState::Inactive)
    {
        return; // Already inactive
    }
    
    if (bImmediate)
    {
        USkeletalMeshComponent* MeshComp = GetTargetMeshComponent();
        if (MeshComp)
        {
            MeshComp->SetSimulatePhysics(false);
            MeshComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
        }
        
        CurrentBlendWeight = 0.0f;
        SetRagdollState(ECore_RagdollState::Inactive);
        
        // Re-enable character movement
        if (ACharacter* Character = Cast<ACharacter>(GetOwner()))
        {
            if (UCharacterMovementComponent* MovementComp = Character->GetCharacterMovement())
            {
                MovementComp->SetMovementMode(MOVE_Walking);
            }
            
            if (UCapsuleComponent* CapsuleComp = Character->GetCapsuleComponent())
            {
                CapsuleComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
            }
        }
    }
    else
    {
        SetRagdollState(ECore_RagdollState::Recovering);
    }
}

void UCore_RagdollComponent::SetRagdollSettings(const FCore_RagdollSettings& NewSettings)
{
    RagdollSettings = NewSettings;
    
    if (CurrentState == ECore_RagdollState::Active)
    {
        ApplyRagdollSettings();
    }
}

void UCore_RagdollComponent::AddImpulseAtBone(const FString& BoneName, const FVector& Impulse)
{
    USkeletalMeshComponent* MeshComp = GetTargetMeshComponent();
    if (!MeshComp || CurrentState != ECore_RagdollState::Active)
    {
        return;
    }
    
    FName BoneNameFName(*BoneName);
    if (MeshComp->GetBoneIndex(BoneNameFName) != INDEX_NONE)
    {
        MeshComp->AddImpulseAtLocation(Impulse, MeshComp->GetBoneLocation(BoneNameFName));
    }
}

void UCore_RagdollComponent::AddForceAtBone(const FString& BoneName, const FVector& Force)
{
    USkeletalMeshComponent* MeshComp = GetTargetMeshComponent();
    if (!MeshComp || CurrentState != ECore_RagdollState::Active)
    {
        return;
    }
    
    FName BoneNameFName(*BoneName);
    if (MeshComp->GetBoneIndex(BoneNameFName) != INDEX_NONE)
    {
        MeshComp->AddForceAtLocation(Force, MeshComp->GetBoneLocation(BoneNameFName));
    }
}

void UCore_RagdollComponent::SetBoneLinearVelocity(const FString& BoneName, const FVector& Velocity)
{
    USkeletalMeshComponent* MeshComp = GetTargetMeshComponent();
    if (!MeshComp || CurrentState != ECore_RagdollState::Active)
    {
        return;
    }
    
    FName BoneNameFName(*BoneName);
    if (MeshComp->GetBoneIndex(BoneNameFName) != INDEX_NONE)
    {
        FBodyInstance* BodyInstance = MeshComp->GetBodyInstance(BoneNameFName);
        if (BodyInstance)
        {
            BodyInstance->SetLinearVelocity(Velocity, false);
        }
    }
}

void UCore_RagdollComponent::StartRecovery()
{
    if (CurrentState == ECore_RagdollState::Active && CanRecover())
    {
        SetRagdollState(ECore_RagdollState::Recovering);
    }
}

bool UCore_RagdollComponent::CanRecover() const
{
    USkeletalMeshComponent* MeshComp = GetTargetMeshComponent();
    if (!MeshComp || CurrentState != ECore_RagdollState::Active)
    {
        return false;
    }
    
    // Check if the character is relatively stable (low velocity)
    FVector LinearVelocity = MeshComp->GetPhysicsLinearVelocity();
    return LinearVelocity.Size() < RagdollSettings.RecoveryThreshold;
}

FVector UCore_RagdollComponent::GetRecoveryPosition() const
{
    USkeletalMeshComponent* MeshComp = GetTargetMeshComponent();
    if (!MeshComp)
    {
        return LastKnownPosition;
    }
    
    // Use pelvis or root bone position for recovery
    FName PelvisBone = TEXT("pelvis");
    if (MeshComp->GetBoneIndex(PelvisBone) != INDEX_NONE)
    {
        return MeshComp->GetBoneLocation(PelvisBone);
    }
    
    return MeshComp->GetComponentLocation();
}

void UCore_RagdollComponent::TestRagdollActivation()
{
    if (CurrentState == ECore_RagdollState::Inactive)
    {
        ActivateRagdoll(false);
        
        // Auto-deactivate after 3 seconds for testing
        GetWorld()->GetTimerManager().SetTimer(
            FTimerHandle(),
            [this]() { DeactivateRagdoll(false); },
            3.0f,
            false
        );
    }
}

void UCore_RagdollComponent::UpdateRagdollTransition(float DeltaTime)
{
    USkeletalMeshComponent* MeshComp = GetTargetMeshComponent();
    if (!MeshComp)
    {
        return;
    }
    
    if (CurrentState == ECore_RagdollState::Transitioning)
    {
        // Blend in to ragdoll
        CurrentBlendWeight = FMath::Clamp(CurrentBlendWeight + (DeltaTime / RagdollSettings.BlendInTime), 0.0f, 1.0f);
        
        if (CurrentBlendWeight >= 1.0f)
        {
            MeshComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
            MeshComp->SetSimulatePhysics(true);
            SetRagdollState(ECore_RagdollState::Active);
            ApplyRagdollSettings();
        }
    }
    else if (CurrentState == ECore_RagdollState::Recovering)
    {
        // Blend out from ragdoll
        CurrentBlendWeight = FMath::Clamp(CurrentBlendWeight - (DeltaTime / RagdollSettings.BlendOutTime), 0.0f, 1.0f);
        
        if (CurrentBlendWeight <= 0.0f)
        {
            MeshComp->SetSimulatePhysics(false);
            MeshComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
            SetRagdollState(ECore_RagdollState::Inactive);
            
            // Re-enable character movement
            if (ACharacter* Character = Cast<ACharacter>(GetOwner()))
            {
                if (UCharacterMovementComponent* MovementComp = Character->GetCharacterMovement())
                {
                    MovementComp->SetMovementMode(MOVE_Walking);
                }
                
                if (UCapsuleComponent* CapsuleComp = Character->GetCapsuleComponent())
                {
                    CapsuleComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
                }
            }
        }
    }
}

void UCore_RagdollComponent::UpdateRecoveryCheck()
{
    if (StateTimer > 2.0f && CanRecover()) // Check after 2 seconds minimum
    {
        StartRecovery();
    }
}

void UCore_RagdollComponent::ApplyRagdollSettings()
{
    USkeletalMeshComponent* MeshComp = GetTargetMeshComponent();
    if (!MeshComp)
    {
        return;
    }
    
    // Apply physics settings to all bodies
    TArray<FName> BoneNames;
    MeshComp->GetBoneNames(BoneNames);
    
    for (const FName& BoneName : BoneNames)
    {
        FBodyInstance* BodyInstance = MeshComp->GetBodyInstance(BoneName);
        if (BodyInstance)
        {
            BodyInstance->LinearDamping = RagdollSettings.LinearDamping;
            BodyInstance->AngularDamping = RagdollSettings.AngularDamping;
            BodyInstance->SetMaxLinearVelocity(RagdollSettings.MaxLinearVelocity, false);
            BodyInstance->SetMaxAngularVelocity(RagdollSettings.MaxAngularVelocity, false);
        }
    }
}

void UCore_RagdollComponent::SetRagdollState(ECore_RagdollState NewState)
{
    if (CurrentState != NewState)
    {
        CurrentState = NewState;
        StateTimer = 0.0f;
        
        // Broadcast state change
        OnRagdollStateChanged.Broadcast(NewState);
        
        UE_LOG(LogTemp, Log, TEXT("Core_RagdollComponent: State changed to %s"), 
               *UEnum::GetValueAsString(NewState));
    }
}

USkeletalMeshComponent* UCore_RagdollComponent::GetTargetMeshComponent() const
{
    return TargetMeshComponent.Get();
}