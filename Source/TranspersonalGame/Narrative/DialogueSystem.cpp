#include "DialogueSystem.h"
#include "GameFramework/Character.h"
#include "Components/SphereComponent.h"
#include "Engine/World.h"

// ─────────────────────────────────────────────────────────────────────────────
// ANarr_DialogueTrigger
// ─────────────────────────────────────────────────────────────────────────────

ANarr_DialogueTrigger::ANarr_DialogueTrigger()
{
    PrimaryActorTick.bCanEverTick = false;

    TriggerSphere = CreateDefaultSubobject<USphereComponent>(TEXT("TriggerSphere"));
    TriggerSphere->SetSphereRadius(TriggerRadius);
    TriggerSphere->SetCollisionProfileName(TEXT("Trigger"));
    RootComponent = TriggerSphere;

    TriggerSphere->OnComponentBeginOverlap.AddDynamic(
        this, &ANarr_DialogueTrigger::OnPlayerEnterRange);
}

void ANarr_DialogueTrigger::BeginPlay()
{
    Super::BeginPlay();

    // Update sphere radius from property
    if (TriggerSphere)
    {
        TriggerSphere->SetSphereRadius(TriggerRadius);
    }

    // Register with world dialogue manager
    if (UWorld* World = GetWorld())
    {
        if (UNarr_DialogueManager* Manager = World->GetSubsystem<UNarr_DialogueManager>())
        {
            Manager->RegisterTrigger(this);
        }
    }
}

void ANarr_DialogueTrigger::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void ANarr_DialogueTrigger::OnPlayerEnterRange(
    UPrimitiveComponent* OverlappedComp,
    AActor* OtherActor,
    UPrimitiveComponent* OtherComp,
    int32 OtherBodyIndex,
    bool bFromSweep,
    const FHitResult& SweepResult)
{
    if (!OtherActor) return;

    // Only trigger for player character
    ACharacter* PlayerChar = Cast<ACharacter>(OtherActor);
    if (!PlayerChar) return;

    if (bOneShot && bHasTriggered) return;

    TriggerDialogue(OtherActor);
}

void ANarr_DialogueTrigger::TriggerDialogue(AActor* PlayerActor)
{
    if (!PlayerActor) return;
    if (NPCProfile.DialogueLines.Num() == 0) return;

    bHasTriggered = true;

    FNarr_DialogueLine& Line = NPCProfile.DialogueLines[CurrentLineIndex];
    UE_LOG(LogTemp, Log, TEXT("[Dialogue] %s: \"%s\""),
        *NPCProfile.Name, *Line.LineText);
    UE_LOG(LogTemp, Log, TEXT("[Dialogue] Audio: %s"), *Line.AudioURL);

    if (!bOneShot)
    {
        CurrentLineIndex = (CurrentLineIndex + 1) % NPCProfile.DialogueLines.Num();
    }
}

void ANarr_DialogueTrigger::RegisterDialogueLine(const FNarr_DialogueLine& Line)
{
    NPCProfile.DialogueLines.Add(Line);
}

bool ANarr_DialogueTrigger::HasUnplayedLines() const
{
    if (bOneShot) return !bHasTriggered;
    return NPCProfile.DialogueLines.Num() > 0;
}

FNarr_DialogueLine ANarr_DialogueTrigger::GetNextLine()
{
    if (NPCProfile.DialogueLines.Num() == 0)
    {
        return FNarr_DialogueLine();
    }
    int32 Idx = FMath::Clamp(CurrentLineIndex, 0, NPCProfile.DialogueLines.Num() - 1);
    return NPCProfile.DialogueLines[Idx];
}

// ─────────────────────────────────────────────────────────────────────────────
// UNarr_DialogueManager
// ─────────────────────────────────────────────────────────────────────────────

void UNarr_DialogueManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    PopulateDefaultRegistry();
    UE_LOG(LogTemp, Log, TEXT("[DialogueManager] Initialized — %d voice lines registered"),
        VoiceLineRegistry.Num());
}

void UNarr_DialogueManager::Deinitialize()
{
    RegisteredTriggers.Empty();
    VoiceLineRegistry.Empty();
    Super::Deinitialize();
}

void UNarr_DialogueManager::RegisterTrigger(ANarr_DialogueTrigger* Trigger)
{
    if (!Trigger) return;
    RegisteredTriggers.AddUnique(Trigger);
    UE_LOG(LogTemp, Log, TEXT("[DialogueManager] Registered trigger: %s"),
        *Trigger->GetActorLabel());
}

void UNarr_DialogueManager::UnregisterTrigger(ANarr_DialogueTrigger* Trigger)
{
    if (!Trigger) return;
    RegisteredTriggers.Remove(Trigger);
}

int32 UNarr_DialogueManager::GetActiveTriggerCount() const
{
    return RegisteredTriggers.Num();
}

void UNarr_DialogueManager::LoadVoiceLineRegistry()
{
    PopulateDefaultRegistry();
}

void UNarr_DialogueManager::PopulateDefaultRegistry()
{
    // Cycle PROD_CYCLE_AUTO_20260630_001 — 4 voice lines generated via ElevenLabs TTS
    VoiceLineRegistry.Add(
        TEXT("Kael_Hunter"),
        TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1782781107555_Kael_Hunter.mp3")
    );
    VoiceLineRegistry.Add(
        TEXT("Mara_Scout"),
        TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1782781123869_Mara_Scout.mp3")
    );
    VoiceLineRegistry.Add(
        TEXT("Dara_Elder"),
        TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1782781130765_Dara_Elder.mp3")
    );
    VoiceLineRegistry.Add(
        TEXT("Bron_Tracker"),
        TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1782781132919_Bron_Tracker.mp3")
    );
}
