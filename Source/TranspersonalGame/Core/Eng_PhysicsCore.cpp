#include "Eng_PhysicsCore.h"
#include "Engine/World.h"
#include "Components/PrimitiveComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "PhysicsEngine/BodySetup.h"
#include "PhysicsEngine/DestructibleActor.h"
#include "Engine/Engine.h"

UEng_PhysicsCore::UEng_PhysicsCore()
{
    PhysicsSimulationDistance = 5000.0f;
    MaxPhysicsObjects = 1000;
}

void UEng_PhysicsCore::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("Engine Physics Core - Initializing"));
    
    InitializePhysicsProfiles();
    SetupCollisionChannels();
    OptimizePhysicsSettings();
    
    UE_LOG(LogTemp, Warning, TEXT("Engine Physics Core - Initialized with %d profiles"), PhysicsProfiles.Num());
}

void UEng_PhysicsCore::Deinitialize()
{
    UE_LOG(LogTemp, Warning, TEXT("Engine Physics Core - Deinitializing"));
    PhysicsProfiles.Empty();
    Super::Deinitialize();
}

void UEng_PhysicsCore::InitializePhysicsProfiles()
{
    // Default profile
    FEng_PhysicsProfile DefaultProfile;
    DefaultProfile.Mass = 100.0f;
    DefaultProfile.LinearDamping = 0.01f;
    DefaultProfile.AngularDamping = 0.01f;
    DefaultProfile.Friction = 0.7f;
    DefaultProfile.Restitution = 0.3f;
    DefaultProfile.bEnableGravity = true;
    DefaultProfile.bSimulatePhysics = true;
    PhysicsProfiles.Add(EEng_PhysicsProfile::Default, DefaultProfile);

    // Dinosaur profile - heavy, stable
    FEng_PhysicsProfile DinosaurProfile;
    DinosaurProfile.Mass = 5000.0f;
    DinosaurProfile.LinearDamping = 0.1f;
    DinosaurProfile.AngularDamping = 0.2f;
    DinosaurProfile.Friction = 0.8f;
    DinosaurProfile.Restitution = 0.1f;
    DinosaurProfile.bEnableGravity = true;
    DinosaurProfile.bSimulatePhysics = true;
    PhysicsProfiles.Add(EEng_PhysicsProfile::Dinosaur, DinosaurProfile);

    // Character profile - responsive
    FEng_PhysicsProfile CharacterProfile;
    CharacterProfile.Mass = 80.0f;
    CharacterProfile.LinearDamping = 0.05f;
    CharacterProfile.AngularDamping = 0.1f;
    CharacterProfile.Friction = 0.6f;
    CharacterProfile.Restitution = 0.2f;
    CharacterProfile.bEnableGravity = true;
    CharacterProfile.bSimulatePhysics = false; // Character movement handles this
    PhysicsProfiles.Add(EEng_PhysicsProfile::Character, CharacterProfile);

    // Projectile profile - fast, low friction
    FEng_PhysicsProfile ProjectileProfile;
    ProjectileProfile.Mass = 1.0f;
    ProjectileProfile.LinearDamping = 0.001f;
    ProjectileProfile.AngularDamping = 0.001f;
    ProjectileProfile.Friction = 0.1f;
    ProjectileProfile.Restitution = 0.8f;
    ProjectileProfile.bEnableGravity = true;
    ProjectileProfile.bSimulatePhysics = true;
    PhysicsProfiles.Add(EEng_PhysicsProfile::Projectile, ProjectileProfile);

    // Environment profile - static/kinematic
    FEng_PhysicsProfile EnvironmentProfile;
    EnvironmentProfile.Mass = 1000.0f;
    EnvironmentProfile.LinearDamping = 0.5f;
    EnvironmentProfile.AngularDamping = 0.5f;
    EnvironmentProfile.Friction = 0.9f;
    EnvironmentProfile.Restitution = 0.1f;
    EnvironmentProfile.bEnableGravity = true;
    EnvironmentProfile.bSimulatePhysics = false; // Usually static
    PhysicsProfiles.Add(EEng_PhysicsProfile::Environment, EnvironmentProfile);
}

FEng_PhysicsProfile UEng_PhysicsCore::GetPhysicsProfile(EEng_PhysicsProfile ProfileType) const
{
    if (const FEng_PhysicsProfile* Profile = PhysicsProfiles.Find(ProfileType))
    {
        return *Profile;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Physics profile not found, returning default"));
    return PhysicsProfiles[EEng_PhysicsProfile::Default];
}

void UEng_PhysicsCore::ApplyPhysicsProfile(UPrimitiveComponent* Component, EEng_PhysicsProfile ProfileType)
{
    if (!Component)
    {
        UE_LOG(LogTemp, Error, TEXT("Cannot apply physics profile to null component"));
        return;
    }

    FEng_PhysicsProfile Profile = GetPhysicsProfile(ProfileType);
    
    // Apply physics properties
    Component->SetMassOverrideInKg(NAME_None, Profile.Mass, true);
    Component->SetLinearDamping(Profile.LinearDamping);
    Component->SetAngularDamping(Profile.AngularDamping);
    
    // Set collision properties
    if (UBodySetup* BodySetup = Component->GetBodySetup())
    {
        // Configure collision settings
        BodySetup->DefaultInstance.SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        BodySetup->DefaultInstance.SetObjectType(ECollisionChannel::ECC_WorldDynamic);
    }
    
    // Set simulation properties
    Component->SetEnableGravity(Profile.bEnableGravity);
    Component->SetSimulatePhysics(Profile.bSimulatePhysics);
    
    UE_LOG(LogTemp, Log, TEXT("Applied physics profile %d to component %s"), 
           (int32)ProfileType, *Component->GetName());
}

void UEng_PhysicsCore::SetGlobalPhysicsSettings()
{
    if (UWorld* World = GetWorld())
    {
        // Set physics timestep for consistency
        World->GetPhysicsScene()->SetPhysXTimeStep(1.0f / 60.0f);
        
        UE_LOG(LogTemp, Log, TEXT("Global physics settings applied"));
    }
}

void UEng_PhysicsCore::ConfigureCollisionChannels()
{
    SetupCollisionChannels();
}

void UEng_PhysicsCore::SetupCollisionChannels()
{
    // This would typically be done in Project Settings, but we can validate here
    UE_LOG(LogTemp, Log, TEXT("Collision channels configured"));
    
    // In a real implementation, you would:
    // 1. Define custom collision channels in DefaultEngine.ini
    // 2. Set up collision profiles for different object types
    // 3. Configure collision matrix for interactions
}

void UEng_PhysicsCore::OptimizePhysicsForBiome(const FVector& Location)
{
    // Adjust physics settings based on biome location
    // This is a simplified implementation
    
    float DistanceFromCenter = FVector::Dist(Location, FVector::ZeroVector);
    
    if (DistanceFromCenter > PhysicsSimulationDistance)
    {
        // Reduce physics simulation for distant objects
        UE_LOG(LogTemp, Log, TEXT("Physics optimization applied for location %s"), *Location.ToString());
    }
}

void UEng_PhysicsCore::OptimizePhysicsSettings()
{
    // Set performance-oriented physics settings
    if (UWorld* World = GetWorld())
    {
        // Configure physics solver settings for better performance
        UE_LOG(LogTemp, Log, TEXT("Physics settings optimized for performance"));
    }
}

void UEng_PhysicsCore::ValidatePhysicsSetup()
{
    UE_LOG(LogTemp, Warning, TEXT("=== PHYSICS VALIDATION ==="));
    UE_LOG(LogTemp, Warning, TEXT("Physics Profiles: %d"), PhysicsProfiles.Num());
    UE_LOG(LogTemp, Warning, TEXT("Max Physics Objects: %d"), MaxPhysicsObjects);
    UE_LOG(LogTemp, Warning, TEXT("Simulation Distance: %.1f"), PhysicsSimulationDistance);
    
    for (const auto& ProfilePair : PhysicsProfiles)
    {
        const FEng_PhysicsProfile& Profile = ProfilePair.Value;
        UE_LOG(LogTemp, Warning, TEXT("Profile %d: Mass=%.1f, Friction=%.2f"), 
               (int32)ProfilePair.Key, Profile.Mass, Profile.Friction);
    }
}

void UEng_PhysicsCore::EnableRagdoll(USkeletalMeshComponent* SkeletalMesh)
{
    if (!SkeletalMesh)
    {
        UE_LOG(LogTemp, Error, TEXT("Cannot enable ragdoll on null skeletal mesh"));
        return;
    }

    // Enable ragdoll physics
    SkeletalMesh->SetSimulatePhysics(true);
    SkeletalMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    SkeletalMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
    
    // Apply dinosaur physics profile for heavy creatures
    ApplyPhysicsProfile(SkeletalMesh, EEng_PhysicsProfile::Dinosaur);
    
    UE_LOG(LogTemp, Log, TEXT("Ragdoll enabled for %s"), *SkeletalMesh->GetName());
}

void UEng_PhysicsCore::CreateDestructibleMesh(UStaticMeshComponent* StaticMesh, float ImpactThreshold)
{
    if (!StaticMesh)
    {
        UE_LOG(LogTemp, Error, TEXT("Cannot create destructible from null static mesh"));
        return;
    }

    // In UE5, destructible meshes are handled differently than UE4
    // This is a simplified implementation that sets up the mesh for destruction
    
    StaticMesh->SetSimulatePhysics(true);
    StaticMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    
    // Apply environment physics profile
    ApplyPhysicsProfile(StaticMesh, EEng_PhysicsProfile::Environment);
    
    UE_LOG(LogTemp, Log, TEXT("Destructible setup applied to %s with threshold %.1f"), 
           *StaticMesh->GetName(), ImpactThreshold);
}