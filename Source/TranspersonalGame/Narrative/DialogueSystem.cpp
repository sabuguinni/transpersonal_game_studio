// DialogueSystem.cpp
// Agent #15 — Narrative & Dialogue
// Cycle: PROD_CYCLE_AUTO_20260628_009

#include "DialogueSystem.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "UObject/ConstructorHelpers.h"
#include "Kismet/GameplayStatics.h"

// ─── ANarr_DialogueTriggerVolume ──────────────────────────────────────────────

ANarr_DialogueTriggerVolume::ANarr_DialogueTriggerVolume()
{
    PrimaryActorTick.bCanEverTick = false;
    CurrentLineIndex = 0;

    TriggerSphere = CreateDefaultSubobject<USphereComponent>(TEXT("TriggerSphere"));
    TriggerSphere->SetSphereRadius(300.0f);
    TriggerSphere->SetCollisionProfileName(TEXT("Trigger"));
    RootComponent = TriggerSphere;

    DebugMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DebugMesh"));
    DebugMesh->SetupAttachment(RootComponent);
    DebugMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMesh(
        TEXT("/Engine/BasicShapes/Sphere.Sphere"));
    if (SphereMesh.Succeeded())
    {
        DebugMesh->SetStaticMesh(SphereMesh.Object);
        DebugMesh->SetWorldScale3D(FVector(0.15f));
    }
}

void ANarr_DialogueTriggerVolume::BeginPlay()
{
    Super::BeginPlay();
    TriggerSphere->SetSphereRadius(TriggerRadius);
    TriggerSphere->OnComponentBeginOverlap.AddDynamic(
        this, &ANarr_DialogueTriggerVolume::OnSphereOverlap);
}

void ANarr_DialogueTriggerVolume::OnSphereOverlap(
    UPrimitiveComponent* OverlappedComp,
    AActor* OtherActor,
    UPrimitiveComponent* OtherComp,
    int32 OtherBodyIndex,
    bool bFromSweep,
    const FHitResult& SweepResult)
{
    if (!OtherActor || OtherActor == this) return;

    // Only trigger for the player pawn
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (OtherActor != PlayerPawn) return;

    if (TriggerType == ENarr_DialogueTrigger::Proximity)
    {
        TriggerDialogue(OtherActor);
    }
}

void ANarr_DialogueTriggerVolume::TriggerDialogue(AActor* Instigator)
{
    if (DialogueLines.Num() == 0) return;

    if (CurrentLineIndex >= DialogueLines.Num())
    {
        if (bTriggered) return; // All lines played, already triggered
        CurrentLineIndex = 0;
    }

    FNarr_DialogueLine& Line = DialogueLines[CurrentLineIndex];

    if (Line.bPlayOnce && Line.bHasBeenPlayed) 
    {
        CurrentLineIndex++;
        return;
    }

    // Log dialogue to screen (in editor, this shows as on-screen debug message)
    UE_LOG(LogTemp, Log, TEXT("[DIALOGUE] %s: %s"), *Line.SpeakerName, *Line.DialogueText);

    Line.bHasBeenPlayed = true;
    bTriggered = true;
    CurrentLineIndex++;
}

void ANarr_DialogueTriggerVolume::ResetTrigger()
{
    bTriggered = false;
    CurrentLineIndex = 0;
    for (FNarr_DialogueLine& Line : DialogueLines)
    {
        Line.bHasBeenPlayed = false;
    }
}

int32 ANarr_DialogueTriggerVolume::GetUnplayedLineCount() const
{
    int32 Count = 0;
    for (const FNarr_DialogueLine& Line : DialogueLines)
    {
        if (!Line.bHasBeenPlayed) Count++;
    }
    return Count;
}

// ─── ANarr_LoreStone ─────────────────────────────────────────────────────────

ANarr_LoreStone::ANarr_LoreStone()
{
    PrimaryActorTick.bCanEverTick = false;

    StoneMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StoneMesh"));
    RootComponent = StoneMesh;

    static ConstructorHelpers::FObjectFinder<UStaticMesh> RockMesh(
        TEXT("/Engine/BasicShapes/Cube.Cube"));
    if (RockMesh.Succeeded())
    {
        StoneMesh->SetStaticMesh(RockMesh.Object);
        StoneMesh->SetWorldScale3D(FVector(0.4f, 0.25f, 0.5f));
    }

    InteractSphere = CreateDefaultSubobject<USphereComponent>(TEXT("InteractSphere"));
    InteractSphere->SetupAttachment(RootComponent);
    InteractSphere->SetSphereRadius(150.0f);
    InteractSphere->SetCollisionProfileName(TEXT("Trigger"));
}

void ANarr_LoreStone::BeginPlay()
{
    Super::BeginPlay();
    InteractSphere->OnComponentBeginOverlap.AddDynamic(
        this, &ANarr_LoreStone::OnInteractOverlap);
}

void ANarr_LoreStone::OnInteractOverlap(
    UPrimitiveComponent* OverlappedComp,
    AActor* OtherActor,
    UPrimitiveComponent* OtherComp,
    int32 OtherBodyIndex,
    bool bFromSweep,
    const FHitResult& SweepResult)
{
    if (!OtherActor || OtherActor == this) return;

    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (OtherActor != PlayerPawn) return;

    ReadLore(OtherActor);
}

void ANarr_LoreStone::ReadLore(AActor* Reader)
{
    if (bHasBeenRead && LoreData.bDiscovered) return;

    LoreData.bDiscovered = true;
    bHasBeenRead = true;

    UE_LOG(LogTemp, Log, TEXT("[LORE] Discovered: %s — %s"),
           *LoreData.Title, *LoreData.BodyText);
}

FString ANarr_LoreStone::GetLoreText() const
{
    return FString::Printf(TEXT("[%s] %s: %s"),
        *LoreData.FragmentID, *LoreData.Title, *LoreData.BodyText);
}

// ─── ANarr_DialogueManager ───────────────────────────────────────────────────

ANarr_DialogueManager* ANarr_DialogueManager::Instance = nullptr;

ANarr_DialogueManager::ANarr_DialogueManager()
{
    PrimaryActorTick.bCanEverTick = false;
}

void ANarr_DialogueManager::BeginPlay()
{
    Super::BeginPlay();
    Instance = this;
    UE_LOG(LogTemp, Log, TEXT("[DialogueManager] Initialized. Ready to track lore and dialogue."));
}

void ANarr_DialogueManager::RegisterLoreDiscovery(const FNarr_LoreFragment& Fragment)
{
    // Check if already registered
    for (const FNarr_LoreFragment& Existing : DiscoveredLore)
    {
        if (Existing.FragmentID == Fragment.FragmentID) return;
    }
    DiscoveredLore.Add(Fragment);
    UE_LOG(LogTemp, Log, TEXT("[DialogueManager] Lore registered: %s"), *Fragment.FragmentID);
}

bool ANarr_DialogueManager::HasLoreBeenDiscovered(const FString& FragmentID) const
{
    for (const FNarr_LoreFragment& Fragment : DiscoveredLore)
    {
        if (Fragment.FragmentID == FragmentID) return true;
    }
    return false;
}

int32 ANarr_DialogueManager::GetTotalLoreCount() const
{
    // Total lore in the world — hardcoded to match placed LoreStones
    return 12;
}

int32 ANarr_DialogueManager::GetDiscoveredLoreCount() const
{
    return DiscoveredLore.Num();
}

void ANarr_DialogueManager::LogDialoguePlayed(const FString& DialogueID)
{
    if (!PlayedDialogueIDs.Contains(DialogueID))
    {
        PlayedDialogueIDs.Add(DialogueID);
    }
}

bool ANarr_DialogueManager::WasDialoguePlayed(const FString& DialogueID) const
{
    return PlayedDialogueIDs.Contains(DialogueID);
}
