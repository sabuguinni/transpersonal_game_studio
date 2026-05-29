#include "Core_RagdollPhysics.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/KismetMathLibrary.h"
#include "PhysicsEngine/PhysicsSettings.h"

UCore_RagdollPhysics::UCore_RagdollPhysics()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PostPhysics;
    
    // Initialize default configuration
    RagdollConfig = FCore_RagdollConfiguration();
    RagdollState = FCore_RagdollState();
    
    SkeletalMeshComponent = nullptr;
    AnimInstance = nullptr;
    DefaultPhysicsAsset = nullptr;
    
    RecoveryTimer = 0.0f;
    bRecoveryPending = false;
    PreRagdollLocation = FVector::ZeroVector;
    PreRagdollRotation = FRotator::ZeroRotator;
}

void UCore_RagdollPhysics::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeSkeletalMeshReference();
    
    if (SkeletalMeshComponent)
    {
        // Bind to hit events for impact detection
        SkeletalMeshComponent->OnComponentHit.AddDynamic(this, &UCore_RagdollPhysics::OnSkeletalMeshHit);
        
        // Store default physics asset if not set
        if (!DefaultPhysicsAsset && SkeletalMeshComponent->GetPhysicsAsset())
        {
            DefaultPhysicsAsset = SkeletalMeshComponent->GetPhysicsAsset();
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Core_RagdollPhysics: Component initialized for %s"), 
           GetOwner() ? *GetOwner()->GetName() : TEXT("Unknown"));
}

void UCore_RagdollPhysics::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (RagdollState.bIsRagdollActive)
    {
        RagdollState.RagdollTime += DeltaTime;
        UpdateRagdollBlending(DeltaTime);
        
        if (RagdollConfig.bAutoRecover)
        {
            CheckAutoRecovery(DeltaTime);
        }
        
        // Force deactivation if max time exceeded
        if (RagdollState.RagdollTime >= RagdollConfig.MaxRagdollTime)
        {
            UE_LOG(LogTemp, Warning, TEXT("Core_RagdollPhysics: Max ragdoll time exceeded, forcing recovery"));
            DeactivateRagdoll();
        }
    }
    else if (bRecoveryPending)
    {
        RecoveryTimer += DeltaTime;
        if (RecoveryTimer >= RagdollConfig.BlendOutTime)
        {
            bRecoveryPending = false;
            RecoveryTimer = 0.0f;
            RestoreAnimationState();
        }
    }
}

void UCore_RagdollPhysics::ActivateRagdoll(const FVector& ImpactLocation, float ImpactMagnitude)
{
    if (!SkeletalMeshComponent || RagdollState.bIsRagdollActive)
    {
        return;
    }
    
    UE_LOG(LogTemp, Log, TEXT("Core_RagdollPhysics: Activating ragdoll for %s"), 
           GetOwner() ? *GetOwner()->GetName() : TEXT("Unknown"));
    
    // Save current state
    SavePreRagdollTransform();
    RagdollState.LastImpactLocation = ImpactLocation;
    RagdollState.LastImpactMagnitude = ImpactMagnitude;
    
    // Disable character movement
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
    
    // Enable physics simulation
    SkeletalMeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    SkeletalMeshComponent->SetSimulatePhysics(true);
    SkeletalMeshComponent->SetCollisionObjectType(ECollisionChannel::ECC_PhysicsBody);
    
    // Apply physics settings
    ApplyPhysicsSettings();
    
    // Apply impact impulse if provided
    if (ImpactMagnitude > 0.0f && ImpactLocation != FVector::ZeroVector)
    {
        FVector ImpactDirection = (ImpactLocation - GetOwner()->GetActorLocation()).GetSafeNormal();
        FVector ImpactForce = ImpactDirection * ImpactMagnitude;
        SkeletalMeshComponent->AddImpulseAtLocation(ImpactForce, ImpactLocation);
    }
    
    // Update state
    RagdollState.bIsRagdollActive = true;
    RagdollState.RagdollTime = 0.0f;
    RagdollState.BlendAlpha = 0.0f;
    RecoveryTimer = 0.0f;
    bRecoveryPending = false;
}

void UCore_RagdollPhysics::DeactivateRagdoll()
{
    if (!RagdollState.bIsRagdollActive)
    {
        return;
    }
    
    UE_LOG(LogTemp, Log, TEXT("Core_RagdollPhysics: Deactivating ragdoll for %s"), 
           GetOwner() ? *GetOwner()->GetName() : TEXT("Unknown"));
    
    // Disable physics simulation
    if (SkeletalMeshComponent)
    {
        SkeletalMeshComponent->SetSimulatePhysics(false);
        SkeletalMeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    }
    
    // Re-enable character movement
    if (ACharacter* Character = Cast<ACharacter>(GetOwner()))
    {
        if (UCharacterMovementComponent* MovementComp = Character->GetCharacterMovement())
        {
            MovementComp->SetMovementMode(MOVE_Walking);
        }
        
        // Re-enable collision on capsule component
        if (UCapsuleComponent* CapsuleComp = Character->GetCapsuleComponent())
        {
            CapsuleComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        }
        
        // Position character at ragdoll location
        FVector RagdollLocation = SkeletalMeshComponent->GetComponentLocation();
        Character->SetActorLocation(RagdollLocation);
    }
    
    // Update state
    RagdollState.bIsRagdollActive = false;
    bRecoveryPending = true;
    RecoveryTimer = 0.0f;
}

void UCore_RagdollPhysics::SetRagdollConfiguration(const FCore_RagdollConfiguration& NewConfig)
{
    RagdollConfig = NewConfig;
    
    if (RagdollState.bIsRagdollActive)
    {
        ApplyPhysicsSettings();
    }
}

void UCore_RagdollPhysics::OnImpactReceived(const FVector& ImpactLocation, float ImpactMagnitude, AActor* Instigator)
{
    if (ImpactMagnitude >= RagdollConfig.ImpactThreshold)
    {
        UE_LOG(LogTemp, Log, TEXT("Core_RagdollPhysics: Impact threshold exceeded (%.2f >= %.2f), activating ragdoll"), 
               ImpactMagnitude, RagdollConfig.ImpactThreshold);
        ActivateRagdoll(ImpactLocation, ImpactMagnitude);
    }
}

void UCore_RagdollPhysics::ApplyBiomeRagdollModifiers(EBiomeType BiomeType)
{
    switch (BiomeType)
    {
        case EBiomeType::Savanna:
            ApplySavannaRagdollSettings();
            break;
        case EBiomeType::Swamp:
            ApplySwampRagdollSettings();
            break;
        case EBiomeType::Forest:
            ApplyForestRagdollSettings();
            break;
        case EBiomeType::Desert:
            ApplyDesertRagdollSettings();
            break;
        case EBiomeType::Mountain:
            ApplyMountainRagdollSettings();
            break;
        default:
            break;
    }
}

void UCore_RagdollPhysics::ForceRecovery()
{
    if (RagdollState.bIsRagdollActive)
    {
        DeactivateRagdoll();
    }
}

bool UCore_RagdollPhysics::CanRecover() const
{
    return RagdollState.bIsRagdollActive && 
           RagdollState.RagdollTime >= RagdollConfig.RecoveryDelay;
}

void UCore_RagdollPhysics::SetPhysicsAsset(UPhysicsAsset* NewPhysicsAsset)
{
    if (SkeletalMeshComponent && NewPhysicsAsset)
    {
        SkeletalMeshComponent->SetPhysicsAsset(NewPhysicsAsset);
        DefaultPhysicsAsset = NewPhysicsAsset;
        
        if (RagdollState.bIsRagdollActive)
        {
            ConfigurePhysicsBodies();
        }
    }
}

void UCore_RagdollPhysics::ConfigurePhysicsBodies()
{
    if (!SkeletalMeshComponent || !SkeletalMeshComponent->GetPhysicsAsset())
    {
        return;
    }
    
    // Configure physics bodies for optimal ragdoll behavior
    TArray<FName> BoneNames;
    SkeletalMeshComponent->GetBoneNames(BoneNames);
    
    for (const FName& BoneName : BoneNames)
    {
        int32 BoneIndex = SkeletalMeshComponent->GetBoneIndex(BoneName);
        if (BoneIndex != INDEX_NONE)
        {
            FBodyInstance* BodyInstance = SkeletalMeshComponent->GetBodyInstance(BoneName);
            if (BodyInstance)
            {
                BodyInstance->SetLinearDamping(RagdollConfig.LinearDamping);
                BodyInstance->SetAngularDamping(RagdollConfig.AngularDamping);
            }
        }
    }
}

void UCore_RagdollPhysics::UpdateRagdollBlending(float DeltaTime)
{
    if (RagdollState.RagdollTime < RagdollConfig.BlendInTime)
    {
        BlendToRagdoll(DeltaTime);
    }
    else if (bRecoveryPending)
    {
        BlendFromRagdoll(DeltaTime);
    }
}

void UCore_RagdollPhysics::CheckAutoRecovery(float DeltaTime)
{
    if (!CanRecover())
    {
        return;
    }
    
    // Check if ragdoll has settled (low velocity)
    if (SkeletalMeshComponent)
    {
        FVector Velocity = SkeletalMeshComponent->GetPhysicsLinearVelocity();
        float Speed = Velocity.Size();
        
        if (Speed < 50.0f) // Threshold for "settled"
        {
            RecoveryTimer += DeltaTime;
            if (RecoveryTimer >= 1.0f) // Wait 1 second after settling
            {
                DeactivateRagdoll();
            }
        }
        else
        {
            RecoveryTimer = 0.0f;
        }
    }
}

void UCore_RagdollPhysics::BlendToRagdoll(float DeltaTime)
{
    float BlendProgress = RagdollState.RagdollTime / RagdollConfig.BlendInTime;
    RagdollState.BlendAlpha = FMath::Clamp(BlendProgress, 0.0f, 1.0f);
}

void UCore_RagdollPhysics::BlendFromRagdoll(float DeltaTime)
{
    float BlendProgress = RecoveryTimer / RagdollConfig.BlendOutTime;
    RagdollState.BlendAlpha = FMath::Clamp(1.0f - BlendProgress, 0.0f, 1.0f);
}

void UCore_RagdollPhysics::InitializeSkeletalMeshReference()
{
    if (ACharacter* Character = Cast<ACharacter>(GetOwner()))
    {
        SkeletalMeshComponent = Character->GetMesh();
        if (SkeletalMeshComponent)
        {
            AnimInstance = SkeletalMeshComponent->GetAnimInstance();
        }
    }
    else
    {
        // Try to find skeletal mesh component on the owner
        SkeletalMeshComponent = GetOwner()->FindComponentByClass<USkeletalMeshComponent>();
        if (SkeletalMeshComponent)
        {
            AnimInstance = SkeletalMeshComponent->GetAnimInstance();
        }
    }
}

void UCore_RagdollPhysics::ApplyPhysicsSettings()
{
    if (!SkeletalMeshComponent)
    {
        return;
    }
    
    ConfigurePhysicsBodies();
    
    // Set collision responses
    SkeletalMeshComponent->SetCollisionResponseToAllChannels(ECR_Block);
    SkeletalMeshComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
}

void UCore_RagdollPhysics::RestoreAnimationState()
{
    if (SkeletalMeshComponent && AnimInstance)
    {
        // Reset animation blueprint state
        AnimInstance->Montage_Stop(0.0f);
    }
}

void UCore_RagdollPhysics::SavePreRagdollTransform()
{
    if (GetOwner())
    {
        PreRagdollLocation = GetOwner()->GetActorLocation();
        PreRagdollRotation = GetOwner()->GetActorRotation();
    }
}

void UCore_RagdollPhysics::ApplySavannaRagdollSettings()
{
    // Savanna: Normal physics, good for rolling on grass
    RagdollConfig.LinearDamping = 0.1f;
    RagdollConfig.AngularDamping = 0.1f;
    RagdollConfig.RecoveryDelay = 2.0f;
}

void UCore_RagdollPhysics::ApplySwampRagdollSettings()
{
    // Swamp: Higher damping due to mud and water
    RagdollConfig.LinearDamping = 0.3f;
    RagdollConfig.AngularDamping = 0.3f;
    RagdollConfig.RecoveryDelay = 3.0f;
}

void UCore_RagdollPhysics::ApplyForestRagdollSettings()
{
    // Forest: Medium damping, obstacles may extend ragdoll time
    RagdollConfig.LinearDamping = 0.15f;
    RagdollConfig.AngularDamping = 0.15f;
    RagdollConfig.RecoveryDelay = 2.5f;
}

void UCore_RagdollPhysics::ApplyDesertRagdollSettings()
{
    // Desert: Low damping on sand, quick recovery
    RagdollConfig.LinearDamping = 0.05f;
    RagdollConfig.AngularDamping = 0.05f;
    RagdollConfig.RecoveryDelay = 1.5f;
}

void UCore_RagdollPhysics::ApplyMountainRagdollSettings()
{
    // Mountain: High damping on rocky terrain, longer recovery
    RagdollConfig.LinearDamping = 0.2f;
    RagdollConfig.AngularDamping = 0.2f;
    RagdollConfig.RecoveryDelay = 3.5f;
}

void UCore_RagdollPhysics::OnSkeletalMeshHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
    if (!RagdollState.bIsRagdollActive)
    {
        float ImpactMagnitude = NormalImpulse.Size();
        OnImpactReceived(Hit.Location, ImpactMagnitude, OtherActor);
    }
}