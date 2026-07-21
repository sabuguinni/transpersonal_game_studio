#include "Anim_PhysicsBasedAnimation.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Components/CapsuleComponent.h"
#include "Engine/Engine.h"
#include "Kismet/KismetMathLibrary.h"
#include "DrawDebugHelpers.h"

DEFINE_LOG_CATEGORY_STATIC(LogPhysicsAnimation, Log, All);

UAnim_PhysicsBasedAnimation::UAnim_PhysicsBasedAnimation()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PrePhysics;
    
    // Physics simulation parameters
    PhysicsBlendWeight = 1.0f;
    RagdollBlendWeight = 0.0f;
    ImpactThreshold = 500.0f;
    RecoveryTime = 2.0f;
    
    // Bone physics settings
    BoneLinearDamping = 0.1f;
    BoneAngularDamping = 0.05f;
    BoneRestitution = 0.3f;
    BoneFriction = 0.7f;
    
    // State tracking
    bIsRagdollActive = false;
    bIsRecovering = false;
    CurrentRecoveryTime = 0.0f;
    LastImpactVelocity = FVector::ZeroVector;
    
    // Impact detection
    bEnableImpactDetection = true;
    ImpactCooldownTime = 1.0f;
    LastImpactTime = 0.0f;
    
    // Physics bodies to simulate
    PhysicsBones.Add(TEXT("spine_01"));
    PhysicsBones.Add(TEXT("spine_02"));
    PhysicsBones.Add(TEXT("spine_03"));
    PhysicsBones.Add(TEXT("head"));
    PhysicsBones.Add(TEXT("upperarm_l"));
    PhysicsBones.Add(TEXT("upperarm_r"));
    PhysicsBones.Add(TEXT("lowerarm_l"));
    PhysicsBones.Add(TEXT("lowerarm_r"));
    PhysicsBones.Add(TEXT("thigh_l"));
    PhysicsBones.Add(TEXT("thigh_r"));
    PhysicsBones.Add(TEXT("calf_l"));
    PhysicsBones.Add(TEXT("calf_r"));
}

void UAnim_PhysicsBasedAnimation::BeginPlay()
{
    Super::BeginPlay();
    
    // Get owner character
    OwnerCharacter = Cast<ACharacter>(GetOwner());
    if (!OwnerCharacter)
    {
        UE_LOG(LogPhysicsAnimation, Error, TEXT("PhysicsBasedAnimation component requires ACharacter owner"));
        return;
    }
    
    // Get skeletal mesh component
    SkeletalMeshComp = OwnerCharacter->GetMesh();
    if (!SkeletalMeshComp)
    {
        UE_LOG(LogPhysicsAnimation, Error, TEXT("No skeletal mesh component found on character"));
        return;
    }
    
    // Get movement component
    MovementComponent = OwnerCharacter->GetCharacterMovement();
    
    // Initialize physics bodies
    InitializePhysicsBodies();
    
    // Bind to hit events for impact detection
    if (OwnerCharacter->GetCapsuleComponent())
    {
        OwnerCharacter->GetCapsuleComponent()->OnComponentHit.AddDynamic(this, &UAnim_PhysicsBasedAnimation::OnCharacterHit);
    }
    
    UE_LOG(LogPhysicsAnimation, Log, TEXT("PhysicsBasedAnimation initialized for %s"), *OwnerCharacter->GetName());
}

void UAnim_PhysicsBasedAnimation::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!OwnerCharacter || !SkeletalMeshComp)
    {
        return;
    }
    
    // Update physics simulation
    UpdatePhysicsSimulation(DeltaTime);
    
    // Handle ragdoll recovery
    if (bIsRecovering)
    {
        UpdateRagdollRecovery(DeltaTime);
    }
    
    // Update impact detection cooldown
    if (GetWorld())
    {
        float CurrentTime = GetWorld()->GetTimeSeconds();
        if (CurrentTime - LastImpactTime > ImpactCooldownTime)
        {
            LastImpactVelocity = FVector::ZeroVector;
        }
    }
}

void UAnim_PhysicsBasedAnimation::InitializePhysicsBodies()
{
    if (!SkeletalMeshComp)
    {
        return;
    }
    
    // Enable physics simulation on specified bones
    for (const FName& BoneName : PhysicsBones)
    {
        if (SkeletalMeshComp->GetBoneIndex(BoneName) != INDEX_NONE)
        {
            // Set up physics properties for this bone
            SkeletalMeshComp->SetBodyNotifyRigidBodyCollision(true, BoneName);
            
            // Configure physics material properties
            FBodyInstance* BodyInstance = SkeletalMeshComp->GetBodyInstance(BoneName);
            if (BodyInstance)
            {
                BodyInstance->SetLinearDamping(BoneLinearDamping);
                BodyInstance->SetAngularDamping(BoneAngularDamping);
                BodyInstance->SetRestitution(BoneRestitution);
                BodyInstance->SetFriction(BoneFriction);
                
                UE_LOG(LogPhysicsAnimation, Log, TEXT("Initialized physics for bone: %s"), *BoneName.ToString());
            }
        }
        else
        {
            UE_LOG(LogPhysicsAnimation, Warning, TEXT("Bone not found: %s"), *BoneName.ToString());
        }
    }
}

void UAnim_PhysicsBasedAnimation::UpdatePhysicsSimulation(float DeltaTime)
{
    if (!SkeletalMeshComp || !MovementComponent)
    {
        return;
    }
    
    // Calculate physics blend weight based on movement state
    float TargetPhysicsWeight = CalculatePhysicsBlendWeight();
    
    // Smooth blend weight transition
    PhysicsBlendWeight = FMath::FInterpTo(PhysicsBlendWeight, TargetPhysicsWeight, DeltaTime, 5.0f);
    
    // Apply physics simulation to bones
    if (PhysicsBlendWeight > 0.0f)
    {
        ApplyPhysicsSimulation(DeltaTime);
    }
    
    // Update ragdoll blend weight
    if (bIsRagdollActive)
    {
        RagdollBlendWeight = FMath::FInterpTo(RagdollBlendWeight, 1.0f, DeltaTime, 3.0f);
    }
    else
    {
        RagdollBlendWeight = FMath::FInterpTo(RagdollBlendWeight, 0.0f, DeltaTime, 2.0f);
    }
}

float UAnim_PhysicsBasedAnimation::CalculatePhysicsBlendWeight() const
{
    if (!MovementComponent || !OwnerCharacter)
    {
        return 0.0f;
    }
    
    // Base weight on movement state
    float BaseWeight = 0.0f;
    
    if (MovementComponent->IsFalling())
    {
        // Increase physics simulation when falling
        BaseWeight = 0.8f;
    }
    else if (MovementComponent->IsMovingOnGround())
    {
        // Moderate physics simulation when moving
        float Speed = MovementComponent->Velocity.Size();
        float MaxSpeed = MovementComponent->GetMaxSpeed();
        BaseWeight = FMath::Clamp(Speed / MaxSpeed, 0.1f, 0.5f);
    }
    else
    {
        // Minimal physics simulation when idle
        BaseWeight = 0.1f;
    }
    
    // Increase weight if ragdoll is active
    if (bIsRagdollActive)
    {
        BaseWeight = FMath::Max(BaseWeight, 0.9f);
    }
    
    return BaseWeight;
}

void UAnim_PhysicsBasedAnimation::ApplyPhysicsSimulation(float DeltaTime)
{
    if (!SkeletalMeshComp || !MovementComponent)
    {
        return;
    }
    
    FVector CharacterVelocity = MovementComponent->Velocity;
    FVector CharacterAcceleration = (CharacterVelocity - PreviousVelocity) / DeltaTime;
    PreviousVelocity = CharacterVelocity;
    
    // Apply forces to physics bones based on character movement
    for (const FName& BoneName : PhysicsBones)
    {
        FBodyInstance* BodyInstance = SkeletalMeshComp->GetBodyInstance(BoneName);
        if (BodyInstance && BodyInstance->IsValidBodyInstance())
        {
            // Apply inertial forces
            FVector InertialForce = -CharacterAcceleration * BodyInstance->GetBodyMass() * PhysicsBlendWeight;
            
            // Add some randomness for natural movement
            FVector RandomForce = FMath::VRand() * 50.0f * PhysicsBlendWeight;
            
            FVector TotalForce = InertialForce + RandomForce;
            
            // Apply the force
            BodyInstance->AddForce(TotalForce, false, true);
            
            // Apply torque for rotational effects
            FVector RandomTorque = FMath::VRand() * 100.0f * PhysicsBlendWeight;
            BodyInstance->AddTorqueInRadians(RandomTorque, false, true);
        }
    }
}

void UAnim_PhysicsBasedAnimation::OnCharacterHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
    if (!bEnableImpactDetection || !GetWorld())
    {
        return;
    }
    
    float CurrentTime = GetWorld()->GetTimeSeconds();
    if (CurrentTime - LastImpactTime < ImpactCooldownTime)
    {
        return; // Still in cooldown
    }
    
    float ImpactMagnitude = NormalImpulse.Size();
    
    UE_LOG(LogPhysicsAnimation, Log, TEXT("Character hit detected - Impact: %f, Threshold: %f"), ImpactMagnitude, ImpactThreshold);
    
    if (ImpactMagnitude > ImpactThreshold)
    {
        // Significant impact detected
        LastImpactVelocity = NormalImpulse;
        LastImpactTime = CurrentTime;
        
        // Trigger ragdoll if impact is severe enough
        if (ImpactMagnitude > ImpactThreshold * 2.0f)
        {
            ActivateRagdoll(RecoveryTime);
        }
        
        // Apply impact forces to physics bones
        ApplyImpactForces(NormalImpulse, Hit.Location);
        
        UE_LOG(LogPhysicsAnimation, Log, TEXT("Impact processed - Magnitude: %f"), ImpactMagnitude);
    }
}

void UAnim_PhysicsBasedAnimation::ApplyImpactForces(const FVector& ImpactForce, const FVector& ImpactLocation)
{
    if (!SkeletalMeshComp)
    {
        return;
    }
    
    // Apply impact forces to nearby bones
    for (const FName& BoneName : PhysicsBones)
    {
        FBodyInstance* BodyInstance = SkeletalMeshComp->GetBodyInstance(BoneName);
        if (BodyInstance && BodyInstance->IsValidBodyInstance())
        {
            FVector BoneLocation = SkeletalMeshComp->GetBoneLocation(BoneName);
            float Distance = FVector::Dist(BoneLocation, ImpactLocation);
            
            // Apply force with falloff based on distance
            float ForceFalloff = FMath::Clamp(1.0f - (Distance / 200.0f), 0.1f, 1.0f);
            FVector ScaledForce = ImpactForce * ForceFalloff * 0.5f;
            
            BodyInstance->AddForceAtPosition(ScaledForce, ImpactLocation, false, true);
            
            // Add rotational component
            FVector TorqueDirection = (BoneLocation - ImpactLocation).GetSafeNormal();
            FVector Torque = FVector::CrossProduct(TorqueDirection, ImpactForce.GetSafeNormal()) * ImpactForce.Size() * 0.1f;
            BodyInstance->AddTorqueInRadians(Torque, false, true);
        }
    }
}

void UAnim_PhysicsBasedAnimation::ActivateRagdoll(float Duration)
{
    if (!SkeletalMeshComp || bIsRagdollActive)
    {
        return;
    }
    
    UE_LOG(LogPhysicsAnimation, Log, TEXT("Activating ragdoll for %f seconds"), Duration);
    
    bIsRagdollActive = true;
    bIsRecovering = false;
    CurrentRecoveryTime = Duration;
    
    // Enable physics simulation on all physics bones
    for (const FName& BoneName : PhysicsBones)
    {
        SkeletalMeshComp->SetAllBodiesBelowSimulatePhysics(BoneName, true, true);
    }
    
    // Disable character movement
    if (MovementComponent)
    {
        MovementComponent->SetMovementMode(MOVE_None);
    }
    
    // Broadcast ragdoll activation
    OnRagdollActivated.Broadcast();
}

void UAnim_PhysicsBasedAnimation::DeactivateRagdoll()
{
    if (!SkeletalMeshComp || !bIsRagdollActive)
    {
        return;
    }
    
    UE_LOG(LogPhysicsAnimation, Log, TEXT("Deactivating ragdoll"));
    
    bIsRagdollActive = false;
    bIsRecovering = true;
    CurrentRecoveryTime = 0.0f;
    
    // Disable physics simulation
    SkeletalMeshComp->SetSimulatePhysics(false);
    SkeletalMeshComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    
    // Re-enable character movement
    if (MovementComponent)
    {
        MovementComponent->SetMovementMode(MOVE_Walking);
    }
    
    // Broadcast ragdoll deactivation
    OnRagdollDeactivated.Broadcast();
}

void UAnim_PhysicsBasedAnimation::UpdateRagdollRecovery(float DeltaTime)
{
    CurrentRecoveryTime -= DeltaTime;
    
    if (CurrentRecoveryTime <= 0.0f)
    {
        bIsRecovering = false;
        UE_LOG(LogPhysicsAnimation, Log, TEXT("Ragdoll recovery completed"));
    }
}

void UAnim_PhysicsBasedAnimation::SetPhysicsBlendWeight(float NewWeight)
{
    PhysicsBlendWeight = FMath::Clamp(NewWeight, 0.0f, 1.0f);
}

void UAnim_PhysicsBasedAnimation::SetImpactThreshold(float NewThreshold)
{
    ImpactThreshold = FMath::Max(NewThreshold, 0.0f);
}

void UAnim_PhysicsBasedAnimation::EnableImpactDetection(bool bEnable)
{
    bEnableImpactDetection = bEnable;
    UE_LOG(LogPhysicsAnimation, Log, TEXT("Impact detection %s"), bEnable ? TEXT("enabled") : TEXT("disabled"));
}

bool UAnim_PhysicsBasedAnimation::IsRagdollActive() const
{
    return bIsRagdollActive;
}

float UAnim_PhysicsBasedAnimation::GetPhysicsBlendWeight() const
{
    return PhysicsBlendWeight;
}

float UAnim_PhysicsBasedAnimation::GetRagdollBlendWeight() const
{
    return RagdollBlendWeight;
}