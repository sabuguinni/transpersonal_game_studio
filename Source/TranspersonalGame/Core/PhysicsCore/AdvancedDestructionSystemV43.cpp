#include "AdvancedDestructionSystemV43.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "TimerManager.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "GeometryCollection/GeometryCollectionComponent.h"
#include "Field/FieldSystemComponent.h"
#include "Field/FieldSystemActor.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/StaticMeshActor.h"

DEFINE_LOG_CATEGORY(LogDestructionV43);

UAdvancedDestructionSystemV43::UAdvancedDestructionSystemV43()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PostPhysics;
    
    // Create geometry collection component
    GeometryCollectionComponent = CreateDefaultSubobject<UGeometryCollectionComponent>(TEXT(\"GeometryCollection\"));
    
    // Create field system component
    FieldSystemComponent = CreateDefaultSubobject<UFieldSystemComponent>(TEXT(\"FieldSystem\"));
    
    UE_LOG(LogDestructionV43, Log, TEXT(\"Advanced Destruction System V43 component created\"));
}

void UAdvancedDestructionSystemV43::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogDestructionV43, Log, TEXT(\"Advanced Destruction System V43 beginning play\"));
    
    InitializeDestructionThresholds();
    InitializeIntensityMultipliers();
    InitializeDestructionSystem();
    
    // Setup cleanup timer
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(CleanupTimer, this, 
            &UAdvancedDestructionSystemV43::CleanupDestroyedPieces, 5.0f, true);
    }
}

void UAdvancedDestructionSystemV43::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    // Clear cleanup timer
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(CleanupTimer);
    }
    
    // Cleanup spawned debris
    for (TObjectPtr<AActor> Debris : SpawnedDebris)
    {
        if (IsValid(Debris.Get()))
        {
            Debris->Destroy();
        }
    }
    SpawnedDebris.Empty();
    
    Super::EndPlay(EndPlayReason);
}

void UAdvancedDestructionSystemV43::TickComponent(float DeltaTime, ELevelTick TickType, 
    FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Update destruction health regeneration (if applicable)
    if (DestructionHealth < 1.0f && !bIsDestroyed)
    {
        // Very slow natural healing for some materials
        DestructionHealth = FMath::Min(1.0f, DestructionHealth + (DeltaTime * 0.01f)); // 1% per second
    }
    
    // Performance monitoring
    if (DestructionEventCount > 0)
    {
        LastDestructionTime += DeltaTime;
        if (LastDestructionTime > 1.0f)
        {
            DestructionEventCount = 0;
            LastDestructionTime = 0.0f;
        }
    }
}

void UAdvancedDestructionSystemV43::InitializeDestructionSystem()
{
    UE_LOG(LogDestructionV43, Log, TEXT(\"Initializing destruction system for actor: %s\"), 
        GetOwner() ? *GetOwner()->GetName() : TEXT(\"Unknown\"));
    
    if (!bDestructionEnabled)
    {
        UE_LOG(LogDestructionV43, Log, TEXT(\"Destruction disabled for this actor\"));
        return;
    }
    
    // Setup geometry collection if we have a static mesh
    if (AActor* Owner = GetOwner())
    {
        if (UStaticMeshComponent* MeshComp = Owner->FindComponentByClass<UStaticMeshComponent>())
        {
            SetupGeometryCollection(MeshComp);
        }
    }
    
    UE_LOG(LogDestructionV43, Log, TEXT(\"Destruction system initialized successfully\"));
}

void UAdvancedDestructionSystemV43::SetupGeometryCollection(UStaticMeshComponent* MeshComponent)
{
    if (!MeshComponent || !GeometryCollectionComponent)
    {
        UE_LOG(LogDestructionV43, Warning, TEXT(\"Invalid mesh component or geometry collection component\"));
        return;
    }
    
    UE_LOG(LogDestructionV43, Log, TEXT(\"Setting up geometry collection for destruction\"));
    
    // Copy transform from mesh component
    GeometryCollectionComponent->SetWorldTransform(MeshComponent->GetComponentTransform());
    
    // Configure destruction settings
    GeometryCollectionComponent->SetNotifyBreaks(true);
    GeometryCollectionComponent->SetNotifyCollisions(true);
    GeometryCollectionComponent->SetNotifyTrailing(true);
    
    // Set damage threshold
    GeometryCollectionComponent->SetDamageThreshold(MaxDestructionHealth);
    
    // Attach to owner
    if (AActor* Owner = GetOwner())
    {
        GeometryCollectionComponent->AttachToComponent(Owner->GetRootComponent(), 
            FAttachmentTransformRules::KeepWorldTransform);
    }
}

void UAdvancedDestructionSystemV43::ApplyDestructionForce(FVector Location, float Radius, float Strength, 
    EDestructionEventType EventType)
{
    if (!bDestructionEnabled || bIsDestroyed)
    {
        return;
    }
    
    if (!ShouldProcessDestruction())
    {
        return;
    }
    
    UE_LOG(LogDestructionV43, Log, TEXT(\"Applying destruction force at %s with radius %f and strength %f\"), 
        *Location.ToString(), Radius, Strength);
    
    // Calculate damage based on force and event type
    float* ThresholdPtr = DestructionThresholds.Find(EventType);
    float Threshold = ThresholdPtr ? *ThresholdPtr : 50.0f;
    
    float Damage = (Strength / Threshold) * 10.0f; // Convert force to damage
    
    ProcessDestructionDamage(Damage, EventType, Location);
    
    // Create physics field for the destruction force
    if (FieldSystemComponent && GetWorld())
    {
        // Create radial force field
        AFieldSystemActor* FieldActor = GetWorld()->SpawnActor<AFieldSystemActor>();
        if (FieldActor)
        {
            FieldActor->SetActorLocation(Location);
            
            // Schedule cleanup of field actor
            FTimerHandle FieldCleanupTimer;
            GetWorld()->GetTimerManager().SetTimer(FieldCleanupTimer, [FieldActor]()
            {
                if (IsValid(FieldActor))
                {
                    FieldActor->Destroy();
                }
            }, 2.0f, false);
        }
    }
    
    DestructionEventCount++;
}

void UAdvancedDestructionSystemV43::ApplyDirectionalDestruction(FVector Direction, float Force, 
    EDestructionEventType EventType)
{
    if (!bDestructionEnabled || bIsDestroyed)
    {
        return;
    }
    
    if (!ShouldProcessDestruction())
    {
        return;
    }
    
    UE_LOG(LogDestructionV43, Log, TEXT(\"Applying directional destruction in direction %s with force %f\"), 
        *Direction.ToString(), Force);
    
    FVector ActorLocation = GetOwner() ? GetOwner()->GetActorLocation() : FVector::ZeroVector;
    
    // Calculate damage based on force and event type
    float* ThresholdPtr = DestructionThresholds.Find(EventType);
    float Threshold = ThresholdPtr ? *ThresholdPtr : 50.0f;
    
    float Damage = (Force / Threshold) * 15.0f; // Directional forces are more effective
    
    ProcessDestructionDamage(Damage, EventType, ActorLocation);
    
    // Apply directional impulse if we have a geometry collection
    if (GeometryCollectionComponent)
    {
        GeometryCollectionComponent->ApplyLinearVelocity(0, Direction * Force);
    }
    
    DestructionEventCount++;
}

void UAdvancedDestructionSystemV43::TriggerCatastrophicDestruction(EDestructionIntensity Intensity)
{
    if (!bDestructionEnabled || bIsDestroyed)
    {
        return;
    }
    
    UE_LOG(LogDestructionV43, Warning, TEXT(\"Triggering catastrophic destruction with intensity %d\"), (int32)Intensity);
    
    FVector ActorLocation = GetOwner() ? GetOwner()->GetActorLocation() : FVector::ZeroVector;
    
    // Apply massive damage to trigger immediate destruction
    float CatastrophicDamage = MaxDestructionHealth * 2.0f;
    ProcessDestructionDamage(CatastrophicDamage, EDestructionEventType::StructureCollapse, ActorLocation);
    
    ExecuteDestruction(EDestructionEventType::StructureCollapse, Intensity, ActorLocation);
}

void UAdvancedDestructionSystemV43::SetDestructionThreshold(EDestructionEventType EventType, float Threshold)
{
    DestructionThresholds.FindOrAdd(EventType) = Threshold;
    
    UE_LOG(LogDestructionV43, Log, TEXT(\"Set destruction threshold for event type %d to %f\"), 
        (int32)EventType, Threshold);
}

void UAdvancedDestructionSystemV43::SetDestructionEnabled(bool bEnabled)
{
    bDestructionEnabled = bEnabled;
    
    if (GeometryCollectionComponent)
    {
        GeometryCollectionComponent->SetNotifyBreaks(bEnabled);
        GeometryCollectionComponent->SetNotifyCollisions(bEnabled);
    }
    
    UE_LOG(LogDestructionV43, Log, TEXT(\"Destruction %s for actor\"), 
        bEnabled ? TEXT(\"enabled\") : TEXT(\"disabled\"));
}

bool UAdvancedDestructionSystemV43::CanBeDestroyed() const
{
    return bDestructionEnabled && !bIsDestroyed && DestructionHealth > 0.0f;
}

void UAdvancedDestructionSystemV43::ResetDestructionState()
{
    DestructionHealth = 1.0f;
    AccumulatedDamage = 0.0f;
    bIsDestroyed = false;
    
    // Cleanup existing debris
    for (TObjectPtr<AActor> Debris : SpawnedDebris)
    {
        if (IsValid(Debris.Get()))
        {
            Debris->Destroy();
        }
    }
    SpawnedDebris.Empty();
    
    UE_LOG(LogDestructionV43, Log, TEXT(\"Destruction state reset for actor\"));
}

void UAdvancedDestructionSystemV43::InitializeDestructionThresholds()
{
    // Set default destruction thresholds for different event types
    DestructionThresholds.Add(EDestructionEventType::TreeFall, 75.0f);
    DestructionThresholds.Add(EDestructionEventType::RockCrush, 100.0f);
    DestructionThresholds.Add(EDestructionEventType::StructureCollapse, 50.0f);
    DestructionThresholds.Add(EDestructionEventType::GroundCrack, 150.0f);
    DestructionThresholds.Add(EDestructionEventType::VegetationTrampling, 25.0f);
    DestructionThresholds.Add(EDestructionEventType::WaterSplash, 200.0f);
    
    UE_LOG(LogDestructionV43, Log, TEXT(\"Destruction thresholds initialized\"));
}

void UAdvancedDestructionSystemV43::InitializeIntensityMultipliers()
{
    // Set intensity multipliers for different destruction levels
    IntensityMultipliers.Add(EDestructionIntensity::Light, 0.5f);
    IntensityMultipliers.Add(EDestructionIntensity::Medium, 1.0f);
    IntensityMultipliers.Add(EDestructionIntensity::Heavy, 2.0f);
    IntensityMultipliers.Add(EDestructionIntensity::Catastrophic, 5.0f);
    
    UE_LOG(LogDestructionV43, Log, TEXT(\"Intensity multipliers initialized\"));
}

void UAdvancedDestructionSystemV43::ProcessDestructionDamage(float Damage, EDestructionEventType EventType, FVector Location)
{
    if (!CanBeDestroyed())
    {
        return;
    }
    
    // Apply damage to destruction health
    AccumulatedDamage += Damage;
    DestructionHealth = FMath::Max(0.0f, 1.0f - (AccumulatedDamage / MaxDestructionHealth));
    
    UE_LOG(LogDestructionV43, Log, TEXT(\"Processing destruction damage: %f, Health: %f\"), 
        Damage, DestructionHealth);
    
    // Determine destruction intensity based on damage
    EDestructionIntensity Intensity = EDestructionIntensity::Light;
    if (Damage > MaxDestructionHealth * 0.8f)
    {
        Intensity = EDestructionIntensity::Catastrophic;
    }
    else if (Damage > MaxDestructionHealth * 0.5f)
    {
        Intensity = EDestructionIntensity::Heavy;
    }
    else if (Damage > MaxDestructionHealth * 0.25f)
    {
        Intensity = EDestructionIntensity::Medium;
    }
    
    // Check if destruction threshold is reached
    if (DestructionHealth <= 0.0f && !bIsDestroyed)
    {
        ExecuteDestruction(EventType, Intensity, Location);
    }
    else
    {
        // Spawn minor effects for partial damage
        SpawnDestructionEffects(EventType, Location, EDestructionIntensity::Light);
    }
}

void UAdvancedDestructionSystemV43::ExecuteDestruction(EDestructionEventType EventType, 
    EDestructionIntensity Intensity, FVector Location)
{
    if (bIsDestroyed)
    {
        return;
    }
    
    bIsDestroyed = true;
    CurrentDestructionCount++;
    
    UE_LOG(LogDestructionV43, Warning, TEXT(\"Executing destruction of type %d with intensity %d\"), 
        (int32)EventType, (int32)Intensity);
    
    // Broadcast destruction event
    OnDestructionEvent.Broadcast(EventType, GetOwner(), Location, Intensity);
    
    // Spawn destruction effects
    SpawnDestructionEffects(EventType, Location, Intensity);
    
    // Spawn debris
    if (bSpawnDebris)
    {
        SpawnDebris(Location, Intensity);
    }
    
    // Trigger geometry collection destruction
    if (GeometryCollectionComponent)
    {
        GeometryCollectionComponent->ApplyBreakingLinearVelocity(0, FVector::UpVector * 500.0f);
    }
    
    // Schedule cleanup
    if (UWorld* World = GetWorld())
    {
        FTimerHandle DestructionCleanupTimer;
        World->GetTimerManager().SetTimer(DestructionCleanupTimer, [this]()
        {
            CurrentDestructionCount = FMath::Max(0, CurrentDestructionCount - 1);
        }, DebrisLifetime, false);
    }
}

void UAdvancedDestructionSystemV43::SpawnDestructionEffects(EDestructionEventType EventType, 
    FVector Location, EDestructionIntensity Intensity)
{
    // Spawn visual effects
    if (UNiagaraSystem** EffectPtr = DestructionEffects.Find(EventType))
    {
        if (UNiagaraSystem* Effect = *EffectPtr)
        {
            UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), Effect, Location);
        }
    }
    
    // Play destruction sound
    if (USoundBase** SoundPtr = DestructionSounds.Find(EventType))
    {
        if (USoundBase* Sound = *SoundPtr)
        {
            UGameplayStatics::PlaySoundAtLocation(GetWorld(), Sound, Location);
        }
    }
    
    UE_LOG(LogDestructionV43, Log, TEXT(\"Spawned destruction effects at %s\"), *Location.ToString());
}

void UAdvancedDestructionSystemV43::SpawnDebris(FVector Location, EDestructionIntensity Intensity)
{
    if (!bSpawnDebris || CurrentDestructionCount >= MaxSimultaneousDestructions)
    {
        return;
    }
    
    // Calculate debris count based on intensity
    float* IntensityMultiplier = IntensityMultipliers.Find(Intensity);
    float Multiplier = IntensityMultiplier ? *IntensityMultiplier : 1.0f;
    
    int32 DebrisCount = FMath::RoundToInt(MaxDebrisCount * Multiplier * 0.5f);
    DebrisCount = FMath::Min(DebrisCount, MaxDebrisCount);
    
    UE_LOG(LogDestructionV43, Log, TEXT(\"Spawning %d debris pieces at %s\"), DebrisCount, *Location.ToString());
    
    for (int32 i = 0; i < DebrisCount; i++)
    {
        // Create debris actor (simplified - would use proper debris meshes in production)
        if (UWorld* World = GetWorld())
        {
            AStaticMeshActor* DebrisActor = World->SpawnActor<AStaticMeshActor>();
            if (DebrisActor)
            {
                // Random position around destruction location
                FVector RandomOffset = FVector(
                    FMath::RandRange(-100.0f, 100.0f),
                    FMath::RandRange(-100.0f, 100.0f),
                    FMath::RandRange(0.0f, 50.0f)
                );
                
                DebrisActor->SetActorLocation(Location + RandomOffset);
                
                // Add to debris list for cleanup
                SpawnedDebris.Add(DebrisActor);
                
                // Schedule individual debris cleanup
                FTimerHandle DebrisCleanupTimer;
                World->GetTimerManager().SetTimer(DebrisCleanupTimer, [this, DebrisActor]()
                {
                    if (IsValid(DebrisActor))
                    {
                        SpawnedDebris.Remove(DebrisActor);
                        DebrisActor->Destroy();
                    }
                }, DebrisLifetime, false);
            }
        }
    }
}

bool UAdvancedDestructionSystemV43::ShouldProcessDestruction() const
{
    if (!bUseDistanceLOD)
    {
        return true;
    }
    
    float PlayerDistance = GetPlayerDistance();
    return PlayerDistance <= MaxDestructionDistance;
}

float UAdvancedDestructionSystemV43::GetPlayerDistance() const
{
    if (UWorld* World = GetWorld())
    {
        if (APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(World, 0))
        {
            FVector ActorLocation = GetOwner() ? GetOwner()->GetActorLocation() : FVector::ZeroVector;
            return FVector::Dist(PlayerPawn->GetActorLocation(), ActorLocation);
        }
    }
    
    return 0.0f; // Assume close if no player found
}

void UAdvancedDestructionSystemV43::CleanupDestroyedPieces()
{
    // Remove invalid debris from tracking
    SpawnedDebris.RemoveAll([](const TObjectPtr<AActor>& Debris)
    {
        return !IsValid(Debris.Get());
    });
    
    // Log cleanup statistics
    if (SpawnedDebris.Num() > 0)
    {
        UE_LOG(LogDestructionV43, Log, TEXT(\"Cleanup: %d debris pieces remaining\"), SpawnedDebris.Num());
    }
}