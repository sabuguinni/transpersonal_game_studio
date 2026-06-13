#include "Audio_MetaSoundManager.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "MetasoundSource.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"

UAudio_MetaSoundManager::UAudio_MetaSoundManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Update 10 times per second

    // Initialize default values
    CurrentBiome = EAudio_BiomeType::Forest;
    CurrentThreatLevel = EAudio_ThreatLevel::Safe;
    BiomeTransitionTimer = 0.0f;
    ThreatTransitionTimer = 0.0f;
    bIsTransitioning = false;

    // Initialize audio components as null - will be created in BeginPlay
    BiomeAudioComponent = nullptr;
    ThreatAudioComponent = nullptr;
    NarrativeAudioComponent = nullptr;
}

void UAudio_MetaSoundManager::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeAudioComponents();
    ConfigureBiomeAudio();
    ConfigureThreatAudio();

    UE_LOG(LogTemp, Log, TEXT("Audio_MetaSoundManager: Initialized with %d biome configs"), BiomeAudioConfigs.Num());
}

void UAudio_MetaSoundManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (bIsTransitioning)
    {
        UpdateBiomeTransition(DeltaTime);
        UpdateThreatTransition(DeltaTime);
    }
}

void UAudio_MetaSoundManager::InitializeAudioComponents()
{
    if (!GetOwner())
    {
        UE_LOG(LogTemp, Error, TEXT("Audio_MetaSoundManager: No owner actor found"));
        return;
    }

    // Create biome audio component
    BiomeAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("BiomeAudioComponent"));
    if (BiomeAudioComponent)
    {
        BiomeAudioComponent->SetAutoActivate(false);
        BiomeAudioComponent->SetVolumeMultiplier(0.5f);
        BiomeAudioComponent->bIsUISound = false;
        BiomeAudioComponent->bAllowSpatialization = true;
    }

    // Create threat audio component
    ThreatAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("ThreatAudioComponent"));
    if (ThreatAudioComponent)
    {
        ThreatAudioComponent->SetAutoActivate(false);
        ThreatAudioComponent->SetVolumeMultiplier(0.3f);
        ThreatAudioComponent->bIsUISound = false;
        ThreatAudioComponent->bAllowSpatialization = true;
    }

    // Create narrative audio component
    NarrativeAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("NarrativeAudioComponent"));
    if (NarrativeAudioComponent)
    {
        NarrativeAudioComponent->SetAutoActivate(false);
        NarrativeAudioComponent->SetVolumeMultiplier(0.8f);
        NarrativeAudioComponent->bIsUISound = true; // Narrative should not be spatialized
        NarrativeAudioComponent->bAllowSpatialization = false;
    }

    UE_LOG(LogTemp, Log, TEXT("Audio_MetaSoundManager: Audio components initialized"));
}

void UAudio_MetaSoundManager::SetBiome(EAudio_BiomeType NewBiome)
{
    if (NewBiome == CurrentBiome)
    {
        return;
    }

    EAudio_BiomeType PreviousBiome = CurrentBiome;
    CurrentBiome = NewBiome;
    bIsTransitioning = true;
    BiomeTransitionTimer = 0.0f;

    UE_LOG(LogTemp, Log, TEXT("Audio_MetaSoundManager: Transitioning from %d to %d"), 
           static_cast<int32>(PreviousBiome), static_cast<int32>(NewBiome));

    ConfigureBiomeAudio();
}

void UAudio_MetaSoundManager::SetThreatLevel(EAudio_ThreatLevel NewThreatLevel)
{
    if (NewThreatLevel == CurrentThreatLevel)
    {
        return;
    }

    EAudio_ThreatLevel PreviousThreatLevel = CurrentThreatLevel;
    CurrentThreatLevel = NewThreatLevel;
    bIsTransitioning = true;
    ThreatTransitionTimer = 0.0f;

    UE_LOG(LogTemp, Log, TEXT("Audio_MetaSoundManager: Threat level changed from %d to %d"), 
           static_cast<int32>(PreviousThreatLevel), static_cast<int32>(NewThreatLevel));

    ConfigureThreatAudio();
}

void UAudio_MetaSoundManager::PlaySurvivalNarrative(int32 NarrativeIndex)
{
    if (!NarrativeAudioComponent || SurvivalNarrativeLines.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("Audio_MetaSoundManager: Cannot play survival narrative - component or lines missing"));
        return;
    }

    if (NarrativeIndex >= 0 && NarrativeIndex < SurvivalNarrativeLines.Num())
    {
        if (SurvivalNarrativeLines[NarrativeIndex])
        {
            NarrativeAudioComponent->SetSound(SurvivalNarrativeLines[NarrativeIndex]);
            NarrativeAudioComponent->Play();
            UE_LOG(LogTemp, Log, TEXT("Audio_MetaSoundManager: Playing survival narrative %d"), NarrativeIndex);
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Audio_MetaSoundManager: Invalid narrative index %d"), NarrativeIndex);
    }
}

void UAudio_MetaSoundManager::PlayTribalDialogue(int32 DialogueIndex)
{
    if (!NarrativeAudioComponent || TribalDialogueLines.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("Audio_MetaSoundManager: Cannot play tribal dialogue - component or lines missing"));
        return;
    }

    if (DialogueIndex >= 0 && DialogueIndex < TribalDialogueLines.Num())
    {
        if (TribalDialogueLines[DialogueIndex])
        {
            NarrativeAudioComponent->SetSound(TribalDialogueLines[DialogueIndex]);
            NarrativeAudioComponent->Play();
            UE_LOG(LogTemp, Log, TEXT("Audio_MetaSoundManager: Playing tribal dialogue %d"), DialogueIndex);
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Audio_MetaSoundManager: Invalid dialogue index %d"), DialogueIndex);
    }
}

void UAudio_MetaSoundManager::StopNarrative()
{
    if (NarrativeAudioComponent && NarrativeAudioComponent->IsPlaying())
    {
        NarrativeAudioComponent->Stop();
        UE_LOG(LogTemp, Log, TEXT("Audio_MetaSoundManager: Stopped narrative audio"));
    }
}

void UAudio_MetaSoundManager::SetMetaSoundParameter(const FName& ParameterName, float Value)
{
    if (BiomeAudioComponent && BiomeAudioComponent->GetSound())
    {
        // Note: MetaSound parameter setting requires UE5.1+ specific API
        // This is a placeholder for the actual MetaSound parameter interface
        UE_LOG(LogTemp, Log, TEXT("Audio_MetaSoundManager: Setting MetaSound parameter %s to %f"), 
               *ParameterName.ToString(), Value);
    }
}

void UAudio_MetaSoundManager::SetBiomeIntensity(float Intensity)
{
    SetMetaSoundParameter(FName("BiomeIntensity"), FMath::Clamp(Intensity, 0.0f, 1.0f));
}

void UAudio_MetaSoundManager::SetThreatIntensity(float Intensity)
{
    SetMetaSoundParameter(FName("ThreatIntensity"), FMath::Clamp(Intensity, 0.0f, 1.0f));
}

void UAudio_MetaSoundManager::UpdateBiomeTransition(float DeltaTime)
{
    if (!BiomeAudioComponent)
    {
        return;
    }

    BiomeTransitionTimer += DeltaTime;

    const FAudio_BiomeAudioConfig* CurrentConfig = BiomeAudioConfigs.Find(CurrentBiome);
    if (!CurrentConfig)
    {
        return;
    }

    float TransitionProgress = FMath::Clamp(BiomeTransitionTimer / CurrentConfig->FadeInTime, 0.0f, 1.0f);
    float NewVolume = FMath::Lerp(0.0f, CurrentConfig->BaseVolume, TransitionProgress);
    
    BiomeAudioComponent->SetVolumeMultiplier(NewVolume);

    if (TransitionProgress >= 1.0f)
    {
        bIsTransitioning = false;
        BiomeTransitionTimer = 0.0f;
        UE_LOG(LogTemp, Log, TEXT("Audio_MetaSoundManager: Biome transition completed"));
    }
}

void UAudio_MetaSoundManager::UpdateThreatTransition(float DeltaTime)
{
    if (!ThreatAudioComponent)
    {
        return;
    }

    ThreatTransitionTimer += DeltaTime;

    // Simple threat audio fade based on threat level
    float TargetVolume = 0.0f;
    switch (CurrentThreatLevel)
    {
        case EAudio_ThreatLevel::Safe:
            TargetVolume = 0.0f;
            break;
        case EAudio_ThreatLevel::Caution:
            TargetVolume = 0.2f;
            break;
        case EAudio_ThreatLevel::Danger:
            TargetVolume = 0.5f;
            break;
        case EAudio_ThreatLevel::Critical:
            TargetVolume = 0.8f;
            break;
    }

    float TransitionProgress = FMath::Clamp(ThreatTransitionTimer / 2.0f, 0.0f, 1.0f); // 2 second transition
    float CurrentVolume = ThreatAudioComponent->GetVolumeMultiplier();
    float NewVolume = FMath::Lerp(CurrentVolume, TargetVolume, TransitionProgress);
    
    ThreatAudioComponent->SetVolumeMultiplier(NewVolume);

    if (TransitionProgress >= 1.0f)
    {
        ThreatTransitionTimer = 0.0f;
    }
}

void UAudio_MetaSoundManager::ConfigureBiomeAudio()
{
    if (!BiomeAudioComponent)
    {
        return;
    }

    const FAudio_BiomeAudioConfig* Config = BiomeAudioConfigs.Find(CurrentBiome);
    if (Config && Config->AmbientMetaSound)
    {
        BiomeAudioComponent->SetSound(Config->AmbientMetaSound);
        BiomeAudioComponent->SetVolumeMultiplier(0.0f); // Start at 0 for transition
        BiomeAudioComponent->Play();
        UE_LOG(LogTemp, Log, TEXT("Audio_MetaSoundManager: Configured biome audio for type %d"), static_cast<int32>(CurrentBiome));
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Audio_MetaSoundManager: No audio config found for biome %d"), static_cast<int32>(CurrentBiome));
    }
}

void UAudio_MetaSoundManager::ConfigureThreatAudio()
{
    if (!ThreatAudioComponent)
    {
        return;
    }

    UMetaSoundSource** ThreatSound = ThreatAudioSources.Find(CurrentThreatLevel);
    if (ThreatSound && *ThreatSound)
    {
        ThreatAudioComponent->SetSound(*ThreatSound);
        ThreatAudioComponent->SetVolumeMultiplier(0.0f); // Start at 0 for transition
        ThreatAudioComponent->Play();
        UE_LOG(LogTemp, Log, TEXT("Audio_MetaSoundManager: Configured threat audio for level %d"), static_cast<int32>(CurrentThreatLevel));
    }
    else
    {
        // Stop threat audio if no sound configured for this level
        if (ThreatAudioComponent->IsPlaying())
        {
            ThreatAudioComponent->Stop();
        }
        UE_LOG(LogTemp, Log, TEXT("Audio_MetaSoundManager: No threat audio for level %d"), static_cast<int32>(CurrentThreatLevel));
    }
}