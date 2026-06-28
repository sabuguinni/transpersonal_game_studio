#include "AudioSystemManager.h"
#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "TimerManager.h"

// ============================================================
// UAudio_AmbientZoneComponent — Implementation
// ============================================================

UAudio_AmbientZoneComponent::UAudio_AmbientZoneComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.5f; // Check every 0.5s — not every frame
}

void UAudio_AmbientZoneComponent::BeginPlay()
{
    Super::BeginPlay();
    // Initialize audio components for each ambient layer
    for (const FAudio_AmbientLayer& Layer : AmbientLayers)
    {
        if (Layer.SoundAsset.IsValid())
        {
            UAudioComponent* AudioComp = NewObject<UAudioComponent>(GetOwner());
            if (AudioComp)
            {
                AudioComp->RegisterComponent();
                AudioComp->SetSound(Layer.SoundAsset.Get());
                AudioComp->SetVolumeMultiplier(Layer.BaseVolume);
                AudioComp->bAutoActivate = false;
                ActiveAudioComponents.Add(AudioComp);
            }
        }
    }
}

void UAudio_AmbientZoneComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (bTransitioning)
    {
        ThreatTransitionTimer -= DeltaTime;
        if (ThreatTransitionTimer <= 0.0f)
        {
            bTransitioning = false;
            ThreatTransitionTimer = 0.0f;
        }
    }
}

void UAudio_AmbientZoneComponent::SetThreatLevel(EAudio_ThreatLevel NewLevel)
{
    if (NewLevel == CurrentThreatLevel) return;

    EAudio_ThreatLevel OldLevel = CurrentThreatLevel;
    CurrentThreatLevel = NewLevel;
    bTransitioning = true;
    ThreatTransitionTimer = 2.0f;

    // Volume multiplier based on threat — danger suppresses ambient, heightens tension
    float VolumeScale = 1.0f;
    switch (NewLevel)
    {
        case EAudio_ThreatLevel::Safe:     VolumeScale = 1.0f;  break;
        case EAudio_ThreatLevel::Cautious: VolumeScale = 0.7f;  break;
        case EAudio_ThreatLevel::Danger:   VolumeScale = 0.3f;  break;
        case EAudio_ThreatLevel::Critical: VolumeScale = 0.0f;  break; // Silence = maximum tension
    }

    for (UAudioComponent* Comp : ActiveAudioComponents)
    {
        if (Comp && Comp->IsActive())
        {
            Comp->SetVolumeMultiplier(VolumeScale);
        }
    }
}

void UAudio_AmbientZoneComponent::FadeToZone(EAudio_AmbientZone NewZone, float FadeTime)
{
    ZoneType = NewZone;
    // Fade out current, fade in new — handled by volume interpolation in Tick
    for (UAudioComponent* Comp : ActiveAudioComponents)
    {
        if (Comp)
        {
            Comp->FadeOut(FadeTime, 0.0f);
        }
    }
    ActiveAudioComponents.Empty();
}

FString UAudio_AmbientZoneComponent::GetZoneName() const
{
    switch (ZoneType)
    {
        case EAudio_AmbientZone::Camp:       return TEXT("Camp");
        case EAudio_AmbientZone::Forest:     return TEXT("Forest");
        case EAudio_AmbientZone::River:      return TEXT("River");
        case EAudio_AmbientZone::OpenPlain:  return TEXT("Open Plain");
        case EAudio_AmbientZone::Cave:       return TEXT("Cave");
        case EAudio_AmbientZone::DangerZone: return TEXT("Danger Zone");
        default:                             return TEXT("Unknown");
    }
}

// ============================================================
// AAudio_AmbientSourceActor — Implementation
// ============================================================

AAudio_AmbientSourceActor::AAudio_AmbientSourceActor()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.1f;

    AmbientZoneComp = CreateDefaultSubobject<UAudio_AmbientZoneComponent>(TEXT("AmbientZoneComp"));
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComp"));

    PrimaryAudioComp = CreateDefaultSubobject<UAudioComponent>(TEXT("PrimaryAudioComp"));
    PrimaryAudioComp->SetupAttachment(RootComponent);
    PrimaryAudioComp->bAutoActivate = false;
}

void AAudio_AmbientSourceActor::BeginPlay()
{
    Super::BeginPlay();
    PopulateDefaultDialogueEntries();
}

void AAudio_AmbientSourceActor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (bDialoguePlaying)
    {
        DialogueTimer -= DeltaTime;
        if (DialogueTimer <= 0.0f)
        {
            bDialoguePlaying = false;
            DialogueTimer = 0.0f;
        }
    }
}

void AAudio_AmbientSourceActor::PopulateDefaultDialogueEntries()
{
    DialogueEntries.Empty();

    // Wire all TTS-generated voice lines from cycles 012 and 013
    TArray<TTuple<FString, FString, FString, float>> VoiceData = {
        MakeTuple(
            FString(TEXT("ElderTracker")),
            FString(TEXT("Fresh tracks — three-toed, deep. T-Rex, this morning.")),
            ElderTrackerURL,
            8.0f
        ),
        MakeTuple(
            FString(TEXT("ChiefHunter")),
            FString(TEXT("We lost Darak at the river crossing. The Spinosaurus came from beneath.")),
            ChiefHunterURL,
            10.0f
        ),
        MakeTuple(
            FString(TEXT("Craftmaster")),
            FString(TEXT("Craft the spear tip first. Obsidian from the black ridge.")),
            CraftmasterURL,
            9.0f
        ),
        MakeTuple(
            FString(TEXT("ScoutRanger")),
            FString(TEXT("The herd moved south three days ago. Two hundred Parasaurolophus.")),
            ScoutRangerURL,
            11.0f
        ),
        MakeTuple(
            FString(TEXT("ScoutWarning")),
            FString(TEXT("Raptors hunt in packs along the water's edge at dusk. Freeze when you hear clicking.")),
            ScoutWarningURL,
            20.0f
        ),
        MakeTuple(
            FString(TEXT("ElderSurvivor")),
            FString(TEXT("Keep the fire low. High flame draws predators from three ridges away.")),
            ElderSurvivorURL,
            16.0f
        ),
    };

    for (const auto& Entry : VoiceData)
    {
        FAudio_DialogueEntry DE;
        DE.CharacterName = Entry.Get<0>();
        DE.DialogueText  = Entry.Get<1>();
        DE.AudioURL      = Entry.Get<2>();
        DE.Duration      = Entry.Get<3>();
        DE.bHasBeenPlayed = false;
        DialogueEntries.Add(DE);
    }
}

void AAudio_AmbientSourceActor::PlayDialogueEntry(int32 EntryIndex)
{
    if (!DialogueEntries.IsValidIndex(EntryIndex)) return;
    if (bDialoguePlaying) return;

    FAudio_DialogueEntry& Entry = DialogueEntries[EntryIndex];
    Entry.bHasBeenPlayed = true;
    bDialoguePlaying = true;
    DialogueTimer = Entry.Duration;
    CurrentDialogueIndex = EntryIndex;

    // If a bound audio asset exists, play it via AudioComponent
    if (Entry.AudioAsset.IsValid() && PrimaryAudioComp)
    {
        PrimaryAudioComp->SetSound(Entry.AudioAsset.Get());
        PrimaryAudioComp->Play();
    }
    // Otherwise the URL is available for Blueprint/UI to display subtitle + stream
}
