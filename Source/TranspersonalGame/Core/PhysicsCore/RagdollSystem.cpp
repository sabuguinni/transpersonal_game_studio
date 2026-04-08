#include "RagdollSystem.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Character.h"
#include "PhysicsEngine/PhysicsAsset.h"
#include "PhysicsSystemManager.h"
#include "Engine/Engine.h"

URagdollSystemComponent::URagdollSystemComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    bWantsInitializeComponent = true;
    
    // Default ragdoll parameters
    RagdollMass = 75.0f; // Average human mass in kg
    LinearDamping = 0.1f;
    AngularDamping = 0.1f;
    AutoDeactivateTime = 10.0f;
    bRagdollActive = false;
}

void URagdollSystemComponent::BeginPlay()
{
    Super::BeginPlay();
    
    // Find the skeletal mesh component
    if (AActor* Owner = GetOwner())
    {
        SkeletalMeshComponent = Owner->FindComponentByClass<USkeletalMeshComponent>();
        
        if (!SkeletalMeshComponent)
        {
            // Try to get from Character
            if (ACharacter* Character = Cast<ACharacter>(Owner))
            {
                SkeletalMeshComponent = Character->GetMesh();
            }
        }
        
        if (!SkeletalMeshComponent)
        {
            UE_LOG(LogTemp, Warning, TEXT("RagdollSystem: No SkeletalMeshComponent found on %s"), *Owner->GetName());
            return;
        }
        
        // Ensure the mesh has a physics asset
        if (!SkeletalMeshComponent->GetPhysicsAsset())
        {
            UE_LOG(LogTemp, Warning, TEXT("RagdollSystem: SkeletalMeshComponent on %s has no PhysicsAsset"), *Owner->GetName());
        }
    }
    
    RegisterWithPhysicsSystem();
}

void URagdollSystemComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    UnregisterFromPhysicsSystem();
    
    // Clear any active timers
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(AutoDeactivateTimer);
    }
    
    // Ensure ragdoll is deactivated
    if (bRagdollActive)
    {
        DeactivateRagdoll();
    }
    
    Super::EndPlay(EndPlayReason);
}

void URagdollSystemComponent::ActivateRagdoll(const FVector& ImpulseLocation, float ImpulseStrength)
{
    if (bRagdollActive || !SkeletalMeshComponent)
    {
        return;
    }
    
    UE_LOG(LogTemp, Log, TEXT("RagdollSystem: Activating ragdoll for %s"), *GetOwner()->GetName());
    
    // Store original collision settings
    OriginalCollisionEnabled = SkeletalMeshComponent->GetCollisionEnabled();
    OriginalCollisionObjectType = SkeletalMeshComponent->GetCollisionObjectType();
    
    // Enable physics simulation
    SkeletalMeshComponent->SetSimulatePhysics(true);
    SkeletalMeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    SkeletalMeshComponent->SetCollisionObjectType(ECO_WorldDynamic);
    
    // Configure physics properties
    ConfigureRagdoll(RagdollMass, LinearDamping, AngularDamping);
    
    // Apply impulse if specified
    if (ImpulseStrength > 0.0f && ImpulseLocation != FVector::ZeroVector)
    {
        SkeletalMeshComponent->AddImpulseAtLocation(
            (SkeletalMeshComponent->GetComponentLocation() - ImpulseLocation).GetSafeNormal() * ImpulseStrength,
            ImpulseLocation
        );
    }
    
    bRagdollActive = true;
    
    // Set auto-deactivate timer if configured
    if (AutoDeactivateTime > 0.0f)
    {
        if (UWorld* World = GetWorld())
        {
            World->GetTimerManager().SetTimer(
                AutoDeactivateTimer,
                this,
                &URagdollSystemComponent::AutoDeactivateRagdoll,
                AutoDeactivateTime,
                false
            );
        }
    }
    
    // Notify physics system
    if (UPhysicsSystemManager* PhysicsManager = UPhysicsSystemManager::Get(GetWorld()))
    {
        PhysicsManager->OnRagdollActivated(this);
    }
}

void URagdollSystemComponent::DeactivateRagdoll()
{
    if (!bRagdollActive || !SkeletalMeshComponent)
    {
        return;
    }
    
    UE_LOG(LogTemp, Log, TEXT("RagdollSystem: Deactivating ragdoll for %s"), *GetOwner()->GetName());
    
    // Clear auto-deactivate timer
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(AutoDeactivateTimer);
    }
    
    // Disable physics simulation
    SkeletalMeshComponent->SetSimulatePhysics(false);
    
    // Restore original collision settings
    SkeletalMeshComponent->SetCollisionEnabled(OriginalCollisionEnabled);
    SkeletalMeshComponent->SetCollisionObjectType(OriginalCollisionObjectType);
    
    bRagdollActive = false;
    
    // Notify physics system
    if (UPhysicsSystemManager* PhysicsManager = UPhysicsSystemManager::Get(GetWorld()))
    {
        PhysicsManager->OnRagdollDeactivated(this);
    }
}

void URagdollSystemComponent::ConfigureRagdoll(float Mass, float LinearDamping, float AngularDamping)
{
    if (!SkeletalMeshComponent)
    {
        return;
    }
    
    RagdollMass = Mass;
    this->LinearDamping = LinearDamping;
    this->AngularDamping = AngularDamping;
    
    // Apply settings to all bodies
    TArray<FName> BoneNames;
    SkeletalMeshComponent->GetBoneNames(BoneNames);
    
    for (const FName& BoneName : BoneNames)
    {
        if (FBodyInstance* BodyInstance = SkeletalMeshComponent->GetBodyInstance(BoneName))
        {
            BodyInstance->SetMassOverride(Mass / BoneNames.Num()); // Distribute mass across bones
            BodyInstance->LinearDamping = LinearDamping;
            BodyInstance->AngularDamping = AngularDamping;
            BodyInstance->UpdateMassProperties();
        }
    }
}

void URagdollSystemComponent::RegisterWithPhysicsSystem()
{
    if (UPhysicsSystemManager* PhysicsManager = UPhysicsSystemManager::Get(GetWorld()))
    {
        PhysicsManager->RegisterRagdollSystem(this);
    }
}

void URagdollSystemComponent::UnregisterFromPhysicsSystem()
{
    if (UPhysicsSystemManager* PhysicsManager = UPhysicsSystemManager::Get(GetWorld()))
    {
        PhysicsManager->UnregisterRagdollSystem(this);
    }
}

void URagdollSystemComponent::AutoDeactivateRagdoll()
{
    UE_LOG(LogTemp, Log, TEXT("RagdollSystem: Auto-deactivating ragdoll for %s after %.1f seconds"), 
           *GetOwner()->GetName(), AutoDeactivateTime);
    
    DeactivateRagdoll();
}