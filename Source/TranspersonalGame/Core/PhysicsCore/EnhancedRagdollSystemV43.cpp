// Copyright Transpersonal Game Studio. All Rights Reserved.

#include "EnhancedRagdollSystemV43.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "Components/SkeletalMeshComponent.h"
#include "PhysicsEngine/PhysicsAsset.h"
#include "Components/CapsuleComponent.h"
#include "Animation/AnimInstance.h"
#include "PhysicsEngine/BodyInstance.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "Engine/LocalPlayer.h"
#include "Components/PoseableMeshComponent.h"
#include "PhysicsEngine/PhysicalAnimationComponent.h"

DEFINE_LOG_CATEGORY(LogRagdollV43);

UEnhancedRagdollSystemV43::UEnhancedRagdollSystemV43()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.016f; // 60 FPS for smooth ragdoll updates
    
    // Initialize default settings
    bRagdollEnabled = true;
    bAutoRecovery = true;
    AutoRecoveryTime = 5.0f;
    MinActivationVelocity = 500.0f;
    DefaultBlendTime = 0.2f;
    RecoveryBlendTime = 0.5f;
    LinearDamping = 0.1f;
    AngularDamping = 0.1f;
    MaxRagdollDistance = 5000.0f;
    bUseDistanceLOD = true;
    MaxSimultaneousRagdolls = 10;
    
    // Initialize state
    CurrentRagdollState = ERagdollState::Inactive;
    LastActivationReason = ERagdollActivationReason::Death;
    CurrentBlendTime = 0.0f;
    TargetBlendTime = 0.0f;
    BlendAlpha = 0.0f;
    TimeInCurrentState = 0.0f;
}

void UEnhancedRagdollSystemV43::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogRagdollV43, Log, TEXT("Enhanced Ragdoll System V43 initialized"));
    
    // Initialize the ragdoll system
    InitializeRagdollSystem();
}

void UEnhancedRagdollSystemV43::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    CleanupRagdollState();
    Super::EndPlay(EndPlayReason);
}

void UEnhancedRagdollSystemV43::TickComponent(float DeltaTime, ELevelTick TickType, 
    FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!bRagdollEnabled || !TargetMeshComponent)
    {
        return;
    }
    
    // Update ragdoll state machine
    UpdateRagdollStateMachine(DeltaTime);
    
    // Update time in current state
    TimeInCurrentState += DeltaTime;
}

void UEnhancedRagdollSystemV43::InitializeRagdollSystem()
{
    AActor* Owner = GetOwner();
    if (!Owner)
    {
        UE_LOG(LogRagdollV43, Error, TEXT("No owner actor found for ragdoll system"));
        return;
    }
    
    // Find the skeletal mesh component
    TargetMeshComponent = Owner->FindComponentByClass<USkeletalMeshComponent>();
    if (!TargetMeshComponent)
    {
        UE_LOG(LogRagdollV43, Error, TEXT("No skeletal mesh component found on owner actor"));
        return;
    }
    
    // Create physical animation component if it doesn't exist
    PhysicalAnimationComponent = Owner->FindComponentByClass<UPhysicalAnimationComponent>();
    if (!PhysicalAnimationComponent)
    {
        PhysicalAnimationComponent = NewObject<UPhysicalAnimationComponent>(Owner);
        Owner->AddInstanceComponent(PhysicalAnimationComponent);
        PhysicalAnimationComponent->RegisterComponent();
    }
    
    // Initialize bone name mappings
    InitializeBoneNameMappings();
    
    // Initialize body part masses
    InitializeBodyPartMasses();
    
    // Setup physical animation profiles
    SetupPhysicalAnimationProfiles();
    
    UE_LOG(LogRagdollV43, Log, TEXT("Ragdoll system initialized successfully for actor: %s"), 
        *Owner->GetName());
}

void UEnhancedRagdollSystemV43::ActivateRagdoll(ERagdollActivationReason Reason, float BlendTime)
{
    if (!bRagdollEnabled || !TargetMeshComponent || CurrentRagdollState == ERagdollState::Active)
    {
        return;
    }
    
    // Check distance LOD
    if (!ShouldUseRagdoll())
    {
        UE_LOG(LogRagdollV43, Log, TEXT("Ragdoll activation skipped due to distance LOD"));
        return;
    }
    
    UE_LOG(LogRagdollV43, Log, TEXT("Activating ragdoll with reason: %d"), (int32)Reason);
    
    LastActivationReason = Reason;
    CurrentRagdollState = ERagdollState::Activating;
    TargetBlendTime = BlendTime > 0.0f ? BlendTime : DefaultBlendTime;
    CurrentBlendTime = 0.0f;
    BlendAlpha = 0.0f;
    TimeInCurrentState = 0.0f;
    
    // Disable character collision capsule
    if (ACharacter* Character = Cast<ACharacter>(GetOwner()))
    {
        if (UCapsuleComponent* CapsuleComp = Character->GetCapsuleComponent())
        {
            CapsuleComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        }
    }
    
    // Apply initial physics settings
    ApplyPhysicsSettings();
    
    // Broadcast state change
    OnRagdollStateChanged.Broadcast(CurrentRagdollState, ERagdollState::Inactive, Reason);
}

void UEnhancedRagdollSystemV43::DeactivateRagdoll(float BlendTime)
{
    if (CurrentRagdollState == ERagdollState::Inactive || CurrentRagdollState == ERagdollState::Recovering)
    {
        return;
    }
    
    UE_LOG(LogRagdollV43, Log, TEXT("Deactivating ragdoll"));
    
    ERagdollState PreviousState = CurrentRagdollState;
    CurrentRagdollState = ERagdollState::Recovering;
    TargetBlendTime = BlendTime > 0.0f ? BlendTime : RecoveryBlendTime;
    CurrentBlendTime = 0.0f;
    TimeInCurrentState = 0.0f;
    
    // Broadcast state change
    OnRagdollStateChanged.Broadcast(CurrentRagdollState, PreviousState, LastActivationReason);
}

void UEnhancedRagdollSystemV43::ActivatePartialRagdoll(const TArray<EBodyPartType>& BodyParts, float Strength)
{
    if (!bRagdollEnabled || !TargetMeshComponent || !PhysicalAnimationComponent)
    {
        return;
    }
    
    UE_LOG(LogRagdollV43, Log, TEXT("Activating partial ragdoll for %d body parts"), BodyParts.Num());
    
    for (EBodyPartType BodyPart : BodyParts)
    {
        FName BoneName = GetBoneNameForBodyPart(BodyPart);
        if (BoneName != NAME_None)
        {
            // Apply physical animation to specific bone
            FPhysicalAnimationData PhysAnimData;
            PhysAnimData.bIsLocalSimulation = true;
            PhysAnimData.OrientationStrength = Strength * 1000.0f;
            PhysAnimData.AngularVelocityStrength = Strength * 100.0f;
            PhysAnimData.PositionStrength = Strength * 500.0f;
            PhysAnimData.VelocityStrength = Strength * 50.0f;
            
            PhysicalAnimationComponent->SetSkeletalMeshComponent(TargetMeshComponent);
            PhysicalAnimationComponent->ApplyPhysicalAnimationSettingsBelow(BoneName, PhysAnimData, true);
        }
    }
}

void UEnhancedRagdollSystemV43::ApplyRagdollImpulse(EBodyPartType BodyPart, FVector Impulse, bool bVelChange)
{
    if (!TargetMeshComponent)
    {
        return;
    }
    
    FName BoneName = GetBoneNameForBodyPart(BodyPart);
    if (BoneName != NAME_None)
    {
        TargetMeshComponent->AddImpulseAtLocation(Impulse, 
            TargetMeshComponent->GetBoneLocation(BoneName), BoneName);
        
        UE_LOG(LogRagdollV43, Log, TEXT("Applied impulse %.2f to body part %d"), 
            Impulse.Size(), (int32)BodyPart);
    }
}

void UEnhancedRagdollSystemV43::ApplyRagdollForce(FVector Force, FVector Location)
{
    if (!TargetMeshComponent)
    {
        return;
    }
    
    TargetMeshComponent->AddForceAtLocation(Force, Location);
    
    UE_LOG(LogRagdollV43, Log, TEXT("Applied force %.2f at location %.2f,%.2f,%.2f"), 
        Force.Size(), Location.X, Location.Y, Location.Z);
}

void UEnhancedRagdollSystemV43::SetRagdollPhysicsProperties(float LinearDampingValue, float AngularDampingValue)
{
    LinearDamping = LinearDampingValue;
    AngularDamping = AngularDampingValue;
    
    if (CurrentRagdollState == ERagdollState::Active)
    {
        ApplyPhysicsSettings();
    }
}

void UEnhancedRagdollSystemV43::SetRagdollCollisionEnabled(bool bEnabled)
{
    if (!TargetMeshComponent)
    {
        return;
    }
    
    ECollisionEnabled::Type CollisionType = bEnabled ? 
        ECollisionEnabled::QueryAndPhysics : ECollisionEnabled::NoCollision;
    
    TargetMeshComponent->SetCollisionEnabled(CollisionType);
}

void UEnhancedRagdollSystemV43::SetRagdollEnabled(bool bEnabled)
{
    bRagdollEnabled = bEnabled;
    
    if (!bEnabled && CurrentRagdollState != ERagdollState::Inactive)
    {
        DeactivateRagdoll();
    }
}

FName UEnhancedRagdollSystemV43::GetBoneNameForBodyPart(EBodyPartType BodyPart) const
{
    const FName* BoneName = BodyPartBoneNames.Find(BodyPart);
    return BoneName ? *BoneName : NAME_None;
}

FVector UEnhancedRagdollSystemV43::GetBodyPartVelocity(EBodyPartType BodyPart) const
{
    if (!TargetMeshComponent)
    {
        return FVector::ZeroVector;
    }
    
    FName BoneName = GetBoneNameForBodyPart(BodyPart);
    if (BoneName != NAME_None)
    {
        FBodyInstance* BodyInstance = TargetMeshComponent->GetBodyInstance(BoneName);
        if (BodyInstance)
        {
            return BodyInstance->GetUnrealWorldVelocity();
        }
    }
    
    return FVector::ZeroVector;
}

bool UEnhancedRagdollSystemV43::IsOnGround() const
{
    if (!TargetMeshComponent)
    {
        return false;
    }
    
    // Check if pelvis is close to ground
    FName PelvisBone = GetBoneNameForBodyPart(EBodyPartType::Pelvis);
    if (PelvisBone != NAME_None)
    {
        FVector PelvisLocation = TargetMeshComponent->GetBoneLocation(PelvisBone);
        
        // Perform ground trace
        FHitResult HitResult;
        FVector StartLocation = PelvisLocation;
        FVector EndLocation = PelvisLocation - FVector(0, 0, 100); // 1m down
        
        UWorld* World = GetWorld();
        if (World && World->LineTraceSingleByChannel(HitResult, StartLocation, EndLocation, 
            ECollisionChannel::ECC_WorldStatic))
        {
            return HitResult.Distance < 50.0f; // Within 50cm of ground
        }
    }
    
    return false;
}

void UEnhancedRagdollSystemV43::ForceRecovery()
{
    if (CurrentRagdollState != ERagdollState::Inactive)
    {
        DeactivateRagdoll(0.1f); // Quick recovery
    }
}

void UEnhancedRagdollSystemV43::InitializeBoneNameMappings()
{
    // Default bone name mappings for humanoid characters
    // These can be overridden in Blueprint or by specific character types
    BodyPartBoneNames.Add(EBodyPartType::Head, FName(TEXT("head")));
    BodyPartBoneNames.Add(EBodyPartType::Torso, FName(TEXT("spine_03")));
    BodyPartBoneNames.Add(EBodyPartType::LeftArm, FName(TEXT("upperarm_l")));
    BodyPartBoneNames.Add(EBodyPartType::RightArm, FName(TEXT("upperarm_r")));
    BodyPartBoneNames.Add(EBodyPartType::LeftLeg, FName(TEXT("thigh_l")));
    BodyPartBoneNames.Add(EBodyPartType::RightLeg, FName(TEXT("thigh_r")));
    BodyPartBoneNames.Add(EBodyPartType::Spine, FName(TEXT("spine_01")));
    BodyPartBoneNames.Add(EBodyPartType::Pelvis, FName(TEXT("pelvis")));
}

void UEnhancedRagdollSystemV43::InitializeBodyPartMasses()
{
    // Realistic body part mass distribution (kg)
    BodyPartMasses.Add(EBodyPartType::Head, 5.0f);
    BodyPartMasses.Add(EBodyPartType::Torso, 25.0f);
    BodyPartMasses.Add(EBodyPartType::LeftArm, 3.5f);
    BodyPartMasses.Add(EBodyPartType::RightArm, 3.5f);
    BodyPartMasses.Add(EBodyPartType::LeftLeg, 8.0f);
    BodyPartMasses.Add(EBodyPartType::RightLeg, 8.0f);
    BodyPartMasses.Add(EBodyPartType::Spine, 15.0f);
    BodyPartMasses.Add(EBodyPartType::Pelvis, 10.0f);
}

void UEnhancedRagdollSystemV43::UpdateRagdollStateMachine(float DeltaTime)
{
    switch (CurrentRagdollState)
    {
        case ERagdollState::Activating:
            HandleRagdollActivation(DeltaTime);
            break;
            
        case ERagdollState::Active:
            // Check for auto-recovery
            if (bAutoRecovery && TimeInCurrentState >= AutoRecoveryTime && IsOnGround())
            {
                DeactivateRagdoll();
            }
            break;
            
        case ERagdollState::Recovering:
            HandleRagdollRecovery(DeltaTime);
            break;
            
        case ERagdollState::Blending:
            // Handle blending between ragdoll and animation
            break;
            
        case ERagdollState::Inactive:
        default:
            // Nothing to do
            break;
    }
}

void UEnhancedRagdollSystemV43::HandleRagdollActivation(float DeltaTime)
{
    CurrentBlendTime += DeltaTime;
    BlendAlpha = FMath::Clamp(CurrentBlendTime / TargetBlendTime, 0.0f, 1.0f);
    
    if (BlendAlpha >= 1.0f)
    {
        // Transition to fully active ragdoll
        ERagdollState PreviousState = CurrentRagdollState;
        CurrentRagdollState = ERagdollState::Active;
        TimeInCurrentState = 0.0f;
        
        // Enable full physics simulation
        if (TargetMeshComponent)
        {
            TargetMeshComponent->SetSimulatePhysics(true);
            TargetMeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        }
        
        // Broadcast state change
        OnRagdollStateChanged.Broadcast(CurrentRagdollState, PreviousState, LastActivationReason);
        
        UE_LOG(LogRagdollV43, Log, TEXT("Ragdoll fully activated"));
    }
}

void UEnhancedRagdollSystemV43::HandleRagdollRecovery(float DeltaTime)
{
    CurrentBlendTime += DeltaTime;
    BlendAlpha = 1.0f - FMath::Clamp(CurrentBlendTime / TargetBlendTime, 0.0f, 1.0f);
    
    if (BlendAlpha <= 0.0f)
    {
        // Transition to inactive
        ERagdollState PreviousState = CurrentRagdollState;
        CurrentRagdollState = ERagdollState::Inactive;
        TimeInCurrentState = 0.0f;
        
        // Disable physics simulation
        if (TargetMeshComponent)
        {
            TargetMeshComponent->SetSimulatePhysics(false);
            TargetMeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
        }
        
        // Re-enable character collision capsule
        if (ACharacter* Character = Cast<ACharacter>(GetOwner()))
        {
            if (UCapsuleComponent* CapsuleComp = Character->GetCapsuleComponent())
            {
                CapsuleComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
            }
        }
        
        // Broadcast state change
        OnRagdollStateChanged.Broadcast(CurrentRagdollState, PreviousState, LastActivationReason);
        
        UE_LOG(LogRagdollV43, Log, TEXT("Ragdoll recovery complete"));
    }
}

void UEnhancedRagdollSystemV43::ApplyPhysicsSettings()
{
    if (!TargetMeshComponent)
    {
        return;
    }
    
    // Apply damping settings to all body parts
    for (const auto& BodyPartPair : BodyPartBoneNames)
    {
        FBodyInstance* BodyInstance = GetBodyInstanceForBodyPart(BodyPartPair.Key);
        if (BodyInstance)
        {
            BodyInstance->LinearDamping = LinearDamping;
            BodyInstance->AngularDamping = AngularDamping;
            
            // Set mass if specified
            const float* Mass = BodyPartMasses.Find(BodyPartPair.Key);
            if (Mass)
            {
                BodyInstance->SetMassOverride(*Mass, true);
            }
        }
    }
}

bool UEnhancedRagdollSystemV43::ShouldUseRagdoll() const
{
    if (!bUseDistanceLOD)
    {
        return true;
    }
    
    float PlayerDistance = GetPlayerDistance();
    return PlayerDistance <= MaxRagdollDistance;
}

float UEnhancedRagdollSystemV43::GetPlayerDistance() const
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return 0.0f;
    }
    
    APlayerController* PlayerController = World->GetFirstPlayerController();
    if (!PlayerController || !PlayerController->GetPawn())
    {
        return 0.0f;
    }
    
    FVector PlayerLocation = PlayerController->GetPawn()->GetActorLocation();
    FVector OwnerLocation = GetOwner()->GetActorLocation();
    
    return FVector::Dist(PlayerLocation, OwnerLocation);
}

void UEnhancedRagdollSystemV43::SetupPhysicalAnimationProfiles()
{
    if (!PhysicalAnimationComponent || !TargetMeshComponent)
    {
        return;
    }
    
    PhysicalAnimationComponent->SetSkeletalMeshComponent(TargetMeshComponent);
    
    // Setup default physical animation profile
    FPhysicalAnimationData DefaultProfile;
    DefaultProfile.bIsLocalSimulation = false;
    DefaultProfile.OrientationStrength = 1000.0f;
    DefaultProfile.AngularVelocityStrength = 100.0f;
    DefaultProfile.PositionStrength = 500.0f;
    DefaultProfile.VelocityStrength = 50.0f;
    DefaultProfile.MaxLinearForce = 10000.0f;
    DefaultProfile.MaxAngularForce = 1000.0f;
    
    // Apply to all bones
    PhysicalAnimationComponent->ApplyPhysicalAnimationSettings(FName(TEXT("pelvis")), DefaultProfile);
}

void UEnhancedRagdollSystemV43::CleanupRagdollState()
{
    if (CurrentRagdollState != ERagdollState::Inactive)
    {
        ForceRecovery();
    }
}

FBodyInstance* UEnhancedRagdollSystemV43::GetBodyInstanceForBodyPart(EBodyPartType BodyPart) const
{
    if (!TargetMeshComponent)
    {
        return nullptr;
    }
    
    FName BoneName = GetBoneNameForBodyPart(BodyPart);
    if (BoneName != NAME_None)
    {
        return TargetMeshComponent->GetBodyInstance(BoneName);
    }
    
    return nullptr;
}