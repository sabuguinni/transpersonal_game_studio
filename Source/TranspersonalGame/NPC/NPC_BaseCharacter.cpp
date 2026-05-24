#include "NPC_BaseCharacter.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"

ANPC_BaseCharacter::ANPC_BaseCharacter()
{
    PrimaryActorTick.bCanEverTick = true;

    // Initialize stats
    MaxHealth = 100.0f;
    Health = MaxHealth;
    Hunger = 0.0f;
    Thirst = 0.0f;
    MaxStamina = 100.0f;
    Stamina = MaxStamina;
    Fear = 0.0f;

    // Initialize memory system
    MaxMemoryEntries = 50;
    MemoryDecayRate = 1.0f;

    // Initialize social system
    PersonalityType = ENPC_PersonalityType::Balanced;
    SocialRadius = 1000.0f;

    // Initialize routine
    CurrentActivity = ENPC_ActivityType::Idle;
    ActivityTimer = 0.0f;
    CurrentState = ENPC_NPCState::Idle;
    LastActivityTime = 0.0f;
    CurrentTarget = nullptr;
    CurrentDestination = FVector::ZeroVector;

    // Setup perception component
    PawnSensingComponent = CreateDefaultSubobject<UPawnSensingComponent>(TEXT("PawnSensingComponent"));
    PawnSensingComponent->SetPeripheralVisionAngle(90.0f);
    PawnSensingComponent->SightRadius = 1500.0f;
    PawnSensingComponent->HearingThreshold = 600.0f;
    PawnSensingComponent->LOSHearingThreshold = 1200.0f;

    // Bind perception events
    PawnSensingComponent->OnSeePawn.AddDynamic(this, &ANPC_BaseCharacter::OnSeePawn);
    PawnSensingComponent->OnHearNoise.AddDynamic(this, &ANPC_BaseCharacter::OnHearNoise);

    // Setup movement
    GetCharacterMovement()->MaxWalkSpeed = 300.0f;
    GetCharacterMovement()->MaxAcceleration = 1000.0f;

    // Set home location to spawn location
    HomeLocation = GetActorLocation();
}

void ANPC_BaseCharacter::BeginPlay()
{
    Super::BeginPlay();

    // Set home location to actual spawn location
    HomeLocation = GetActorLocation();

    // Initialize patrol points around home
    PatrolPoints.Empty();
    for (int32 i = 0; i < 4; i++)
    {
        float Angle = i * 90.0f * PI / 180.0f;
        FVector PatrolPoint = HomeLocation + FVector(FMath::Cos(Angle), FMath::Sin(Angle), 0.0f) * 500.0f;
        PatrolPoints.Add(PatrolPoint);
    }

    // Start behavior tree if available
    if (BehaviorTree && GetController())
    {
        if (AAIController* AIController = Cast<AAIController>(GetController()))
        {
            AIController->RunBehaviorTree(BehaviorTree);
        }
    }
}

void ANPC_BaseCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    UpdateStats(DeltaTime);
    UpdateMemory(DeltaTime);
    ExecuteDailyRoutine(DeltaTime);
    UpdateBlackboardValues();
}

void ANPC_BaseCharacter::AddMemory(FVector Location, AActor* Actor, ENPC_MemoryType MemoryType, float Importance)
{
    FNPC_MemoryEntry NewMemory;
    NewMemory.Location = Location;
    NewMemory.Actor = Actor;
    NewMemory.MemoryType = MemoryType;
    NewMemory.Importance = Importance;
    NewMemory.Timestamp = GetWorld()->GetTimeSeconds();

    MemoryEntries.Add(NewMemory);

    // Remove oldest memories if we exceed the limit
    if (MemoryEntries.Num() > MaxMemoryEntries)
    {
        MemoryEntries.RemoveAt(0);
    }
}

void ANPC_BaseCharacter::UpdateMemory(float DeltaTime)
{
    float CurrentTime = GetWorld()->GetTimeSeconds();

    // Decay memory importance over time
    for (FNPC_MemoryEntry& Memory : MemoryEntries)
    {
        float TimeSinceMemory = CurrentTime - Memory.Timestamp;
        Memory.Importance *= FMath::Exp(-MemoryDecayRate * DeltaTime);

        // Remove very old or unimportant memories
        if (Memory.Importance < 0.1f || TimeSinceMemory > 300.0f) // 5 minutes
        {
            MemoryEntries.RemoveSingle(Memory);
        }
    }

    CleanupOldMemories();
}

TArray<FNPC_MemoryEntry> ANPC_BaseCharacter::GetMemoriesOfType(ENPC_MemoryType MemoryType)
{
    TArray<FNPC_MemoryEntry> FilteredMemories;

    for (const FNPC_MemoryEntry& Memory : MemoryEntries)
    {
        if (Memory.MemoryType == MemoryType)
        {
            FilteredMemories.Add(Memory);
        }
    }

    return FilteredMemories;
}

FNPC_MemoryEntry ANPC_BaseCharacter::GetMostImportantMemory()
{
    FNPC_MemoryEntry MostImportant;
    float HighestImportance = 0.0f;

    for (const FNPC_MemoryEntry& Memory : MemoryEntries)
    {
        if (Memory.Importance > HighestImportance)
        {
            HighestImportance = Memory.Importance;
            MostImportant = Memory;
        }
    }

    return MostImportant;
}

void ANPC_BaseCharacter::UpdateRelationship(AActor* TargetActor, ENPC_RelationshipType NewType, float TrustChange)
{
    if (!TargetActor) return;

    // Find existing relationship
    for (FNPC_SocialRelationship& Relationship : SocialRelationships)
    {
        if (Relationship.TargetActor == TargetActor)
        {
            Relationship.RelationshipType = NewType;
            Relationship.TrustLevel = FMath::Clamp(Relationship.TrustLevel + TrustChange, -100.0f, 100.0f);
            Relationship.LastInteractionTime = GetWorld()->GetTimeSeconds();
            return;
        }
    }

    // Create new relationship
    FNPC_SocialRelationship NewRelationship;
    NewRelationship.TargetActor = TargetActor;
    NewRelationship.RelationshipType = NewType;
    NewRelationship.TrustLevel = TrustChange;
    NewRelationship.LastInteractionTime = GetWorld()->GetTimeSeconds();
    SocialRelationships.Add(NewRelationship);
}

FNPC_SocialRelationship ANPC_BaseCharacter::GetRelationshipWith(AActor* TargetActor)
{
    for (const FNPC_SocialRelationship& Relationship : SocialRelationships)
    {
        if (Relationship.TargetActor == TargetActor)
        {
            return Relationship;
        }
    }

    // Return neutral relationship if none found
    FNPC_SocialRelationship NeutralRelationship;
    NeutralRelationship.TargetActor = TargetActor;
    return NeutralRelationship;
}

TArray<AActor*> ANPC_BaseCharacter::GetAlliesInRadius(float Radius)
{
    TArray<AActor*> Allies;

    for (const FNPC_SocialRelationship& Relationship : SocialRelationships)
    {
        if (Relationship.RelationshipType == ENPC_RelationshipType::Ally && 
            Relationship.TargetActor &&
            FVector::Dist(GetActorLocation(), Relationship.TargetActor->GetActorLocation()) <= Radius)
        {
            Allies.Add(Relationship.TargetActor);
        }
    }

    return Allies;
}

void ANPC_BaseCharacter::SetActivity(ENPC_ActivityType NewActivity)
{
    if (CurrentActivity != NewActivity)
    {
        CurrentActivity = NewActivity;
        ActivityTimer = 0.0f;
        LastActivityTime = GetWorld()->GetTimeSeconds();

        // Log activity change for debugging
        UE_LOG(LogTemp, Log, TEXT("NPC %s changed activity to %d"), *GetName(), (int32)NewActivity);
    }
}

void ANPC_BaseCharacter::ReactToThreat(AActor* ThreatActor, ENPC_ThreatLevel ThreatLevel)
{
    if (!ThreatActor) return;

    // Add threat to memory
    AddMemory(ThreatActor->GetActorLocation(), ThreatActor, ENPC_MemoryType::Threat, (float)ThreatLevel * 2.0f);

    // Update fear based on threat level
    Fear = FMath::Clamp(Fear + (float)ThreatLevel * 20.0f, 0.0f, 100.0f);

    // Decide reaction based on personality and threat level
    if (ShouldFlee(ThreatActor))
    {
        CurrentState = ENPC_NPCState::Fleeing;
        CurrentTarget = ThreatActor;
        SetActivity(ENPC_ActivityType::Fleeing);
    }
    else if (ThreatLevel >= ENPC_ThreatLevel::High)
    {
        CurrentState = ENPC_NPCState::Combat;
        CurrentTarget = ThreatActor;
        SetActivity(ENPC_ActivityType::Fighting);
    }
    else
    {
        CurrentState = ENPC_NPCState::Alert;
        SetActivity(ENPC_ActivityType::Investigating);
    }
}

void ANPC_BaseCharacter::ExecuteDailyRoutine(float DeltaTime)
{
    ActivityTimer += DeltaTime;

    switch (CurrentActivity)
    {
        case ENPC_ActivityType::Idle:
            if (ActivityTimer > 10.0f) // Idle for 10 seconds
            {
                SetActivity(ENPC_ActivityType::Patrolling);
                CurrentDestination = GetRandomPatrolPoint();
            }
            break;

        case ENPC_ActivityType::Patrolling:
            if (FVector::Dist(GetActorLocation(), CurrentDestination) < 100.0f || ActivityTimer > 30.0f)
            {
                SetActivity(ENPC_ActivityType::Resting);
            }
            break;

        case ENPC_ActivityType::Resting:
            if (ActivityTimer > 5.0f) // Rest for 5 seconds
            {
                if (Hunger > 50.0f)
                {
                    SetActivity(ENPC_ActivityType::Foraging);
                }
                else if (Thirst > 50.0f)
                {
                    SetActivity(ENPC_ActivityType::DrinkingWater);
                }
                else
                {
                    SetActivity(ENPC_ActivityType::Patrolling);
                    CurrentDestination = GetRandomPatrolPoint();
                }
            }
            break;

        case ENPC_ActivityType::Foraging:
            if (ActivityTimer > 15.0f) // Forage for 15 seconds
            {
                Hunger = FMath::Max(0.0f, Hunger - 30.0f);
                SetActivity(ENPC_ActivityType::Idle);
            }
            break;

        case ENPC_ActivityType::DrinkingWater:
            if (ActivityTimer > 8.0f) // Drink for 8 seconds
            {
                Thirst = FMath::Max(0.0f, Thirst - 40.0f);
                SetActivity(ENPC_ActivityType::Idle);
            }
            break;

        default:
            break;
    }
}

void ANPC_BaseCharacter::OnSeePawn(APawn* SeenPawn)
{
    if (!SeenPawn || SeenPawn == this) return;

    // Add to memory
    AddMemory(SeenPawn->GetActorLocation(), SeenPawn, ENPC_MemoryType::Neutral, 1.5f);

    // Check if it's a threat
    if (SeenPawn->IsA<ANPC_BaseCharacter>())
    {
        FNPC_SocialRelationship Relationship = GetRelationshipWith(SeenPawn);
        if (Relationship.RelationshipType == ENPC_RelationshipType::Enemy)
        {
            ReactToThreat(SeenPawn, ENPC_ThreatLevel::Medium);
        }
    }
    else
    {
        // Assume player or unknown entity is a potential threat
        ReactToThreat(SeenPawn, ENPC_ThreatLevel::Low);
    }
}

void ANPC_BaseCharacter::OnHearNoise(APawn* NoiseInstigator, const FVector& Location, float Volume)
{
    if (!NoiseInstigator) return;

    // Add noise location to memory
    AddMemory(Location, NoiseInstigator, ENPC_MemoryType::Sound, Volume / 100.0f);

    // React to loud noises
    if (Volume > 50.0f)
    {
        CurrentState = ENPC_NPCState::Investigating;
        CurrentDestination = Location;
        SetActivity(ENPC_ActivityType::Investigating);
    }
}

void ANPC_BaseCharacter::TakeDamage(float DamageAmount, AActor* DamageSource)
{
    Health = FMath::Max(0.0f, Health - DamageAmount);
    Fear = FMath::Clamp(Fear + DamageAmount, 0.0f, 100.0f);

    if (DamageSource)
    {
        AddMemory(DamageSource->GetActorLocation(), DamageSource, ENPC_MemoryType::Threat, 5.0f);
        UpdateRelationship(DamageSource, ENPC_RelationshipType::Enemy, -50.0f);
        ReactToThreat(DamageSource, ENPC_ThreatLevel::High);
    }

    if (Health <= 0.0f)
    {
        CurrentState = ENPC_NPCState::Dead;
        SetActivity(ENPC_ActivityType::Dead);
    }
}

void ANPC_BaseCharacter::RestoreHealth(float HealAmount)
{
    Health = FMath::Min(MaxHealth, Health + HealAmount);
}

void ANPC_BaseCharacter::ConsumeFood(float FoodValue)
{
    Hunger = FMath::Max(0.0f, Hunger - FoodValue);
}

void ANPC_BaseCharacter::DrinkWater(float WaterValue)
{
    Thirst = FMath::Max(0.0f, Thirst - WaterValue);
}

bool ANPC_BaseCharacter::IsAlive() const
{
    return Health > 0.0f;
}

float ANPC_BaseCharacter::GetHealthPercentage() const
{
    return MaxHealth > 0.0f ? (Health / MaxHealth) * 100.0f : 0.0f;
}

ENPC_NPCState ANPC_BaseCharacter::GetCurrentState() const
{
    return CurrentState;
}

void ANPC_BaseCharacter::UpdateStats(float DeltaTime)
{
    // Gradually increase hunger and thirst
    Hunger = FMath::Min(100.0f, Hunger + DeltaTime * 2.0f);
    Thirst = FMath::Min(100.0f, Thirst + DeltaTime * 3.0f);

    // Regenerate stamina when not in combat
    if (CurrentState != ENPC_NPCState::Combat && CurrentState != ENPC_NPCState::Fleeing)
    {
        Stamina = FMath::Min(MaxStamina, Stamina + DeltaTime * 10.0f);
    }

    // Reduce fear over time
    Fear = FMath::Max(0.0f, Fear - DeltaTime * 5.0f);

    // Health regeneration when well-fed and hydrated
    if (Hunger < 20.0f && Thirst < 20.0f && Health < MaxHealth)
    {
        Health = FMath::Min(MaxHealth, Health + DeltaTime * 2.0f);
    }
}

void ANPC_BaseCharacter::CleanupOldMemories()
{
    MemoryEntries.RemoveAll([](const FNPC_MemoryEntry& Memory) {
        return Memory.Importance < 0.1f || !IsValid(Memory.Actor);
    });
}

void ANPC_BaseCharacter::UpdateBlackboardValues()
{
    if (AAIController* AIController = Cast<AAIController>(GetController()))
    {
        if (UBlackboardComponent* BlackboardComp = AIController->GetBlackboardComponent())
        {
            BlackboardComp->SetValueAsFloat(TEXT("Health"), Health);
            BlackboardComp->SetValueAsFloat(TEXT("Hunger"), Hunger);
            BlackboardComp->SetValueAsFloat(TEXT("Thirst"), Thirst);
            BlackboardComp->SetValueAsFloat(TEXT("Fear"), Fear);
            BlackboardComp->SetValueAsEnum(TEXT("CurrentState"), (uint8)CurrentState);
            BlackboardComp->SetValueAsEnum(TEXT("CurrentActivity"), (uint8)CurrentActivity);
            
            if (CurrentTarget)
            {
                BlackboardComp->SetValueAsObject(TEXT("CurrentTarget"), CurrentTarget);
            }
            
            BlackboardComp->SetValueAsVector(TEXT("HomeLocation"), HomeLocation);
            BlackboardComp->SetValueAsVector(TEXT("CurrentDestination"), CurrentDestination);
        }
    }
}

FVector ANPC_BaseCharacter::GetRandomPatrolPoint()
{
    if (PatrolPoints.Num() > 0)
    {
        int32 RandomIndex = FMath::RandRange(0, PatrolPoints.Num() - 1);
        return PatrolPoints[RandomIndex];
    }
    
    return HomeLocation;
}

bool ANPC_BaseCharacter::ShouldFlee(AActor* ThreatActor)
{
    if (!ThreatActor) return false;

    // Flee if health is low
    if (GetHealthPercentage() < 30.0f) return true;

    // Flee if fear is high
    if (Fear > 70.0f) return true;

    // Personality-based decisions
    switch (PersonalityType)
    {
        case ENPC_PersonalityType::Aggressive:
            return Fear > 90.0f; // Only flee when extremely afraid
        case ENPC_PersonalityType::Cautious:
            return Fear > 40.0f; // Flee more readily
        case ENPC_PersonalityType::Balanced:
        default:
            return Fear > 60.0f; // Moderate flee threshold
    }
}