#include "NarrativeVoiceManager.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundWave.h"
#include "Engine/Engine.h"
#include "TimerManager.h"

UNarrativeVoiceManager::UNarrativeVoiceManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = true;
    
    GlobalNarrativeVolume = 1.0f;
    CurrentSequenceIndex = 0;
    bIsPlayingSequence = false;
    SequenceTimer = 0.0f;
    bWaitingForNextLine = false;

    // Initialize default voice volumes
    VoiceTypeVolumes.Add(EAudio_VoiceType::PrimordialNarrator, 1.0f);
    VoiceTypeVolumes.Add(EAudio_VoiceType::SpiritGuide, 0.9f);
    VoiceTypeVolumes.Add(EAudio_VoiceType::AncientWisdom, 0.8f);
    VoiceTypeVolumes.Add(EAudio_VoiceType::ConsciousnessVoice, 0.85f);
    VoiceTypeVolumes.Add(EAudio_VoiceType::NatureSpirit, 0.75f);
}

void UNarrativeVoiceManager::BeginPlay()
{
    Super::BeginPlay();
    InitializeVoiceComponents();
}

void UNarrativeVoiceManager::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    StopAllVoices();
    Super::EndPlay(EndPlayReason);
}

void UNarrativeVoiceManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (bIsPlayingSequence && bWaitingForNextLine)
    {
        SequenceTimer += DeltaTime;
        if (SequenceTimer >= CurrentSequence.DelayBetweenLines)
        {
            PlayNextLineInSequence();
        }
    }
}

void UNarrativeVoiceManager::InitializeVoiceComponents()
{
    if (!GetOwner())
    {
        UE_LOG(LogTemp, Warning, TEXT("NarrativeVoiceManager: No owner actor found"));
        return;
    }

    // Create audio components for each voice type
    TArray<EAudio_VoiceType> VoiceTypes = {
        EAudio_VoiceType::PrimordialNarrator,
        EAudio_VoiceType::SpiritGuide,
        EAudio_VoiceType::AncientWisdom,
        EAudio_VoiceType::ConsciousnessVoice,
        EAudio_VoiceType::NatureSpirit
    };

    for (EAudio_VoiceType VoiceType : VoiceTypes)
    {
        UAudioComponent* AudioComp = NewObject<UAudioComponent>(GetOwner());
        if (AudioComp)
        {
            AudioComp->AttachToComponent(GetOwner()->GetRootComponent(), 
                FAttachmentTransformRules::KeepWorldTransform);
            AudioComp->SetAutoActivate(false);
            AudioComp->bIsUISound = true; // Narrative voices are UI sounds
            AudioComp->OnAudioFinished.AddDynamic(this, &UNarrativeVoiceManager::HandleAudioFinished);
            
            VoiceAudioComponents.Add(VoiceType, AudioComp);
            
            UE_LOG(LogTemp, Log, TEXT("NarrativeVoiceManager: Created audio component for voice type %d"), 
                static_cast<int32>(VoiceType));
        }
    }
}

void UNarrativeVoiceManager::PlayVoiceLine(const FAudio_VoiceLine& VoiceLine)
{
    if (!VoiceLine.VoiceAudio.IsValid())
    {
        UE_LOG(LogTemp, Warning, TEXT("NarrativeVoiceManager: Invalid voice audio for line %s"), 
            *VoiceLine.LineID);
        return;
    }

    UAudioComponent* AudioComp = GetAudioComponentForVoiceType(VoiceLine.VoiceType);
    if (!AudioComp)
    {
        UE_LOG(LogTemp, Warning, TEXT("NarrativeVoiceManager: No audio component for voice type %d"), 
            static_cast<int32>(VoiceLine.VoiceType));
        return;
    }

    // Stop current voice if playing
    StopCurrentVoice();

    // Load and set the sound wave
    USoundWave* SoundWave = VoiceLine.VoiceAudio.LoadSynchronous();
    if (!SoundWave)
    {
        UE_LOG(LogTemp, Warning, TEXT("NarrativeVoiceManager: Failed to load sound wave for line %s"), 
            *VoiceLine.LineID);
        return;
    }

    CurrentVoiceLine = VoiceLine;
    
    // Configure audio component
    AudioComp->SetSound(SoundWave);
    
    // Calculate final volume
    float VoiceTypeVolume = VoiceTypeVolumes.FindRef(VoiceLine.VoiceType);
    float FinalVolume = GlobalNarrativeVolume * VoiceTypeVolume * VoiceLine.EmotionalIntensity;
    AudioComp->SetVolumeMultiplier(FinalVolume);
    
    // Apply fade in if specified
    if (VoiceLine.FadeInDuration > 0.0f)
    {
        AudioComp->FadeIn(VoiceLine.FadeInDuration, FinalVolume);
    }
    else
    {
        AudioComp->Play();
    }

    // Broadcast event
    OnVoiceLineStarted.Broadcast(VoiceLine.LineID, VoiceLine.VoiceType);
    
    UE_LOG(LogTemp, Log, TEXT("NarrativeVoiceManager: Playing voice line %s with volume %.2f"), 
        *VoiceLine.LineID, FinalVolume);
}

void UNarrativeVoiceManager::PlayVoiceLineByID(const FString& LineID)
{
    if (FAudio_VoiceLine* VoiceLine = RegisteredVoiceLines.Find(LineID))
    {
        PlayVoiceLine(*VoiceLine);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("NarrativeVoiceManager: Voice line %s not found in registry"), *LineID);
    }
}

void UNarrativeVoiceManager::PlayVoiceSequence(const FAudio_VoiceSequence& Sequence)
{
    if (Sequence.VoiceLines.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("NarrativeVoiceManager: Empty voice sequence %s"), *Sequence.SequenceID);
        return;
    }

    StopAllVoices();
    
    CurrentSequence = Sequence;
    CurrentSequenceIndex = 0;
    bIsPlayingSequence = true;
    bWaitingForNextLine = false;
    SequenceTimer = 0.0f;

    // Play first line
    PlayVoiceLine(CurrentSequence.VoiceLines[0]);
    
    UE_LOG(LogTemp, Log, TEXT("NarrativeVoiceManager: Started voice sequence %s with %d lines"), 
        *Sequence.SequenceID, Sequence.VoiceLines.Num());
}

void UNarrativeVoiceManager::PlayVoiceSequenceByID(const FString& SequenceID)
{
    if (FAudio_VoiceSequence* Sequence = RegisteredSequences.Find(SequenceID))
    {
        PlayVoiceSequence(*Sequence);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("NarrativeVoiceManager: Voice sequence %s not found in registry"), *SequenceID);
    }
}

void UNarrativeVoiceManager::StopCurrentVoice()
{
    for (auto& VoiceCompPair : VoiceAudioComponents)
    {
        if (VoiceCompPair.Value && VoiceCompPair.Value->IsPlaying())
        {
            if (CurrentVoiceLine.FadeOutDuration > 0.0f)
            {
                VoiceCompPair.Value->FadeOut(CurrentVoiceLine.FadeOutDuration, 0.0f);
            }
            else
            {
                VoiceCompPair.Value->Stop();
            }
        }
    }
}

void UNarrativeVoiceManager::StopAllVoices()
{
    for (auto& VoiceCompPair : VoiceAudioComponents)
    {
        if (VoiceCompPair.Value)
        {
            VoiceCompPair.Value->Stop();
        }
    }
    
    bIsPlayingSequence = false;
    bWaitingForNextLine = false;
    CurrentSequenceIndex = 0;
    SequenceTimer = 0.0f;
}

void UNarrativeVoiceManager::RegisterVoiceLine(const FAudio_VoiceLine& VoiceLine)
{
    RegisteredVoiceLines.Add(VoiceLine.LineID, VoiceLine);
    UE_LOG(LogTemp, Log, TEXT("NarrativeVoiceManager: Registered voice line %s"), *VoiceLine.LineID);
}

void UNarrativeVoiceManager::RegisterVoiceSequence(const FAudio_VoiceSequence& Sequence)
{
    RegisteredSequences.Add(Sequence.SequenceID, Sequence);
    UE_LOG(LogTemp, Log, TEXT("NarrativeVoiceManager: Registered voice sequence %s with %d lines"), 
        *Sequence.SequenceID, Sequence.VoiceLines.Num());
}

void UNarrativeVoiceManager::SetVoiceVolume(EAudio_VoiceType VoiceType, float Volume)
{
    VoiceTypeVolumes.Add(VoiceType, FMath::Clamp(Volume, 0.0f, 1.0f));
    UE_LOG(LogTemp, Log, TEXT("NarrativeVoiceManager: Set volume for voice type %d to %.2f"), 
        static_cast<int32>(VoiceType), Volume);
}

void UNarrativeVoiceManager::SetGlobalNarrativeVolume(float Volume)
{
    GlobalNarrativeVolume = FMath::Clamp(Volume, 0.0f, 1.0f);
    UE_LOG(LogTemp, Log, TEXT("NarrativeVoiceManager: Set global narrative volume to %.2f"), GlobalNarrativeVolume);
}

void UNarrativeVoiceManager::PlayContextualVoice(EAudio_NarrativeContext Context, float SpiritualIntensity)
{
    TArray<FAudio_VoiceLine> ContextualVoices = GetVoicesForContext(Context);
    
    if (ContextualVoices.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("NarrativeVoiceManager: No voices found for context %d"), 
            static_cast<int32>(Context));
        return;
    }

    // Find the voice line with spiritual resonance closest to the requested intensity
    FAudio_VoiceLine* BestMatch = nullptr;
    float BestMatchDifference = FLT_MAX;
    
    for (FAudio_VoiceLine& VoiceLine : ContextualVoices)
    {
        float Difference = FMath::Abs(VoiceLine.SpiritualResonance - SpiritualIntensity);
        if (Difference < BestMatchDifference)
        {
            BestMatchDifference = Difference;
            BestMatch = &VoiceLine;
        }
    }

    if (BestMatch)
    {
        PlayVoiceLine(*BestMatch);
    }
}

TArray<FAudio_VoiceLine> UNarrativeVoiceManager::GetVoicesForContext(EAudio_NarrativeContext Context) const
{
    TArray<FAudio_VoiceLine> ContextualVoices;
    
    for (const auto& VoiceLinePair : RegisteredVoiceLines)
    {
        if (VoiceLinePair.Value.Context == Context)
        {
            ContextualVoices.Add(VoiceLinePair.Value);
        }
    }
    
    return ContextualVoices;
}

bool UNarrativeVoiceManager::IsVoicePlaying() const
{
    for (const auto& VoiceCompPair : VoiceAudioComponents)
    {
        if (VoiceCompPair.Value && VoiceCompPair.Value->IsPlaying())
        {
            return true;
        }
    }
    return false;
}

EAudio_VoiceType UNarrativeVoiceManager::GetCurrentVoiceType() const
{
    return CurrentVoiceLine.VoiceType;
}

FString UNarrativeVoiceManager::GetCurrentLineID() const
{
    return CurrentVoiceLine.LineID;
}

UAudioComponent* UNarrativeVoiceManager::GetAudioComponentForVoiceType(EAudio_VoiceType VoiceType)
{
    if (UAudioComponent** AudioCompPtr = VoiceAudioComponents.Find(VoiceType))
    {
        return *AudioCompPtr;
    }
    return nullptr;
}

void UNarrativeVoiceManager::PlayNextLineInSequence()
{
    if (!bIsPlayingSequence || CurrentSequenceIndex >= CurrentSequence.VoiceLines.Num() - 1)
    {
        // Sequence completed
        bIsPlayingSequence = false;
        bWaitingForNextLine = false;
        OnVoiceSequenceCompleted.Broadcast(CurrentSequence.SequenceID, CurrentSequence.SequenceContext);
        UE_LOG(LogTemp, Log, TEXT("NarrativeVoiceManager: Completed voice sequence %s"), 
            *CurrentSequence.SequenceID);
        return;
    }

    CurrentSequenceIndex++;
    bWaitingForNextLine = false;
    SequenceTimer = 0.0f;
    
    PlayVoiceLine(CurrentSequence.VoiceLines[CurrentSequenceIndex]);
}

void UNarrativeVoiceManager::OnVoiceLineFinished()
{
    OnVoiceLineCompleted.Broadcast(CurrentVoiceLine.LineID, CurrentVoiceLine.VoiceType);
    
    if (bIsPlayingSequence && CurrentSequence.bAutoAdvance)
    {
        bWaitingForNextLine = true;
        SequenceTimer = 0.0f;
    }
}

void UNarrativeVoiceManager::HandleAudioFinished()
{
    OnVoiceLineFinished();
}