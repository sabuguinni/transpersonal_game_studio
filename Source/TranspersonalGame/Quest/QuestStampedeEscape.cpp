// QuestStampedeEscape.cpp
// Agent #14 — Quest & Mission Designer
// CYCLE: PROD_CYCLE_AUTO_20260628_011
// Full implementation of stampede escape quest system

#include "QuestStampedeEscape.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

// ─────────────────────────────────────────────────────────────────────────────
// AQuestStampedeTrigger — Constructor
// ─────────────────────────────────────────────────────────────────────────────

AQuestStampedeTrigger::AQuestStampedeTrigger()
{
    PrimaryActorTick.bCanEverTick = true;

    // Root trigger sphere
    TriggerSphere = CreateDefaultSubobject<USphereComponent>(TEXT("TriggerSphere"));
    TriggerSphere->InitSphereRadius(600.f);
    TriggerSphere->SetCollisionProfileName(TEXT("Trigger"));
    RootComponent = TriggerSphere;

    // Visual marker
    MarkerMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MarkerMesh"));
    MarkerMesh->SetupAttachment(RootComponent);
    MarkerMesh->SetRelativeScale3D(FVector(1.5f, 1.5f, 0.2f));
    MarkerMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    // Defaults
    DetectionRadius = 600.f;
    EscapeTimeLimit = 45.f;
    ElapsedEscapeTime = 0.f;
    XPReward = 150;
    bQuestTriggered = false;
    CurrentPhase = EQuest_StampedePhase::Inactive;

    // Stampede path corridor — northeast from herd center (3000, 2000)
    StampedePathStart = FVector(3000.f, 2000.f, 100.f);
    StampedePathEnd   = FVector(5000.f, 4500.f, 100.f);
    StampedePathWidth = 800.f;

    // Default objectives
    FQuest_StampedeObjectiveData Obj1;
    Obj1.ObjectiveType   = EQuest_StampedeObjective::ReachHighGround;
    Obj1.TargetLocation  = FVector(4200.f, 1200.f, 400.f);
    Obj1.CompletionRadius = 350.f;
    Obj1.ObjectiveLabel  = TEXT("Reach the ridge before the herd arrives");
    Objectives.Add(Obj1);

    FQuest_StampedeObjectiveData Obj2;
    Obj2.ObjectiveType   = EQuest_StampedeObjective::AvoidStampedePath;
    Obj2.TargetLocation  = FVector(3000.f, 2000.f, 100.f);
    Obj2.CompletionRadius = 800.f;
    Obj2.ObjectiveLabel  = TEXT("Stay out of the stampede corridor");
    Objectives.Add(Obj2);

    FQuest_StampedeObjectiveData Obj3;
    Obj3.ObjectiveType   = EQuest_StampedeObjective::InvestigateThreat;
    Obj3.TargetLocation  = FVector(2200.f, 2000.f, 100.f);
    Obj3.CompletionRadius = 300.f;
    Obj3.ObjectiveLabel  = TEXT("Investigate what caused the stampede");
    Objectives.Add(Obj3);
}

void AQuestStampedeTrigger::BeginPlay()
{
    Super::BeginPlay();
    TriggerSphere->OnComponentBeginOverlap.AddDynamic(this, &AQuestStampedeTrigger::OnTriggerOverlapBegin);
    UE_LOG(LogTemp, Log, TEXT("[QuestStampede] Trigger ready at %s"), *GetActorLocation().ToString());
}

void AQuestStampedeTrigger::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (CurrentPhase == EQuest_StampedePhase::EscapeWindow)
    {
        ElapsedEscapeTime += DeltaTime;

        // Check player position every tick during escape window
        ACharacter* Player = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
        if (Player)
        {
            EvaluatePlayerPosition(Player->GetActorLocation());
        }

        // Time expired — player failed to escape
        if (ElapsedEscapeTime >= EscapeTimeLimit)
        {
            QuestFail();
        }
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// Quest Activation
// ─────────────────────────────────────────────────────────────────────────────

void AQuestStampedeTrigger::OnTriggerOverlapBegin(UPrimitiveComponent* OverlappedComp,
    AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
    bool bFromSweep, const FHitResult& SweepResult)
{
    if (bQuestTriggered) return;

    ACharacter* Player = Cast<ACharacter>(OtherActor);
    if (!Player) return;

    ActivateQuest();
}

void AQuestStampedeTrigger::ActivateQuest()
{
    if (bQuestTriggered) return;
    bQuestTriggered = true;
    CurrentPhase = EQuest_StampedePhase::HerdDetected;

    UE_LOG(LogTemp, Warning, TEXT("[QuestStampede] QUEST ACTIVATED: Stampede Warning"));
    UE_LOG(LogTemp, Warning, TEXT("[QuestStampede] Phase: HerdDetected — herd is restless"));

    // After 5 seconds (simulated), stampede begins
    FTimerHandle StampedeTimer;
    GetWorldTimerManager().SetTimer(StampedeTimer, this,
        &AQuestStampedeTrigger::TriggerStampedePhase, 5.f, false);
}

void AQuestStampedeTrigger::TriggerStampedePhase()
{
    CurrentPhase = EQuest_StampedePhase::StampedeBegins;
    UE_LOG(LogTemp, Warning, TEXT("[QuestStampede] Phase: StampedeBegins — HERD IS RUNNING!"));

    // Short delay then open escape window
    FTimerHandle EscapeTimer;
    GetWorldTimerManager().SetTimer(EscapeTimer, [this]()
    {
        CurrentPhase = EQuest_StampedePhase::EscapeWindow;
        ElapsedEscapeTime = 0.f;
        UE_LOG(LogTemp, Warning, TEXT("[QuestStampede] Phase: EscapeWindow — %.0f seconds to escape!"),
            EscapeTimeLimit);
    }, 2.f, false);
}

// ─────────────────────────────────────────────────────────────────────────────
// Objective & Position Evaluation
// ─────────────────────────────────────────────────────────────────────────────

void AQuestStampedeTrigger::CompleteObjective(EQuest_StampedeObjective ObjectiveType)
{
    for (FQuest_StampedeObjectiveData& Obj : Objectives)
    {
        if (Obj.ObjectiveType == ObjectiveType && !Obj.bIsCompleted)
        {
            Obj.bIsCompleted = true;
            UE_LOG(LogTemp, Log, TEXT("[QuestStampede] Objective COMPLETE: %s"), *Obj.ObjectiveLabel);
            break;
        }
    }
}

void AQuestStampedeTrigger::EvaluatePlayerPosition(FVector PlayerLocation)
{
    if (CurrentPhase != EQuest_StampedePhase::EscapeWindow) return;

    // Check if player is in the danger corridor
    if (IsPlayerInStampedePath(PlayerLocation))
    {
        // Player is in path — warn them (in a real game, apply damage/knockback)
        UE_LOG(LogTemp, Warning, TEXT("[QuestStampede] DANGER: Player in stampede path at %s"),
            *PlayerLocation.ToString());
    }

    // Check objective: reach high ground (Z > 350)
    if (PlayerLocation.Z > 350.f)
    {
        CompleteObjective(EQuest_StampedeObjective::ReachHighGround);
        QuestSuccess();
    }
}

bool AQuestStampedeTrigger::IsPlayerInStampedePath(FVector PlayerLocation) const
{
    // Project player onto stampede corridor line
    FVector PathDir = (StampedePathEnd - StampedePathStart).GetSafeNormal();
    FVector ToPlayer = PlayerLocation - StampedePathStart;
    float Projection = FVector::DotProduct(ToPlayer, PathDir);
    float PathLength = FVector::Dist(StampedePathStart, StampedePathEnd);

    if (Projection < 0.f || Projection > PathLength) return false;

    FVector ClosestPoint = StampedePathStart + PathDir * Projection;
    float DistFromPath = FVector::Dist2D(PlayerLocation, ClosestPoint);

    return DistFromPath < StampedePathWidth;
}

float AQuestStampedeTrigger::GetEscapeTimeRemaining() const
{
    return FMath::Max(0.f, EscapeTimeLimit - ElapsedEscapeTime);
}

int32 AQuestStampedeTrigger::GetCompletedObjectiveCount() const
{
    int32 Count = 0;
    for (const FQuest_StampedeObjectiveData& Obj : Objectives)
    {
        if (Obj.bIsCompleted) Count++;
    }
    return Count;
}

void AQuestStampedeTrigger::QuestSuccess()
{
    if (CurrentPhase == EQuest_StampedePhase::PlayerEscaped) return;
    CurrentPhase = EQuest_StampedePhase::PlayerEscaped;
    SetActorTickEnabled(false);

    UE_LOG(LogTemp, Warning, TEXT("[QuestStampede] QUEST SUCCESS! Player escaped the stampede. XP: %d"), XPReward);
}

void AQuestStampedeTrigger::QuestFail()
{
    if (CurrentPhase == EQuest_StampedePhase::PlayerCrushed) return;
    CurrentPhase = EQuest_StampedePhase::PlayerCrushed;
    SetActorTickEnabled(false);

    UE_LOG(LogTemp, Error, TEXT("[QuestStampede] QUEST FAILED — Player crushed by stampede!"));
}

// ─────────────────────────────────────────────────────────────────────────────
// AQuestStampedeEscapeZone — Constructor & Implementation
// ─────────────────────────────────────────────────────────────────────────────

AQuestStampedeEscapeZone::AQuestStampedeEscapeZone()
{
    PrimaryActorTick.bCanEverTick = false;

    EscapeSphere = CreateDefaultSubobject<USphereComponent>(TEXT("EscapeSphere"));
    EscapeSphere->InitSphereRadius(350.f);
    EscapeSphere->SetCollisionProfileName(TEXT("Trigger"));
    RootComponent = EscapeSphere;

    EscapeMarker = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("EscapeMarker"));
    EscapeMarker->SetupAttachment(RootComponent);
    EscapeMarker->SetRelativeScale3D(FVector(2.f, 2.f, 0.3f));
    EscapeMarker->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    EscapeRadius = 350.f;
}

void AQuestStampedeEscapeZone::BeginPlay()
{
    Super::BeginPlay();
    EscapeSphere->OnComponentBeginOverlap.AddDynamic(this, &AQuestStampedeEscapeZone::OnEscapeOverlapBegin);
    UE_LOG(LogTemp, Log, TEXT("[QuestStampede] EscapeZone ready at %s"), *GetActorLocation().ToString());
}

void AQuestStampedeEscapeZone::OnEscapeOverlapBegin(UPrimitiveComponent* OverlappedComp,
    AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
    bool bFromSweep, const FHitResult& SweepResult)
{
    ACharacter* Player = Cast<ACharacter>(OtherActor);
    if (!Player) return;

    NotifyPlayerEscaped(OtherActor);
}

void AQuestStampedeEscapeZone::NotifyPlayerEscaped(AActor* PlayerActor)
{
    UE_LOG(LogTemp, Warning, TEXT("[QuestStampede] Player reached escape zone at %s"),
        *GetActorLocation().ToString());

    // Resolve linked quest trigger
    if (LinkedQuestTrigger.IsValid())
    {
        AQuestStampedeTrigger* Trigger = LinkedQuestTrigger.Get();
        if (Trigger)
        {
            Trigger->CompleteObjective(EQuest_StampedeObjective::ReachHighGround);
            Trigger->QuestSuccess();
        }
    }
}
