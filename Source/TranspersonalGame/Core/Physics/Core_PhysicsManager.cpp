#include "Core_PhysicsManager.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Components/PrimitiveComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "PhysicsEngine/BodyInstance.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "TimerManager.h"

UCore_PhysicsManager::UCore_PhysicsManager()
{
    LastPhysicsFrameTime = 0.0f;
    ActivePhysicsActorCount = 0;
}

void UCore_PhysicsManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    InitializePhysicsProfiles();
    
    // Start metrics update timer
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(MetricsUpdateTimer, this, &UCore_PhysicsManager::UpdatePhysicsMetrics, 1.0f, true);
    }
    
    UE_LOG(LogTemp, Log, TEXT("Core_PhysicsManager initialized"));
}

void UCore_PhysicsManager::Deinitialize()
{
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(MetricsUpdateTimer);
    }
    
    TrackedPhysicsActors.Empty();
    Super::Deinitialize();
}

bool UCore_PhysicsManager::ShouldCreateSubsystem(UObject* Outer) const
{
    return Super::ShouldCreateSubsystem(Outer);
}

void UCore_PhysicsManager::InitializePhysicsProfiles()
{
    // Default profile
    FCore_PhysicsSettings DefaultSettings;
    DefaultSettings.Mass = 1.0f;
    DefaultSettings.LinearDamping = 0.01f;
    DefaultSettings.AngularDamping = 0.0f;
    DefaultSettings.Restitution = 0.3f;
    DefaultSettings.Friction = 0.7f;
    PhysicsProfiles.Add(ECore_PhysicsProfile::Default, DefaultSettings);
    
    // Character profile
    FCore_PhysicsSettings CharacterSettings;
    CharacterSettings.Mass = 80.0f;
    CharacterSettings.LinearDamping = 0.1f;
    CharacterSettings.AngularDamping = 0.1f;
    CharacterSettings.Restitution = 0.1f;
    CharacterSettings.Friction = 1.0f;
    PhysicsProfiles.Add(ECore_PhysicsProfile::Character, CharacterSettings);
    
    // Dinosaur profile
    FCore_PhysicsSettings DinosaurSettings;
    DinosaurSettings.Mass = 500.0f;
    DinosaurSettings.LinearDamping = 0.05f;
    DinosaurSettings.AngularDamping = 0.05f;
    DinosaurSettings.Restitution = 0.2f;
    DinosaurSettings.Friction = 0.8f;
    PhysicsProfiles.Add(ECore_PhysicsProfile::Dinosaur, DinosaurSettings);
    
    // Environment profile
    FCore_PhysicsSettings EnvironmentSettings;
    EnvironmentSettings.Mass = 100.0f;
    EnvironmentSettings.LinearDamping = 0.02f;
    EnvironmentSettings.AngularDamping = 0.02f;
    EnvironmentSettings.Restitution = 0.4f;
    EnvironmentSettings.Friction = 0.9f;
    EnvironmentSettings.bSimulatePhysics = false;
    PhysicsProfiles.Add(ECore_PhysicsProfile::Environment, EnvironmentSettings);
    
    // Projectile profile
    FCore_PhysicsSettings ProjectileSettings;
    ProjectileSettings.Mass = 0.1f;
    ProjectileSettings.LinearDamping = 0.0f;
    ProjectileSettings.AngularDamping = 0.0f;
    ProjectileSettings.Restitution = 0.6f;
    ProjectileSettings.Friction = 0.3f;
    PhysicsProfiles.Add(ECore_PhysicsProfile::Projectile, ProjectileSettings);
    
    // Debris profile
    FCore_PhysicsSettings DebrisSettings;
    DebrisSettings.Mass = 5.0f;
    DebrisSettings.LinearDamping = 0.1f;
    DebrisSettings.AngularDamping = 0.1f;
    DebrisSettings.Restitution = 0.5f;
    DebrisSettings.Friction = 0.6f;
    PhysicsProfiles.Add(ECore_PhysicsProfile::Debris, DebrisSettings);
    
    // Ragdoll profile
    FCore_PhysicsSettings RagdollSettings;
    RagdollSettings.Mass = 80.0f;
    RagdollSettings.LinearDamping = 0.2f;
    RagdollSettings.AngularDamping = 0.2f;
    RagdollSettings.Restitution = 0.1f;
    RagdollSettings.Friction = 0.8f;
    PhysicsProfiles.Add(ECore_PhysicsProfile::Ragdoll, RagdollSettings);
}

void UCore_PhysicsManager::ApplyPhysicsProfile(UPrimitiveComponent* Component, ECore_PhysicsProfile Profile)
{
    if (!Component)
    {
        return;
    }
    
    const FCore_PhysicsSettings* Settings = PhysicsProfiles.Find(Profile);
    if (!Settings)
    {
        UE_LOG(LogTemp, Warning, TEXT("Physics profile not found: %d"), (int32)Profile);
        return;
    }
    
    FBodyInstance* BodyInstance = Component->GetBodyInstance();
    if (BodyInstance)
    {
        BodyInstance->SetMassOverride(Settings->Mass, true);
        BodyInstance->LinearDamping = Settings->LinearDamping;
        BodyInstance->AngularDamping = Settings->AngularDamping;
        BodyInstance->bEnableGravity = Settings->bEnableGravity;
        BodyInstance->SetInstanceSimulatePhysics(Settings->bSimulatePhysics);
        
        // Set physical material properties
        if (UPhysicalMaterial* PhysMat = BodyInstance->GetSimplePhysicalMaterial())
        {
            PhysMat->Restitution = Settings->Restitution;
            PhysMat->Friction = Settings->Friction;
        }
        
        // Track this actor if it has physics simulation enabled
        if (Settings->bSimulatePhysics && Component->GetOwner())
        {
            TrackedPhysicsActors.AddUnique(Component->GetOwner());
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Applied physics profile %d to component %s"), 
           (int32)Profile, *Component->GetName());
}

FCore_PhysicsSettings UCore_PhysicsManager::GetPhysicsProfileSettings(ECore_PhysicsProfile Profile) const
{
    const FCore_PhysicsSettings* Settings = PhysicsProfiles.Find(Profile);
    return Settings ? *Settings : FCore_PhysicsSettings();
}

void UCore_PhysicsManager::SetPhysicsProfileSettings(ECore_PhysicsProfile Profile, const FCore_PhysicsSettings& Settings)
{
    PhysicsProfiles.Add(Profile, Settings);
}

void UCore_PhysicsManager::EnablePhysicsSimulation(AActor* Actor, bool bEnable)
{
    if (!Actor)
    {
        return;
    }
    
    TArray<UPrimitiveComponent*> PrimitiveComponents;
    Actor->GetComponents<UPrimitiveComponent>(PrimitiveComponents);
    
    for (UPrimitiveComponent* Component : PrimitiveComponents)
    {
        Component->SetSimulatePhysics(bEnable);
        
        if (bEnable)
        {
            TrackedPhysicsActors.AddUnique(Actor);
        }
        else
        {
            TrackedPhysicsActors.RemoveSingle(Actor);
        }
    }
}

void UCore_PhysicsManager::SetActorMass(AActor* Actor, float NewMass)
{
    if (!Actor)
    {
        return;
    }
    
    TArray<UPrimitiveComponent*> PrimitiveComponents;
    Actor->GetComponents<UPrimitiveComponent>(PrimitiveComponents);
    
    for (UPrimitiveComponent* Component : PrimitiveComponents)
    {
        if (FBodyInstance* BodyInstance = Component->GetBodyInstance())
        {
            BodyInstance->SetMassOverride(NewMass, true);
        }
    }
}

void UCore_PhysicsManager::AddImpulseToActor(AActor* Actor, FVector Impulse, bool bVelChange)
{
    if (!Actor)
    {
        return;
    }
    
    if (UPrimitiveComponent* RootComponent = Cast<UPrimitiveComponent>(Actor->GetRootComponent()))
    {
        RootComponent->AddImpulse(Impulse, NAME_None, bVelChange);
        
        // Broadcast impact event
        OnPhysicsImpact.Broadcast(Actor, Actor->GetActorLocation(), Impulse.Size());
    }
}

void UCore_PhysicsManager::AddForceToActor(AActor* Actor, FVector Force, bool bAccelChange)
{
    if (!Actor)
    {
        return;
    }
    
    if (UPrimitiveComponent* RootComponent = Cast<UPrimitiveComponent>(Actor->GetRootComponent()))
    {
        RootComponent->AddForce(Force, NAME_None, bAccelChange);
    }
}

bool UCore_PhysicsManager::LineTracePhysics(const FVector& Start, const FVector& End, FHitResult& HitResult, bool bTraceComplex)
{
    if (UWorld* World = GetWorld())
    {
        FCollisionQueryParams QueryParams;
        QueryParams.bTraceComplex = bTraceComplex;
        QueryParams.bReturnPhysicalMaterial = true;
        
        return World->LineTraceSingleByChannel(HitResult, Start, End, ECC_WorldStatic, QueryParams);
    }
    
    return false;
}

bool UCore_PhysicsManager::SphereTracePhysics(const FVector& Start, const FVector& End, float Radius, FHitResult& HitResult)
{
    if (UWorld* World = GetWorld())
    {
        FCollisionQueryParams QueryParams;
        QueryParams.bReturnPhysicalMaterial = true;
        
        return World->SweepSingleByChannel(HitResult, Start, End, FQuat::Identity, 
                                          ECC_WorldStatic, FCollisionShape::MakeSphere(Radius), QueryParams);
    }
    
    return false;
}

TArray<AActor*> UCore_PhysicsManager::GetOverlappingActors(const FVector& Location, float Radius)
{
    TArray<AActor*> OverlappingActors;
    
    if (UWorld* World = GetWorld())
    {
        TArray<FOverlapResult> OverlapResults;
        FCollisionQueryParams QueryParams;
        
        if (World->OverlapMultiByChannel(OverlapResults, Location, FQuat::Identity, 
                                        ECC_WorldDynamic, FCollisionShape::MakeSphere(Radius), QueryParams))
        {
            for (const FOverlapResult& Result : OverlapResults)
            {
                if (Result.GetActor())
                {
                    OverlappingActors.Add(Result.GetActor());
                }
            }
        }
    }
    
    return OverlappingActors;
}

int32 UCore_PhysicsManager::GetActivePhysicsActorCount() const
{
    return ActivePhysicsActorCount;
}

float UCore_PhysicsManager::GetPhysicsFrameTime() const
{
    return LastPhysicsFrameTime;
}

void UCore_PhysicsManager::SetPhysicsSubsteps(int32 MaxSubsteps, float FixedTimeStep)
{
    if (UWorld* World = GetWorld())
    {
        if (UPhysicsSettings* PhysicsSettings = UPhysicsSettings::Get())
        {
            PhysicsSettings->MaxSubsteps = MaxSubsteps;
            PhysicsSettings->FixedTimeStep = FixedTimeStep;
        }
    }
}

void UCore_PhysicsManager::UpdatePhysicsMetrics()
{
    // Clean up invalid actor references
    TrackedPhysicsActors.RemoveAll([](const TWeakObjectPtr<AActor>& ActorPtr) {
        return !ActorPtr.IsValid();
    });
    
    // Count active physics actors
    ActivePhysicsActorCount = 0;
    for (const TWeakObjectPtr<AActor>& ActorPtr : TrackedPhysicsActors)
    {
        if (AActor* Actor = ActorPtr.Get())
        {
            if (UPrimitiveComponent* RootComponent = Cast<UPrimitiveComponent>(Actor->GetRootComponent()))
            {
                if (RootComponent->IsSimulatingPhysics())
                {
                    ActivePhysicsActorCount++;
                }
            }
        }
    }
    
    // Update frame time
    if (UWorld* World = GetWorld())
    {
        LastPhysicsFrameTime = World->GetDeltaSeconds();
    }
}

void UCore_PhysicsManager::ValidatePhysicsSetup()
{
    UE_LOG(LogTemp, Log, TEXT("=== Physics System Validation ==="));
    UE_LOG(LogTemp, Log, TEXT("Active Physics Actors: %d"), GetActivePhysicsActorCount());
    UE_LOG(LogTemp, Log, TEXT("Physics Frame Time: %f ms"), GetPhysicsFrameTime() * 1000.0f);
    UE_LOG(LogTemp, Log, TEXT("Physics Profiles Loaded: %d"), PhysicsProfiles.Num());
    
    // Validate each physics profile
    for (const auto& ProfilePair : PhysicsProfiles)
    {
        const FCore_PhysicsSettings& Settings = ProfilePair.Value;
        UE_LOG(LogTemp, Log, TEXT("Profile %d - Mass: %f, Damping: %f/%f, Restitution: %f"), 
               (int32)ProfilePair.Key, Settings.Mass, Settings.LinearDamping, 
               Settings.AngularDamping, Settings.Restitution);
    }
}

void UCore_PhysicsManager::OptimizePhysicsPerformance()
{
    int32 OptimizedCount = 0;
    
    for (const TWeakObjectPtr<AActor>& ActorPtr : TrackedPhysicsActors)
    {
        if (AActor* Actor = ActorPtr.Get())
        {
            // Disable physics for distant actors
            if (UWorld* World = GetWorld())
            {
                if (APawn* PlayerPawn = World->GetFirstPlayerController()->GetPawn())
                {
                    float Distance = FVector::Dist(Actor->GetActorLocation(), PlayerPawn->GetActorLocation());
                    if (Distance > 5000.0f) // 50 meters
                    {
                        EnablePhysicsSimulation(Actor, false);
                        OptimizedCount++;
                    }
                }
            }
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Physics optimization disabled simulation for %d distant actors"), OptimizedCount);
}