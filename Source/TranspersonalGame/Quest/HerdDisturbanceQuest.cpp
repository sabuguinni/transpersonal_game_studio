// HerdDisturbanceQuest.cpp — Agent #14 Quest & Mission Designer
// Full implementation: approach trigger → stampede → survive → collect resources → return to elder

#include "HerdDisturbanceQuest.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "GameFramework/Character.h"
#include "TimerManager.h"

// ─────────────────────────────────────────────────────────────────────────────
// AQuest_HerdDisturbanceManager
// ─────────────────────────────────────────────────────────────────────────────

AQuest_HerdDisturbanceManager::AQuest_HerdDisturbanceManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.1f;

    // Root
    USceneComponent* Root = CreateDefaultSubobject<USceneComponent>(TEXT("QuestRoot"));
    SetRootComponent(Root);

    // Approach trigger — 800m radius, player enters → quest activates
    ApproachTrigger = CreateDefaultSubobject<USphereComponent>(TEXT("ApproachTrigger"));
    ApproachTrigger->SetupAttachment(Root);
    ApproachTrigger->SetSphereRadius(800.0f);
    ApproachTrigger->SetCollisionProfileName(TEXT("Trigger"));
    ApproachTrigger->SetGenerateOverlapEvents(true);

    // Stampede kill zone — 300m radius, player inside during stampede → fail
    StampedeKillZone = CreateDefaultSubobject<USphereComponent>(TEXT("StampedeKillZone"));
    StampedeKillZone->SetupAttachment(Root);
    StampedeKillZone->SetSphereRadius(300.0f);
    StampedeKillZone->SetCollisionProfileName(TEXT("Trigger"));
    StampedeKillZone->SetGenerateOverlapEvents(true);

    // Quest marker mesh (visible pillar/beacon in world)
    QuestMarkerMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("QuestMarker"));
    QuestMarkerMesh->SetupAttachment(Root);
    QuestMarkerMesh->SetRelativeLocation(FVector(0.0f, 0.0f, 200.0f));
    QuestMarkerMesh->SetRelativeScale3D(FVector(0.5f, 0.5f, 4.0f));

    // Default config
    ApproachRadius = 800.0f;
    KillZoneRadius = 300.0f;
    StampedeDuration = 20.0f;
    StampedeTimeLimit = 20.0f;
    StampedeElapsed = 0.0f;
    bPlayerInKillZone = false;
    ResourcesCollected = 0;
    ResourcesRequired = 3;
    CurrentPhase = EQuest_HerdPhase::Inactive;
    HerdLocation = FVector(3000.0f, 1500.0f, 100.0f);

    // Default reward
    QuestReward.BoneCount = 3;
    QuestReward.HideCount = 2;
    QuestReward.MeatCount = 5;
    QuestReward.RewardDescription = TEXT("Bones, hide, and meat from the stampede aftermath.");
}

void AQuest_HerdDisturbanceManager::BeginPlay()
{
    Super::BeginPlay();

    // Bind overlap events
    ApproachTrigger->OnComponentBeginOverlap.AddDynamic(
        this, &AQuest_HerdDisturbanceManager::OnApproachOverlapBegin);

    StampedeKillZone->OnComponentBeginOverlap.AddDynamic(
        this, &AQuest_HerdDisturbanceManager::OnKillZoneOverlapBegin);

    StampedeKillZone->OnComponentEndOverlap.AddDynamic(
        this, &AQuest_HerdDisturbanceManager::OnKillZoneOverlapEnd);

    InitializeObjectives();

    UE_LOG(LogTemp, Log, TEXT("[HerdDisturbanceQuest] Initialized. Phase: Inactive. Approach radius: %.0f"),
        ApproachRadius);
}

void AQuest_HerdDisturbanceManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (CurrentPhase == EQuest_HerdPhase::StampedeActive)
    {
        TickStampede(DeltaTime);
    }

#if WITH_EDITOR
    // Debug visualization in editor
    if (CurrentPhase != EQuest_HerdPhase::Inactive && GetWorld())
    {
        DrawDebugSphere(GetWorld(), GetActorLocation(), ApproachRadius, 16,
            FColor::Yellow, false, 0.15f);
        DrawDebugSphere(GetWorld(), GetActorLocation(), KillZoneRadius, 12,
            FColor::Red, false, 0.15f);
    }
#endif
}

void AQuest_HerdDisturbanceManager::InitializeObjectives()
{
    Objectives.Empty();

    FQuest_HerdObjectiveState Obj1;
    Obj1.Objective = EQuest_HerdObjective::ApproachHerd;
    Obj1.bCompleted = false;
    Obj1.Description = TEXT("Approach the herbivore herd within 800 meters.");
    Objectives.Add(Obj1);

    FQuest_HerdObjectiveState Obj2;
    Obj2.Objective = EQuest_HerdObjective::SurviveStampede;
    Obj2.bCompleted = false;
    Obj2.Description = TEXT("Survive the stampede — get to high ground!");
    Objectives.Add(Obj2);

    FQuest_HerdObjectiveState Obj3;
    Obj3.Objective = EQuest_HerdObjective::CollectResources;
    Obj3.bCompleted = false;
    Obj3.Description = TEXT("Collect 3 resources from the stampede aftermath.");
    Objectives.Add(Obj3);

    FQuest_HerdObjectiveState Obj4;
    Obj4.Objective = EQuest_HerdObjective::ReturnToElder;
    Obj4.bCompleted = false;
    Obj4.Description = TEXT("Return to the Tribal Elder with your findings.");
    Objectives.Add(Obj4);
}

void AQuest_HerdDisturbanceManager::ActivateQuest()
{
    if (CurrentPhase != EQuest_HerdPhase::Inactive)
    {
        return;
    }
    SetPhase(EQuest_HerdPhase::WatchingHerd);
    UE_LOG(LogTemp, Log, TEXT("[HerdDisturbanceQuest] Quest ACTIVATED — watching herd."));
}

void AQuest_HerdDisturbanceManager::SetPhase(EQuest_HerdPhase NewPhase)
{
    if (CurrentPhase == NewPhase)
    {
        return;
    }
    CurrentPhase = NewPhase;
    OnHerdPhaseChanged.Broadcast(NewPhase);
    UE_LOG(LogTemp, Log, TEXT("[HerdDisturbanceQuest] Phase changed to: %d"), (int32)NewPhase);
}

void AQuest_HerdDisturbanceManager::CompleteObjective(EQuest_HerdObjective Objective)
{
    for (FQuest_HerdObjectiveState& Obj : Objectives)
    {
        if (Obj.Objective == Objective && !Obj.bCompleted)
        {
            Obj.bCompleted = true;
            OnObjectiveCompleted.Broadcast(Objective);
            UE_LOG(LogTemp, Log, TEXT("[HerdDisturbanceQuest] Objective completed: %s"),
                *Obj.Description);
            CheckQuestCompletion();
            return;
        }
    }
}

void AQuest_HerdDisturbanceManager::OnStampedeTriggeredCallback(FVector TriggerLocation, int32 AffectedAgents)
{
    if (CurrentPhase == EQuest_HerdPhase::WatchingHerd)
    {
        SetPhase(EQuest_HerdPhase::StampedeActive);
        StampedeElapsed = 0.0f;
        CompleteObjective(EQuest_HerdObjective::ApproachHerd);
        UE_LOG(LogTemp, Warning, TEXT("[HerdDisturbanceQuest] STAMPEDE TRIGGERED at %s — %d agents affected!"),
            *TriggerLocation.ToString(), AffectedAgents);
    }
}

void AQuest_HerdDisturbanceManager::OnResourceCollected()
{
    ResourcesCollected = FMath::Min(ResourcesCollected + 1, ResourcesRequired);
    UE_LOG(LogTemp, Log, TEXT("[HerdDisturbanceQuest] Resource collected: %d/%d"),
        ResourcesCollected, ResourcesRequired);

    if (ResourcesCollected >= ResourcesRequired)
    {
        CompleteObjective(EQuest_HerdObjective::CollectResources);
        SetPhase(EQuest_HerdPhase::EscapeWindow);
    }
}

void AQuest_HerdDisturbanceManager::CheckQuestCompletion()
{
    bool bAllDone = true;
    for (const FQuest_HerdObjectiveState& Obj : Objectives)
    {
        if (!Obj.bCompleted)
        {
            bAllDone = false;
            break;
        }
    }

    if (bAllDone)
    {
        SetPhase(EQuest_HerdPhase::Completed);
        OnHerdQuestCompleted.Broadcast();
        UE_LOG(LogTemp, Log, TEXT("[HerdDisturbanceQuest] QUEST COMPLETE! Reward: %d bones, %d hide, %d meat."),
            QuestReward.BoneCount, QuestReward.HideCount, QuestReward.MeatCount);
    }
}

bool AQuest_HerdDisturbanceManager::IsObjectiveComplete(EQuest_HerdObjective Objective) const
{
    for (const FQuest_HerdObjectiveState& Obj : Objectives)
    {
        if (Obj.Objective == Objective)
        {
            return Obj.bCompleted;
        }
    }
    return false;
}

FString AQuest_HerdDisturbanceManager::GetCurrentObjectiveText() const
{
    for (const FQuest_HerdObjectiveState& Obj : Objectives)
    {
        if (!Obj.bCompleted)
        {
            return Obj.Description;
        }
    }
    return TEXT("Quest Complete.");
}

float AQuest_HerdDisturbanceManager::GetStampedeProgress() const
{
    if (StampedeDuration <= 0.0f)
    {
        return 0.0f;
    }
    return FMath::Clamp(StampedeElapsed / StampedeDuration, 0.0f, 1.0f);
}

void AQuest_HerdDisturbanceManager::DebugLogQuestState() const
{
    UE_LOG(LogTemp, Log, TEXT("=== HerdDisturbanceQuest Debug ==="));
    UE_LOG(LogTemp, Log, TEXT("Phase: %d"), (int32)CurrentPhase);
    UE_LOG(LogTemp, Log, TEXT("Resources: %d/%d"), ResourcesCollected, ResourcesRequired);
    UE_LOG(LogTemp, Log, TEXT("Stampede elapsed: %.1f / %.1f"), StampedeElapsed, StampedeDuration);
    for (const FQuest_HerdObjectiveState& Obj : Objectives)
    {
        UE_LOG(LogTemp, Log, TEXT("  [%s] %s"),
            Obj.bCompleted ? TEXT("DONE") : TEXT("    "),
            *Obj.Description);
    }
    UE_LOG(LogTemp, Log, TEXT("=================================="));
}

void AQuest_HerdDisturbanceManager::TickStampede(float DeltaTime)
{
    StampedeElapsed += DeltaTime;

    // Player in kill zone during stampede → fail
    if (bPlayerInKillZone)
    {
        SetPhase(EQuest_HerdPhase::Failed);
        OnHerdQuestFailed.Broadcast();
        UE_LOG(LogTemp, Error, TEXT("[HerdDisturbanceQuest] FAILED — player caught in stampede kill zone!"));
        return;
    }

    // Stampede over — player survived
    if (StampedeElapsed >= StampedeDuration)
    {
        CompleteObjective(EQuest_HerdObjective::SurviveStampede);
        UE_LOG(LogTemp, Log, TEXT("[HerdDisturbanceQuest] Stampede ended — player survived!"));
    }
}

void AQuest_HerdDisturbanceManager::OnApproachOverlapBegin(
    UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
    bool bFromSweep, const FHitResult& SweepResult)
{
    if (!OtherActor || OtherActor == this)
    {
        return;
    }

    // Check if it's a player character
    if (OtherActor->IsA(ACharacter::StaticClass()))
    {
        if (CurrentPhase == EQuest_HerdPhase::Inactive)
        {
            ActivateQuest();
        }
        else if (CurrentPhase == EQuest_HerdPhase::WatchingHerd)
        {
            // Player close enough — trigger stampede
            OnStampedeTriggeredCallback(GetActorLocation(), 10);
        }
    }
}

void AQuest_HerdDisturbanceManager::OnKillZoneOverlapBegin(
    UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
    bool bFromSweep, const FHitResult& SweepResult)
{
    if (OtherActor && OtherActor->IsA(ACharacter::StaticClass()))
    {
        bPlayerInKillZone = true;
        UE_LOG(LogTemp, Warning, TEXT("[HerdDisturbanceQuest] Player entered stampede kill zone!"));
    }
}

void AQuest_HerdDisturbanceManager::OnKillZoneOverlapEnd(
    UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    if (OtherActor && OtherActor->IsA(ACharacter::StaticClass()))
    {
        bPlayerInKillZone = false;
        UE_LOG(LogTemp, Log, TEXT("[HerdDisturbanceQuest] Player escaped stampede kill zone!"));
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// AQuest_TribalElderNPC
// ─────────────────────────────────────────────────────────────────────────────

AQuest_TribalElderNPC::AQuest_TribalElderNPC()
{
    PrimaryActorTick.bCanEverTick = false;

    USceneComponent* Root = CreateDefaultSubobject<USceneComponent>(TEXT("ElderRoot"));
    SetRootComponent(Root);

    ElderMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ElderMesh"));
    ElderMesh->SetupAttachment(Root);
    ElderMesh->SetRelativeScale3D(FVector(1.0f, 1.0f, 2.0f));

    InteractionRadius = CreateDefaultSubobject<USphereComponent>(TEXT("InteractionRadius"));
    InteractionRadius->SetupAttachment(Root);
    InteractionRadius->SetSphereRadius(200.0f);
    InteractionRadius->SetCollisionProfileName(TEXT("Trigger"));
    InteractionRadius->SetGenerateOverlapEvents(true);

    GreetingLine = TEXT("The herd moves at dawn. Approach carefully — they are easily startled.");
    QuestOfferLine = TEXT("Careful. The herd is moving. If they stampede, get to high ground — fast.");
    QuestCompleteLine = TEXT("You did it. The herd passed without casualties. Collect what they left behind.");

    bQuestOffered = false;
    bQuestComplete = false;
}

void AQuest_TribalElderNPC::BeginPlay()
{
    Super::BeginPlay();

    InteractionRadius->OnComponentBeginOverlap.AddDynamic(
        this, &AQuest_TribalElderNPC::OnInteractionOverlapBegin);

    UE_LOG(LogTemp, Log, TEXT("[TribalElderNPC] Elder ready at %s"), *GetActorLocation().ToString());
}

void AQuest_TribalElderNPC::InteractWithPlayer(AActor* Player)
{
    if (!Player)
    {
        return;
    }

    FString Dialogue = GetCurrentDialogue();
    UE_LOG(LogTemp, Log, TEXT("[TribalElderNPC] '%s'"), *Dialogue);

    if (!bQuestOffered && LinkedQuest.IsValid())
    {
        LinkedQuest->ActivateQuest();
        bQuestOffered = true;
    }
    else if (bQuestComplete)
    {
        LinkedQuest->CompleteObjective(EQuest_HerdObjective::ReturnToElder);
    }
}

FString AQuest_TribalElderNPC::GetCurrentDialogue() const
{
    if (bQuestComplete)
    {
        return QuestCompleteLine;
    }
    if (bQuestOffered)
    {
        return QuestOfferLine;
    }
    return GreetingLine;
}

void AQuest_TribalElderNPC::OnQuestCompleted()
{
    bQuestComplete = true;
    UE_LOG(LogTemp, Log, TEXT("[TribalElderNPC] Quest completed — elder acknowledges player."));
}

void AQuest_TribalElderNPC::OnInteractionOverlapBegin(
    UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
    bool bFromSweep, const FHitResult& SweepResult)
{
    if (OtherActor && OtherActor->IsA(ACharacter::StaticClass()))
    {
        InteractWithPlayer(OtherActor);
    }
}
