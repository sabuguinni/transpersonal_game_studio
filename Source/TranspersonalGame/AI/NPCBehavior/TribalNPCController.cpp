#include "TribalNPCController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "NavigationSystem.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

ATribalNPCController::ATribalNPCController()
{
    PrimaryActorTick.bCanEverTick = true;

    // Perception component
    AIPerceptionComp = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerception"));
    SetPerceptionComponent(*AIPerceptionComp);

    // Sight config
    SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
    SightConfig->SightRadius = SightRadius;
    SightConfig->LoseSightRadius = LoseSightRadius;
    SightConfig->PeripheralVisionAngleDegrees = 90.0f;
    SightConfig->SetMaxAge(5.0f);
    SightConfig->DetectionByAffiliation.bDetectEnemies = true;
    SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
    SightConfig->DetectionByAffiliation.bDetectFriendlies = true;
    AIPerceptionComp->ConfigureSense(*SightConfig);

    // Hearing config
    HearingConfig = CreateDefaultSubobject<UAISenseConfig_Hearing>(TEXT("HearingConfig"));
    HearingConfig->HearingRange = HearingRadius;
    HearingConfig->SetMaxAge(3.0f);
    HearingConfig->DetectionByAffiliation.bDetectEnemies = true;
    HearingConfig->DetectionByAffiliation.bDetectNeutrals = true;
    HearingConfig->DetectionByAffiliation.bDetectFriendlies = true;
    AIPerceptionComp->ConfigureSense(*HearingConfig);

    AIPerceptionComp->SetDominantSense(SightConfig->GetSenseImplementation());

    CurrentState = ENPC_TribalState::Idle;
    FearLevel = 0.0f;
}

void ATribalNPCController::BeginPlay()
{
    Super::BeginPlay();

    AIPerceptionComp->OnPerceptionUpdated.AddDynamic(this, &ATribalNPCController::OnPerceptionUpdated);

    if (BehaviorTree && GetBlackboardComponent())
    {
        RunBehaviorTree(BehaviorTree);
        UpdateBlackboard();
    }
}

void ATribalNPCController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    UpdateFear(DeltaTime);
    ProcessMemoryDecay(DeltaTime);
    DetermineStateFromPerception();
    UpdateBlackboard();
}

void ATribalNPCController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);

    if (BehaviorTree)
    {
        RunBehaviorTree(BehaviorTree);
    }
}

void ATribalNPCController::OnUnPossess()
{
    Super::OnUnPossess();
}

void ATribalNPCController::OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors)
{
    for (AActor* Actor : UpdatedActors)
    {
        if (!Actor) continue;

        FActorPerceptionBlueprintInfo PerceptionInfo;
        AIPerceptionComp->GetActorsPerception(Actor, PerceptionInfo);

        bool bCurrentlySensed = false;
        for (const FAIStimulus& Stimulus : PerceptionInfo.LastSensedStimuli)
        {
            if (Stimulus.WasSuccessfullySensed())
            {
                bCurrentlySensed = true;
                break;
            }
        }

        if (bCurrentlySensed)
        {
            // Determine threat level from actor class name
            FString ActorClass = Actor->GetClass()->GetName();
            ENPC_ThreatLevel Level = ENPC_ThreatLevel::Low;

            if (ActorClass.Contains(TEXT("TRex")) || ActorClass.Contains(TEXT("Rex")))
            {
                Level = ENPC_ThreatLevel::Critical;
            }
            else if (ActorClass.Contains(TEXT("Raptor")))
            {
                Level = ENPC_ThreatLevel::High;
            }
            else if (ActorClass.Contains(TEXT("Dino")) || ActorClass.Contains(TEXT("Dinosaur")))
            {
                Level = ENPC_ThreatLevel::Medium;
            }

            RegisterThreat(Actor, Level, Actor->GetActorLocation());
        }
    }
}

void ATribalNPCController::SetNPCState(ENPC_TribalState NewState)
{
    if (CurrentState == NewState) return;
    CurrentState = NewState;
    UpdateBlackboard();
}

void ATribalNPCController::RegisterThreat(AActor* ThreatActor, ENPC_ThreatLevel Level, const FVector& Location)
{
    // Check if this actor is already in memory
    for (FNPC_MemoryEntry& Entry : MemoryEntries)
    {
        if (Entry.ThreatActorName == ThreatActor->GetName())
        {
            Entry.ThreatLocation = Location;
            Entry.ThreatTime = GetWorld()->GetTimeSeconds();
            Entry.ThreatLevel = Level;
            return;
        }
    }

    // Add new entry (respect max)
    if (MemoryEntries.Num() >= MaxMemoryEntries)
    {
        MemoryEntries.RemoveAt(0);
    }

    FNPC_MemoryEntry NewEntry;
    NewEntry.ThreatLocation = Location;
    NewEntry.ThreatTime = GetWorld()->GetTimeSeconds();
    NewEntry.ThreatLevel = Level;
    NewEntry.ThreatActorName = ThreatActor->GetName();
    MemoryEntries.Add(NewEntry);

    // Spike fear based on threat level
    float FearSpike = 0.0f;
    switch (Level)
    {
        case ENPC_ThreatLevel::Low:      FearSpike = 0.1f; break;
        case ENPC_ThreatLevel::Medium:   FearSpike = 0.25f; break;
        case ENPC_ThreatLevel::High:     FearSpike = 0.5f; break;
        case ENPC_ThreatLevel::Critical: FearSpike = 1.0f; break;
        default: break;
    }
    FearLevel = FMath::Clamp(FearLevel + FearSpike, 0.0f, 1.0f);
}

void ATribalNPCController::ClearMemory()
{
    MemoryEntries.Empty();
}

bool ATribalNPCController::HasRecentThreat(float WithinSeconds) const
{
    float Now = GetWorld()->GetTimeSeconds();
    for (const FNPC_MemoryEntry& Entry : MemoryEntries)
    {
        if ((Now - Entry.ThreatTime) <= WithinSeconds)
        {
            return true;
        }
    }
    return false;
}

FVector ATribalNPCController::GetLastKnownThreatLocation() const
{
    if (MemoryEntries.Num() == 0) return FVector::ZeroVector;

    float MostRecent = -1.0f;
    FVector LastLocation = FVector::ZeroVector;
    for (const FNPC_MemoryEntry& Entry : MemoryEntries)
    {
        if (Entry.ThreatTime > MostRecent)
        {
            MostRecent = Entry.ThreatTime;
            LastLocation = Entry.ThreatLocation;
        }
    }
    return LastLocation;
}

void ATribalNPCController::UpdateFear(float DeltaTime)
{
    if (!HasRecentThreat(5.0f))
    {
        FearLevel = FMath::Clamp(FearLevel - FearDecayRate * DeltaTime, 0.0f, 1.0f);
    }
}

void ATribalNPCController::ProcessMemoryDecay(float DeltaTime)
{
    float Now = GetWorld()->GetTimeSeconds();
    MemoryEntries.RemoveAll([&](const FNPC_MemoryEntry& Entry)
    {
        return (Now - Entry.ThreatTime) > MemoryDecayTime;
    });
}

void ATribalNPCController::UpdateBlackboard()
{
    UBlackboardComponent* BB = GetBlackboardComponent();
    if (!BB) return;

    BB->SetValueAsEnum(BB_NPCState, (uint8)CurrentState);
    BB->SetValueAsFloat(BB_FearLevel, FearLevel);

    if (HasRecentThreat())
    {
        FVector ThreatLoc = GetLastKnownThreatLocation();
        BB->SetValueAsVector(BB_ThreatActor, ThreatLoc);

        if (FearLevel >= 0.5f)
        {
            FVector FleeTarget = FindFleeLocation(ThreatLoc);
            BB->SetValueAsVector(BB_FleeTarget, FleeTarget);
        }
    }
}

void ATribalNPCController::DetermineStateFromPerception()
{
    if (FearLevel >= 0.8f)
    {
        SetNPCState(ENPC_TribalState::Flee);
    }
    else if (FearLevel >= 0.4f)
    {
        SetNPCState(ENPC_TribalState::Hide);
    }
    else if (FearLevel >= 0.1f)
    {
        SetNPCState(ENPC_TribalState::Alert);
    }
    else if (HasRecentThreat(60.0f))
    {
        SetNPCState(ENPC_TribalState::Patrol);
    }
    else
    {
        SetNPCState(ENPC_TribalState::Idle);
    }
}

FVector ATribalNPCController::FindFleeLocation(const FVector& ThreatLocation) const
{
    APawn* MyPawn = GetPawn();
    if (!MyPawn) return FVector::ZeroVector;

    FVector MyLocation = MyPawn->GetActorLocation();
    FVector AwayDirection = (MyLocation - ThreatLocation).GetSafeNormal();
    FVector FleeTarget = MyLocation + AwayDirection * 3000.0f;

    UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(GetWorld());
    if (NavSys)
    {
        FNavLocation NavLocation;
        if (NavSys->GetRandomPointInNavigableRadius(FleeTarget, 500.0f, NavLocation))
        {
            return NavLocation.Location;
        }
    }
    return FleeTarget;
}

FVector ATribalNPCController::FindHideLocation(const FVector& ThreatLocation) const
{
    APawn* MyPawn = GetPawn();
    if (!MyPawn) return FVector::ZeroVector;

    FVector MyLocation = MyPawn->GetActorLocation();
    FVector AwayDirection = (MyLocation - ThreatLocation).GetSafeNormal();
    // Hide behind cover — offset perpendicular to threat direction
    FVector Perp = FVector(-AwayDirection.Y, AwayDirection.X, 0.0f);
    FVector HideTarget = MyLocation + AwayDirection * 800.0f + Perp * 400.0f;

    UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(GetWorld());
    if (NavSys)
    {
        FNavLocation NavLocation;
        if (NavSys->GetRandomPointInNavigableRadius(HideTarget, 300.0f, NavLocation))
        {
            return NavLocation.Location;
        }
    }
    return HideTarget;
}
