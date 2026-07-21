#include "Eng_PhysicsArchitecture.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "TimerManager.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "Kismet/GameplayStatics.h"

void UEng_PhysicsArchitecture::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("ENGINE ARCHITECT: Physics Architecture Subsystem Initialized"));
    
    // Initialize default settings
    MaxPhysicsBodies = 1000;
    PhysicsLODDistance = 5000.0f;
    bEnableEnvironmentalDestruction = true;
    EventHistoryDuration = 30.0f;
    
    // Clear registries
    DinosaurPhysicsRegistry.Empty();
    EnvironmentalRegistry.Empty();
    RecentEvents.Empty();
}

void UEng_PhysicsArchitecture::Deinitialize()
{
    // Clear all timers
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(OptimizationTimer);
        World->GetTimerManager().ClearTimer(ValidationTimer);
    }
    
    // Clear registries
    DinosaurPhysicsRegistry.Empty();
    EnvironmentalRegistry.Empty();
    RecentEvents.Empty();
    
    UE_LOG(LogTemp, Warning, TEXT("ENGINE ARCHITECT: Physics Architecture Subsystem Deinitialized"));
    
    Super::Deinitialize();
}

void UEng_PhysicsArchitecture::OnWorldBeginPlay(UWorld& InWorld)
{
    Super::OnWorldBeginPlay(InWorld);
    
    UE_LOG(LogTemp, Warning, TEXT("ENGINE ARCHITECT: Physics Architecture World Begin Play"));
    
    // Start optimization timer (every 5 seconds)
    InWorld.GetTimerManager().SetTimer(OptimizationTimer, this, &UEng_PhysicsArchitecture::UpdatePhysicsLOD, 5.0f, true);
    
    // Start validation timer (every 30 seconds)
    InWorld.GetTimerManager().SetTimer(ValidationTimer, this, &UEng_PhysicsArchitecture::ValidatePhysicsIntegrity, 30.0f, true);
    
    // Clean up old events periodically
    InWorld.GetTimerManager().SetTimer(FTimerHandle(), this, &UEng_PhysicsArchitecture::CleanupOldEvents, 10.0f, true);
}

bool UEng_PhysicsArchitecture::RegisterDinosaurPhysics(AActor* DinosaurActor, const FEng_DinosaurPhysicsConfig& Config)
{
    if (!IsValid(DinosaurActor))
    {
        UE_LOG(LogTemp, Error, TEXT("ENGINE ARCHITECT: Cannot register null dinosaur actor"));
        return false;
    }
    
    // Register the configuration
    DinosaurPhysicsRegistry.Add(DinosaurActor, Config);
    
    // Apply physics settings to the actor
    if (UPrimitiveComponent* PrimComp = DinosaurActor->FindComponentByClass<UPrimitiveComponent>())
    {
        // Set mass
        if (PrimComp->IsSimulatingPhysics())
        {
            PrimComp->SetMassOverrideInKg(NAME_None, Config.Mass);
        }
        
        // Set collision settings based on category
        switch (Config.Category)
        {
        case EEng_DinosaurPhysicsCategory::MegaHerbivore:
        case EEng_DinosaurPhysicsCategory::ApexPredator:
            PrimComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
            PrimComp->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
            break;
        default:
            PrimComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
            break;
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("ENGINE ARCHITECT: Registered dinosaur physics - %s (Mass: %.1fkg)"), 
           *DinosaurActor->GetName(), Config.Mass);
    
    return true;
}

bool UEng_PhysicsArchitecture::UnregisterDinosaurPhysics(AActor* DinosaurActor)
{
    if (!IsValid(DinosaurActor))
    {
        return false;
    }
    
    int32 RemovedCount = DinosaurPhysicsRegistry.Remove(DinosaurActor);
    
    UE_LOG(LogTemp, Warning, TEXT("ENGINE ARCHITECT: Unregistered dinosaur physics - %s"), 
           *DinosaurActor->GetName());
    
    return RemovedCount > 0;
}

FEng_DinosaurPhysicsConfig UEng_PhysicsArchitecture::GetDinosaurPhysicsConfig(AActor* DinosaurActor)
{
    if (FEng_DinosaurPhysicsConfig* Config = DinosaurPhysicsRegistry.Find(DinosaurActor))
    {
        return *Config;
    }
    
    // Return default configuration
    return FEng_DinosaurPhysicsConfig();
}

bool UEng_PhysicsArchitecture::RegisterEnvironmentalObject(AActor* EnvironmentActor, const FEng_PhysicsMaterialProperties& Properties)
{
    if (!IsValid(EnvironmentActor))
    {
        UE_LOG(LogTemp, Error, TEXT("ENGINE ARCHITECT: Cannot register null environment actor"));
        return false;
    }
    
    // Register the properties
    EnvironmentalRegistry.Add(EnvironmentActor, Properties);
    
    // Apply physics material properties
    if (UPrimitiveComponent* PrimComp = EnvironmentActor->FindComponentByClass<UPrimitiveComponent>())
    {
        // Set physics material properties if possible
        if (UPhysicalMaterial* PhysMat = PrimComp->GetBodyInstance()->GetSimplePhysicalMaterial())
        {
            // Note: In a real implementation, you'd create custom physics materials
            // For now, we'll log the registration
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("ENGINE ARCHITECT: Registered environmental object - %s (Type: %s, Destructible: %s)"), 
           *EnvironmentActor->GetName(), *Properties.MaterialType, 
           Properties.bDestructible ? TEXT("Yes") : TEXT("No"));
    
    return true;
}

void UEng_PhysicsArchitecture::ProcessEnvironmentalDestruction(const FEng_PhysicsEventData& EventData)
{
    if (!bEnableEnvironmentalDestruction)
    {
        return;
    }
    
    if (!IsValid(EventData.TargetActor))
    {
        return;
    }
    
    // Check if target is registered as destructible
    if (FEng_PhysicsMaterialProperties* Properties = EnvironmentalRegistry.Find(EventData.TargetActor))
    {
        if (Properties->bDestructible && EventData.ImpactForce > Properties->DestructionThreshold)
        {
            UE_LOG(LogTemp, Warning, TEXT("ENGINE ARCHITECT: Environmental destruction triggered - %s (Force: %.1f)"), 
                   *EventData.TargetActor->GetName(), EventData.ImpactForce);
            
            // In a full implementation, this would trigger destruction effects
            // For now, we'll just log and potentially hide the actor
            EventData.TargetActor->SetActorHiddenInGame(true);
            
            // Broadcast the destruction event
            BroadcastPhysicsEvent(EventData);
        }
    }
}

void UEng_PhysicsArchitecture::HandleDinosaurCollision(AActor* Dinosaur1, AActor* Dinosaur2, const FHitResult& HitResult)
{
    if (!IsValid(Dinosaur1) || !IsValid(Dinosaur2))
    {
        return;
    }
    
    FEng_DinosaurPhysicsConfig* Config1 = DinosaurPhysicsRegistry.Find(Dinosaur1);
    FEng_DinosaurPhysicsConfig* Config2 = DinosaurPhysicsRegistry.Find(Dinosaur2);
    
    if (!Config1 || !Config2)
    {
        UE_LOG(LogTemp, Warning, TEXT("ENGINE ARCHITECT: Dinosaur collision with unregistered actors"));
        return;
    }
    
    // Calculate collision force based on masses and velocities
    float CollisionForce = (Config1->Mass + Config2->Mass) * 0.1f; // Simplified calculation
    
    // Create physics event
    FEng_PhysicsEventData EventData;
    EventData.InteractionType = EEng_PhysicsInteractionType::DinosaurCollision;
    EventData.ImpactLocation = HitResult.Location;
    EventData.ImpactNormal = HitResult.Normal;
    EventData.ImpactForce = CollisionForce;
    EventData.SourceActor = Dinosaur1;
    EventData.TargetActor = Dinosaur2;
    EventData.EventDescription = FString::Printf(TEXT("Dinosaur collision: %s vs %s"), 
                                                 *Dinosaur1->GetName(), *Dinosaur2->GetName());
    EventData.Timestamp = GetWorld()->GetTimeSeconds();
    
    BroadcastPhysicsEvent(EventData);
    
    UE_LOG(LogTemp, Warning, TEXT("ENGINE ARCHITECT: Dinosaur collision - %s vs %s (Force: %.1f)"), 
           *Dinosaur1->GetName(), *Dinosaur2->GetName(), CollisionForce);
}

void UEng_PhysicsArchitecture::HandlePlayerDinosaurCollision(AActor* Player, AActor* Dinosaur, const FHitResult& HitResult)
{
    if (!IsValid(Player) || !IsValid(Dinosaur))
    {
        return;
    }
    
    FEng_DinosaurPhysicsConfig* DinosaurConfig = DinosaurPhysicsRegistry.Find(Dinosaur);
    if (!DinosaurConfig)
    {
        return;
    }
    
    // Calculate damage/effect based on dinosaur size and speed
    float ImpactForce = DinosaurConfig->Mass * 0.01f; // Simplified calculation
    
    // Create physics event
    FEng_PhysicsEventData EventData;
    EventData.InteractionType = EEng_PhysicsInteractionType::PlayerInteraction;
    EventData.ImpactLocation = HitResult.Location;
    EventData.ImpactNormal = HitResult.Normal;
    EventData.ImpactForce = ImpactForce;
    EventData.SourceActor = Dinosaur;
    EventData.TargetActor = Player;
    EventData.EventDescription = FString::Printf(TEXT("Player-Dinosaur collision: %s"), *Dinosaur->GetName());
    EventData.Timestamp = GetWorld()->GetTimeSeconds();
    
    BroadcastPhysicsEvent(EventData);
    
    UE_LOG(LogTemp, Warning, TEXT("ENGINE ARCHITECT: Player-Dinosaur collision - %s (Force: %.1f)"), 
           *Dinosaur->GetName(), ImpactForce);
}

void UEng_PhysicsArchitecture::OptimizePhysicsSimulation()
{
    int32 ActiveBodies = GetActivePhysicsBodies();
    
    if (ActiveBodies > MaxPhysicsBodies)
    {
        UE_LOG(LogTemp, Warning, TEXT("ENGINE ARCHITECT: Physics optimization triggered - %d bodies (limit: %d)"), 
               ActiveBodies, MaxPhysicsBodies);
        
        // In a full implementation, this would reduce physics fidelity for distant objects
        UpdatePhysicsLOD();
    }
}

void UEng_PhysicsArchitecture::SetPhysicsLODLevel(AActor* Actor, EEng_PhysicsSimulationLevel LODLevel)
{
    if (!IsValid(Actor))
    {
        return;
    }
    
    if (UPrimitiveComponent* PrimComp = Actor->FindComponentByClass<UPrimitiveComponent>())
    {
        switch (LODLevel)
        {
        case EEng_PhysicsSimulationLevel::Disabled:
            PrimComp->SetSimulatePhysics(false);
            break;
        case EEng_PhysicsSimulationLevel::Kinematic:
            PrimComp->SetSimulatePhysics(false);
            PrimComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
            break;
        case EEng_PhysicsSimulationLevel::Simple:
            PrimComp->SetSimulatePhysics(true);
            PrimComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
            break;
        case EEng_PhysicsSimulationLevel::Complex:
        case EEng_PhysicsSimulationLevel::HighFidelity:
            PrimComp->SetSimulatePhysics(true);
            PrimComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
            break;
        }
    }
}

int32 UEng_PhysicsArchitecture::GetActivePhysicsBodies()
{
    int32 ActiveBodies = 0;
    
    if (UWorld* World = GetWorld())
    {
        for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
        {
            AActor* Actor = *ActorItr;
            if (IsValid(Actor))
            {
                if (UPrimitiveComponent* PrimComp = Actor->FindComponentByClass<UPrimitiveComponent>())
                {
                    if (PrimComp->IsSimulatingPhysics())
                    {
                        ActiveBodies++;
                    }
                }
            }
        }
    }
    
    return ActiveBodies;
}

void UEng_PhysicsArchitecture::BroadcastPhysicsEvent(const FEng_PhysicsEventData& EventData)
{
    // Add to recent events
    RecentEvents.Add(EventData);
    
    // In a full implementation, this would broadcast to interested systems
    UE_LOG(LogTemp, Log, TEXT("ENGINE ARCHITECT: Physics Event - %s"), *EventData.EventDescription);
}

TArray<FEng_PhysicsEventData> UEng_PhysicsArchitecture::GetRecentPhysicsEvents(float TimeWindow)
{
    TArray<FEng_PhysicsEventData> FilteredEvents;
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    for (const FEng_PhysicsEventData& Event : RecentEvents)
    {
        if (CurrentTime - Event.Timestamp <= TimeWindow)
        {
            FilteredEvents.Add(Event);
        }
    }
    
    return FilteredEvents;
}

bool UEng_PhysicsArchitecture::ValidatePhysicsSetup()
{
    bool bIsValid = true;
    
    // Validate dinosaur registrations
    for (auto& Pair : DinosaurPhysicsRegistry)
    {
        if (!IsValid(Pair.Key))
        {
            UE_LOG(LogTemp, Error, TEXT("ENGINE ARCHITECT: Invalid dinosaur actor in registry"));
            bIsValid = false;
        }
    }
    
    // Validate environmental registrations
    for (auto& Pair : EnvironmentalRegistry)
    {
        if (!IsValid(Pair.Key))
        {
            UE_LOG(LogTemp, Error, TEXT("ENGINE ARCHITECT: Invalid environmental actor in registry"));
            bIsValid = false;
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("ENGINE ARCHITECT: Physics validation %s"), 
           bIsValid ? TEXT("PASSED") : TEXT("FAILED"));
    
    return bIsValid;
}

void UEng_PhysicsArchitecture::LogPhysicsStatistics()
{
    int32 ActiveBodies = GetActivePhysicsBodies();
    int32 RegisteredDinosaurs = DinosaurPhysicsRegistry.Num();
    int32 RegisteredEnvironmental = EnvironmentalRegistry.Num();
    int32 RecentEventCount = RecentEvents.Num();
    
    UE_LOG(LogTemp, Warning, TEXT("ENGINE ARCHITECT: Physics Statistics"));
    UE_LOG(LogTemp, Warning, TEXT("  Active Physics Bodies: %d"), ActiveBodies);
    UE_LOG(LogTemp, Warning, TEXT("  Registered Dinosaurs: %d"), RegisteredDinosaurs);
    UE_LOG(LogTemp, Warning, TEXT("  Environmental Objects: %d"), RegisteredEnvironmental);
    UE_LOG(LogTemp, Warning, TEXT("  Recent Events: %d"), RecentEventCount);
}

void UEng_PhysicsArchitecture::UpdatePhysicsLOD()
{
    if (!GetWorld())
    {
        return;
    }
    
    // Get player location for distance calculations
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (!IsValid(PlayerPawn))
    {
        return;
    }
    
    FVector PlayerLocation = PlayerPawn->GetActorLocation();
    
    // Update LOD for registered dinosaurs
    for (auto& Pair : DinosaurPhysicsRegistry)
    {
        AActor* DinosaurActor = Pair.Key;
        if (IsValid(DinosaurActor))
        {
            float Distance = FVector::Dist(PlayerLocation, DinosaurActor->GetActorLocation());
            
            if (Distance > PhysicsLODDistance * 2.0f)
            {
                SetPhysicsLODLevel(DinosaurActor, EEng_PhysicsSimulationLevel::Disabled);
            }
            else if (Distance > PhysicsLODDistance)
            {
                SetPhysicsLODLevel(DinosaurActor, EEng_PhysicsSimulationLevel::Simple);
            }
            else
            {
                SetPhysicsLODLevel(DinosaurActor, EEng_PhysicsSimulationLevel::Complex);
            }
        }
    }
}

void UEng_PhysicsArchitecture::CleanupOldEvents()
{
    if (!GetWorld())
    {
        return;
    }
    
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    // Remove events older than the history duration
    RecentEvents.RemoveAll([CurrentTime, this](const FEng_PhysicsEventData& Event)
    {
        return (CurrentTime - Event.Timestamp) > EventHistoryDuration;
    });
}

void UEng_PhysicsArchitecture::ValidatePhysicsIntegrity()
{
    // Clean up invalid actor references
    TArray<AActor*> InvalidDinosaurs;
    for (auto& Pair : DinosaurPhysicsRegistry)
    {
        if (!IsValid(Pair.Key))
        {
            InvalidDinosaurs.Add(Pair.Key);
        }
    }
    
    for (AActor* InvalidActor : InvalidDinosaurs)
    {
        DinosaurPhysicsRegistry.Remove(InvalidActor);
    }
    
    TArray<AActor*> InvalidEnvironmental;
    for (auto& Pair : EnvironmentalRegistry)
    {
        if (!IsValid(Pair.Key))
        {
            InvalidEnvironmental.Add(Pair.Key);
        }
    }
    
    for (AActor* InvalidActor : InvalidEnvironmental)
    {
        EnvironmentalRegistry.Remove(InvalidActor);
    }
    
    if (InvalidDinosaurs.Num() > 0 || InvalidEnvironmental.Num() > 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("ENGINE ARCHITECT: Cleaned up %d invalid physics registrations"), 
               InvalidDinosaurs.Num() + InvalidEnvironmental.Num());
    }
}