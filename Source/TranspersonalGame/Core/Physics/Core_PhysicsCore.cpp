#include "Core_PhysicsCore.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

UCore_PhysicsCore::UCore_PhysicsCore()
{
    bInitialized = false;
    ActivePhysicsObjects = 0;
    LastFrameTime = 0.0f;
    CollisionHistoryDuration = 5.0f;
}

void UCore_PhysicsCore::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    InitializePhysicsProfiles();
    
    // Set up performance monitoring timer
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(PerformanceUpdateTimer, 
            FTimerDelegate::CreateUObject(this, &UCore_PhysicsCore::UpdatePerformanceMetrics), 
            1.0f, true);
    }
    
    bInitialized = true;
    UE_LOG(LogTemp, Log, TEXT("Core_PhysicsCore: Initialized successfully"));
}

void UCore_PhysicsCore::Deinitialize()
{
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(PerformanceUpdateTimer);
    }
    
    PhysicsProfiles.Empty();
    RecentCollisions.Empty();
    bInitialized = false;
    
    Super::Deinitialize();
    UE_LOG(LogTemp, Log, TEXT("Core_PhysicsCore: Deinitialized"));
}

bool UCore_PhysicsCore::ShouldCreateSubsystem(UObject* Outer) const
{
    return true;
}

void UCore_PhysicsCore::InitializePhysicsProfiles()
{
    // Default Profile
    FCore_PhysicsSettings DefaultSettings;
    DefaultSettings.Mass = 100.0f;
    DefaultSettings.LinearDamping = 0.01f;
    DefaultSettings.AngularDamping = 0.0f;
    DefaultSettings.Restitution = 0.3f;
    DefaultSettings.Friction = 0.7f;
    PhysicsProfiles.Add(ECore_PhysicsProfile::Default, DefaultSettings);
    
    // Character Profile
    FCore_PhysicsSettings CharacterSettings;
    CharacterSettings.Mass = 80.0f;
    CharacterSettings.LinearDamping = 0.1f;
    CharacterSettings.AngularDamping = 0.1f;
    CharacterSettings.Restitution = 0.1f;
    CharacterSettings.Friction = 1.0f;
    CharacterSettings.bSimulatePhysics = false;
    PhysicsProfiles.Add(ECore_PhysicsProfile::Character, CharacterSettings);
    
    // Dinosaur Profile
    FCore_PhysicsSettings DinosaurSettings;
    DinosaurSettings.Mass = 500.0f;
    DinosaurSettings.LinearDamping = 0.05f;
    DinosaurSettings.AngularDamping = 0.05f;
    DinosaurSettings.Restitution = 0.2f;
    DinosaurSettings.Friction = 0.8f;
    DinosaurSettings.bSimulatePhysics = false;
    PhysicsProfiles.Add(ECore_PhysicsProfile::Dinosaur, DinosaurSettings);
    
    // Environment Profile
    FCore_PhysicsSettings EnvironmentSettings;
    EnvironmentSettings.Mass = 1000.0f;
    EnvironmentSettings.LinearDamping = 0.5f;
    EnvironmentSettings.AngularDamping = 0.5f;
    EnvironmentSettings.Restitution = 0.4f;
    EnvironmentSettings.Friction = 0.9f;
    EnvironmentSettings.bSimulatePhysics = false;
    PhysicsProfiles.Add(ECore_PhysicsProfile::Environment, EnvironmentSettings);
    
    // Projectile Profile
    FCore_PhysicsSettings ProjectileSettings;
    ProjectileSettings.Mass = 1.0f;
    ProjectileSettings.LinearDamping = 0.0f;
    ProjectileSettings.AngularDamping = 0.0f;
    ProjectileSettings.Restitution = 0.8f;
    ProjectileSettings.Friction = 0.1f;
    ProjectileSettings.bEnableGravity = true;
    PhysicsProfiles.Add(ECore_PhysicsProfile::Projectile, ProjectileSettings);
    
    // Debris Profile
    FCore_PhysicsSettings DebrisSettings;
    DebrisSettings.Mass = 10.0f;
    DebrisSettings.LinearDamping = 0.2f;
    DebrisSettings.AngularDamping = 0.2f;
    DebrisSettings.Restitution = 0.5f;
    DebrisSettings.Friction = 0.6f;
    PhysicsProfiles.Add(ECore_PhysicsProfile::Debris, DebrisSettings);
    
    // Ragdoll Profile
    FCore_PhysicsSettings RagdollSettings;
    RagdollSettings.Mass = 75.0f;
    RagdollSettings.LinearDamping = 0.3f;
    RagdollSettings.AngularDamping = 0.3f;
    RagdollSettings.Restitution = 0.1f;
    RagdollSettings.Friction = 0.8f;
    PhysicsProfiles.Add(ECore_PhysicsProfile::Ragdoll, RagdollSettings);
    
    // Vehicle Profile
    FCore_PhysicsSettings VehicleSettings;
    VehicleSettings.Mass = 1500.0f;
    VehicleSettings.LinearDamping = 0.1f;
    VehicleSettings.AngularDamping = 0.1f;
    VehicleSettings.Restitution = 0.2f;
    VehicleSettings.Friction = 0.7f;
    PhysicsProfiles.Add(ECore_PhysicsProfile::Vehicle, VehicleSettings);
}

void UCore_PhysicsCore::ApplyPhysicsProfile(UPrimitiveComponent* Component, ECore_PhysicsProfile Profile)
{
    if (!Component || !PhysicsProfiles.Contains(Profile))
    {
        return;
    }
    
    const FCore_PhysicsSettings& Settings = PhysicsProfiles[Profile];
    SetCustomPhysicsSettings(Component, Settings);
}

FCore_PhysicsSettings UCore_PhysicsCore::GetPhysicsSettings(ECore_PhysicsProfile Profile) const
{
    if (PhysicsProfiles.Contains(Profile))
    {
        return PhysicsProfiles[Profile];
    }
    return FCore_PhysicsSettings();
}

void UCore_PhysicsCore::SetCustomPhysicsSettings(UPrimitiveComponent* Component, const FCore_PhysicsSettings& Settings)
{
    if (!Component)
    {
        return;
    }
    
    // Apply physics settings
    Component->SetMassOverrideInKg(NAME_None, Settings.Mass, true);
    Component->SetLinearDamping(Settings.LinearDamping);
    Component->SetAngularDamping(Settings.AngularDamping);
    
    // Set material properties
    if (UBodyInstance* BodyInstance = Component->GetBodyInstance())
    {
        BodyInstance->SetResponseToAllChannels(ECR_Block);
        BodyInstance->bGenerateHitEvents = Settings.bGenerateHitEvents;
    }
    
    Component->SetNotifyRigidBodyCollision(Settings.bGenerateHitEvents);
    Component->SetSimulatePhysics(Settings.bSimulatePhysics);
    Component->SetEnableGravity(Settings.bEnableGravity);
    
    // Bind collision events if needed
    if (Settings.bGenerateHitEvents && !Component->OnComponentHit.IsBound())
    {
        Component->OnComponentHit.AddDynamic(this, &UCore_PhysicsCore::OnComponentHit);
    }
}

void UCore_PhysicsCore::ApplyForceAtLocation(UPrimitiveComponent* Component, FVector Force, FVector Location, bool bAccelChange)
{
    if (!Component)
    {
        return;
    }
    
    Component->AddForceAtLocation(Force, Location, NAME_None, bAccelChange);
}

void UCore_PhysicsCore::ApplyImpulseAtLocation(UPrimitiveComponent* Component, FVector Impulse, FVector Location, bool bVelChange)
{
    if (!Component)
    {
        return;
    }
    
    Component->AddImpulseAtLocation(Impulse, Location, NAME_None, bVelChange);
}

void UCore_PhysicsCore::ApplyRadialForce(FVector Origin, float Radius, float Strength, bool bImpulse, bool bIgnoreMass)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    TArray<FOverlapResult> OverlapResults;
    FCollisionQueryParams QueryParams;
    QueryParams.bTraceComplex = false;
    
    bool bHit = World->OverlapMultiByChannel(
        OverlapResults,
        Origin,
        FQuat::Identity,
        ECC_WorldDynamic,
        FCollisionShape::MakeSphere(Radius),
        QueryParams
    );
    
    if (bHit)
    {
        for (const FOverlapResult& Result : OverlapResults)
        {
            if (UPrimitiveComponent* PrimComp = Result.GetComponent())
            {
                FVector Direction = (PrimComp->GetComponentLocation() - Origin).GetSafeNormal();
                float Distance = FVector::Dist(PrimComp->GetComponentLocation(), Origin);
                float ForceMagnitude = Strength * (1.0f - (Distance / Radius));
                
                if (bImpulse)
                {
                    PrimComp->AddImpulse(Direction * ForceMagnitude, NAME_None, bIgnoreMass);
                }
                else
                {
                    PrimComp->AddForce(Direction * ForceMagnitude, NAME_None, bIgnoreMass);
                }
            }
        }
    }
}

bool UCore_PhysicsCore::LineTrace(FVector Start, FVector End, FHitResult& HitResult, bool bTraceComplex)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return false;
    }
    
    FCollisionQueryParams QueryParams;
    QueryParams.bTraceComplex = bTraceComplex;
    
    return World->LineTraceSingleByChannel(HitResult, Start, End, ECC_WorldStatic, QueryParams);
}

bool UCore_PhysicsCore::SphereTrace(FVector Start, FVector End, float Radius, FHitResult& HitResult, bool bTraceComplex)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return false;
    }
    
    FCollisionQueryParams QueryParams;
    QueryParams.bTraceComplex = bTraceComplex;
    
    return World->SweepSingleByChannel(HitResult, Start, End, FQuat::Identity, ECC_WorldStatic, FCollisionShape::MakeSphere(Radius), QueryParams);
}

bool UCore_PhysicsCore::BoxTrace(FVector Start, FVector End, FVector HalfSize, FRotator Orientation, FHitResult& HitResult, bool bTraceComplex)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return false;
    }
    
    FCollisionQueryParams QueryParams;
    QueryParams.bTraceComplex = bTraceComplex;
    
    return World->SweepSingleByChannel(HitResult, Start, End, Orientation.Quaternion(), ECC_WorldStatic, FCollisionShape::MakeBox(HalfSize), QueryParams);
}

void UCore_PhysicsCore::EnablePhysicsSimulation(UPrimitiveComponent* Component, bool bEnable)
{
    if (Component)
    {
        Component->SetSimulatePhysics(bEnable);
    }
}

void UCore_PhysicsCore::SetGravityEnabled(UPrimitiveComponent* Component, bool bEnable)
{
    if (Component)
    {
        Component->SetEnableGravity(bEnable);
    }
}

void UCore_PhysicsCore::SetCollisionEnabled(UPrimitiveComponent* Component, bool bEnable)
{
    if (Component)
    {
        Component->SetCollisionEnabled(bEnable ? ECollisionEnabled::QueryAndPhysics : ECollisionEnabled::NoCollision);
    }
}

void UCore_PhysicsCore::SetMass(UPrimitiveComponent* Component, float Mass)
{
    if (Component)
    {
        Component->SetMassOverrideInKg(NAME_None, Mass, true);
    }
}

float UCore_PhysicsCore::GetMass(UPrimitiveComponent* Component) const
{
    if (Component && Component->GetBodyInstance())
    {
        return Component->GetMass();
    }
    return 0.0f;
}

void UCore_PhysicsCore::SetDensity(UPrimitiveComponent* Component, float Density)
{
    if (Component && Component->GetBodyInstance())
    {
        Component->GetBodyInstance()->MassScale = Density;
        Component->GetBodyInstance()->UpdateMassProperties();
    }
}

FVector UCore_PhysicsCore::GetVelocity(UPrimitiveComponent* Component) const
{
    if (Component && Component->GetBodyInstance())
    {
        return Component->GetPhysicsLinearVelocity();
    }
    return FVector::ZeroVector;
}

void UCore_PhysicsCore::SetVelocity(UPrimitiveComponent* Component, FVector NewVelocity)
{
    if (Component)
    {
        Component->SetPhysicsLinearVelocity(NewVelocity);
    }
}

FVector UCore_PhysicsCore::GetAngularVelocity(UPrimitiveComponent* Component) const
{
    if (Component && Component->GetBodyInstance())
    {
        return Component->GetPhysicsAngularVelocityInRadians();
    }
    return FVector::ZeroVector;
}

void UCore_PhysicsCore::SetAngularVelocity(UPrimitiveComponent* Component, FVector NewAngularVelocity)
{
    if (Component)
    {
        Component->SetPhysicsAngularVelocityInRadians(NewAngularVelocity);
    }
}

int32 UCore_PhysicsCore::GetActivePhysicsObjectCount() const
{
    return ActivePhysicsObjects;
}

float UCore_PhysicsCore::GetPhysicsFrameTime() const
{
    return LastFrameTime;
}

void UCore_PhysicsCore::SetPhysicsSubstepEnabled(bool bEnable)
{
    if (UWorld* World = GetWorld())
    {
        if (UPhysicsSettings* PhysicsSettings = UPhysicsSettings::Get())
        {
            PhysicsSettings->bSubstepping = bEnable;
        }
    }
}

void UCore_PhysicsCore::UpdatePerformanceMetrics()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    // Count active physics objects
    ActivePhysicsObjects = 0;
    for (TActorIterator<AActor> ActorIterator(World); ActorIterator; ++ActorIterator)
    {
        AActor* Actor = *ActorIterator;
        if (Actor)
        {
            TArray<UPrimitiveComponent*> PrimitiveComponents;
            Actor->GetComponents<UPrimitiveComponent>(PrimitiveComponents);
            
            for (UPrimitiveComponent* PrimComp : PrimitiveComponents)
            {
                if (PrimComp && PrimComp->IsSimulatingPhysics())
                {
                    ActivePhysicsObjects++;
                }
            }
        }
    }
    
    // Update frame time
    LastFrameTime = World->GetDeltaSeconds();
}

void UCore_PhysicsCore::ProcessCollisionEvent(const FHitResult& HitResult, float ImpactForce)
{
    FCore_CollisionData CollisionData;
    CollisionData.HitActor = HitResult.GetActor();
    CollisionData.HitComponent = HitResult.GetComponent();
    CollisionData.ImpactPoint = HitResult.ImpactPoint;
    CollisionData.ImpactNormal = HitResult.ImpactNormal;
    CollisionData.ImpactForce = ImpactForce;
    CollisionData.TimeStamp = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
    
    // Add to recent collisions
    RecentCollisions.Add(CollisionData);
    
    // Clean up old collision data
    float CurrentTime = CollisionData.TimeStamp;
    RecentCollisions.RemoveAll([CurrentTime, this](const FCore_CollisionData& Data) {
        return (CurrentTime - Data.TimeStamp) > CollisionHistoryDuration;
    });
    
    // Broadcast collision event
    ECore_PhysicsProfile ProfileType = ECore_PhysicsProfile::Default;
    OnPhysicsCollision.Broadcast(CollisionData, ProfileType);
}

void UCore_PhysicsCore::OnComponentHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
    float ImpactForce = NormalImpulse.Size();
    ProcessCollisionEvent(Hit, ImpactForce);
}