#include "Core_RagdollSystem.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/Character.h"
#include "PhysicsEngine/PhysicsAsset.h"
#include "Animation/AnimInstance.h"

UCore_RagdollSystem::UCore_RagdollSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = true;
    
    CurrentState = ECore_RagdollState::Inactive;
    StateTimer = 0.0f;
    BlendWeight = 0.0f;
    TargetMesh = nullptr;
    CachedPhysicsAsset = nullptr;
}

void UCore_RagdollSystem::BeginPlay()
{
    Super::BeginPlay();
    
    FindTargetMesh();
    
    if (TargetMesh && TargetMesh->GetPhysicsAsset())
    {
        CachedPhysicsAsset = TargetMesh->GetPhysicsAsset();
        UE_LOG(LogTemp, Log, TEXT("RagdollSystem: Found physics asset %s"), *CachedPhysicsAsset->GetName());
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("RagdollSystem: No physics asset found for target mesh"));
    }
}

void UCore_RagdollSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (CurrentState == ECore_RagdollState::Inactive)
        return;
    
    StateTimer += DeltaTime;
    
    switch (CurrentState)
    {
        case ECore_RagdollState::Activating:
            UpdateRagdollBlending(DeltaTime);
            if (BlendWeight >= 1.0f)
            {
                CurrentState = ECore_RagdollState::Active;
                BlendWeight = 1.0f;
                UE_LOG(LogTemp, Log, TEXT("RagdollSystem: Ragdoll fully activated"));
            }
            break;
            
        case ECore_RagdollState::Active:
            if (RagdollConfig.bAutoDeactivate && ShouldAutoDeactivate())
            {
                DeactivateRagdoll();
            }
            break;
            
        case ECore_RagdollState::Deactivating:
            UpdateRagdollBlending(DeltaTime);
            if (BlendWeight <= 0.0f)
            {
                CurrentState = ECore_RagdollState::Inactive;
                BlendWeight = 0.0f;
                RestorePreRagdollState();
                UE_LOG(LogTemp, Log, TEXT("RagdollSystem: Ragdoll fully deactivated"));
            }
            break;
    }
}

void UCore_RagdollSystem::ActivateRagdoll(const FVector& ImpactForce, const FVector& ImpactLocation)
{
    if (!TargetMesh || !CachedPhysicsAsset)
    {
        UE_LOG(LogTemp, Error, TEXT("RagdollSystem: Cannot activate ragdoll - missing mesh or physics asset"));
        return;
    }
    
    if (CurrentState != ECore_RagdollState::Inactive)
    {
        UE_LOG(LogTemp, Warning, TEXT("RagdollSystem: Ragdoll already active or transitioning"));
        return;
    }
    
    CachePreRagdollState();
    
    // Disable animation and enable physics simulation
    TargetMesh->SetAllBodiesSimulatePhysics(true);
    TargetMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    TargetMesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
    
    // Apply impact force if provided
    if (!ImpactForce.IsZero())
    {
        FVector ForceLocation = ImpactLocation.IsZero() ? TargetMesh->GetComponentLocation() : ImpactLocation;
        TargetMesh->AddImpulseAtLocation(ImpactForce, ForceLocation);
        UE_LOG(LogTemp, Log, TEXT("RagdollSystem: Applied impact force %s at %s"), *ImpactForce.ToString(), *ForceLocation.ToString());
    }
    
    CurrentState = ECore_RagdollState::Activating;
    StateTimer = 0.0f;
    BlendWeight = 0.0f;
    
    UE_LOG(LogTemp, Log, TEXT("RagdollSystem: Ragdoll activation started"));
}

void UCore_RagdollSystem::DeactivateRagdoll()
{
    if (CurrentState == ECore_RagdollState::Inactive || CurrentState == ECore_RagdollState::Deactivating)
    {
        return;
    }
    
    CurrentState = ECore_RagdollState::Deactivating;
    StateTimer = 0.0f;
    
    UE_LOG(LogTemp, Log, TEXT("RagdollSystem: Ragdoll deactivation started"));
}

void UCore_RagdollSystem::SetRagdollConfig(const FCore_RagdollConfig& NewConfig)
{
    RagdollConfig = NewConfig;
    UE_LOG(LogTemp, Log, TEXT("RagdollSystem: Configuration updated"));
}

void UCore_RagdollSystem::ApplyImpulseToRagdoll(const FVector& Impulse, const FName& BoneName)
{
    if (!TargetMesh || CurrentState != ECore_RagdollState::Active)
    {
        return;
    }
    
    if (BoneName == NAME_None)
    {
        TargetMesh->AddImpulse(Impulse);
    }
    else
    {
        TargetMesh->AddImpulseToAllBodiesBelow(Impulse, BoneName, true);
    }
    
    UE_LOG(LogTemp, Log, TEXT("RagdollSystem: Applied impulse %s to bone %s"), *Impulse.ToString(), *BoneName.ToString());
}

void UCore_RagdollSystem::SetBoneLinearVelocity(const FName& BoneName, const FVector& Velocity)
{
    if (!TargetMesh || CurrentState != ECore_RagdollState::Active)
    {
        return;
    }
    
    TargetMesh->SetAllPhysicsLinearVelocity(Velocity);
    UE_LOG(LogTemp, Log, TEXT("RagdollSystem: Set bone velocity %s to %s"), *BoneName.ToString(), *Velocity.ToString());
}

FVector UCore_RagdollSystem::GetBoneVelocity(const FName& BoneName) const
{
    if (!TargetMesh)
    {
        return FVector::ZeroVector;
    }
    
    return TargetMesh->GetPhysicsLinearVelocity(BoneName);
}

void UCore_RagdollSystem::UpdateRagdollBlending(float DeltaTime)
{
    float BlendSpeed = 1.0f / FMath::Max(RagdollConfig.BlendTime, 0.1f);
    
    if (CurrentState == ECore_RagdollState::Activating)
    {
        BlendWeight = FMath::Clamp(BlendWeight + (BlendSpeed * DeltaTime), 0.0f, 1.0f);
    }
    else if (CurrentState == ECore_RagdollState::Deactivating)
    {
        BlendWeight = FMath::Clamp(BlendWeight - (BlendSpeed * DeltaTime), 0.0f, 1.0f);
    }
}

void UCore_RagdollSystem::CachePreRagdollState()
{
    if (!TargetMesh)
        return;
    
    PreRagdollTransform = TargetMesh->GetComponentTransform();
    
    // Cache bone transforms for potential restoration
    PreRagdollBoneTransforms.Empty();
    if (TargetMesh->GetAnimInstance())
    {
        // Store current pose
        UE_LOG(LogTemp, Log, TEXT("RagdollSystem: Cached pre-ragdoll state"));
    }
}

void UCore_RagdollSystem::RestorePreRagdollState()
{
    if (!TargetMesh)
        return;
    
    // Disable physics simulation
    TargetMesh->SetAllBodiesSimulatePhysics(false);
    TargetMesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    
    // Re-enable animation
    if (TargetMesh->GetAnimInstance())
    {
        TargetMesh->GetAnimInstance()->SetRootMotionMode(ERootMotionMode::RootMotionFromMontagesOnly);
    }
    
    UE_LOG(LogTemp, Log, TEXT("RagdollSystem: Restored pre-ragdoll state"));
}

bool UCore_RagdollSystem::ShouldAutoDeactivate() const
{
    if (StateTimer < RagdollConfig.AutoDeactivateTime)
        return false;
    
    if (!TargetMesh)
        return true;
    
    // Check if ragdoll has settled (low velocity)
    FVector CurrentVelocity = TargetMesh->GetPhysicsLinearVelocity();
    return CurrentVelocity.Size() < RagdollConfig.MinVelocityThreshold;
}

void UCore_RagdollSystem::FindTargetMesh()
{
    AActor* Owner = GetOwner();
    if (!Owner)
        return;
    
    // Try to find skeletal mesh component
    TargetMesh = Owner->FindComponentByClass<USkeletalMeshComponent>();
    
    if (!TargetMesh)
    {
        // If owner is a character, get the mesh component
        if (ACharacter* Character = Cast<ACharacter>(Owner))
        {
            TargetMesh = Character->GetMesh();
        }
    }
    
    if (TargetMesh)
    {
        UE_LOG(LogTemp, Log, TEXT("RagdollSystem: Found target mesh %s"), *TargetMesh->GetName());
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("RagdollSystem: No skeletal mesh component found on owner %s"), *Owner->GetName());
    }
}