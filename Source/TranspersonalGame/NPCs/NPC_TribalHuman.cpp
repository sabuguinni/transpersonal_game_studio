#include "NPC_TribalHuman.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"

ANPC_TribalHuman::ANPC_TribalHuman()
{
    PrimaryActorTick.bCanEverTick = true;

    // Initialize collision
    GetCapsuleComponent()->SetCapsuleHalfHeight(88.0f);
    GetCapsuleComponent()->SetCapsuleRadius(34.0f);

    // Initialize movement
    GetCharacterMovement()->MaxWalkSpeed = 400.0f;
    GetCharacterMovement()->JumpZVelocity = 600.0f;
    GetCharacterMovement()->AirControl = 0.2f;

    // Create behavior component
    BehaviorComponent = CreateDefaultSubobject<UNPCBehaviorComponent>(TEXT("BehaviorComponent"));

    // Initialize default values
    TribalRole = ENPC_TribalRole::Gatherer;
    CurrentMood = ENPC_TribalMood::Calm;
    DetectionRadius = 1500.0f;
    FleeRadius = 800.0f;
    PatrolRadius = 2000.0f;
    bIsNocturnal = false;
    CurrentActivityIndex = 0;

    // Initialize stats
    TribalStats = FNPC_TribalStats();
    Memory = FNPC_TribalMemory();
}

void ANPC_TribalHuman::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeTribalHuman();
    
    // Set home location to spawn location
    HomeLocation = GetActorLocation();
    
    // Initialize daily routine based on role
    SetTribalRole(TribalRole);
}

void ANPC_TribalHuman::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    UpdateStats(DeltaTime);
    UpdateDailyRoutine(DeltaTime);
    ProcessFear(DeltaTime);
    ProcessNeeds(DeltaTime);
    
    // Check for player
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (PlayerPawn)
    {
        float Distance = FVector::Dist(GetActorLocation(), PlayerPawn->GetActorLocation());
        if (Distance <= DetectionRadius)
        {
            ReactToPlayer(PlayerPawn, Distance);
        }
    }
    
    ExecuteRoleSpecificBehavior();
}

void ANPC_TribalHuman::InitializeTribalHuman()
{
    // Set up default daily routine
    DailyRoutine.Empty();
    
    FNPC_DailyActivity WakeUp;
    WakeUp.ActivityType = ENPC_ActivityType::Idle;
    WakeUp.StartTime = 6.0f;
    WakeUp.Duration = 1.0f;
    WakeUp.Location = HomeLocation;
    DailyRoutine.Add(WakeUp);
    
    FNPC_DailyActivity MorningWork;
    MorningWork.ActivityType = ENPC_ActivityType::Work;
    MorningWork.StartTime = 7.0f;
    MorningWork.Duration = 4.0f;
    MorningWork.Location = HomeLocation + FVector(500, 0, 0);
    DailyRoutine.Add(MorningWork);
    
    FNPC_DailyActivity Lunch;
    Lunch.ActivityType = ENPC_ActivityType::Eat;
    Lunch.StartTime = 12.0f;
    Lunch.Duration = 1.0f;
    Lunch.Location = HomeLocation;
    DailyRoutine.Add(Lunch);
    
    FNPC_DailyActivity AfternoonWork;
    AfternoonWork.ActivityType = ENPC_ActivityType::Work;
    AfternoonWork.StartTime = 13.0f;
    AfternoonWork.Duration = 4.0f;
    AfternoonWork.Location = HomeLocation + FVector(-500, 0, 0);
    DailyRoutine.Add(AfternoonWork);
    
    FNPC_DailyActivity Evening;
    Evening.ActivityType = ENPC_ActivityType::Social;
    Evening.StartTime = 18.0f;
    Evening.Duration = 2.0f;
    Evening.Location = HomeLocation;
    DailyRoutine.Add(Evening);
    
    FNPC_DailyActivity Sleep;
    Sleep.ActivityType = ENPC_ActivityType::Sleep;
    Sleep.StartTime = 22.0f;
    Sleep.Duration = 8.0f;
    Sleep.Location = HomeLocation;
    DailyRoutine.Add(Sleep);
}

void ANPC_TribalHuman::SetTribalRole(ENPC_TribalRole NewRole)
{
    TribalRole = NewRole;
    
    // Adjust stats based on role
    switch (TribalRole)
    {
        case ENPC_TribalRole::Hunter:
            TribalStats.Aggression = 40.0f;
            GetCharacterMovement()->MaxWalkSpeed = 500.0f;
            DetectionRadius = 2000.0f;
            break;
            
        case ENPC_TribalRole::Warrior:
            TribalStats.Aggression = 60.0f;
            TribalStats.MaxHealth = 150.0f;
            TribalStats.Health = 150.0f;
            FleeRadius = 400.0f;
            break;
            
        case ENPC_TribalRole::Scout:
            GetCharacterMovement()->MaxWalkSpeed = 600.0f;
            DetectionRadius = 2500.0f;
            PatrolRadius = 3000.0f;
            break;
            
        case ENPC_TribalRole::Elder:
            GetCharacterMovement()->MaxWalkSpeed = 200.0f;
            TribalStats.Aggression = 10.0f;
            break;
            
        case ENPC_TribalRole::Gatherer:
            PatrolRadius = 1500.0f;
            break;
            
        case ENPC_TribalRole::Crafter:
            PatrolRadius = 800.0f;
            break;
    }
}

void ANPC_TribalHuman::UpdateMood(ENPC_TribalMood NewMood)
{
    CurrentMood = NewMood;
    
    // Adjust behavior based on mood
    switch (CurrentMood)
    {
        case ENPC_TribalMood::Fearful:
            GetCharacterMovement()->MaxWalkSpeed *= 1.5f;
            FleeRadius *= 1.5f;
            break;
            
        case ENPC_TribalMood::Aggressive:
            TribalStats.Aggression *= 1.3f;
            FleeRadius *= 0.7f;
            break;
            
        case ENPC_TribalMood::Alert:
            DetectionRadius *= 1.2f;
            break;
            
        case ENPC_TribalMood::Tired:
            GetCharacterMovement()->MaxWalkSpeed *= 0.7f;
            break;
    }
}

void ANPC_TribalHuman::ReactToPlayer(AActor* Player, float Distance)
{
    if (!Player) return;
    
    // Remember player location
    RememberPlayerLocation(Player->GetActorLocation(), false);
    
    if (ShouldFleeFromPlayer(Distance))
    {
        UpdateMood(ENPC_TribalMood::Fearful);
        
        // Calculate flee direction
        FVector FleeDirection = (GetActorLocation() - Player->GetActorLocation()).GetSafeNormal();
        FVector FleeTarget = GetActorLocation() + FleeDirection * 1000.0f;
        
        // Move away from player
        if (BehaviorComponent)
        {
            BehaviorComponent->SetCurrentGoal(ENPC_BehaviorGoal::Flee);
            BehaviorComponent->SetTargetLocation(FleeTarget);
        }
    }
    else if (ShouldApproachPlayer(Distance))
    {
        UpdateMood(ENPC_TribalMood::Curious);
        
        if (BehaviorComponent)
        {
            BehaviorComponent->SetCurrentGoal(ENPC_BehaviorGoal::Investigate);
            BehaviorComponent->SetTargetLocation(Player->GetActorLocation());
        }
    }
    else
    {
        UpdateMood(ENPC_TribalMood::Alert);
    }
}

void ANPC_TribalHuman::UpdateStats(float DeltaTime)
{
    // Decrease hunger and thirst over time
    TribalStats.Hunger = FMath::Max(0.0f, TribalStats.Hunger - DeltaTime * 2.0f);
    TribalStats.Thirst = FMath::Max(0.0f, TribalStats.Thirst - DeltaTime * 3.0f);
    
    // Decrease energy if active
    if (CurrentMood != ENPC_TribalMood::Tired)
    {
        TribalStats.Energy = FMath::Max(0.0f, TribalStats.Energy - DeltaTime * 1.0f);
    }
    
    // Regenerate health slowly
    if (TribalStats.Health < TribalStats.MaxHealth)
    {
        TribalStats.Health = FMath::Min(TribalStats.MaxHealth, TribalStats.Health + DeltaTime * 5.0f);
    }
    
    // Decrease fear over time
    TribalStats.Fear = FMath::Max(0.0f, TribalStats.Fear - DeltaTime * 10.0f);
}

void ANPC_TribalHuman::PerformActivity(ENPC_ActivityType Activity)
{
    switch (Activity)
    {
        case ENPC_ActivityType::Eat:
            TribalStats.Hunger = FMath::Min(100.0f, TribalStats.Hunger + 30.0f);
            Memory.LastMealTime = GetWorld()->GetTimeSeconds();
            break;
            
        case ENPC_ActivityType::Sleep:
            TribalStats.Energy = 100.0f;
            Memory.LastRestTime = GetWorld()->GetTimeSeconds();
            UpdateMood(ENPC_TribalMood::Calm);
            break;
            
        case ENPC_ActivityType::Work:
            TribalStats.Energy = FMath::Max(0.0f, TribalStats.Energy - 20.0f);
            break;
            
        case ENPC_ActivityType::Social:
            TribalStats.Fear = FMath::Max(0.0f, TribalStats.Fear - 10.0f);
            UpdateMood(ENPC_TribalMood::Calm);
            break;
    }
}

void ANPC_TribalHuman::RememberPlayerLocation(FVector Location, bool bWasHostile)
{
    Memory.LastPlayerLocation = Location;
    Memory.LastPlayerSeen = GetWorld()->GetTimeSeconds();
    Memory.bPlayerWasHostile = bWasHostile;
    
    if (bWasHostile)
    {
        AddDangerZone(Location);
    }
}

void ANPC_TribalHuman::AddDangerZone(FVector Location)
{
    Memory.DangerZones.AddUnique(Location);
    
    // Limit danger zones to prevent memory bloat
    if (Memory.DangerZones.Num() > 10)
    {
        Memory.DangerZones.RemoveAt(0);
    }
}

void ANPC_TribalHuman::AddSafeZone(FVector Location)
{
    Memory.SafeZones.AddUnique(Location);
    
    // Limit safe zones
    if (Memory.SafeZones.Num() > 5)
    {
        Memory.SafeZones.RemoveAt(0);
    }
}

bool ANPC_TribalHuman::IsPlayerInMemory() const
{
    float TimeSinceLastSeen = GetWorld()->GetTimeSeconds() - Memory.LastPlayerSeen;
    return TimeSinceLastSeen < 300.0f; // Remember for 5 minutes
}

void ANPC_TribalHuman::UpdateDailyRoutine(float DeltaTime)
{
    if (DailyRoutine.Num() == 0) return;
    
    // Get current time of day (simplified)
    float CurrentTime = FMath::Fmod(GetWorld()->GetTimeSeconds() / 60.0f, 24.0f);
    
    // Find current activity
    for (int32 i = 0; i < DailyRoutine.Num(); i++)
    {
        const FNPC_DailyActivity& Activity = DailyRoutine[i];
        float EndTime = Activity.StartTime + Activity.Duration;
        
        if (CurrentTime >= Activity.StartTime && CurrentTime < EndTime)
        {
            if (CurrentActivityIndex != i)
            {
                CurrentActivityIndex = i;
                PerformActivity(Activity.ActivityType);
                
                // Set behavior goal based on activity
                if (BehaviorComponent)
                {
                    switch (Activity.ActivityType)
                    {
                        case ENPC_ActivityType::Work:
                            BehaviorComponent->SetCurrentGoal(ENPC_BehaviorGoal::Work);
                            break;
                        case ENPC_ActivityType::Patrol:
                            BehaviorComponent->SetCurrentGoal(ENPC_BehaviorGoal::Patrol);
                            break;
                        case ENPC_ActivityType::Sleep:
                            BehaviorComponent->SetCurrentGoal(ENPC_BehaviorGoal::Rest);
                            break;
                        default:
                            BehaviorComponent->SetCurrentGoal(ENPC_BehaviorGoal::Idle);
                            break;
                    }
                    BehaviorComponent->SetTargetLocation(Activity.Location);
                }
            }
            break;
        }
    }
}

void ANPC_TribalHuman::ProcessFear(float DeltaTime)
{
    if (TribalStats.Fear > 70.0f)
    {
        UpdateMood(ENPC_TribalMood::Fearful);
    }
    else if (TribalStats.Fear > 40.0f)
    {
        UpdateMood(ENPC_TribalMood::Alert);
    }
}

void ANPC_TribalHuman::ProcessNeeds(float DeltaTime)
{
    // Check if needs are critical
    if (TribalStats.Hunger < 20.0f || TribalStats.Thirst < 20.0f)
    {
        if (BehaviorComponent)
        {
            BehaviorComponent->SetCurrentGoal(ENPC_BehaviorGoal::Survive);
        }
    }
    
    if (TribalStats.Energy < 20.0f)
    {
        UpdateMood(ENPC_TribalMood::Tired);
        if (BehaviorComponent)
        {
            BehaviorComponent->SetCurrentGoal(ENPC_BehaviorGoal::Rest);
        }
    }
}

bool ANPC_TribalHuman::ShouldFleeFromPlayer(float Distance) const
{
    // Flee based on role, mood, and stats
    float FleeThreshold = FleeRadius;
    
    if (CurrentMood == ENPC_TribalMood::Fearful)
    {
        FleeThreshold *= 1.5f;
    }
    
    if (TribalRole == ENPC_TribalRole::Elder || TribalRole == ENPC_TribalRole::Gatherer)
    {
        FleeThreshold *= 1.3f;
    }
    
    if (TribalStats.Health < 50.0f)
    {
        FleeThreshold *= 1.4f;
    }
    
    return Distance < FleeThreshold && TribalStats.Fear > 30.0f;
}

bool ANPC_TribalHuman::ShouldApproachPlayer(float Distance) const
{
    // Approach based on role and mood
    if (TribalRole == ENPC_TribalRole::Scout || TribalRole == ENPC_TribalRole::Elder)
    {
        return Distance > FleeRadius && Distance < DetectionRadius && TribalStats.Fear < 20.0f;
    }
    
    return false;
}

void ANPC_TribalHuman::ExecuteRoleSpecificBehavior()
{
    switch (TribalRole)
    {
        case ENPC_TribalRole::Scout:
            // Scouts patrol larger areas
            if (BehaviorComponent && BehaviorComponent->GetCurrentGoal() == ENPC_BehaviorGoal::Idle)
            {
                FVector PatrolTarget = HomeLocation + FVector(
                    FMath::RandRange(-PatrolRadius, PatrolRadius),
                    FMath::RandRange(-PatrolRadius, PatrolRadius),
                    0
                );
                BehaviorComponent->SetCurrentGoal(ENPC_BehaviorGoal::Patrol);
                BehaviorComponent->SetTargetLocation(PatrolTarget);
            }
            break;
            
        case ENPC_TribalRole::Hunter:
            // Hunters look for prey
            break;
            
        case ENPC_TribalRole::Warrior:
            // Warriors defend territory
            break;
    }
}