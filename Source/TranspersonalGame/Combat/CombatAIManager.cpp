#include "CombatAIManager.h"
#include "Components/SphereComponent.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "NavigationSystem.h"
#include "AI/NavigationSystemBase.h"

ACombatAIManager::ACombatAIManager()
{
    PrimaryActorTick.bCanEverTick = true;
    
    // Create combat detection sphere
    CombatDetectionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CombatDetectionSphere"));
    RootComponent = CombatDetectionSphere;
    CombatDetectionSphere->SetSphereRadius(CombatRange);
    CombatDetectionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    CombatDetectionSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
    CombatDetectionSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
    
    // Initialize default values
    AggressionLevel = 0.7f;
    HuntingEfficiency = 0.8f;
    TerritorialRadius = 2500.0f;
    PlayerSkillLevel = 0.5f;
    DifficultyScaling = 1.0f;
    
    bEnablePackHunting = true;
    bEnableFlankingBehavior = true;
    bEnableAmbushTactics = true;
}

void ACombatAIManager::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize combat detection
    CombatDetectionSphere->SetSphereRadius(CombatRange);
    
    UE_LOG(LogTemp, Warning, TEXT("CombatAIManager initialized with combat range: %f"), CombatRange);
}

void ACombatAIManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    UpdateCombatState(DeltaTime);
    UpdateCombatTimers(DeltaTime);
    
    if (bEnablePackHunting)
    {
        ManagePackCoordination();
    }
    
    if (bEnableFlankingBehavior)
    {
        CalculateFlankingPositions();
    }
}

void ACombatAIManager::InitiateCombatEncounter(AActor* Player, AActor* Dinosaur)
{
    if (!Player || !Dinosaur)
    {
        return;
    }
    
    // Check if already in combat
    if (CombatPairs.Contains(Dinosaur))
    {
        return;
    }
    
    // Add to combat tracking
    CombatPairs.Add(Dinosaur, Player);
    CombatTimers.Add(Dinosaur, 0.0f);
    ActiveCombatants.AddUnique(Dinosaur);
    
    // Apply difficulty scaling
    float ScaledAggression = AggressionLevel * DifficultyScaling;
    
    UE_LOG(LogTemp, Warning, TEXT("Combat initiated between %s and %s (Aggression: %f)"), 
           *Player->GetName(), *Dinosaur->GetName(), ScaledAggression);
    
    // Trigger pack hunting if enabled and applicable
    if (bEnablePackHunting)
    {
        TArray<AActor*> NearbyDinosaurs;
        TArray<AActor*> AllActors;
        UGameplayStatics::GetAllActorsOfClass(GetWorld(), APawn::StaticClass(), AllActors);
        
        for (AActor* Actor : AllActors)
        {
            if (Actor != Dinosaur && Actor != Player)
            {
                float Distance = FVector::Dist(Dinosaur->GetActorLocation(), Actor->GetActorLocation());
                if (Distance <= PackCoordinationRange)
                {
                    FString ActorLabel = Actor->GetActorNameOrLabel().ToLower();
                    if (ActorLabel.Contains(TEXT("veloci")) || ActorLabel.Contains(TEXT("raptor")))
                    {
                        NearbyDinosaurs.Add(Actor);
                    }
                }
            }
        }
        
        if (NearbyDinosaurs.Num() > 0)
        {
            CoordinatePackHunting(NearbyDinosaurs, Player);
        }
    }
}

void ACombatAIManager::EndCombatEncounter(AActor* Player, AActor* Dinosaur)
{
    if (!Dinosaur)
    {
        return;
    }
    
    float EncounterDuration = 0.0f;
    if (CombatTimers.Contains(Dinosaur))
    {
        EncounterDuration = CombatTimers[Dinosaur];
        CombatTimers.Remove(Dinosaur);
    }
    
    if (CombatPairs.Contains(Dinosaur))
    {
        CombatPairs.Remove(Dinosaur);
    }
    
    ActiveCombatants.Remove(Dinosaur);
    
    // Assume player won if they survived the encounter
    bool PlayerWon = Player && IsValid(Player);
    AdjustDifficultyBasedOnPerformance(PlayerWon, EncounterDuration);
    
    UE_LOG(LogTemp, Warning, TEXT("Combat ended for %s. Duration: %f seconds. Player won: %s"), 
           *Dinosaur->GetName(), EncounterDuration, PlayerWon ? TEXT("Yes") : TEXT("No"));
}

void ACombatAIManager::UpdateCombatState(float DeltaTime)
{
    // Update active combat encounters
    for (AActor* Combatant : ActiveCombatants)
    {
        if (!IsValid(Combatant))
        {
            continue;
        }
        
        AActor* Target = CombatPairs.FindRef(Combatant);
        if (!IsValid(Target))
        {
            EndCombatEncounter(Target, Combatant);
            continue;
        }
        
        // Check if still in combat range
        float Distance = FVector::Dist(Combatant->GetActorLocation(), Target->GetActorLocation());
        if (Distance > CombatRange * 1.5f) // Add some hysteresis
        {
            EndCombatEncounter(Target, Combatant);
            continue;
        }
        
        // Execute combat behaviors
        if (bEnableFlankingBehavior && FMath::RandRange(0.0f, 1.0f) < 0.1f) // 10% chance per second
        {
            ExecuteFlankingManeuver(Combatant, Target);
        }
        
        if (bEnableAmbushTactics && FMath::RandRange(0.0f, 1.0f) < 0.05f) // 5% chance per second
        {
            TriggerAmbushBehavior(Combatant, Target);
        }
    }
}

void ACombatAIManager::CoordinatePackHunting(TArray<AActor*> PackMembers, AActor* Target)
{
    if (!Target || PackMembers.Num() == 0)
    {
        return;
    }
    
    // Store pack group for coordination
    for (AActor* Member : PackMembers)
    {
        PackGroups.FindOrAdd(Member) = PackMembers;
        
        // Initiate combat for pack members
        if (!CombatPairs.Contains(Member))
        {
            InitiateCombatEncounter(Target, Member);
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Pack hunting coordinated: %d members targeting %s"), 
           PackMembers.Num(), *Target->GetName());
}

void ACombatAIManager::ExecuteFlankingManeuver(AActor* Dinosaur, AActor* Target)
{
    if (!Dinosaur || !Target)
    {
        return;
    }
    
    FVector FlankingPos = GetFlankingPosition(Dinosaur, Target);
    if (ValidateFlankingPosition(FlankingPos, Target))
    {
        FlankingPositions.FindOrAdd(Dinosaur) = FlankingPos;
        
        UE_LOG(LogTemp, Log, TEXT("Flanking maneuver executed by %s targeting %s"), 
               *Dinosaur->GetName(), *Target->GetName());
    }
}

void ACombatAIManager::TriggerAmbushBehavior(AActor* Dinosaur, AActor* Target)
{
    if (!Dinosaur || !Target)
    {
        return;
    }
    
    // Simple ambush: move to hiding position and wait
    FVector DinosaurLoc = Dinosaur->GetActorLocation();
    FVector TargetLoc = Target->GetActorLocation();
    FVector Direction = (TargetLoc - DinosaurLoc).GetSafeNormal();
    FVector AmbushPos = TargetLoc + (Direction * -500.0f); // Behind target
    
    // Add some randomness to ambush position
    AmbushPos += FVector(FMath::RandRange(-200.0f, 200.0f), FMath::RandRange(-200.0f, 200.0f), 0.0f);
    
    UE_LOG(LogTemp, Log, TEXT("Ambush behavior triggered by %s"), *Dinosaur->GetName());
}

bool ACombatAIManager::IsInCombat(AActor* Dinosaur) const
{
    return CombatPairs.Contains(Dinosaur);
}

bool ACombatAIManager::CanEngageCombat(AActor* Dinosaur, AActor* Target) const
{
    if (!Dinosaur || !Target)
    {
        return false;
    }
    
    float Distance = FVector::Dist(Dinosaur->GetActorLocation(), Target->GetActorLocation());
    return Distance <= CombatRange;
}

float ACombatAIManager::GetOptimalAttackDistance(AActor* Dinosaur) const
{
    if (!Dinosaur)
    {
        return 0.0f;
    }
    
    // Different attack distances based on dinosaur type
    FString DinosaurName = Dinosaur->GetActorNameOrLabel().ToLower();
    
    if (DinosaurName.Contains(TEXT("trex")))
    {
        return 300.0f; // Close range for T-Rex
    }
    else if (DinosaurName.Contains(TEXT("veloci")) || DinosaurName.Contains(TEXT("raptor")))
    {
        return 150.0f; // Very close for raptors
    }
    else if (DinosaurName.Contains(TEXT("tricera")))
    {
        return 400.0f; // Medium range for Triceratops
    }
    
    return 250.0f; // Default attack distance
}

FVector ACombatAIManager::GetFlankingPosition(AActor* Dinosaur, AActor* Target) const
{
    if (!Dinosaur || !Target)
    {
        return FVector::ZeroVector;
    }
    
    FVector TargetLoc = Target->GetActorLocation();
    FVector DinosaurLoc = Dinosaur->GetActorLocation();
    
    // Calculate perpendicular flanking position
    FVector ToTarget = (TargetLoc - DinosaurLoc).GetSafeNormal();
    FVector RightVector = FVector::CrossProduct(ToTarget, FVector::UpVector);
    
    // Choose left or right flanking
    float FlankDirection = FMath::RandBool() ? 1.0f : -1.0f;
    float FlankDistance = GetOptimalAttackDistance(Dinosaur) * 1.5f;
    
    FVector FlankingPos = TargetLoc + (RightVector * FlankDirection * FlankDistance);
    
    return FlankingPos;
}

void ACombatAIManager::AdjustDifficultyBasedOnPerformance(bool PlayerWon, float EncounterDuration)
{
    TotalEncounters++;
    
    // Update win rate
    PlayerWinRate = ((PlayerWinRate * (TotalEncounters - 1)) + (PlayerWon ? 1.0f : 0.0f)) / TotalEncounters;
    
    // Update average encounter duration
    AverageEncounterDuration = ((AverageEncounterDuration * (TotalEncounters - 1)) + EncounterDuration) / TotalEncounters;
    
    // Adjust difficulty based on player performance
    if (PlayerWinRate > 0.7f && AverageEncounterDuration < 20.0f)
    {
        // Player is winning too easily, increase difficulty
        DifficultyScaling = FMath::Clamp(DifficultyScaling + 0.1f, 0.5f, 2.0f);
    }
    else if (PlayerWinRate < 0.3f || AverageEncounterDuration > 60.0f)
    {
        // Player is struggling, decrease difficulty
        DifficultyScaling = FMath::Clamp(DifficultyScaling - 0.1f, 0.5f, 2.0f);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Difficulty adjusted: Win Rate: %f, Avg Duration: %f, Scaling: %f"), 
           PlayerWinRate, AverageEncounterDuration, DifficultyScaling);
}

void ACombatAIManager::SetDynamicDifficulty(float NewDifficulty)
{
    DifficultyScaling = FMath::Clamp(NewDifficulty, 0.1f, 3.0f);
    UE_LOG(LogTemp, Warning, TEXT("Dynamic difficulty set to: %f"), DifficultyScaling);
}

void ACombatAIManager::UpdateCombatTimers(float DeltaTime)
{
    for (auto& TimerPair : CombatTimers)
    {
        TimerPair.Value += DeltaTime;
    }
}

void ACombatAIManager::ManagePackCoordination()
{
    // Update pack coordination logic
    for (auto& PackPair : PackGroups)
    {
        AActor* Leader = PackPair.Key;
        TArray<AActor*>& PackMembers = PackPair.Value;
        
        if (!IsValid(Leader))
        {
            continue;
        }
        
        // Coordinate pack movements and attacks
        AActor* Target = CombatPairs.FindRef(Leader);
        if (IsValid(Target))
        {
            for (AActor* Member : PackMembers)
            {
                if (IsValid(Member) && Member != Leader)
                {
                    // Assign flanking positions to pack members
                    if (!FlankingPositions.Contains(Member))
                    {
                        FVector FlankPos = GetFlankingPosition(Member, Target);
                        FlankingPositions.Add(Member, FlankPos);
                    }
                }
            }
        }
    }
}

void ACombatAIManager::CalculateFlankingPositions()
{
    // Update flanking positions for active combatants
    for (AActor* Combatant : ActiveCombatants)
    {
        if (!IsValid(Combatant))
        {
            continue;
        }
        
        AActor* Target = CombatPairs.FindRef(Combatant);
        if (IsValid(Target))
        {
            FVector CurrentFlankPos = FlankingPositions.FindRef(Combatant);
            FVector NewFlankPos = GetFlankingPosition(Combatant, Target);
            
            // Update flanking position if target has moved significantly
            if (FVector::Dist(CurrentFlankPos, NewFlankPos) > 200.0f)
            {
                FlankingPositions.FindOrAdd(Combatant) = NewFlankPos;
            }
        }
    }
}

bool ACombatAIManager::ValidateFlankingPosition(const FVector& Position, AActor* Target) const
{
    if (!Target)
    {
        return false;
    }
    
    // Basic validation - check if position is not too close to target
    float Distance = FVector::Dist(Position, Target->GetActorLocation());
    return Distance > 100.0f && Distance < CombatRange;
}