#include "Narr_DialogueSystem.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"

UNarr_DialogueSystem::UNarr_DialogueSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = UpdateInterval;

    // Criar componente de áudio
    AudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("DialogueAudioComponent"));
    
    // Configurações padrão
    UpdateInterval = 1.0f;
    MaxDialogueDistance = 2000.0f;
    bEnableContextualDialogue = true;
    bIsPlayingDialogue = false;
    DialogueTimer = 0.0f;
    CurrentContext = ENarr_NarrativeContext::Safe;
    CurrentZone = TEXT("");
}

void UNarr_DialogueSystem::BeginPlay()
{
    Super::BeginPlay();

    // Inicializar zonas narrativas padrão
    FNarr_NarrativeZone TutorialZone;
    TutorialZone.ZoneName = TEXT("Tutorial");
    TutorialZone.Context = ENarr_NarrativeContext::Safe;
    TutorialZone.ZoneCenter = FVector(0, 0, 0);
    TutorialZone.ZoneRadius = 800.0f;
    
    FNarr_DialogueEntry TutorialDialogue;
    TutorialDialogue.DialogueType = ENarr_DialogueType::Tutorial;
    TutorialDialogue.DialogueText = TEXT("Welcome to the Cretaceous period. Your survival depends on understanding this prehistoric world.");
    TutorialDialogue.Duration = 8.0f;
    TutorialDialogue.Priority = 10;
    TutorialDialogue.bCanRepeat = false;
    TutorialDialogue.CooldownTime = 0.0f;
    
    TutorialZone.AvailableDialogue.Add(TutorialDialogue);
    NarrativeZones.Add(TutorialZone);

    // Zona de perigo com dinossauros
    FNarr_NarrativeZone DangerZone;
    DangerZone.ZoneName = TEXT("DinosaurTerritory");
    DangerZone.Context = ENarr_NarrativeContext::Danger;
    DangerZone.ZoneCenter = FVector(2000, 1000, 0);
    DangerZone.ZoneRadius = 1200.0f;
    
    FNarr_DialogueEntry DangerDialogue;
    DangerDialogue.DialogueType = ENarr_DialogueType::Warning;
    DangerDialogue.DialogueText = TEXT("Large predator detected in your vicinity. Maintain distance and avoid sudden movements.");
    DangerDialogue.Duration = 6.0f;
    DangerDialogue.Priority = 8;
    DangerDialogue.bCanRepeat = true;
    DangerDialogue.CooldownTime = 45.0f;
    
    DangerZone.AvailableDialogue.Add(DangerDialogue);
    NarrativeZones.Add(DangerZone);

    // Zona de descoberta
    FNarr_NarrativeZone DiscoveryZone;
    DiscoveryZone.ZoneName = TEXT("ResourceArea");
    DiscoveryZone.Context = ENarr_NarrativeContext::Discovery;
    DiscoveryZone.ZoneCenter = FVector(-1500, 2000, 0);
    DiscoveryZone.ZoneRadius = 900.0f;
    
    FNarr_DialogueEntry DiscoveryDialogue;
    DiscoveryDialogue.DialogueType = ENarr_DialogueType::Discovery;
    DiscoveryDialogue.DialogueText = TEXT("Interesting geological formations detected. This area may contain valuable resources.");
    DiscoveryDialogue.Duration = 7.0f;
    DiscoveryDialogue.Priority = 5;
    DiscoveryDialogue.bCanRepeat = true;
    DiscoveryDialogue.CooldownTime = 60.0f;
    
    DiscoveryZone.AvailableDialogue.Add(DiscoveryDialogue);
    NarrativeZones.Add(DiscoveryZone);

    UE_LOG(LogTemp, Warning, TEXT("Narrative Dialogue System initialized with %d zones"), NarrativeZones.Num());
}

void UNarr_DialogueSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (!bEnableContextualDialogue)
        return;

    // Actualizar timer do diálogo actual
    if (bIsPlayingDialogue)
    {
        DialogueTimer += DeltaTime;
        if (DialogueTimer >= CurrentDialogue.Duration)
        {
            StopCurrentDialogue();
        }
    }

    // Actualizar contexto narrativo
    LastUpdateTime += DeltaTime;
    if (LastUpdateTime >= UpdateInterval)
    {
        UpdateNarrativeContext();
        ProcessDialogueQueue();
        LastUpdateTime = 0.0f;
    }

    // Actualizar cooldowns
    for (auto& Cooldown : DialogueCooldowns)
    {
        Cooldown.Value -= DeltaTime;
    }
}

void UNarr_DialogueSystem::UpdateNarrativeContext()
{
    UWorld* World = GetWorld();
    if (!World)
        return;

    APlayerController* PlayerController = World->GetFirstPlayerController();
    if (!PlayerController || !PlayerController->GetPawn())
        return;

    FVector PlayerLocation = PlayerController->GetPawn()->GetActorLocation();
    FString ZoneName;
    
    if (IsInNarrativeZone(PlayerLocation, ZoneName))
    {
        // Encontrar a zona correspondente
        for (const FNarr_NarrativeZone& Zone : NarrativeZones)
        {
            if (Zone.ZoneName == ZoneName && Zone.bIsActive)
            {
                if (CurrentZone != ZoneName)
                {
                    CurrentZone = ZoneName;
                    CurrentContext = Zone.Context;
                    UE_LOG(LogTemp, Log, TEXT("Player entered narrative zone: %s"), *ZoneName);
                }
                break;
            }
        }
    }
    else
    {
        if (!CurrentZone.IsEmpty())
        {
            CurrentZone = TEXT("");
            CurrentContext = ENarr_NarrativeContext::Safe;
            UE_LOG(LogTemp, Log, TEXT("Player left narrative zones"));
        }
    }
}

void UNarr_DialogueSystem::ProcessDialogueQueue()
{
    if (bIsPlayingDialogue || CurrentZone.IsEmpty())
        return;

    // Encontrar a zona actual
    for (const FNarr_NarrativeZone& Zone : NarrativeZones)
    {
        if (Zone.ZoneName == CurrentZone && Zone.bIsActive)
        {
            FNarr_DialogueEntry BestDialogue = SelectBestDialogue(Zone);
            
            if (!BestDialogue.DialogueText.IsEmpty() && ShouldPlayDialogue(BestDialogue))
            {
                TriggerDialogue(BestDialogue.DialogueType, Zone.ZoneName);
                CurrentDialogue = BestDialogue;
            }
            break;
        }
    }
}

FNarr_DialogueEntry UNarr_DialogueSystem::SelectBestDialogue(const FNarr_NarrativeZone& Zone)
{
    FNarr_DialogueEntry BestDialogue;
    int32 HighestPriority = -1;

    for (const FNarr_DialogueEntry& Dialogue : Zone.AvailableDialogue)
    {
        if (ShouldPlayDialogue(Dialogue) && Dialogue.Priority > HighestPriority)
        {
            BestDialogue = Dialogue;
            HighestPriority = Dialogue.Priority;
        }
    }

    return BestDialogue;
}

bool UNarr_DialogueSystem::ShouldPlayDialogue(const FNarr_DialogueEntry& Dialogue)
{
    // Verificar se não está em cooldown
    FString DialogueKey = CurrentZone + TEXT("_") + UEnum::GetValueAsString(Dialogue.DialogueType);
    
    if (DialogueCooldowns.Contains(DialogueKey))
    {
        if (DialogueCooldowns[DialogueKey] > 0.0f)
        {
            return false;
        }
    }

    // Verificar se pode repetir
    if (!Dialogue.bCanRepeat && DialogueCooldowns.Contains(DialogueKey))
    {
        return false;
    }

    return true;
}

void UNarr_DialogueSystem::TriggerDialogue(ENarr_DialogueType DialogueType, const FString& ZoneName)
{
    if (bIsPlayingDialogue)
    {
        StopCurrentDialogue();
    }

    bIsPlayingDialogue = true;
    DialogueTimer = 0.0f;

    // Reproduzir áudio se disponível
    if (AudioComponent && CurrentDialogue.AudioClip.IsValid())
    {
        USoundCue* SoundCue = CurrentDialogue.AudioClip.LoadSynchronous();
        if (SoundCue)
        {
            AudioComponent->SetSound(SoundCue);
            AudioComponent->Play();
        }
    }

    // Adicionar cooldown
    FString DialogueKey = ZoneName + TEXT("_") + UEnum::GetValueAsString(DialogueType);
    DialogueCooldowns.Add(DialogueKey, CurrentDialogue.CooldownTime);

    UE_LOG(LogTemp, Warning, TEXT("Triggered dialogue: %s in zone %s"), 
           *UEnum::GetValueAsString(DialogueType), *ZoneName);
}

void UNarr_DialogueSystem::StopCurrentDialogue()
{
    bIsPlayingDialogue = false;
    DialogueTimer = 0.0f;
    
    if (AudioComponent && AudioComponent->IsPlaying())
    {
        AudioComponent->Stop();
    }

    CurrentDialogue = FNarr_DialogueEntry();
}

bool UNarr_DialogueSystem::IsInNarrativeZone(const FVector& PlayerLocation, FString& OutZoneName)
{
    for (const FNarr_NarrativeZone& Zone : NarrativeZones)
    {
        if (!Zone.bIsActive)
            continue;

        float Distance = FVector::Dist(PlayerLocation, Zone.ZoneCenter);
        if (Distance <= Zone.ZoneRadius)
        {
            OutZoneName = Zone.ZoneName;
            return true;
        }
    }

    OutZoneName = TEXT("");
    return false;
}

void UNarr_DialogueSystem::AddNarrativeZone(const FNarr_NarrativeZone& NewZone)
{
    NarrativeZones.Add(NewZone);
    UE_LOG(LogTemp, Log, TEXT("Added narrative zone: %s"), *NewZone.ZoneName);
}

void UNarr_DialogueSystem::RemoveNarrativeZone(const FString& ZoneName)
{
    NarrativeZones.RemoveAll([ZoneName](const FNarr_NarrativeZone& Zone)
    {
        return Zone.ZoneName == ZoneName;
    });
    UE_LOG(LogTemp, Log, TEXT("Removed narrative zone: %s"), *ZoneName);
}

ENarr_NarrativeContext UNarr_DialogueSystem::GetCurrentContext() const
{
    return CurrentContext;
}