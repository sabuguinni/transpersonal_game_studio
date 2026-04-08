// Copyright Transpersonal Game Studio. All Rights Reserved.

#include "DestructionSystem.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Components/StaticMeshComponent.h"
#include "GeometryCollection/GeometryCollectionActor.h"
#include "Field/FieldSystemActor.h"
#include "Field/FieldSystemComponent.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "Sound/SoundBase.h"
#include "TimerManager.h"

DEFINE_LOG_CATEGORY(LogDestructionSystem);

UDestructionSystem::UDestructionSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PostPhysics;

    // Create components
    GeometryCollectionComponent = CreateDefaultSubobject<UGeometryCollectionComponent>(TEXT("GeometryCollectionComponent"));
    FieldSystemComponent = CreateDefaultSubobject<UFieldSystemComponent>(TEXT("FieldSystemComponent"));
    DestructionVFXComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("DestructionVFXComponent"));

    // Configure Geometry Collection
    if (GeometryCollectionComponent)
    {
        GeometryCollectionComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        GeometryCollectionComponent->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
        GeometryCollectionComponent->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
        GeometryCollectionComponent->SetNotifyBreaks(true);
    }

    // Configure VFX component
    if (DestructionVFXComponent)
    {
        DestructionVFXComponent->SetAutoActivate(false);
    }
}

void UDestructionSystem::BeginPlay()
{
    Super::BeginPlay();

    CachedWorld = GetWorld();
    
    if (CachedWorld)
    {
        // Register with destruction manager
        if (UDestructionManagerSubsystem* DestructionManager = UDestructionManagerSubsystem::Get(this))
        {
            DestructionManager->RegisterDestructionSystem(this);
        }

        SetupGeometryCollection();
    }

    UE_LOG(LogDestructionSystem, Log, TEXT("Destruction System initialized for %s"), 
           GetOwner() ? *GetOwner()->GetName() : TEXT("Unknown"));
}

void UDestructionSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    // Update destruction health based on Geometry Collection state
    if (GeometryCollectionComponent && GeometryCollectionComponent->GetRestCollection())
    {
        // Calculate health based on intact vs broken pieces
        // This is a simplified calculation - in production you'd want more sophisticated tracking
        float IntactPercentage = 1.0f; // Would be calculated from actual geometry collection state
        CurrentHealth = FMath::Clamp(IntactPercentage, 0.0f, 1.0f);
        
        if (CurrentHealth <= 0.0f && !bIsDestroyed)
        {
            bIsDestroyed = true;
            ScheduleFragmentCleanup();
        }
    }
}

void UDestructionSystem::InitializeDestruction(UGeometryCollection* DestructionMesh, float InDestructionThreshold, int32 InMaxFragments)
{
    if (!GeometryCollectionComponent)
    {
        UE_LOG(LogDestructionSystem, Error, TEXT("GeometryCollectionComponent is null"));
        return;
    }

    DestructionThreshold = InDestructionThreshold;
    MaxFragments = InMaxFragments;

    if (DestructionMesh)
    {
        GeometryCollectionComponent->SetRestCollection(DestructionMesh);
        
        // Configure destruction settings
        GeometryCollectionComponent->SetDamageThreshold({DestructionThreshold});
        
        UE_LOG(LogDestructionSystem, Log, TEXT("Destruction initialized with threshold: %f, max fragments: %d"), 
               DestructionThreshold, MaxFragments);
    }
    else
    {
        UE_LOG(LogDestructionSystem, Warning, TEXT("DestructionMesh is null"));
    }
}

void UDestructionSystem::TriggerDestruction(FVector ImpactLocation, float ImpactForce, float ImpactRadius, AActor* Instigator)
{
    if (!bDestructionEnabled || bIsDestroyed)
    {
        return;
    }

    if (ImpactForce < DestructionThreshold)
    {
        UE_LOG(LogDestructionSystem, Log, TEXT("Impact force %f below threshold %f"), ImpactForce, DestructionThreshold);
        return;
    }

    UE_LOG(LogDestructionSystem, Log, TEXT("Triggering destruction at location %s with force %f"), 
           *ImpactLocation.ToString(), ImpactForce);

    // Apply destruction field
    ApplyDestructionField(ImpactLocation, ImpactRadius, ImpactForce);

    // Apply damage
    float Damage = ImpactForce / DestructionThreshold;
    ApplyDamage(Damage);

    // Spawn effects
    SpawnDestructionEffects(ImpactLocation);

    // Broadcast destruction event
    OnDestruction.Broadcast(GetOwner(), ImpactLocation, ImpactForce, Instigator);
}

void UDestructionSystem::ApplyDestructionField(FVector FieldLocation, float FieldRadius, float FieldStrength, float Duration)
{
    if (!FieldSystemComponent || !CachedWorld)
    {
        return;
    }

    CreateDestructionField(FieldLocation, FieldRadius, FieldStrength);

    // Schedule field cleanup after duration
    if (Duration > 0.0f)
    {
        FTimerHandle FieldCleanupTimer;
        CachedWorld->GetTimerManager().SetTimer(FieldCleanupTimer, [this]()
        {
            if (FieldSystemComponent)
            {
                FieldSystemComponent->ResetFieldSystem();
            }
        }, Duration, false);
    }
}

bool UDestructionSystem::CanBeDestroyed() const
{
    return bDestructionEnabled && !bIsDestroyed && CurrentHealth > 0.0f;
}

void UDestructionSystem::CleanupFragments()
{
    for (AActor* Fragment : CreatedFragments)
    {
        if (IsValid(Fragment))
        {
            Fragment->Destroy();
        }
    }
    CreatedFragments.Empty();

    UE_LOG(LogDestructionSystem, Log, TEXT("Cleaned up destruction fragments"));
}

void UDestructionSystem::SetupGeometryCollection()
{
    if (!GeometryCollectionComponent)
    {
        return;
    }

    // Configure physics simulation
    GeometryCollectionComponent->SetSimulatePhysics(true);
    GeometryCollectionComponent->SetEnableGravity(true);
    
    // Set up collision
    GeometryCollectionComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    GeometryCollectionComponent->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
    
    // Configure clustering for performance
    GeometryCollectionComponent->SetEnableClustering(true);
    GeometryCollectionComponent->SetClusterGroupIndex(0);

    UE_LOG(LogDestructionSystem, Log, TEXT("Geometry Collection setup complete"));
}

void UDestructionSystem::CreateDestructionField(FVector Location, float Radius, float Strength)
{
    if (!FieldSystemComponent)
    {
        return;
    }

    // Create a radial force field for destruction
    // This would typically use UE5's Field System nodes
    // For now, we'll apply force directly to the geometry collection
    
    if (GeometryCollectionComponent)
    {
        // Apply radial impulse to geometry collection
        GeometryCollectionComponent->ApplyRadialImpulse(Location, Radius, Strength, ERadialImpulseFalloff::RIF_Linear, true);
    }

    UE_LOG(LogDestructionSystem, Log, TEXT("Applied destruction field at %s with radius %f and strength %f"), 
           *Location.ToString(), Radius, Strength);
}

void UDestructionSystem::ApplyDamage(float Damage)
{
    CurrentHealth = FMath::Clamp(CurrentHealth - Damage, 0.0f, 1.0f);
    
    UE_LOG(LogDestructionSystem, Log, TEXT("Applied damage %f, current health: %f"), Damage, CurrentHealth);
    
    if (CurrentHealth <= 0.0f && !bIsDestroyed)
    {
        bIsDestroyed = true;
        UE_LOG(LogDestructionSystem, Log, TEXT("Object fully destroyed"));
    }
}

void UDestructionSystem::SpawnDestructionEffects(FVector Location)
{
    // Spawn Niagara VFX
    if (DestructionEffect && CachedWorld)
    {
        UNiagaraFunctionLibrary::SpawnSystemAtLocation(
            CachedWorld, 
            DestructionEffect, 
            Location, 
            FRotator::ZeroRotator
        );
    }

    // Play destruction sound
    if (DestructionSound && CachedWorld)
    {
        UGameplayStatics::PlaySoundAtLocation(
            CachedWorld, 
            DestructionSound, 
            Location
        );
    }

    UE_LOG(LogDestructionSystem, Log, TEXT("Spawned destruction effects at %s"), *Location.ToString());
}

void UDestructionSystem::ScheduleFragmentCleanup()
{
    if (!CachedWorld)
    {
        return;
    }

    CachedWorld->GetTimerManager().SetTimer(FragmentCleanupTimer, [this]()
    {
        CleanupFragments();
    }, FragmentLifetime, false);

    UE_LOG(LogDestructionSystem, Log, TEXT("Scheduled fragment cleanup in %f seconds"), FragmentLifetime);
}

// Destruction Manager Subsystem Implementation

void UDestructionManagerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    // Start metrics update timer
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(MetricsUpdateTimer, this, &UDestructionManagerSubsystem::UpdateMetrics, 1.0f, true);
    }

    UE_LOG(LogDestructionSystem, Log, TEXT("Destruction Manager Subsystem initialized"));
}

void UDestructionManagerSubsystem::Deinitialize()
{
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(MetricsUpdateTimer);
    }

    RegisteredSystems.Empty();
    Super::Deinitialize();
}

void UDestructionManagerSubsystem::RegisterDestructionSystem(UDestructionSystem* DestructionSystem)
{
    if (DestructionSystem && !RegisteredSystems.Contains(DestructionSystem))
    {
        RegisteredSystems.Add(DestructionSystem);
        UE_LOG(LogDestructionSystem, Log, TEXT("Registered destruction system: %s"), 
               DestructionSystem->GetOwner() ? *DestructionSystem->GetOwner()->GetName() : TEXT("Unknown"));
    }
}

void UDestructionManagerSubsystem::UnregisterDestructionSystem(UDestructionSystem* DestructionSystem)
{
    RegisteredSystems.Remove(DestructionSystem);
    UE_LOG(LogDestructionSystem, Log, TEXT("Unregistered destruction system"));
}

UDestructionManagerSubsystem* UDestructionManagerSubsystem::Get(const UObject* WorldContext)
{
    if (const UWorld* World = GEngine->GetWorldFromContextObject(WorldContext, EGetWorldErrorMode::LogAndReturnNull))
    {
        return World->GetSubsystem<UDestructionManagerSubsystem>();
    }
    return nullptr;
}

void UDestructionManagerSubsystem::CleanupAllFragments()
{
    for (UDestructionSystem* System : RegisteredSystems)
    {
        if (IsValid(System))
        {
            System->CleanupFragments();
        }
    }

    UE_LOG(LogDestructionSystem, Log, TEXT("Cleaned up all destruction fragments"));
}

void UDestructionManagerSubsystem::GetDestructionMetrics(int32& ActiveFragments, int32& ActiveDestructions) const
{
    ActiveFragments = TotalActiveFragments;
    ActiveDestructions = TotalActiveDestructions;
}

void UDestructionManagerSubsystem::UpdateMetrics()
{
    TotalActiveFragments = 0;
    TotalActiveDestructions = 0;

    for (UDestructionSystem* System : RegisteredSystems)
    {
        if (IsValid(System))
        {
            if (System->GetDestructionHealth() < 1.0f)
            {
                TotalActiveDestructions++;
            }
            // Count fragments would require access to GeometryCollection internals
        }
    }
}