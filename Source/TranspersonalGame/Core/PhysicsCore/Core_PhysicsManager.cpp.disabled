#include "Core_PhysicsManager.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "Engine/Engine.h"

UCore_PhysicsManagerComponent::UCore_PhysicsManagerComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = true;
    
    StaticMeshComp = nullptr;
    SkeletalMeshComp = nullptr;
    bIsRagdollActive = false;
}

void UCore_PhysicsManagerComponent::BeginPlay()
{
    Super::BeginPlay();
    
    FindMeshComponents();
    ApplyPhysicsSettings();
}

void UCore_PhysicsManagerComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Monitor ragdoll state and physics simulation
    if (bIsRagdollActive && SkeletalMeshComp)
    {
        // Check if ragdoll should be disabled based on velocity
        FVector Velocity = SkeletalMeshComp->GetPhysicsLinearVelocity();
        if (Velocity.Size() < 50.0f)
        {
            // Ragdoll has settled, consider disabling
        }
    }
}

void UCore_PhysicsManagerComponent::FindMeshComponents()
{
    AActor* Owner = GetOwner();
    if (!Owner) return;
    
    StaticMeshComp = Owner->FindComponentByClass<UStaticMeshComponent>();
    SkeletalMeshComp = Owner->FindComponentByClass<USkeletalMeshComponent>();
}

void UCore_PhysicsManagerComponent::ApplyPhysicsSettings()
{
    if (StaticMeshComp)
    {
        ApplySettingsToStaticMesh();
    }
    
    if (SkeletalMeshComp)
    {
        ApplySettingsToSkeletalMesh();
    }
}

void UCore_PhysicsManagerComponent::ApplySettingsToStaticMesh()
{
    if (!StaticMeshComp) return;
    
    // Apply collision settings
    StaticMeshComp->SetCollisionEnabled(CollisionSettings.CollisionEnabled);
    StaticMeshComp->SetCollisionObjectType(CollisionSettings.ObjectType);
    StaticMeshComp->SetCollisionResponseToAllChannels(CollisionSettings.CollisionResponse);
    
    // Apply physics settings
    StaticMeshComp->SetMassOverrideInKg(NAME_None, PhysicsSettings.Mass, true);
    StaticMeshComp->SetLinearDamping(PhysicsSettings.LinearDamping);
    StaticMeshComp->SetAngularDamping(PhysicsSettings.AngularDamping);
    StaticMeshComp->SetEnableGravity(PhysicsSettings.bEnableGravity);
    
    // Set physics mode
    switch (PhysicsSettings.PhysicsMode)
    {
        case ECore_PhysicsMode::Static:
            StaticMeshComp->SetMobility(EComponentMobility::Static);
            break;
        case ECore_PhysicsMode::Kinematic:
            StaticMeshComp->SetMobility(EComponentMobility::Movable);
            StaticMeshComp->SetSimulatePhysics(false);
            break;
        case ECore_PhysicsMode::Simulated:
            StaticMeshComp->SetMobility(EComponentMobility::Movable);
            StaticMeshComp->SetSimulatePhysics(true);
            break;
        default:
            break;
    }
}

void UCore_PhysicsManagerComponent::ApplySettingsToSkeletalMesh()
{
    if (!SkeletalMeshComp) return;
    
    // Apply collision settings
    SkeletalMeshComp->SetCollisionEnabled(CollisionSettings.CollisionEnabled);
    SkeletalMeshComp->SetCollisionObjectType(CollisionSettings.ObjectType);
    SkeletalMeshComp->SetCollisionResponseToAllChannels(CollisionSettings.CollisionResponse);
    
    // Apply physics settings
    SkeletalMeshComp->SetAllMassScale(PhysicsSettings.Mass / 100.0f);
    SkeletalMeshComp->SetLinearDamping(PhysicsSettings.LinearDamping);
    SkeletalMeshComp->SetAngularDamping(PhysicsSettings.AngularDamping);
    SkeletalMeshComp->SetEnableGravity(PhysicsSettings.bEnableGravity);
    
    // Set physics mode
    switch (PhysicsSettings.PhysicsMode)
    {
        case ECore_PhysicsMode::Ragdoll:
            if (bCanEnterRagdoll)
            {
                EnableRagdoll();
            }
            break;
        case ECore_PhysicsMode::Simulated:
            SkeletalMeshComp->SetSimulatePhysics(true);
            break;
        default:
            SkeletalMeshComp->SetSimulatePhysics(false);
            break;
    }
}

void UCore_PhysicsManagerComponent::SetPhysicsMode(ECore_PhysicsMode NewMode)
{
    PhysicsSettings.PhysicsMode = NewMode;
    ApplyPhysicsSettings();
}

void UCore_PhysicsManagerComponent::EnableRagdoll()
{
    if (!SkeletalMeshComp || !bCanEnterRagdoll) return;
    
    // Disable character movement if this is a character
    if (ACharacter* Character = Cast<ACharacter>(GetOwner()))
    {
        if (UCharacterMovementComponent* MovementComp = Character->GetCharacterMovement())
        {
            MovementComp->DisableMovement();
        }
    }
    
    // Enable ragdoll physics
    SkeletalMeshComp->SetSimulatePhysics(true);
    SkeletalMeshComp->SetAllBodiesSimulatePhysics(true);
    SkeletalMeshComp->WakeAllRigidBodies();
    
    bIsRagdollActive = true;
}

void UCore_PhysicsManagerComponent::DisableRagdoll()
{
    if (!SkeletalMeshComp) return;
    
    // Disable ragdoll physics
    SkeletalMeshComp->SetSimulatePhysics(false);
    SkeletalMeshComp->SetAllBodiesSimulatePhysics(false);
    SkeletalMeshComp->PutAllRigidBodiesToSleep();
    
    // Re-enable character movement if this is a character
    if (ACharacter* Character = Cast<ACharacter>(GetOwner()))
    {
        if (UCharacterMovementComponent* MovementComp = Character->GetCharacterMovement())
        {
            MovementComp->SetDefaultMovementMode();
        }
    }
    
    bIsRagdollActive = false;
}

void UCore_PhysicsManagerComponent::ApplyImpulse(FVector Impulse, FVector Location)
{
    if (StaticMeshComp && StaticMeshComp->IsSimulatingPhysics())
    {
        if (Location == FVector::ZeroVector)
        {
            StaticMeshComp->AddImpulse(Impulse);
        }
        else
        {
            StaticMeshComp->AddImpulseAtLocation(Impulse, Location);
        }
    }
    
    if (SkeletalMeshComp && SkeletalMeshComp->IsSimulatingPhysics())
    {
        if (Location == FVector::ZeroVector)
        {
            SkeletalMeshComp->AddImpulse(Impulse);
        }
        else
        {
            SkeletalMeshComp->AddImpulseAtLocation(Impulse, Location);
        }
    }
}

bool UCore_PhysicsManagerComponent::IsRagdollActive() const
{
    return bIsRagdollActive;
}

void UCore_PhysicsManagerComponent::SetMass(float NewMass)
{
    PhysicsSettings.Mass = NewMass;
    
    if (StaticMeshComp)
    {
        StaticMeshComp->SetMassOverrideInKg(NAME_None, NewMass, true);
    }
    
    if (SkeletalMeshComp)
    {
        SkeletalMeshComp->SetAllMassScale(NewMass / 100.0f);
    }
}

float UCore_PhysicsManagerComponent::GetMass() const
{
    if (StaticMeshComp)
    {
        return StaticMeshComp->GetMass();
    }
    
    if (SkeletalMeshComp)
    {
        return SkeletalMeshComp->GetMass();
    }
    
    return PhysicsSettings.Mass;
}

// ACore_PhysicsManager Implementation

ACore_PhysicsManager::ACore_PhysicsManager()
{
    PrimaryActorTick.bCanEverTick = true;
    
    PhysicsManagerComponent = CreateDefaultSubobject<UCore_PhysicsManagerComponent>(TEXT("PhysicsManagerComponent"));
    
    GlobalGravityScale = 1.0f;
    GlobalPhysicsTimestep = 0.016667f;
    MaxPhysicsSteps = 8;
}

void ACore_PhysicsManager::BeginPlay()
{
    Super::BeginPlay();
    
    ApplyGlobalPhysicsSettings();
}

void ACore_PhysicsManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // Monitor registered physics actors
    for (int32 i = RegisteredPhysicsActors.Num() - 1; i >= 0; i--)
    {
        if (!IsValid(RegisteredPhysicsActors[i]))
        {
            RegisteredPhysicsActors.RemoveAt(i);
        }
    }
}

void ACore_PhysicsManager::ApplyGlobalPhysicsSettings()
{
    UpdateGlobalPhysicsSettings();
}

void ACore_PhysicsManager::UpdateGlobalPhysicsSettings()
{
    if (UWorld* World = GetWorld())
    {
        if (UPhysicsSettings* PhysicsSettings = UPhysicsSettings::Get())
        {
            // Apply global physics settings
            World->GetPhysicsScene()->GetPxScene()->setGravity(physx::PxVec3(0.0f, 0.0f, -980.0f * GlobalGravityScale));
        }
    }
}

void ACore_PhysicsManager::RegisterPhysicsActor(AActor* Actor)
{
    if (Actor && !RegisteredPhysicsActors.Contains(Actor))
    {
        RegisteredPhysicsActors.Add(Actor);
        
        // Add physics manager component if it doesn't exist
        if (!Actor->FindComponentByClass<UCore_PhysicsManagerComponent>())
        {
            UCore_PhysicsManagerComponent* PhysicsComp = NewObject<UCore_PhysicsManagerComponent>(Actor);
            Actor->AddInstanceComponent(PhysicsComp);
            PhysicsComp->RegisterComponent();
        }
    }
}

void ACore_PhysicsManager::UnregisterPhysicsActor(AActor* Actor)
{
    if (Actor)
    {
        RegisteredPhysicsActors.Remove(Actor);
    }
}

TArray<AActor*> ACore_PhysicsManager::GetAllPhysicsActors() const
{
    return RegisteredPhysicsActors;
}

void ACore_PhysicsManager::EnableRagdollForAllCharacters()
{
    for (AActor* Actor : RegisteredPhysicsActors)
    {
        if (ACharacter* Character = Cast<ACharacter>(Actor))
        {
            if (UCore_PhysicsManagerComponent* PhysicsComp = Character->FindComponentByClass<UCore_PhysicsManagerComponent>())
            {
                PhysicsComp->EnableRagdoll();
            }
        }
    }
}

void ACore_PhysicsManager::DisableRagdollForAllCharacters()
{
    for (AActor* Actor : RegisteredPhysicsActors)
    {
        if (ACharacter* Character = Cast<ACharacter>(Actor))
        {
            if (UCore_PhysicsManagerComponent* PhysicsComp = Character->FindComponentByClass<UCore_PhysicsManagerComponent>())
            {
                PhysicsComp->DisableRagdoll();
            }
        }
    }
}

void ACore_PhysicsManager::ValidatePhysicsSetup()
{
    UE_LOG(LogTemp, Warning, TEXT("Physics Manager Validation:"));
    UE_LOG(LogTemp, Warning, TEXT("- Registered Physics Actors: %d"), RegisteredPhysicsActors.Num());
    UE_LOG(LogTemp, Warning, TEXT("- Global Gravity Scale: %f"), GlobalGravityScale);
    UE_LOG(LogTemp, Warning, TEXT("- Physics Timestep: %f"), GlobalPhysicsTimestep);
    UE_LOG(LogTemp, Warning, TEXT("- Max Physics Steps: %d"), MaxPhysicsSteps);
    
    for (AActor* Actor : RegisteredPhysicsActors)
    {
        if (IsValid(Actor))
        {
            UE_LOG(LogTemp, Warning, TEXT("- Physics Actor: %s"), *Actor->GetName());
        }
    }
}