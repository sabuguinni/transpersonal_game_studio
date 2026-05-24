#include "NPC_PrimitiveHumanBehavior.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/Character.h"
#include "Components/PrimitiveComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "DinosaurBase.h"
#include "TranspersonalCharacter.h"
#include "Engine/Engine.h"

UNPC_PrimitiveHumanBehavior::UNPC_PrimitiveHumanBehavior()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f;

    // Initialize behavior parameters
    CurrentBehaviorState = ENPC_PrimitiveBehaviorState::Idle;
    DetectionRange = 2000.0f;
    FleeRange = 1500.0f;
    SocialRange = 800.0f;
    GatheringRange = 1000.0f;
    BehaviorUpdateInterval = 2.0f;
    SurvivalNeedsDecayRate = 1.0f;
    
    LastBehaviorChangeTime = 0.0f;
    BehaviorStateDuration = 0.0f;
    
    HomeLocation = FVector::ZeroVector;
    CurrentDestination = FVector::ZeroVector;
    CurrentTarget = nullptr;

    // Initialize survival needs
    SurvivalNeeds = FNPC_SurvivalNeeds();

    // Setup default daily routines
    FNPC_DailyRoutine MorningGathering;
    MorningGathering.StartTime = 6.0f;
    MorningGathering.EndTime = 10.0f;
    MorningGathering.ActivityName = TEXT("Morning Gathering");
    MorningGathering.MovementSpeed = 150.0f;
    DailyRoutines.Add(MorningGathering);

    FNPC_DailyRoutine AfternoonHunting;
    AfternoonHunting.StartTime = 14.0f;
    AfternoonHunting.EndTime = 17.0f;
    AfternoonHunting.ActivityName = TEXT("Afternoon Hunting");
    AfternoonHunting.MovementSpeed = 250.0f;
    DailyRoutines.Add(AfternoonHunting);

    FNPC_DailyRoutine EveningRest;
    EveningRest.StartTime = 20.0f;
    EveningRest.EndTime = 6.0f;
    EveningRest.ActivityName = TEXT("Evening Rest");
    EveningRest.MovementSpeed = 100.0f;
    DailyRoutines.Add(EveningRest);
}

void UNPC_PrimitiveHumanBehavior::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeBehavior();
    
    // Set up timers
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(
            BehaviorUpdateTimer,
            this,
            &UNPC_PrimitiveHumanBehavior::UpdateBehaviorState,
            BehaviorUpdateInterval,
            true
        );

        World->GetTimerManager().SetTimer(
            SurvivalNeedsTimer,
            [this]() { UpdateSurvivalNeeds(SurvivalNeedsDecayRate); },
            5.0f,
            true
        );
    }
}

void UNPC_PrimitiveHumanBehavior::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Update nearby actors periodically
    static float ActorScanTimer = 0.0f;
    ActorScanTimer += DeltaTime;
    
    if (ActorScanTimer >= 1.0f)
    {
        UpdateNearbyActors();
        ActorScanTimer = 0.0f;
    }
    
    // Execute current behavior
    ExecuteCurrentBehavior();
    
    // Update behavior state duration
    BehaviorStateDuration += DeltaTime;
}

void UNPC_PrimitiveHumanBehavior::InitializeBehavior()
{
    if (AActor* Owner = GetOwner())
    {
        HomeLocation = Owner->GetActorLocation();
        CurrentDestination = HomeLocation;
        
        // Set initial routine locations relative to home
        for (FNPC_DailyRoutine& Routine : DailyRoutines)
        {
            if (Routine.TargetLocation.IsZero())
            {
                FVector RandomOffset = FVector(
                    FMath::RandRange(-500.0f, 500.0f),
                    FMath::RandRange(-500.0f, 500.0f),
                    0.0f
                );
                Routine.TargetLocation = HomeLocation + RandomOffset;
            }
        }
    }
}

void UNPC_PrimitiveHumanBehavior::UpdateNearbyActors()
{
    if (!GetOwner()) return;

    NearbyDinosaurs.Empty();
    NearbyHumans.Empty();
    
    FVector OwnerLocation = GetOwner()->GetActorLocation();
    
    if (UWorld* World = GetWorld())
    {
        TArray<AActor*> FoundActors;
        UGameplayStatics::GetAllActorsOfClass(World, APawn::StaticClass(), FoundActors);
        
        for (AActor* Actor : FoundActors)
        {
            if (!Actor || Actor == GetOwner()) continue;
            
            float Distance = FVector::Dist(OwnerLocation, Actor->GetActorLocation());
            
            if (Distance <= DetectionRange)
            {
                // Check if it's a dinosaur
                if (ADinosaurBase* Dinosaur = Cast<ADinosaurBase>(Actor))
                {
                    NearbyDinosaurs.Add(Actor);
                }
                // Check if it's another human/NPC
                else if (Actor->FindComponentByClass<UNPC_PrimitiveHumanBehavior>())
                {
                    NearbyHumans.Add(Actor);
                }
            }
        }
    }
}

void UNPC_PrimitiveHumanBehavior::UpdateBehaviorState()
{
    // Check for immediate threats first
    ScanForThreats();
    
    // If not fleeing, check other behaviors
    if (CurrentBehaviorState != ENPC_PrimitiveBehaviorState::Fleeing)
    {
        // Check survival needs
        if (SurvivalNeeds.Hunger > 80.0f || SurvivalNeeds.Thirst > 80.0f)
        {
            SetBehaviorState(ENPC_PrimitiveBehaviorState::Gathering);
        }
        else if (SurvivalNeeds.Fatigue > 70.0f || IsNightTime())
        {
            SetBehaviorState(ENPC_PrimitiveBehaviorState::Resting);
        }
        else if (NearbyHumans.Num() > 0 && SurvivalNeeds.Fear < 30.0f)
        {
            SetBehaviorState(ENPC_PrimitiveBehaviorState::Socializing);
        }
        else
        {
            // Execute daily routine
            ExecuteDailyRoutine();
        }
    }
}

void UNPC_PrimitiveHumanBehavior::ScanForThreats()
{
    AActor* NearestThreat = nullptr;
    float NearestDistance = FleeRange;
    
    for (AActor* Dinosaur : NearbyDinosaurs)
    {
        if (!Dinosaur) continue;
        
        float Distance = FVector::Dist(GetOwner()->GetActorLocation(), Dinosaur->GetActorLocation());
        
        if (Distance < NearestDistance && IsActorDangerous(Dinosaur))
        {
            NearestThreat = Dinosaur;
            NearestDistance = Distance;
        }
    }
    
    if (NearestThreat)
    {
        FleeFromDanger(NearestThreat);
    }
}

void UNPC_PrimitiveHumanBehavior::ScanForResources()
{
    // Simple resource scanning - look for gathering spots
    if (CurrentBehaviorState == ENPC_PrimitiveBehaviorState::Gathering)
    {
        FVector GatheringSpot = FindGatheringLocation();
        if (!GatheringSpot.IsZero())
        {
            CurrentDestination = GatheringSpot;
        }
    }
}

void UNPC_PrimitiveHumanBehavior::ScanForSocialTargets()
{
    if (NearbyHumans.Num() > 0 && CurrentBehaviorState != ENPC_PrimitiveBehaviorState::Fleeing)
    {
        // Find nearest human for social interaction
        AActor* NearestHuman = nullptr;
        float NearestDistance = SocialRange;
        
        for (AActor* Human : NearbyHumans)
        {
            if (!Human) continue;
            
            float Distance = FVector::Dist(GetOwner()->GetActorLocation(), Human->GetActorLocation());
            if (Distance < NearestDistance)
            {
                NearestHuman = Human;
                NearestDistance = Distance;
            }
        }
        
        if (NearestHuman)
        {
            CurrentTarget = NearestHuman;
            if (CurrentBehaviorState == ENPC_PrimitiveBehaviorState::Idle)
            {
                SetBehaviorState(ENPC_PrimitiveBehaviorState::Socializing);
            }
        }
    }
}

void UNPC_PrimitiveHumanBehavior::ExecuteCurrentBehavior()
{
    switch (CurrentBehaviorState)
    {
        case ENPC_PrimitiveBehaviorState::Idle:
            HandleIdleBehavior();
            break;
            
        case ENPC_PrimitiveBehaviorState::Gathering:
            HandleGatheringBehavior();
            break;
            
        case ENPC_PrimitiveBehaviorState::Fleeing:
            HandleFleeingBehavior();
            break;
            
        case ENPC_PrimitiveBehaviorState::Resting:
            HandleRestingBehavior();
            break;
            
        case ENPC_PrimitiveBehaviorState::Socializing:
            HandleSocializingBehavior();
            break;
            
        default:
            HandleIdleBehavior();
            break;
    }
}

void UNPC_PrimitiveHumanBehavior::SetBehaviorState(ENPC_PrimitiveBehaviorState NewState)
{
    if (CurrentBehaviorState != NewState)
    {
        CurrentBehaviorState = NewState;
        LastBehaviorChangeTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
        BehaviorStateDuration = 0.0f;
        
        // Reset target when changing states
        if (NewState != ENPC_PrimitiveBehaviorState::Socializing)
        {
            CurrentTarget = nullptr;
        }
    }
}

void UNPC_PrimitiveHumanBehavior::FleeFromDanger(AActor* ThreatActor)
{
    if (!ThreatActor || !GetOwner()) return;
    
    SetBehaviorState(ENPC_PrimitiveBehaviorState::Fleeing);
    
    // Calculate flee direction (away from threat)
    FVector ThreatLocation = ThreatActor->GetActorLocation();
    FVector OwnerLocation = GetOwner()->GetActorLocation();
    FVector FleeDirection = (OwnerLocation - ThreatLocation).GetSafeNormal();
    
    // Find safe location
    FVector SafeLocation = FindSafeLocation();
    if (!SafeLocation.IsZero())
    {
        CurrentDestination = SafeLocation;
        MoveToLocation(CurrentDestination, 400.0f); // Fast movement when fleeing
    }
    
    // Increase fear
    SurvivalNeeds.Fear = FMath::Min(100.0f, SurvivalNeeds.Fear + 20.0f);
}

void UNPC_PrimitiveHumanBehavior::MoveToLocation(FVector TargetLocation, float Speed)
{
    if (!GetOwner()) return;
    
    CurrentDestination = TargetLocation;
    
    // Simple movement implementation - in a real game this would use AI movement component
    if (APawn* OwnerPawn = Cast<APawn>(GetOwner()))
    {
        FVector Direction = (TargetLocation - OwnerPawn->GetActorLocation()).GetSafeNormal();
        OwnerPawn->AddMovementInput(Direction, 1.0f);
    }
}

void UNPC_PrimitiveHumanBehavior::UpdateSurvivalNeeds(float DeltaTime)
{
    // Gradually increase needs over time
    SurvivalNeeds.Hunger = FMath::Min(100.0f, SurvivalNeeds.Hunger + (SurvivalNeedsDecayRate * DeltaTime));
    SurvivalNeeds.Thirst = FMath::Min(100.0f, SurvivalNeeds.Thirst + (SurvivalNeedsDecayRate * 1.5f * DeltaTime));
    SurvivalNeeds.Fatigue = FMath::Min(100.0f, SurvivalNeeds.Fatigue + (SurvivalNeedsDecayRate * 0.5f * DeltaTime));
    
    // Fear decreases over time if not in danger
    if (CurrentBehaviorState != ENPC_PrimitiveBehaviorState::Fleeing)
    {
        SurvivalNeeds.Fear = FMath::Max(0.0f, SurvivalNeeds.Fear - (SurvivalNeedsDecayRate * 2.0f * DeltaTime));
    }
    
    // Warmth varies with time of day
    float TimeOfDay = GetCurrentTimeOfDay();
    if (TimeOfDay >= 20.0f || TimeOfDay <= 6.0f) // Night time
    {
        SurvivalNeeds.Warmth = FMath::Max(0.0f, SurvivalNeeds.Warmth - (SurvivalNeedsDecayRate * DeltaTime));
    }
    else
    {
        SurvivalNeeds.Warmth = FMath::Min(100.0f, SurvivalNeeds.Warmth + (SurvivalNeedsDecayRate * 0.5f * DeltaTime));
    }
}

void UNPC_PrimitiveHumanBehavior::ExecuteDailyRoutine()
{
    float CurrentTime = GetCurrentTimeOfDay();
    
    for (const FNPC_DailyRoutine& Routine : DailyRoutines)
    {
        bool InTimeRange = false;
        
        if (Routine.EndTime > Routine.StartTime)
        {
            // Normal time range (e.g., 8:00 to 18:00)
            InTimeRange = (CurrentTime >= Routine.StartTime && CurrentTime <= Routine.EndTime);
        }
        else
        {
            // Overnight range (e.g., 20:00 to 6:00)
            InTimeRange = (CurrentTime >= Routine.StartTime || CurrentTime <= Routine.EndTime);
        }
        
        if (InTimeRange)
        {
            CurrentDestination = Routine.TargetLocation;
            
            if (Routine.ActivityName.Contains(TEXT("Gathering")))
            {
                SetBehaviorState(ENPC_PrimitiveBehaviorState::Gathering);
            }
            else if (Routine.ActivityName.Contains(TEXT("Hunting")))
            {
                SetBehaviorState(ENPC_PrimitiveBehaviorState::Hunting);
            }
            else if (Routine.ActivityName.Contains(TEXT("Rest")))
            {
                SetBehaviorState(ENPC_PrimitiveBehaviorState::Resting);
            }
            else
            {
                SetBehaviorState(ENPC_PrimitiveBehaviorState::Idle);
            }
            
            MoveToLocation(CurrentDestination, Routine.MovementSpeed);
            break;
        }
    }
}

float UNPC_PrimitiveHumanBehavior::GetCurrentTimeOfDay() const
{
    if (UWorld* World = GetWorld())
    {
        // Convert world time to 24-hour format
        float WorldTime = World->GetTimeSeconds();
        float DayLength = 1200.0f; // 20 minutes per day
        float TimeOfDay = FMath::Fmod(WorldTime / DayLength * 24.0f, 24.0f);
        return TimeOfDay;
    }
    return 12.0f; // Default to noon
}

bool UNPC_PrimitiveHumanBehavior::IsNightTime() const
{
    float TimeOfDay = GetCurrentTimeOfDay();
    return (TimeOfDay >= 20.0f || TimeOfDay <= 6.0f);
}

void UNPC_PrimitiveHumanBehavior::ReactToPlayer(ATranspersonalCharacter* Player)
{
    if (!Player || !GetOwner()) return;
    
    float Distance = FVector::Dist(GetOwner()->GetActorLocation(), Player->GetActorLocation());
    
    if (Distance <= SocialRange)
    {
        // Friendly reaction to player
        CurrentTarget = Player;
        
        if (SurvivalNeeds.Fear < 50.0f)
        {
            SetBehaviorState(ENPC_PrimitiveBehaviorState::Socializing);
        }
        else
        {
            // Too afraid to socialize
            SetBehaviorState(ENPC_PrimitiveBehaviorState::Idle);
        }
    }
}

bool UNPC_PrimitiveHumanBehavior::IsActorDangerous(AActor* Actor) const
{
    if (!Actor) return false;
    
    // Check if it's a large dinosaur
    if (ADinosaurBase* Dinosaur = Cast<ADinosaurBase>(Actor))
    {
        // Large predators are dangerous
        FString DinosaurName = Dinosaur->GetName();
        if (DinosaurName.Contains(TEXT("TRex")) || DinosaurName.Contains(TEXT("Carnotaurus")))
        {
            return true;
        }
        
        // Raptors in groups are dangerous
        if (DinosaurName.Contains(TEXT("Raptor")) && NearbyDinosaurs.Num() >= 2)
        {
            return true;
        }
    }
    
    return false;
}

FVector UNPC_PrimitiveHumanBehavior::FindSafeLocation() const
{
    if (!GetOwner()) return FVector::ZeroVector;
    
    FVector OwnerLocation = GetOwner()->GetActorLocation();
    
    // Try multiple directions to find a safe spot
    for (int32 i = 0; i < 8; i++)
    {
        float Angle = i * 45.0f;
        FVector Direction = FVector(
            FMath::Cos(FMath::DegreesToRadians(Angle)),
            FMath::Sin(FMath::DegreesToRadians(Angle)),
            0.0f
        );
        
        FVector TestLocation = OwnerLocation + (Direction * 1000.0f);
        
        // Simple check - if no dinosaurs nearby, it's safe
        bool bSafe = true;
        for (AActor* Dinosaur : NearbyDinosaurs)
        {
            if (Dinosaur && FVector::Dist(TestLocation, Dinosaur->GetActorLocation()) < 800.0f)
            {
                bSafe = false;
                break;
            }
        }
        
        if (bSafe)
        {
            return TestLocation;
        }
    }
    
    // If no safe location found, return home
    return HomeLocation;
}

FVector UNPC_PrimitiveHumanBehavior::FindGatheringLocation() const
{
    if (!GetOwner()) return FVector::ZeroVector;
    
    FVector OwnerLocation = GetOwner()->GetActorLocation();
    
    // Find a random location within gathering range
    FVector RandomDirection = FVector(
        FMath::RandRange(-1.0f, 1.0f),
        FMath::RandRange(-1.0f, 1.0f),
        0.0f
    ).GetSafeNormal();
    
    return OwnerLocation + (RandomDirection * FMath::RandRange(200.0f, GatheringRange));
}

void UNPC_PrimitiveHumanBehavior::HandleIdleBehavior()
{
    // Look for something to do
    ScanForSocialTargets();
    ScanForResources();
    
    // If idle for too long, start patrolling
    if (BehaviorStateDuration > 30.0f)
    {
        SetBehaviorState(ENPC_PrimitiveBehaviorState::Patrolling);
        CurrentDestination = HomeLocation + FVector(
            FMath::RandRange(-500.0f, 500.0f),
            FMath::RandRange(-500.0f, 500.0f),
            0.0f
        );
        MoveToLocation(CurrentDestination, 150.0f);
    }
}

void UNPC_PrimitiveHumanBehavior::HandleGatheringBehavior()
{
    // Move to gathering location
    if (FVector::Dist(GetOwner()->GetActorLocation(), CurrentDestination) > 100.0f)
    {
        MoveToLocation(CurrentDestination, 150.0f);
    }
    else
    {
        // "Gathering" - reduce hunger and thirst
        SurvivalNeeds.Hunger = FMath::Max(0.0f, SurvivalNeeds.Hunger - 10.0f);
        SurvivalNeeds.Thirst = FMath::Max(0.0f, SurvivalNeeds.Thirst - 15.0f);
        
        // After gathering for a while, return to idle
        if (BehaviorStateDuration > 20.0f)
        {
            SetBehaviorState(ENPC_PrimitiveBehaviorState::Idle);
        }
    }
}

void UNPC_PrimitiveHumanBehavior::HandleFleeingBehavior()
{
    // Keep moving to safe location
    if (FVector::Dist(GetOwner()->GetActorLocation(), CurrentDestination) > 200.0f)
    {
        MoveToLocation(CurrentDestination, 400.0f);
    }
    else
    {
        // Check if still in danger
        bool bStillInDanger = false;
        for (AActor* Dinosaur : NearbyDinosaurs)
        {
            if (Dinosaur && IsActorDangerous(Dinosaur))
            {
                float Distance = FVector::Dist(GetOwner()->GetActorLocation(), Dinosaur->GetActorLocation());
                if (Distance < FleeRange * 1.5f)
                {
                    bStillInDanger = true;
                    break;
                }
            }
        }
        
        if (!bStillInDanger)
        {
            SetBehaviorState(ENPC_PrimitiveBehaviorState::Idle);
        }
    }
}

void UNPC_PrimitiveHumanBehavior::HandleRestingBehavior()
{
    // Move to home/rest location
    if (FVector::Dist(GetOwner()->GetActorLocation(), HomeLocation) > 100.0f)
    {
        MoveToLocation(HomeLocation, 100.0f);
    }
    else
    {
        // Resting - reduce fatigue and increase warmth
        SurvivalNeeds.Fatigue = FMath::Max(0.0f, SurvivalNeeds.Fatigue - 5.0f);
        SurvivalNeeds.Warmth = FMath::Min(100.0f, SurvivalNeeds.Warmth + 2.0f);
        
        // Rest until fatigue is low or it's day time
        if (SurvivalNeeds.Fatigue < 30.0f && !IsNightTime())
        {
            SetBehaviorState(ENPC_PrimitiveBehaviorState::Idle);
        }
    }
}

void UNPC_PrimitiveHumanBehavior::HandleSocializingBehavior()
{
    if (CurrentTarget)
    {
        // Move closer to social target
        float Distance = FVector::Dist(GetOwner()->GetActorLocation(), CurrentTarget->GetActorLocation());
        
        if (Distance > SocialRange * 0.5f)
        {
            MoveToLocation(CurrentTarget->GetActorLocation(), 120.0f);
        }
        else
        {
            // "Socializing" - reduce fear and increase happiness
            SurvivalNeeds.Fear = FMath::Max(0.0f, SurvivalNeeds.Fear - 2.0f);
            
            // After socializing for a while, return to other activities
            if (BehaviorStateDuration > 15.0f)
            {
                SetBehaviorState(ENPC_PrimitiveBehaviorState::Idle);
            }
        }
    }
    else
    {
        // No social target, return to idle
        SetBehaviorState(ENPC_PrimitiveBehaviorState::Idle);
    }
}