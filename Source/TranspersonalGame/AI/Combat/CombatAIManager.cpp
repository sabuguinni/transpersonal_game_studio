#include "CombatAIManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Components/SceneComponent.h"
#include "CombatBehaviorComponent.h"
#include "Combat_DinosaurPawn.h"

ACombatAIManager::ACombatAIManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.1f; // 10 FPS for combat AI

    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;

    // Combat AI Configuration
    CombatDetectionRange = 2000.0f;
    TacticalUpdateInterval = 0.5f;
    MaxSimultaneousCombats = 5;
    bEnablePackCoordination = true;
    bEnableTacticalPositioning = true;

    CurrentActiveCombats = 0;
    TacticalUpdateTimer = 0.0f;
    EncounterCheckTimer = 0.0f;
}

void ACombatAIManager::BeginPlay()
{
    Super::BeginPlay();
    InitializeCombatSystem();
}

void ACombatAIManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    UpdateCombatSystem(DeltaTime);
}

void ACombatAIManager::InitializeCombatSystem()
{
    UE_LOG(LogTemp, Log, TEXT("CombatAIManager: Initializing combat system"));

    // Create default combat encounters
    CreateCombatEncounter(FVector(2000, 0, 100), ECombat_ThreatLevel::Medium, 3);
    CreateCombatEncounter(FVector(-2000, 1500, 150), ECombat_ThreatLevel::High, 5);
    CreateCombatEncounter(FVector(1000, -2000, 120), ECombat_ThreatLevel::Low, 2);
    CreateCombatEncounter(FVector(-1500, -1000, 180), ECombat_ThreatLevel::Critical, 7);

    UE_LOG(LogTemp, Log, TEXT("CombatAIManager: Created %d combat encounters"), CombatEncounters.Num());
}

void ACombatAIManager::RegisterCombatant(ACombat_DinosaurPawn* Dinosaur)
{
    if (Dinosaur && !ActiveCombatants.Contains(Dinosaur))
    {
        ActiveCombatants.Add(Dinosaur);
        UE_LOG(LogTemp, Log, TEXT("CombatAIManager: Registered combatant %s"), *Dinosaur->GetName());
    }
}

void ACombatAIManager::UnregisterCombatant(ACombat_DinosaurPawn* Dinosaur)
{
    if (Dinosaur)
    {
        ActiveCombatants.Remove(Dinosaur);
        UE_LOG(LogTemp, Log, TEXT("CombatAIManager: Unregistered combatant %s"), *Dinosaur->GetName());
    }
}

bool ACombatAIManager::StartCombatEncounter(const FVector& Location, ECombat_ThreatLevel ThreatLevel)
{
    if (!CanStartNewCombat())
    {
        return false;
    }

    // Find nearby dinosaurs to participate in combat
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ACombat_DinosaurPawn::StaticClass(), FoundActors);

    TArray<ACombat_DinosaurPawn*> NearbyDinosaurs;
    for (AActor* Actor : FoundActors)
    {
        if (ACombat_DinosaurPawn* Dinosaur = Cast<ACombat_DinosaurPawn>(Actor))
        {
            float Distance = FVector::Dist(Actor->GetActorLocation(), Location);
            if (Distance <= CombatDetectionRange)
            {
                NearbyDinosaurs.Add(Dinosaur);
            }
        }
    }

    if (NearbyDinosaurs.Num() > 0)
    {
        CurrentActiveCombats++;
        
        // Coordinate pack attack if multiple dinosaurs
        if (NearbyDinosaurs.Num() > 1 && bEnablePackCoordination)
        {
            CoordinatePackAttack(NearbyDinosaurs, nullptr); // Target will be determined by individual AI
        }

        UE_LOG(LogTemp, Log, TEXT("CombatAIManager: Started combat encounter at %s with %d dinosaurs"), 
               *Location.ToString(), NearbyDinosaurs.Num());
        return true;
    }

    return false;
}

void ACombatAIManager::EndCombatEncounter(const FVector& Location)
{
    if (CurrentActiveCombats > 0)
    {
        CurrentActiveCombats--;
        UE_LOG(LogTemp, Log, TEXT("CombatAIManager: Ended combat encounter at %s"), *Location.ToString());
    }
}

void ACombatAIManager::UpdateTacticalPositions()
{
    if (!bEnableTacticalPositioning)
    {
        return;
    }

    for (ACombat_DinosaurPawn* Combatant : ActiveCombatants)
    {
        if (IsValid(Combatant))
        {
            // Get combat behavior component
            UCombat_BehaviorComponent* BehaviorComp = Combatant->FindComponentByClass<UCombat_BehaviorComponent>();
            if (BehaviorComp)
            {
                // Update tactical position based on behavior type
                FVector OptimalPosition = GetOptimalAttackPosition(Combatant, nullptr);
                // Position will be used by the behavior component
            }
        }
    }
}

FVector ACombatAIManager::GetOptimalAttackPosition(ACombat_DinosaurPawn* Attacker, AActor* Target)
{
    if (!Attacker)
    {
        return FVector::ZeroVector;
    }

    FVector AttackerLocation = Attacker->GetActorLocation();
    
    if (!Target)
    {
        // Return a position slightly forward from current location
        FVector Forward = Attacker->GetActorForwardVector();
        return AttackerLocation + (Forward * 500.0f);
    }

    FVector TargetLocation = Target->GetActorLocation();
    FVector Direction = (TargetLocation - AttackerLocation).GetSafeNormal();
    
    // Calculate flanking position
    FVector RightVector = FVector::CrossProduct(Direction, FVector::UpVector);
    float FlankOffset = FMath::RandRange(-800.0f, 800.0f);
    
    return TargetLocation + (RightVector * FlankOffset) + (Direction * -300.0f);
}

ECombat_Formation ACombatAIManager::GetRecommendedFormation(const TArray<ACombat_DinosaurPawn*>& Pack)
{
    int32 PackSize = Pack.Num();
    
    if (PackSize <= 1)
    {
        return ECombat_Formation::None;
    }
    else if (PackSize <= 3)
    {
        return ECombat_Formation::Line;
    }
    else if (PackSize <= 5)
    {
        return ECombat_Formation::Wedge;
    }
    else
    {
        return ECombat_Formation::Circle;
    }
}

void ACombatAIManager::CoordinatePackAttack(const TArray<ACombat_DinosaurPawn*>& Pack, AActor* Target)
{
    if (Pack.Num() <= 1)
    {
        return;
    }

    ECombat_Formation Formation = GetRecommendedFormation(Pack);
    
    // Assign roles based on formation
    for (int32 i = 0; i < Pack.Num(); i++)
    {
        if (IsValid(Pack[i]))
        {
            UCombat_BehaviorComponent* BehaviorComp = Pack[i]->FindComponentByClass<UCombat_BehaviorComponent>();
            if (BehaviorComp)
            {
                // Set formation role and position
                // This would be implemented in the behavior component
            }
        }
    }

    UE_LOG(LogTemp, Log, TEXT("CombatAIManager: Coordinating pack attack with %d dinosaurs using %s formation"), 
           Pack.Num(), *UEnum::GetValueAsString(Formation));
}

void ACombatAIManager::ExecuteFlankingManeuver(ACombat_DinosaurPawn* Leader, const TArray<ACombat_DinosaurPawn*>& Flankers)
{
    if (!Leader || Flankers.Num() == 0)
    {
        return;
    }

    FVector LeaderLocation = Leader->GetActorLocation();
    
    for (int32 i = 0; i < Flankers.Num(); i++)
    {
        if (IsValid(Flankers[i]))
        {
            // Calculate flanking positions
            float Angle = (360.0f / Flankers.Num()) * i;
            FVector FlankPosition = LeaderLocation + FVector(
                FMath::Cos(FMath::DegreesToRadians(Angle)) * 1000.0f,
                FMath::Sin(FMath::DegreesToRadians(Angle)) * 1000.0f,
                0.0f
            );

            // Move flanker to position (would be handled by behavior component)
            UE_LOG(LogTemp, Log, TEXT("CombatAIManager: Flanker %d moving to position %s"), 
                   i, *FlankPosition.ToString());
        }
    }
}

void ACombatAIManager::CreateCombatEncounter(const FVector& Location, ECombat_ThreatLevel ThreatLevel, int32 DinosaurCount)
{
    FCombat_EncounterData NewEncounter;
    NewEncounter.Location = Location;
    NewEncounter.ThreatLevel = ThreatLevel;
    NewEncounter.DinosaurCount = DinosaurCount;
    NewEncounter.EncounterRadius = 1500.0f;
    NewEncounter.bIsActive = true;

    CombatEncounters.Add(NewEncounter);
}

void ACombatAIManager::CheckEncounterTriggers()
{
    // Find player character
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (!PlayerPawn)
    {
        return;
    }

    FVector PlayerLocation = PlayerPawn->GetActorLocation();

    for (FCombat_EncounterData& Encounter : CombatEncounters)
    {
        if (!Encounter.bIsActive)
        {
            continue;
        }

        float Distance = FVector::Dist(PlayerLocation, Encounter.Location);
        if (Distance <= Encounter.EncounterRadius)
        {
            if (StartCombatEncounter(Encounter.Location, Encounter.ThreatLevel))
            {
                Encounter.bIsActive = false; // Deactivate until cooldown
            }
        }
    }
}

FCombat_EncounterData* ACombatAIManager::GetNearestEncounter(const FVector& Location)
{
    FCombat_EncounterData* NearestEncounter = nullptr;
    float NearestDistance = FLT_MAX;

    for (FCombat_EncounterData& Encounter : CombatEncounters)
    {
        float Distance = FVector::Dist(Location, Encounter.Location);
        if (Distance < NearestDistance)
        {
            NearestDistance = Distance;
            NearestEncounter = &Encounter;
        }
    }

    return NearestEncounter;
}

bool ACombatAIManager::IsLocationInCombat(const FVector& Location) const
{
    for (const FCombat_EncounterData& Encounter : CombatEncounters)
    {
        if (Encounter.bIsActive)
        {
            float Distance = FVector::Dist(Location, Encounter.Location);
            if (Distance <= Encounter.EncounterRadius)
            {
                return true;
            }
        }
    }
    return false;
}

int32 ACombatAIManager::GetCombatantsInRange(const FVector& Location, float Range) const
{
    int32 Count = 0;
    for (ACombat_DinosaurPawn* Combatant : ActiveCombatants)
    {
        if (IsValid(Combatant))
        {
            float Distance = FVector::Dist(Location, Combatant->GetActorLocation());
            if (Distance <= Range)
            {
                Count++;
            }
        }
    }
    return Count;
}

void ACombatAIManager::UpdateCombatSystem(float DeltaTime)
{
    TacticalUpdateTimer += DeltaTime;
    EncounterCheckTimer += DeltaTime;

    // Update tactical positions
    if (TacticalUpdateTimer >= TacticalUpdateInterval)
    {
        UpdateTacticalPositions();
        TacticalUpdateTimer = 0.0f;
    }

    // Check encounter triggers
    if (EncounterCheckTimer >= 1.0f) // Check every second
    {
        CheckEncounterTriggers();
        EncounterCheckTimer = 0.0f;
    }

    // Process active combats
    ProcessActiveCombats();
    
    // Cleanup inactive combatants
    CleanupInactiveCombatants();
}

void ACombatAIManager::ProcessActiveCombats()
{
    // Process ongoing combat encounters
    for (FCombat_EncounterData& Encounter : CombatEncounters)
    {
        if (Encounter.bIsActive)
        {
            // Check if encounter should end (no combatants in range, etc.)
            int32 CombatantsInRange = GetCombatantsInRange(Encounter.Location, Encounter.EncounterRadius);
            if (CombatantsInRange == 0)
            {
                EndCombatEncounter(Encounter.Location);
                Encounter.bIsActive = false;
            }
        }
    }
}

void ACombatAIManager::CleanupInactiveCombatants()
{
    ActiveCombatants.RemoveAll([](ACombat_DinosaurPawn* Combatant)
    {
        return !IsValid(Combatant);
    });
}

bool ACombatAIManager::CanStartNewCombat() const
{
    return CurrentActiveCombats < MaxSimultaneousCombats;
}