#include "Core_RagdollSystem.h"
#include "Engine/Engine.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "PhysicsEngine/PhysicsAsset.h"
#include "TimerManager.h"
#include "Engine/World.h"

UCore_RagdollSystem::UCore_RagdollSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = true;
    
    // Initialize state
    CurrentState = ECore_RagdollState::None;
    StateTimer = 0.0f;
    bIsInitialized = false;
    BlendAlpha = 0.0f;
    
    // Set default configuration optimized for survival gameplay
    RagdollConfig = FCore_RagdollConfig();
    RagdollConfig.ActivationForce = 750.0f; // Higher force for dramatic dinosaur attacks
    RagdollConfig.RecoveryTime = 4.0f; // Longer recovery for tension
    RagdollConfig.BlendInTime = 0.15f; // Quick transition for impact
    RagdollConfig.BlendOutTime = 1.5f; // Slower recovery for realism
    RagdollConfig.bAutoRecover = true;
    RagdollConfig.bPreserveVelocity = true;
}

void UCore_RagdollSystem::BeginPlay()
{
    Super::BeginPlay();
    InitializeComponents();
    ConfigureForSurvivalGameplay();
}

void UCore_RagdollSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (bIsInitialized && CurrentState != ECore_RagdollState::None)
    {
        UpdateRagdollState(DeltaTime);
    }
}

void UCore_RagdollSystem::InitializeComponents()
{
    if (AActor* Owner = GetOwner())
    {
        // Find skeletal mesh component
        SkeletalMeshComponent = Owner->FindComponentByClass<USkeletalMeshComponent>();
        
        if (SkeletalMeshComponent)
        {
            AnimInstance = SkeletalMeshComponent->GetAnimInstance();
            bIsInitialized = true;
            
            UE_LOG(LogTemp, Log, TEXT("Core_RagdollSystem: Initialized for %s"), *Owner->GetName());
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("Core_RagdollSystem: No SkeletalMeshComponent found on %s"), *Owner->GetName());
        }
    }
}

void UCore_RagdollSystem::ConfigureForSurvivalGameplay()
{
    if (!SkeletalMeshComponent)
        return;
    
    // Configure physics for realistic survival gameplay
    SkeletalMeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    SkeletalMeshComponent->SetCollisionObjectType(ECollisionChannel::ECC_Pawn);
    SkeletalMeshComponent->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
    
    // Set physics properties for dramatic but stable ragdoll
    SkeletalMeshComponent->SetLinearDamping(0.1f); // Low damping for momentum preservation
    SkeletalMeshComponent->SetAngularDamping(0.05f);
    
    ApplySurvivalPhysicsSettings();
}

void UCore_RagdollSystem::ApplySurvivalPhysicsSettings()
{
    if (!SkeletalMeshComponent || !SkeletalMeshComponent->GetPhysicsAsset())
        return;
    
    // Configure physics asset for survival gameplay
    UPhysicsAsset* PhysicsAsset = SkeletalMeshComponent->GetPhysicsAsset();
    
    // Set realistic mass distribution
    SkeletalMeshComponent->SetMassScale(NAME_None, 80.0f); // Average human mass
    
    // Configure bone-specific physics for realism
    SkeletalMeshComponent->SetMassScale(FName("head"), 0.1f); // Lighter head
    SkeletalMeshComponent->SetMassScale(FName("spine"), 0.3f); // Heavier torso
    SkeletalMeshComponent->SetMassScale(FName("pelvis"), 0.4f); // Heavy pelvis
}

void UCore_RagdollSystem::ActivateRagdoll(const FVector& ImpactForce, const FName& BoneName)
{
    if (!bIsInitialized || !SkeletalMeshComponent)
    {
        UE_LOG(LogTemp, Warning, TEXT("Core_RagdollSystem: Cannot activate ragdoll - not initialized"));
        return;
    }
    
    if (CurrentState == ECore_RagdollState::Active)
    {
        // Already active, just apply additional force
        ApplyImpactForce(ImpactForce, SkeletalMeshComponent->GetComponentLocation(), BoneName);
        return;
    }
    
    // Store pre-ragdoll state
    if (AActor* Owner = GetOwner())
    {
        PreRagdollTransform = Owner->GetActorTransform();
        
        if (ACharacter* Character = Cast<ACharacter>(Owner))
        {
            if (UCharacterMovementComponent* MovementComp = Character->GetCharacterMovement())
            {
                PreRagdollVelocity = MovementComp->Velocity;
                MovementComp->SetMovementMode(MOVE_None); // Disable character movement
            }
        }
    }
    
    // Transition to ragdoll
    CurrentState = ECore_RagdollState::Activating;
    StateTimer = 0.0f;
    
    BlendToRagdoll();
    
    // Apply impact force if provided
    if (!ImpactForce.IsZero())
    {
        ApplyImpactForce(ImpactForce, SkeletalMeshComponent->GetComponentLocation(), BoneName);
    }
    
    UE_LOG(LogTemp, Log, TEXT("Core_RagdollSystem: Ragdoll activated for %s"), *GetOwner()->GetName());
}

void UCore_RagdollSystem::DeactivateRagdoll()
{
    if (CurrentState == ECore_RagdollState::None || CurrentState == ECore_RagdollState::Recovering)
        return;
    
    CurrentState = ECore_RagdollState::Recovering;
    StateTimer = 0.0f;
    
    BlendFromRagdoll();
    
    UE_LOG(LogTemp, Log, TEXT("Core_RagdollSystem: Ragdoll deactivated for %s"), *GetOwner()->GetName());
}

void UCore_RagdollSystem::BlendToRagdoll()
{
    if (!SkeletalMeshComponent)
        return;
    
    // Enable physics simulation
    SkeletalMeshComponent->SetSimulatePhysics(true);
    SkeletalMeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    
    // Disable animation blueprint temporarily
    if (AnimInstance)
    {
        SkeletalMeshComponent->SetAnimInstanceClass(nullptr);
    }
    
    ApplyPhysicsConfiguration();
}

void UCore_RagdollSystem::BlendFromRagdoll()
{
    if (!SkeletalMeshComponent)
        return;
    
    // Gradually restore animation control
    RestoreAnimationControl();
    
    // Re-enable character movement
    if (AActor* Owner = GetOwner())
    {
        if (ACharacter* Character = Cast<ACharacter>(Owner))
        {
            if (UCharacterMovementComponent* MovementComp = Character->GetCharacterMovement())
            {
                MovementComp->SetMovementMode(MOVE_Walking);
            }
        }
    }
}

void UCore_RagdollSystem::ApplyPhysicsConfiguration()
{
    if (!SkeletalMeshComponent)
        return;
    
    // Set physics properties for dramatic ragdoll
    SkeletalMeshComponent->SetPhysicsLinearVelocity(PreRagdollVelocity * 0.5f); // Preserve some momentum
    SkeletalMeshComponent->SetEnableGravity(true);
    
    // Configure collision for survival gameplay
    SkeletalMeshComponent->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldStatic, ECollisionResponse::ECR_Block);
    SkeletalMeshComponent->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldDynamic, ECollisionResponse::ECR_Block);
}

void UCore_RagdollSystem::RestoreAnimationControl()
{
    if (!SkeletalMeshComponent)
        return;
    
    // Disable physics simulation
    SkeletalMeshComponent->SetSimulatePhysics(false);
    SkeletalMeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    
    // Restore animation instance if it was stored
    if (AnimInstance)
    {
        SkeletalMeshComponent->SetAnimInstanceClass(AnimInstance->GetClass());
    }
}

void UCore_RagdollSystem::UpdateRagdollState(float DeltaTime)
{
    StateTimer += DeltaTime;
    
    switch (CurrentState)
    {
        case ECore_RagdollState::Activating:
            BlendAlpha = FMath::Clamp(StateTimer / RagdollConfig.BlendInTime, 0.0f, 1.0f);
            if (StateTimer >= RagdollConfig.BlendInTime)
            {
                CurrentState = ECore_RagdollState::Active;
                StateTimer = 0.0f;
                
                // Set auto-recovery timer if enabled
                if (RagdollConfig.bAutoRecover)
                {
                    GetWorld()->GetTimerManager().SetTimer(
                        RecoveryTimerHandle,
                        this,
                        &UCore_RagdollSystem::DeactivateRagdoll,
                        RagdollConfig.RecoveryTime,
                        false
                    );
                }
            }
            break;
            
        case ECore_RagdollState::Active:
            // Ragdoll is fully active - physics simulation handles everything
            break;
            
        case ECore_RagdollState::Recovering:
            BlendAlpha = 1.0f - FMath::Clamp(StateTimer / RagdollConfig.BlendOutTime, 0.0f, 1.0f);
            if (StateTimer >= RagdollConfig.BlendOutTime)
            {
                CurrentState = ECore_RagdollState::None;
                StateTimer = 0.0f;
                BlendAlpha = 0.0f;
            }
            break;
            
        default:
            break;
    }
}

void UCore_RagdollSystem::ApplyImpactForce(const FVector& Force, const FVector& Location, const FName& BoneName)
{
    if (!SkeletalMeshComponent || !SkeletalMeshComponent->IsSimulatingPhysics())
        return;
    
    if (BoneName != NAME_None)
    {
        // Apply force to specific bone
        SkeletalMeshComponent->AddImpulseAtLocation(Force, Location, BoneName);
    }
    else
    {
        // Apply force to center of mass
        SkeletalMeshComponent->AddImpulse(Force);
    }
    
    UE_LOG(LogTemp, Log, TEXT("Core_RagdollSystem: Applied impact force %s to %s"), 
           *Force.ToString(), BoneName != NAME_None ? *BoneName.ToString() : TEXT("center of mass"));
}

void UCore_RagdollSystem::TriggerDeathRagdoll()
{
    // Death ragdoll - no auto-recovery
    FCore_RagdollConfig DeathConfig = RagdollConfig;
    DeathConfig.bAutoRecover = false;
    DeathConfig.ActivationForce = 1000.0f; // Strong force for dramatic death
    
    SetRagdollConfig(DeathConfig);
    ActivateRagdoll(FVector(0, 0, -500)); // Downward force for collapse
}

void UCore_RagdollSystem::TriggerUnconsciousnessRagdoll()
{
    // Unconsciousness ragdoll - longer recovery time
    FCore_RagdollConfig UnconsciousConfig = RagdollConfig;
    UnconsciousConfig.RecoveryTime = 8.0f; // Longer unconsciousness period
    UnconsciousConfig.bAutoRecover = true;
    
    SetRagdollConfig(UnconsciousConfig);
    ActivateRagdoll(FVector(0, 0, -300)); // Moderate downward force
}

void UCore_RagdollSystem::TriggerImpactRagdoll(const FVector& ImpactDirection, float ImpactMagnitude)
{
    // Impact ragdoll - quick recovery for gameplay flow
    FCore_RagdollConfig ImpactConfig = RagdollConfig;
    ImpactConfig.RecoveryTime = 2.0f; // Quick recovery
    ImpactConfig.bAutoRecover = true;
    
    SetRagdollConfig(ImpactConfig);
    
    FVector ImpactForce = ImpactDirection.GetSafeNormal() * ImpactMagnitude;
    ActivateRagdoll(ImpactForce);
}

void UCore_RagdollSystem::SetRagdollConfig(const FCore_RagdollConfig& NewConfig)
{
    RagdollConfig = NewConfig;
}