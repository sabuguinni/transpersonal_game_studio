#include "NPCBehaviorActor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"

ANPCBehaviorActor::ANPCBehaviorActor()
{
    PrimaryActorTick.bCanEverTick = true;

    // CREATE COMPONENTS
    NPCMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("NPCMesh"));
    RootComponent = NPCMesh;

    InteractionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("InteractionSphere"));
    InteractionSphere->SetupAttachment(RootComponent);
    InteractionSphere->SetSphereRadius(300.0f);

    DetectionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("DetectionSphere"));
    DetectionSphere->SetupAttachment(RootComponent);
    DetectionSphere->SetSphereRadius(1500.0f);

    // INITIALIZE DEFAULT VALUES
    NPCType = ENPC_BehaviorType::Gatherer;
    NPCName = TEXT("Unknown");
    Age = 25;
    Experience = 50.0f;

    PatrolRadius = 1000.0f;
    InteractionRange = 300.0f;
    DetectionRange = 1500.0f;
    MovementSpeed = 200.0f;

    CurrentEmotion = ENPC_EmotionalState::Calm;
    EmotionIntensity = 1.0f;
    EmotionDecayRate = 0.1f;

    MaxMemories = 50;
    MemoryDecayRate = 0.05f;

    HomeLocation = FVector::ZeroVector;
    WorkLocation = FVector::ZeroVector;
    WorkStartTime = 8.0f;  // 8 AM
    WorkEndTime = 18.0f;   // 6 PM

    SocialRadius = 500.0f;
    bCanTrade = true;
    bCanGiveQuests = true;

    // INTERNAL STATE
    bIsPatrolling = false;
    PatrolCenter = FVector::ZeroVector;
    CurrentTarget = FVector::ZeroVector;
    LastEmotionUpdate = 0.0f;
    LastMemoryCleanup = 0.0f;
    LastRoutineCheck = 0.0f;
}

void ANPCBehaviorActor::BeginPlay()
{
    Super::BeginPlay();

    // INITIALIZE PATROL CENTER
    PatrolCenter = GetActorLocation();
    HomeLocation = GetActorLocation();
    
    // SET WORK LOCATION BASED ON NPC TYPE
    switch (NPCType)
    {
        case ENPC_BehaviorType::Gatherer:
            WorkLocation = PatrolCenter + FVector(500, 0, 0);
            break;
        case ENPC_BehaviorType::Hunter:
            WorkLocation = PatrolCenter + FVector(0, 800, 0);
            break;
        case ENPC_BehaviorType::Elder:
            WorkLocation = PatrolCenter;
            break;
        case ENPC_BehaviorType::Scout:
            WorkLocation = PatrolCenter + FVector(1200, 1200, 100);
            break;
        case ENPC_BehaviorType::Trader:
            WorkLocation = PatrolCenter + FVector(-300, 300, 0);
            break;
        default:
            WorkLocation = PatrolCenter;
            break;
    }

    // START INITIAL BEHAVIOR
    StartPatrol();
}

void ANPCBehaviorActor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // UPDATE EMOTIONAL STATE
    UpdateEmotionalState(DeltaTime);

    // CLEANUP MEMORIES PERIODICALLY
    if (GetWorld()->GetTimeSeconds() - LastMemoryCleanup > 30.0f)
    {
        CleanupMemories();
        LastMemoryCleanup = GetWorld()->GetTimeSeconds();
    }

    // CHECK DAILY ROUTINE
    if (GetWorld()->GetTimeSeconds() - LastRoutineCheck > 60.0f)
    {
        CheckDailyRoutine();
        LastRoutineCheck = GetWorld()->GetTimeSeconds();
    }

    // PATROL BEHAVIOR
    if (bIsPatrolling)
    {
        FVector CurrentLocation = GetActorLocation();
        float DistanceToTarget = FVector::Dist(CurrentLocation, CurrentTarget);

        if (DistanceToTarget < 100.0f)
        {
            CurrentTarget = GetRandomPatrolPoint();
        }
        else
        {
            FVector Direction = (CurrentTarget - CurrentLocation).GetSafeNormal();
            FVector NewLocation = CurrentLocation + (Direction * MovementSpeed * DeltaTime);
            SetActorLocation(NewLocation);
        }
    }

    // CHECK FOR NEARBY ACTORS
    TArray<AActor*> NearbyActors = GetNearbyActors();
    for (AActor* Actor : NearbyActors)
    {
        if (Actor && Actor->IsA<APawn>())
        {
            float Distance = FVector::Dist(GetActorLocation(), Actor->GetActorLocation());
            
            // REACT TO PLAYER
            if (Distance < InteractionRange)
            {
                if (CurrentEmotion == ENPC_EmotionalState::Calm)
                {
                    SetEmotionalState(ENPC_EmotionalState::Curious, 0.7f);
                    AddMemory(Actor->GetActorLocation(), TEXT("PlayerEncounter"), 0.8f);
                }
            }
        }
    }
}

void ANPCBehaviorActor::StartPatrol()
{
    bIsPatrolling = true;
    CurrentTarget = GetRandomPatrolPoint();
}

void ANPCBehaviorActor::StopPatrol()
{
    bIsPatrolling = false;
}

void ANPCBehaviorActor::SetEmotionalState(ENPC_EmotionalState NewEmotion, float Intensity)
{
    CurrentEmotion = NewEmotion;
    EmotionIntensity = FMath::Clamp(Intensity, 0.0f, 2.0f);
    LastEmotionUpdate = GetWorld()->GetTimeSeconds();

    // ADJUST BEHAVIOR BASED ON EMOTION
    switch (NewEmotion)
    {
        case ENPC_EmotionalState::Afraid:
            MovementSpeed = 300.0f;
            DetectionRange = 2000.0f;
            break;
        case ENPC_EmotionalState::Aggressive:
            MovementSpeed = 250.0f;
            DetectionRange = 1800.0f;
            break;
        case ENPC_EmotionalState::Calm:
            MovementSpeed = 200.0f;
            DetectionRange = 1500.0f;
            break;
        default:
            break;
    }
}

void ANPCBehaviorActor::AddMemory(FVector Location, const FString& EventType, float Importance)
{
    FNPC_Memory NewMemory;
    NewMemory.Location = Location;
    NewMemory.EventType = EventType;
    NewMemory.Timestamp = GetWorld()->GetTimeSeconds();
    NewMemory.Importance = Importance;

    Memories.Add(NewMemory);

    // REMOVE OLDEST MEMORY IF LIMIT EXCEEDED
    if (Memories.Num() > MaxMemories)
    {
        Memories.RemoveAt(0);
    }
}

void ANPCBehaviorActor::UpdateRelationship(AActor* OtherActor, float RelationshipChange)
{
    if (!OtherActor) return;

    if (Relationships.Contains(OtherActor))
    {
        float CurrentRelationship = Relationships[OtherActor];
        Relationships[OtherActor] = FMath::Clamp(CurrentRelationship + RelationshipChange, -1.0f, 1.0f);
    }
    else
    {
        Relationships.Add(OtherActor, FMath::Clamp(RelationshipChange, -1.0f, 1.0f));
    }
}

bool ANPCBehaviorActor::CanInteractWith(AActor* OtherActor)
{
    if (!OtherActor) return false;

    float Distance = FVector::Dist(GetActorLocation(), OtherActor->GetActorLocation());
    if (Distance > InteractionRange) return false;

    // CHECK RELATIONSHIP
    if (Relationships.Contains(OtherActor))
    {
        float Relationship = Relationships[OtherActor];
        if (Relationship < -0.5f) return false; // Too hostile
    }

    // CHECK EMOTIONAL STATE
    if (CurrentEmotion == ENPC_EmotionalState::Afraid && EmotionIntensity > 1.0f)
    {
        return false;
    }

    return HasLineOfSight(OtherActor);
}

void ANPCBehaviorActor::PerformDailyRoutine()
{
    // SIMPLIFIED TIME SYSTEM (0-24 hours)
    float CurrentTime = FMath::Fmod(GetWorld()->GetTimeSeconds() / 3600.0f, 24.0f);

    if (CurrentTime >= WorkStartTime && CurrentTime <= WorkEndTime)
    {
        // WORK TIME - MOVE TO WORK LOCATION
        if (FVector::Dist(GetActorLocation(), WorkLocation) > 200.0f)
        {
            FVector Direction = (WorkLocation - GetActorLocation()).GetSafeNormal();
            FVector NewLocation = GetActorLocation() + (Direction * MovementSpeed * 0.016f);
            SetActorLocation(NewLocation);
        }
    }
    else
    {
        // REST TIME - MOVE TO HOME
        if (FVector::Dist(GetActorLocation(), HomeLocation) > 200.0f)
        {
            FVector Direction = (HomeLocation - GetActorLocation()).GetSafeNormal();
            FVector NewLocation = GetActorLocation() + (Direction * MovementSpeed * 0.016f);
            SetActorLocation(NewLocation);
        }
    }
}

TArray<AActor*> ANPCBehaviorActor::GetNearbyActors()
{
    TArray<AActor*> NearbyActors;
    
    if (GetWorld())
    {
        for (TActorIterator<AActor> ActorItr(GetWorld()); ActorItr; ++ActorItr)
        {
            AActor* Actor = *ActorItr;
            if (Actor && Actor != this)
            {
                float Distance = FVector::Dist(GetActorLocation(), Actor->GetActorLocation());
                if (Distance <= DetectionRange)
                {
                    NearbyActors.Add(Actor);
                }
            }
        }
    }
    
    return NearbyActors;
}

bool ANPCBehaviorActor::IsPlayerInRange()
{
    if (GetWorld())
    {
        APawn* Player = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
        if (Player)
        {
            float Distance = FVector::Dist(GetActorLocation(), Player->GetActorLocation());
            return Distance <= DetectionRange;
        }
    }
    return false;
}

bool ANPCBehaviorActor::HasLineOfSight(AActor* Target)
{
    if (!Target || !GetWorld()) return false;

    FVector Start = GetActorLocation() + FVector(0, 0, 50); // Eye level
    FVector End = Target->GetActorLocation() + FVector(0, 0, 50);

    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(this);
    QueryParams.AddIgnoredActor(Target);

    bool bHit = GetWorld()->LineTraceSingleByChannel(
        HitResult,
        Start,
        End,
        ECollisionChannel::ECC_Visibility,
        QueryParams
    );

    return !bHit; // No obstacle = line of sight
}

void ANPCBehaviorActor::UpdateEmotionalState(float DeltaTime)
{
    // DECAY EMOTION INTENSITY OVER TIME
    if (EmotionIntensity > 0.1f)
    {
        EmotionIntensity -= EmotionDecayRate * DeltaTime;
        EmotionIntensity = FMath::Max(EmotionIntensity, 0.1f);
    }

    // RETURN TO CALM STATE IF EMOTION IS LOW
    if (EmotionIntensity <= 0.2f && CurrentEmotion != ENPC_EmotionalState::Calm)
    {
        SetEmotionalState(ENPC_EmotionalState::Calm, 0.1f);
    }
}

void ANPCBehaviorActor::CleanupMemories()
{
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    // REMOVE OLD OR LOW IMPORTANCE MEMORIES
    Memories.RemoveAll([CurrentTime, this](const FNPC_Memory& Memory)
    {
        float Age = CurrentTime - Memory.Timestamp;
        float DecayThreshold = Memory.Importance * 1000.0f; // Important memories last longer
        return Age > DecayThreshold;
    });
}

void ANPCBehaviorActor::CheckDailyRoutine()
{
    PerformDailyRoutine();
}

FVector ANPCBehaviorActor::GetRandomPatrolPoint()
{
    float Angle = FMath::RandRange(0.0f, 360.0f);
    float Distance = FMath::RandRange(PatrolRadius * 0.3f, PatrolRadius);
    
    FVector Offset = FVector(
        FMath::Cos(FMath::DegreesToRadians(Angle)) * Distance,
        FMath::Sin(FMath::DegreesToRadians(Angle)) * Distance,
        0.0f
    );
    
    return PatrolCenter + Offset;
}