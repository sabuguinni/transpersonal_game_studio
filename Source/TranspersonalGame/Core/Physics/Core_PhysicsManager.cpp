#include "Core_PhysicsManager.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Components/PrimitiveComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "PhysicalMaterials/PhysicalMaterial.h"

UCore_PhysicsManager::UCore_PhysicsManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = true;
    
    // Initialize default physics settings
    RealisticSettings.GravityScale = 1.0f;
    RealisticSettings.LinearDamping = 0.01f;
    RealisticSettings.AngularDamping = 0.0f;
    RealisticSettings.MaxAngularVelocity = 3600.0f;
    RealisticSettings.bEnableGravity = true;
    RealisticSettings.bSimulatePhysics = true;

    ArcadeSettings.GravityScale = 0.7f;
    ArcadeSettings.LinearDamping = 0.05f;
    ArcadeSettings.AngularDamping = 0.1f;
    ArcadeSettings.MaxAngularVelocity = 1800.0f;
    ArcadeSettings.bEnableGravity = true;
    ArcadeSettings.bSimulatePhysics = true;

    CinematicSettings.GravityScale = 0.3f;
    CinematicSettings.LinearDamping = 0.2f;
    CinematicSettings.AngularDamping = 0.5f;
    CinematicSettings.MaxAngularVelocity = 900.0f;
    CinematicSettings.bEnableGravity = false;
    CinematicSettings.bSimulatePhysics = false;

    CurrentPhysicsMode = ECore_PhysicsMode::Realistic;
    CurrentPhysicsMaterial = nullptr;
    bPhysicsEnabled = true;
}

void UCore_PhysicsManager::BeginPlay()
{
    Super::BeginPlay();
    
    InitializePhysicsSettings();
    ApplyPhysicsSettings(GetCurrentPhysicsSettings());
}

void UCore_PhysicsManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Update physics state if needed
    if (bPhysicsEnabled)
    {
        UpdateWorldPhysicsSettings();
    }
}

void UCore_PhysicsManager::SetPhysicsMode(ECore_PhysicsMode NewMode)
{
    if (CurrentPhysicsMode != NewMode)
    {
        CurrentPhysicsMode = NewMode;
        ApplyPhysicsSettings(GetCurrentPhysicsSettings());
        
        UE_LOG(LogTemp, Log, TEXT("Physics mode changed to: %d"), (int32)NewMode);
    }
}

void UCore_PhysicsManager::ApplyPhysicsSettings(const FCore_PhysicsSettings& Settings)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    // Apply gravity settings
    if (World->GetPhysicsScene())
    {
        SetGlobalGravity(Settings.GravityScale * -980.0f); // Standard Earth gravity
    }

    // Apply settings to owner actor if it has physics components
    AActor* Owner = GetOwner();
    if (Owner)
    {
        UPrimitiveComponent* PrimComp = GetActorPrimitiveComponent(Owner);
        if (PrimComp && PrimComp->IsSimulatingPhysics())
        {
            PrimComp->SetLinearDamping(Settings.LinearDamping);
            PrimComp->SetAngularDamping(Settings.AngularDamping);
            PrimComp->SetMaxAngularVelocityInRadians(FMath::DegreesToRadians(Settings.MaxAngularVelocity));
            PrimComp->SetEnableGravity(Settings.bEnableGravity);
            PrimComp->SetSimulatePhysics(Settings.bSimulatePhysics);
        }
    }
}

FCore_PhysicsSettings UCore_PhysicsManager::GetCurrentPhysicsSettings() const
{
    switch (CurrentPhysicsMode)
    {
        case ECore_PhysicsMode::Arcade:
            return ArcadeSettings;
        case ECore_PhysicsMode::Cinematic:
            return CinematicSettings;
        case ECore_PhysicsMode::Realistic:
        default:
            return RealisticSettings;
    }
}

void UCore_PhysicsManager::SetGlobalGravity(float NewGravity)
{
    UWorld* World = GetWorld();
    if (World && World->GetPhysicsScene())
    {
        World->GetPhysicsScene()->SetGravityZ(NewGravity);
        UE_LOG(LogTemp, Log, TEXT("Global gravity set to: %f"), NewGravity);
    }
}

float UCore_PhysicsManager::GetGlobalGravity() const
{
    UWorld* World = GetWorld();
    if (World && World->GetPhysicsScene())
    {
        return World->GetPhysicsScene()->GetGravityZ();
    }
    return -980.0f; // Default gravity
}

void UCore_PhysicsManager::EnablePhysicsSimulation(bool bEnable)
{
    bPhysicsEnabled = bEnable;
    
    AActor* Owner = GetOwner();
    if (Owner)
    {
        UPrimitiveComponent* PrimComp = GetActorPrimitiveComponent(Owner);
        if (PrimComp)
        {
            PrimComp->SetSimulatePhysics(bEnable);
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Physics simulation %s"), bEnable ? TEXT("enabled") : TEXT("disabled"));
}

bool UCore_PhysicsManager::IsPhysicsSimulationEnabled() const
{
    return bPhysicsEnabled;
}

void UCore_PhysicsManager::SetPhysicsMaterial(UPhysicalMaterial* NewMaterial)
{
    CurrentPhysicsMaterial = NewMaterial;
    
    AActor* Owner = GetOwner();
    if (Owner && NewMaterial)
    {
        UPrimitiveComponent* PrimComp = GetActorPrimitiveComponent(Owner);
        if (PrimComp)
        {
            PrimComp->SetPhysMaterialOverride(NewMaterial);
        }
    }
}

UPhysicalMaterial* UCore_PhysicsManager::GetPhysicsMaterial() const
{
    return CurrentPhysicsMaterial;
}

void UCore_PhysicsManager::ApplyForceToActor(AActor* TargetActor, FVector Force, bool bAccelChange)
{
    if (!TargetActor)
    {
        return;
    }

    UPrimitiveComponent* PrimComp = GetActorPrimitiveComponent(TargetActor);
    if (PrimComp && PrimComp->IsSimulatingPhysics())
    {
        PrimComp->AddForce(Force, NAME_None, bAccelChange);
    }
}

void UCore_PhysicsManager::ApplyImpulseToActor(AActor* TargetActor, FVector Impulse, bool bVelChange)
{
    if (!TargetActor)
    {
        return;
    }

    UPrimitiveComponent* PrimComp = GetActorPrimitiveComponent(TargetActor);
    if (PrimComp && PrimComp->IsSimulatingPhysics())
    {
        PrimComp->AddImpulse(Impulse, NAME_None, bVelChange);
    }
}

void UCore_PhysicsManager::ApplyTorqueToActor(AActor* TargetActor, FVector Torque, bool bAccelChange)
{
    if (!TargetActor)
    {
        return;
    }

    UPrimitiveComponent* PrimComp = GetActorPrimitiveComponent(TargetActor);
    if (PrimComp && PrimComp->IsSimulatingPhysics())
    {
        PrimComp->AddTorqueInRadians(Torque, NAME_None, bAccelChange);
    }
}

bool UCore_PhysicsManager::IsActorPhysicsEnabled(AActor* TargetActor) const
{
    if (!TargetActor)
    {
        return false;
    }

    UPrimitiveComponent* PrimComp = GetActorPrimitiveComponent(TargetActor);
    return PrimComp ? PrimComp->IsSimulatingPhysics() : false;
}

FVector UCore_PhysicsManager::GetActorVelocity(AActor* TargetActor) const
{
    if (!TargetActor)
    {
        return FVector::ZeroVector;
    }

    UPrimitiveComponent* PrimComp = GetActorPrimitiveComponent(TargetActor);
    return PrimComp ? PrimComp->GetPhysicsLinearVelocity() : FVector::ZeroVector;
}

FVector UCore_PhysicsManager::GetActorAngularVelocity(AActor* TargetActor) const
{
    if (!TargetActor)
    {
        return FVector::ZeroVector;
    }

    UPrimitiveComponent* PrimComp = GetActorPrimitiveComponent(TargetActor);
    return PrimComp ? PrimComp->GetPhysicsAngularVelocityInRadians() : FVector::ZeroVector;
}

float UCore_PhysicsManager::GetActorMass(AActor* TargetActor) const
{
    if (!TargetActor)
    {
        return 0.0f;
    }

    UPrimitiveComponent* PrimComp = GetActorPrimitiveComponent(TargetActor);
    return PrimComp ? PrimComp->GetMass() : 0.0f;
}

void UCore_PhysicsManager::InitializePhysicsSettings()
{
    UE_LOG(LogTemp, Log, TEXT("Core_PhysicsManager initialized with mode: %d"), (int32)CurrentPhysicsMode);
}

void UCore_PhysicsManager::UpdateWorldPhysicsSettings()
{
    // Periodic physics state updates if needed
    // This can be used for dynamic physics adjustments based on gameplay conditions
}

UPrimitiveComponent* UCore_PhysicsManager::GetActorPrimitiveComponent(AActor* Actor) const
{
    if (!Actor)
    {
        return nullptr;
    }

    // Try to get the root primitive component first
    UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(Actor->GetRootComponent());
    if (PrimComp)
    {
        return PrimComp;
    }

    // If root is not primitive, try to find the first primitive component
    UStaticMeshComponent* StaticMeshComp = Actor->FindComponentByClass<UStaticMeshComponent>();
    if (StaticMeshComp)
    {
        return StaticMeshComp;
    }

    USkeletalMeshComponent* SkeletalMeshComp = Actor->FindComponentByClass<USkeletalMeshComponent>();
    if (SkeletalMeshComp)
    {
        return SkeletalMeshComp;
    }

    return nullptr;
}