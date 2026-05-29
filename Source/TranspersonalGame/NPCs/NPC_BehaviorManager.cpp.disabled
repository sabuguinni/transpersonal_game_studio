#include "NPC_BehaviorManager.h"
#include "NPC_TribalHuman.h"
#include "DinosaurBase.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

ANPC_BehaviorManager::ANPC_BehaviorManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.1f;

    // Initialize default settings
    BehaviorSettings.NPCUpdateInterval = 1.0f;
    BehaviorSettings.PlayerDetectionRadius = 5000.0f;
    BehaviorSettings.NPCInteractionRadius = 2000.0f;
    BehaviorSettings.MaxActiveNPCs = 50;
    BehaviorSettings.bEnableAdvancedBehaviors = true;

    // Initialize global state
    GlobalBehaviorState.TimeOfDay = 12.0f;
    GlobalBehaviorState.CurrentWeather = ENPC_WeatherType::Clear;
    GlobalBehaviorState.ThreatLevel = 0.0f;
    GlobalBehaviorState.PlayerLastKnownLocation = FVector::ZeroVector;
    GlobalBehaviorState.bPlayerNearby = false;
}

void ANPC_BehaviorManager::BeginPlay()
{
    Super::BeginPlay();

    // Find and register all existing NPCs in the world
    UWorld* World = GetWorld();
    if (World)
    {
        // Find all tribal humans
        TArray<AActor*> FoundTribalHumans;
        UGameplayStatics::GetAllActorsOfClass(World, ANPC_TribalHuman::StaticClass(), FoundTribalHumans);
        for (AActor* Actor : FoundTribalHumans)
        {
            RegisterNPC(Actor);
        }

        // Find all dinosaurs
        TArray<AActor*> FoundDinosaurs;
        UGameplayStatics::GetAllActorsOfClass(World, ADinosaurBase::StaticClass(), FoundDinosaurs);
        for (AActor* Actor : FoundDinosaurs)
        {
            RegisterNPC(Actor);
        }

        UE_LOG(LogTemp, Warning, TEXT("NPC Behavior Manager initialized with %d NPCs"), RegisteredNPCs.Num());
    }
}

void ANPC_BehaviorManager::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    // Clear all registered NPCs
    RegisteredNPCs.Empty();
    TribalHumans.Empty();
    Dinosaurs.Empty();

    Super::EndPlay(EndPlayReason);
}

void ANPC_BehaviorManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    float CurrentTime = GetWorld()->GetTimeSeconds();

    // Update global behavior state
    if (CurrentTime - LastUpdateTime >= BehaviorSettings.NPCUpdateInterval)
    {
        UpdateGlobalBehaviorState();
        UpdateAllNPCBehaviors();
        LastUpdateTime = CurrentTime;
    }

    // Check player proximity more frequently
    if (CurrentTime - LastPlayerCheckTime >= 0.5f)
    {
        UpdatePlayerProximity();
        LastPlayerCheckTime = CurrentTime;
    }
}

void ANPC_BehaviorManager::RegisterNPC(AActor* NPCActor)
{
    if (!NPCActor || RegisteredNPCs.Contains(NPCActor))
    {
        return;
    }

    RegisteredNPCs.Add(NPCActor);

    // Sort into specific categories
    if (ANPC_TribalHuman* TribalHuman = Cast<ANPC_TribalHuman>(NPCActor))
    {
        TribalHumans.Add(TribalHuman);
        UE_LOG(LogTemp, Log, TEXT("Registered Tribal Human: %s"), *TribalHuman->GetName());
    }
    else if (ADinosaurBase* Dinosaur = Cast<ADinosaurBase>(NPCActor))
    {
        Dinosaurs.Add(Dinosaur);
        UE_LOG(LogTemp, Log, TEXT("Registered Dinosaur: %s"), *Dinosaur->GetName());
    }
}

void ANPC_BehaviorManager::UnregisterNPC(AActor* NPCActor)
{
    if (!NPCActor)
    {
        return;
    }

    RegisteredNPCs.RemoveAll([NPCActor](const TWeakObjectPtr<AActor>& WeakPtr)
    {
        return WeakPtr.Get() == NPCActor;
    });

    TribalHumans.RemoveAll([NPCActor](const TWeakObjectPtr<ANPC_TribalHuman>& WeakPtr)
    {
        return WeakPtr.Get() == NPCActor;
    });

    Dinosaurs.RemoveAll([NPCActor](const TWeakObjectPtr<ADinosaurBase>& WeakPtr)
    {
        return WeakPtr.Get() == NPCActor;
    });
}

void ANPC_BehaviorManager::UpdateAllNPCBehaviors()
{
    CleanupInvalidNPCs();

    if (!BehaviorSettings.bEnableAdvancedBehaviors)
    {
        return;
    }

    // Coordinate different NPC types
    CoordinateTribalBehaviors();
    CoordinateDinosaurBehaviors();
    ProcessNPCInteractions();
}

TArray<AActor*> ANPC_BehaviorManager::GetNPCsInRadius(FVector Location, float Radius)
{
    TArray<AActor*> NPCsInRadius;
    float RadiusSquared = Radius * Radius;

    for (const TWeakObjectPtr<AActor>& WeakNPC : RegisteredNPCs)
    {
        if (AActor* NPC = WeakNPC.Get())
        {
            float DistanceSquared = FVector::DistSquared(NPC->GetActorLocation(), Location);
            if (DistanceSquared <= RadiusSquared)
            {
                NPCsInRadius.Add(NPC);
            }
        }
    }

    return NPCsInRadius;
}

void ANPC_BehaviorManager::UpdateGlobalBehaviorState()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    // Update time of day (simplified - could be linked to actual day/night cycle)
    GlobalBehaviorState.TimeOfDay += World->GetDeltaSeconds() * 0.1f; // Slow time progression
    if (GlobalBehaviorState.TimeOfDay >= 24.0f)
    {
        GlobalBehaviorState.TimeOfDay = 0.0f;
    }

    // Update threat level based on player actions and NPC states
    float NewThreatLevel = 0.0f;
    for (const TWeakObjectPtr<ADinosaurBase>& WeakDinosaur : Dinosaurs)
    {
        if (ADinosaurBase* Dinosaur = WeakDinosaur.Get())
        {
            // Add threat based on dinosaur aggression and proximity to player
            NewThreatLevel += 0.1f; // Simplified calculation
        }
    }
    GlobalBehaviorState.ThreatLevel = FMath::Clamp(NewThreatLevel, 0.0f, 10.0f);
}

void ANPC_BehaviorManager::SetTimeOfDay(float NewTimeOfDay)
{
    GlobalBehaviorState.TimeOfDay = FMath::Clamp(NewTimeOfDay, 0.0f, 24.0f);
}

void ANPC_BehaviorManager::SetWeatherType(ENPC_WeatherType NewWeather)
{
    GlobalBehaviorState.CurrentWeather = NewWeather;
}

void ANPC_BehaviorManager::SetThreatLevel(float NewThreatLevel)
{
    GlobalBehaviorState.ThreatLevel = FMath::Clamp(NewThreatLevel, 0.0f, 10.0f);
}

void ANPC_BehaviorManager::NotifyPlayerLocation(FVector PlayerLocation)
{
    GlobalBehaviorState.PlayerLastKnownLocation = PlayerLocation;
    
    // Check if any NPCs should react to player presence
    TArray<AActor*> NearbyNPCs = GetNPCsInRadius(PlayerLocation, BehaviorSettings.PlayerDetectionRadius);
    for (AActor* NPC : NearbyNPCs)
    {
        TriggerNPCReaction(NPC, ENPC_ReactionType::PlayerDetected);
    }
}

void ANPC_BehaviorManager::TriggerNPCReaction(AActor* NPCActor, ENPC_ReactionType ReactionType)
{
    if (!NPCActor)
    {
        return;
    }

    // Handle different NPC types
    if (ANPC_TribalHuman* TribalHuman = Cast<ANPC_TribalHuman>(NPCActor))
    {
        // Trigger tribal human reaction
        UE_LOG(LogTemp, Log, TEXT("Triggering reaction for Tribal Human: %s"), *TribalHuman->GetName());
    }
    else if (ADinosaurBase* Dinosaur = Cast<ADinosaurBase>(NPCActor))
    {
        // Trigger dinosaur reaction
        UE_LOG(LogTemp, Log, TEXT("Triggering reaction for Dinosaur: %s"), *Dinosaur->GetName());
    }
}

void ANPC_BehaviorManager::CoordinateTribalBehaviors()
{
    // Coordinate tribal human behaviors
    for (const TWeakObjectPtr<ANPC_TribalHuman>& WeakTribal : TribalHumans)
    {
        if (ANPC_TribalHuman* TribalHuman = WeakTribal.Get())
        {
            // Update tribal human with global state
            // This could involve updating their daily routines, fear levels, etc.
        }
    }
}

void ANPC_BehaviorManager::CoordinateDinosaurBehaviors()
{
    // Coordinate dinosaur behaviors
    for (const TWeakObjectPtr<ADinosaurBase>& WeakDinosaur : Dinosaurs)
    {
        if (ADinosaurBase* Dinosaur = WeakDinosaur.Get())
        {
            // Update dinosaur with global state
            // This could involve migration patterns, feeding behaviors, etc.
        }
    }
}

void ANPC_BehaviorManager::HandleNPCConflict(AActor* NPC1, AActor* NPC2)
{
    if (!NPC1 || !NPC2)
    {
        return;
    }

    UE_LOG(LogTemp, Warning, TEXT("Handling conflict between %s and %s"), *NPC1->GetName(), *NPC2->GetName());
    
    // Implement conflict resolution logic
    // This could involve territorial disputes, predator-prey relationships, etc.
}

void ANPC_BehaviorManager::CleanupInvalidNPCs()
{
    RegisteredNPCs.RemoveAll([](const TWeakObjectPtr<AActor>& WeakPtr)
    {
        return !WeakPtr.IsValid();
    });

    TribalHumans.RemoveAll([](const TWeakObjectPtr<ANPC_TribalHuman>& WeakPtr)
    {
        return !WeakPtr.IsValid();
    });

    Dinosaurs.RemoveAll([](const TWeakObjectPtr<ADinosaurBase>& WeakPtr)
    {
        return !WeakPtr.IsValid();
    });
}

void ANPC_BehaviorManager::UpdatePlayerProximity()
{
    AActor* PlayerActor = GetPlayerActor();
    if (!PlayerActor)
    {
        GlobalBehaviorState.bPlayerNearby = false;
        return;
    }

    FVector PlayerLocation = PlayerActor->GetActorLocation();
    GlobalBehaviorState.PlayerLastKnownLocation = PlayerLocation;

    // Check if player is near any NPCs
    bool bPlayerNear = false;
    for (const TWeakObjectPtr<AActor>& WeakNPC : RegisteredNPCs)
    {
        if (AActor* NPC = WeakNPC.Get())
        {
            float Distance = FVector::Dist(NPC->GetActorLocation(), PlayerLocation);
            if (Distance <= BehaviorSettings.PlayerDetectionRadius)
            {
                bPlayerNear = true;
                break;
            }
        }
    }

    GlobalBehaviorState.bPlayerNearby = bPlayerNear;
}

void ANPC_BehaviorManager::ProcessNPCInteractions()
{
    // Process interactions between NPCs
    for (int32 i = 0; i < RegisteredNPCs.Num(); ++i)
    {
        AActor* NPC1 = RegisteredNPCs[i].Get();
        if (!NPC1) continue;

        for (int32 j = i + 1; j < RegisteredNPCs.Num(); ++j)
        {
            AActor* NPC2 = RegisteredNPCs[j].Get();
            if (!NPC2) continue;

            float Distance = FVector::Dist(NPC1->GetActorLocation(), NPC2->GetActorLocation());
            if (Distance <= BehaviorSettings.NPCInteractionRadius)
            {
                // NPCs are close enough to interact
                // Implement specific interaction logic based on NPC types
            }
        }
    }
}

AActor* ANPC_BehaviorManager::GetPlayerActor()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return nullptr;
    }

    APlayerController* PlayerController = World->GetFirstPlayerController();
    if (!PlayerController)
    {
        return nullptr;
    }

    return PlayerController->GetPawn();
}