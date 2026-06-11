#include "Combat_CombatAIManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Pawn.h"
#include "Components/StaticMeshComponent.h"

ACombat_CombatAIManager::ACombat_CombatAIManager()
{
    PrimaryActorTick.bCanEverTick = true;
    
    // Initialize default values
    ZoneUpdateInterval = 2.0f;
    MaxSimultaneousEngagements = 10;
    AggressionMultiplier = 1.0f;
    FleeThreshold = 0.3f;
    PackCoordinationRadius = 1500.0f;
    MaxAIUpdatesPerFrame = 5;
    AIUpdateFrequency = 0.1f;
    
    LastZoneUpdate = 0.0f;
    LastAIUpdate = 0.0f;
    CurrentAIUpdateIndex = 0;
    
    // Create root component
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
}

void ACombat_CombatAIManager::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Warning, TEXT("Combat AI Manager initialized"));
    
    // Initialize default combat zones
    CreateCombatZone(TEXT("TRex Territory"), FVector(2000, 0, 100), 1500.0f, ECombat_ThreatLevel::Extreme);
    CreateCombatZone(TEXT("Raptor Pack Area"), FVector(-1500, 1500, 100), 1000.0f, ECombat_ThreatLevel::High);
    CreateCombatZone(TEXT("Herbivore Grazing"), FVector(0, 2000, 100), 800.0f, ECombat_ThreatLevel::Low);
    CreateCombatZone(TEXT("Ambush Canyon"), FVector(-2000, -1000, 100), 600.0f, ECombat_ThreatLevel::Medium);
    
    LogCombatEvent(TEXT("Combat AI Manager started with 4 default zones"));
}

void ACombat_CombatAIManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    // Update combat zones periodically
    if (CurrentTime - LastZoneUpdate > ZoneUpdateInterval)
    {
        UpdateCombatZones();
        LastZoneUpdate = CurrentTime;
    }
    
    // Update AI behaviors at controlled frequency
    if (CurrentTime - LastAIUpdate > AIUpdateFrequency)
    {
        UpdateAIBehaviors();
        LastAIUpdate = CurrentTime;
    }
}

void ACombat_CombatAIManager::CreateCombatZone(const FString& ZoneName, const FVector& Location, float Radius, ECombat_ThreatLevel ThreatLevel)
{
    FCombat_CombatZone NewZone;
    NewZone.ZoneName = ZoneName;
    NewZone.CenterLocation = Location;
    NewZone.Radius = Radius;
    NewZone.ThreatLevel = ThreatLevel;
    
    CombatZones.Add(NewZone);
    
    UE_LOG(LogTemp, Warning, TEXT("Created combat zone: %s at location %s"), *ZoneName, *Location.ToString());
}

void ACombat_CombatAIManager::RemoveCombatZone(const FString& ZoneName)
{
    for (int32 i = CombatZones.Num() - 1; i >= 0; i--)
    {
        if (CombatZones[i].ZoneName == ZoneName)
        {
            CombatZones.RemoveAt(i);
            UE_LOG(LogTemp, Warning, TEXT("Removed combat zone: %s"), *ZoneName);
            break;
        }
    }
}

FCombat_CombatZone* ACombat_CombatAIManager::GetCombatZoneAtLocation(const FVector& Location)
{
    for (FCombat_CombatZone& Zone : CombatZones)
    {
        float Distance = FVector::Dist(Zone.CenterLocation, Location);
        if (Distance <= Zone.Radius)
        {
            return &Zone;
        }
    }
    return nullptr;
}

void ACombat_CombatAIManager::UpdateCombatZones()
{
    // Get all actors in the world
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), AllActors);
    
    // Clear previous zone assignments
    for (FCombat_CombatZone& Zone : CombatZones)
    {
        Zone.ActiveCombatants.Empty();
    }
    
    // Assign actors to zones
    for (AActor* Actor : AllActors)
    {
        if (!Actor || !Actor->IsValidLowLevel())
            continue;
            
        FString ActorLabel = Actor->GetActorLabel().ToLower();
        
        // Only process dinosaur actors
        if (ActorLabel.Contains(TEXT("trex")) || ActorLabel.Contains(TEXT("veloci")) || 
            ActorLabel.Contains(TEXT("raptor")) || ActorLabel.Contains(TEXT("brachi")) ||
            ActorLabel.Contains(TEXT("tricera")) || ActorLabel.Contains(TEXT("parasauro")))
        {
            FCombat_CombatZone* Zone = GetCombatZoneAtLocation(Actor->GetActorLocation());
            if (Zone)
            {
                Zone->ActiveCombatants.Add(Actor);
            }
        }
    }
}

void ACombat_CombatAIManager::StartCombatEngagement(AActor* Attacker, AActor* Target)
{
    if (!Attacker || !Target)
        return;
        
    // Check if already in combat
    for (const FCombat_EngagementData& Engagement : ActiveEngagements)
    {
        if (Engagement.Attacker == Attacker && Engagement.Target == Target)
            return; // Already engaged
    }
    
    // Create new engagement
    FCombat_EngagementData NewEngagement;
    NewEngagement.Attacker = Attacker;
    NewEngagement.Target = Target;
    NewEngagement.EngagementStartTime = GetWorld()->GetTimeSeconds();
    NewEngagement.CombatState = ECombat_CombatState::Engaging;
    
    ActiveEngagements.Add(NewEngagement);
    
    LogCombatEvent(FString::Printf(TEXT("Combat started: %s vs %s"), 
        *Attacker->GetActorLabel(), *Target->GetActorLabel()));
}

void ACombat_CombatAIManager::EndCombatEngagement(AActor* Attacker, AActor* Target)
{
    for (int32 i = ActiveEngagements.Num() - 1; i >= 0; i--)
    {
        if (ActiveEngagements[i].Attacker == Attacker && ActiveEngagements[i].Target == Target)
        {
            LogCombatEvent(FString::Printf(TEXT("Combat ended: %s vs %s"), 
                *Attacker->GetActorLabel(), *Target->GetActorLabel()));
            ActiveEngagements.RemoveAt(i);
            break;
        }
    }
}

bool ACombat_CombatAIManager::IsInCombat(AActor* Actor)
{
    for (const FCombat_EngagementData& Engagement : ActiveEngagements)
    {
        if (Engagement.Attacker == Actor || Engagement.Target == Actor)
            return true;
    }
    return false;
}

TArray<AActor*> ACombat_CombatAIManager::GetNearbyThreats(AActor* Actor, float SearchRadius)
{
    TArray<AActor*> Threats;
    
    if (!Actor)
        return Threats;
    
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), AllActors);
    
    FVector ActorLocation = Actor->GetActorLocation();
    
    for (AActor* OtherActor : AllActors)
    {
        if (!OtherActor || OtherActor == Actor)
            continue;
            
        FString OtherLabel = OtherActor->GetActorLabel().ToLower();
        
        // Check if it's a predator
        if (OtherLabel.Contains(TEXT("trex")) || OtherLabel.Contains(TEXT("veloci")) || OtherLabel.Contains(TEXT("raptor")))
        {
            float Distance = FVector::Dist(ActorLocation, OtherActor->GetActorLocation());
            if (Distance <= SearchRadius)
            {
                Threats.Add(OtherActor);
            }
        }
    }
    
    return Threats;
}

void ACombat_CombatAIManager::UpdateAIBehaviors()
{
    // Update a limited number of AI actors per frame to maintain performance
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), AllActors);
    
    int32 UpdatesThisFrame = 0;
    int32 DinosaurCount = 0;
    
    // Count dinosaurs first
    for (AActor* Actor : AllActors)
    {
        if (!Actor)
            continue;
            
        FString ActorLabel = Actor->GetActorLabel().ToLower();
        if (ActorLabel.Contains(TEXT("trex")) || ActorLabel.Contains(TEXT("veloci")) || 
            ActorLabel.Contains(TEXT("raptor")) || ActorLabel.Contains(TEXT("brachi")) ||
            ActorLabel.Contains(TEXT("tricera")) || ActorLabel.Contains(TEXT("parasauro")))
        {
            DinosaurCount++;
        }
    }
    
    // Process AI updates
    for (int32 i = 0; i < AllActors.Num() && UpdatesThisFrame < MaxAIUpdatesPerFrame; i++)
    {
        int32 ActorIndex = (CurrentAIUpdateIndex + i) % AllActors.Num();
        AActor* Actor = AllActors[ActorIndex];
        
        if (!Actor)
            continue;
            
        FString ActorLabel = Actor->GetActorLabel().ToLower();
        
        // Process different AI behaviors based on dinosaur type
        if (ActorLabel.Contains(TEXT("trex")))
        {
            // TRex aggressive behavior
            TArray<AActor*> NearbyThreats = GetNearbyThreats(Actor, 2000.0f);
            if (NearbyThreats.Num() > 0 && !IsInCombat(Actor))
            {
                StartCombatEngagement(Actor, NearbyThreats[0]);
            }
            UpdatesThisFrame++;
        }
        else if (ActorLabel.Contains(TEXT("veloci")) || ActorLabel.Contains(TEXT("raptor")))
        {
            // Raptor pack behavior
            TArray<AActor*> PackMembers;
            for (AActor* OtherActor : AllActors)
            {
                if (OtherActor && OtherActor != Actor)
                {
                    FString OtherLabel = OtherActor->GetActorLabel().ToLower();
                    if ((OtherLabel.Contains(TEXT("veloci")) || OtherLabel.Contains(TEXT("raptor"))) &&
                        FVector::Dist(Actor->GetActorLocation(), OtherActor->GetActorLocation()) <= PackCoordinationRadius)
                    {
                        PackMembers.Add(OtherActor);
                    }
                }
            }
            
            if (PackMembers.Num() >= 2)
            {
                AssignPackBehavior(PackMembers);
            }
            UpdatesThisFrame++;
        }
        else if (ActorLabel.Contains(TEXT("brachi")) || ActorLabel.Contains(TEXT("tricera")) || ActorLabel.Contains(TEXT("parasauro")))
        {
            // Herbivore flee behavior
            TArray<AActor*> NearbyThreats = GetNearbyThreats(Actor, 1000.0f);
            if (NearbyThreats.Num() > 0)
            {
                TriggerFleeResponse(Actor, NearbyThreats[0]);
            }
            UpdatesThisFrame++;
        }
    }
    
    CurrentAIUpdateIndex = (CurrentAIUpdateIndex + UpdatesThisFrame) % FMath::Max(1, AllActors.Num());
}

void ACombat_CombatAIManager::AssignPackBehavior(const TArray<AActor*>& PackMembers)
{
    if (PackMembers.Num() < 2)
        return;
        
    // Simple pack coordination - could be expanded
    FVector PackCenter = FVector::ZeroVector;
    for (AActor* Member : PackMembers)
    {
        if (Member)
        {
            PackCenter += Member->GetActorLocation();
        }
    }
    PackCenter /= PackMembers.Num();
    
    LogCombatEvent(FString::Printf(TEXT("Pack behavior assigned to %d raptors at %s"), 
        PackMembers.Num(), *PackCenter.ToString()));
}

void ACombat_CombatAIManager::TriggerFleeResponse(AActor* FleeingActor, AActor* Threat)
{
    if (!FleeingActor || !Threat)
        return;
        
    // Calculate flee direction
    FVector FleeDirection = (FleeingActor->GetActorLocation() - Threat->GetActorLocation()).GetSafeNormal();
    FVector FleeTarget = FleeingActor->GetActorLocation() + (FleeDirection * 2000.0f);
    
    LogCombatEvent(FString::Printf(TEXT("%s fleeing from %s"), 
        *FleeingActor->GetActorLabel(), *Threat->GetActorLabel()));
}

float ACombat_CombatAIManager::CalculateThreatLevel(AActor* Actor)
{
    if (!Actor)
        return 0.0f;
        
    FString ActorLabel = Actor->GetActorLabel().ToLower();
    
    if (ActorLabel.Contains(TEXT("trex")))
        return 10.0f;
    else if (ActorLabel.Contains(TEXT("veloci")) || ActorLabel.Contains(TEXT("raptor")))
        return 7.0f;
    else if (ActorLabel.Contains(TEXT("tricera")))
        return 5.0f;
    else if (ActorLabel.Contains(TEXT("brachi")))
        return 3.0f;
    else if (ActorLabel.Contains(TEXT("parasauro")))
        return 2.0f;
        
    return 1.0f;
}

bool ACombat_CombatAIManager::ShouldEngageInCombat(AActor* PotentialAttacker, AActor* PotentialTarget)
{
    if (!PotentialAttacker || !PotentialTarget)
        return false;
        
    float AttackerThreat = CalculateThreatLevel(PotentialAttacker);
    float TargetThreat = CalculateThreatLevel(PotentialTarget);
    
    // Simple engagement logic - attackers engage if they're stronger
    return AttackerThreat > TargetThreat * 1.2f;
}

void ACombat_CombatAIManager::LogCombatEvent(const FString& EventDescription)
{
    UE_LOG(LogTemp, Warning, TEXT("Combat AI: %s"), *EventDescription);
    
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red, 
            FString::Printf(TEXT("Combat AI: %s"), *EventDescription));
    }
}