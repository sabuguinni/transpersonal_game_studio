#include "QuestStampedeSystem.h"
#include "GameFramework/Character.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"

// ============================================================
// AQuestStampedeTrigger — Implementation
// ============================================================

AQuestStampedeTrigger::AQuestStampedeTrigger()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.bStartWithTickEnabled = false;

    TriggerVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerVolume"));
    TriggerVolume->SetBoxExtent(FVector(2000.0f, 2000.0f, 400.0f));
    TriggerVolume->SetCollisionProfileName(TEXT("Trigger"));
    RootComponent = TriggerVolume;

    DebugMarker = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DebugMarker"));
    DebugMarker->SetupAttachment(RootComponent);

    CurrentPhase = EQuest_StampedePhase::Inactive;
    RemainingTime = EscapeTimeLimit;
}

void AQuestStampedeTrigger::BeginPlay()
{
    Super::BeginPlay();
    InitialiseObjectives();
    TriggerVolume->OnComponentBeginOverlap.AddDynamic(this, &AQuestStampedeTrigger::OnTriggerOverlapBegin);
}

void AQuestStampedeTrigger::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (CurrentPhase != EQuest_StampedePhase::Escaping)
        return;

    RemainingTime -= DeltaTime;

    // Check safe zone every tick
    UWorld* World = GetWorld();
    if (World)
    {
        APlayerController* PC = World->GetFirstPlayerController();
        if (PC && PC->GetPawn())
        {
            CheckSafeZoneReached(PC->GetPawn());
        }
    }

    if (RemainingTime <= 0.0f)
    {
        FailQuest();
    }
}

void AQuestStampedeTrigger::InitialiseObjectives()
{
    Objectives.Empty();

    FQuest_StampedeObjective Obj0;
    Obj0.ObjectiveName = TEXT("Escape the Stampede Corridor");
    Obj0.Description   = TEXT("Reach high ground before the herd crushes you. You have 60 seconds.");
    Obj0.bCompleted    = false;
    Obj0.bOptional     = false;
    Objectives.Add(Obj0);

    FQuest_StampedeObjective Obj1;
    Obj1.ObjectiveName = TEXT("Avoid the Sentinel Parasaurolophus");
    Obj1.Description   = TEXT("Three dominant animals lead the herd. Stay clear of them.");
    Obj1.bCompleted    = false;
    Obj1.bOptional     = true;
    Objectives.Add(Obj1);

    FQuest_StampedeObjective Obj2;
    Obj2.ObjectiveName = TEXT("Reach the High Ground Safe Zone");
    Obj2.Description   = TEXT("Climb to the broken ridge north of the river crossing.");
    Obj2.bCompleted    = false;
    Obj2.bOptional     = false;
    Objectives.Add(Obj2);
}

void AQuestStampedeTrigger::OnTriggerOverlapBegin(
    UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
    bool bFromSweep, const FHitResult& SweepResult)
{
    if (!OtherActor) return;
    if (bQuestAlreadyCompleted) return;
    if (CurrentPhase != EQuest_StampedePhase::Inactive) return;

    // Only trigger for player character
    ACharacter* PlayerChar = Cast<ACharacter>(OtherActor);
    if (!PlayerChar) return;

    APlayerController* PC = Cast<APlayerController>(PlayerChar->GetController());
    if (!PC) return;

    ActivateStampedeQuest(OtherActor);
}

void AQuestStampedeTrigger::ActivateStampedeQuest(AActor* PlayerActor)
{
    if (CurrentPhase != EQuest_StampedePhase::Inactive) return;

    CurrentPhase = EQuest_StampedePhase::Triggered;
    RemainingTime = EscapeTimeLimit;
    bTickActive = true;
    SetActorTickEnabled(true);

    UE_LOG(LogTemp, Warning, TEXT("=== QUEST ACTIVATED: Survive the Stampede ==="));
    UE_LOG(LogTemp, Warning, TEXT("Objective: Reach safe zone at (%.0f, %.0f, %.0f) within %.0f seconds"),
        SafeZoneLocation.X, SafeZoneLocation.Y, SafeZoneLocation.Z, EscapeTimeLimit);

    // Brief delay then transition to Escaping
    CurrentPhase = EQuest_StampedePhase::Escaping;

    UE_LOG(LogTemp, Warning, TEXT("Quest Phase: ESCAPING — Timer started: %.1f seconds"), RemainingTime);
}

void AQuestStampedeTrigger::CheckSafeZoneReached(AActor* PlayerActor)
{
    if (!PlayerActor) return;
    if (CurrentPhase != EQuest_StampedePhase::Escaping) return;

    if (IsPlayerInSafeZone(PlayerActor))
    {
        CompleteObjective(0); // Escape corridor
        CompleteObjective(2); // Reach high ground
        CompleteQuest();
    }
}

void AQuestStampedeTrigger::CompleteObjective(int32 ObjectiveIndex)
{
    if (!Objectives.IsValidIndex(ObjectiveIndex)) return;
    if (Objectives[ObjectiveIndex].bCompleted) return;

    Objectives[ObjectiveIndex].bCompleted = true;
    UE_LOG(LogTemp, Warning, TEXT("Objective COMPLETE: %s"),
        *Objectives[ObjectiveIndex].ObjectiveName);
}

void AQuestStampedeTrigger::FailQuest()
{
    CurrentPhase = EQuest_StampedePhase::Crushed;
    SetActorTickEnabled(false);

    UE_LOG(LogTemp, Error, TEXT("=== QUEST FAILED: Survive the Stampede — Player crushed by herd ==="));
}

void AQuestStampedeTrigger::CompleteQuest()
{
    CurrentPhase = EQuest_StampedePhase::Completed;
    bQuestAlreadyCompleted = true;
    SetActorTickEnabled(false);

    UE_LOG(LogTemp, Warning, TEXT("=== QUEST COMPLETE: Survive the Stampede ==="));
    UE_LOG(LogTemp, Warning, TEXT("Time remaining: %.1f seconds"), RemainingTime);
}

float AQuestStampedeTrigger::GetEscapeProgressPercent() const
{
    if (EscapeTimeLimit <= 0.0f) return 0.0f;
    float Elapsed = EscapeTimeLimit - RemainingTime;
    return FMath::Clamp(Elapsed / EscapeTimeLimit, 0.0f, 1.0f);
}

bool AQuestStampedeTrigger::IsPlayerInSafeZone(AActor* PlayerActor) const
{
    if (!PlayerActor) return false;
    float Dist = FVector::Dist(PlayerActor->GetActorLocation(), SafeZoneLocation);
    return Dist <= SafeZoneRadius;
}

void AQuestStampedeTrigger::ResetQuest()
{
    CurrentPhase = EQuest_StampedePhase::Inactive;
    bQuestAlreadyCompleted = false;
    RemainingTime = EscapeTimeLimit;
    SetActorTickEnabled(false);
    InitialiseObjectives();
    UE_LOG(LogTemp, Warning, TEXT("Quest RESET: Survive the Stampede"));
}

// ============================================================
// AQuestSafeZoneMarker — Implementation
// ============================================================

AQuestSafeZoneMarker::AQuestSafeZoneMarker()
{
    PrimaryActorTick.bCanEverTick = false;

    MarkerMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MarkerMesh"));
    RootComponent = MarkerMesh;

    SafeVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("SafeVolume"));
    SafeVolume->SetupAttachment(RootComponent);
    SafeVolume->SetBoxExtent(FVector(500.0f, 500.0f, 300.0f));
    SafeVolume->SetCollisionProfileName(TEXT("Trigger"));

    bIsActive = false;
}

void AQuestSafeZoneMarker::BeginPlay()
{
    Super::BeginPlay();
    SafeVolume->OnComponentBeginOverlap.AddDynamic(this, &AQuestSafeZoneMarker::OnSafeZoneOverlap);
    SetActorHiddenInGame(!bIsActive);
}

void AQuestSafeZoneMarker::ActivateMarker()
{
    bIsActive = true;
    SetActorHiddenInGame(false);
    UE_LOG(LogTemp, Warning, TEXT("SafeZoneMarker ACTIVATED at %s"), *GetActorLocation().ToString());
}

void AQuestSafeZoneMarker::DeactivateMarker()
{
    bIsActive = false;
    SetActorHiddenInGame(true);
}

void AQuestSafeZoneMarker::OnSafeZoneOverlap(
    UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
    bool bFromSweep, const FHitResult& SweepResult)
{
    if (!OtherActor || !bIsActive) return;

    ACharacter* PlayerChar = Cast<ACharacter>(OtherActor);
    if (!PlayerChar) return;

    UE_LOG(LogTemp, Warning, TEXT("Player reached SafeZone! Quest completion triggered."));
}
