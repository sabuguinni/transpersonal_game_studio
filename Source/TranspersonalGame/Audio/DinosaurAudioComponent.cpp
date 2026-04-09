#include "DinosaurAudioComponent.h"
#include "Components/AudioComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"
#include "Sound/SoundAttenuation.h"

UDinosaurAudioComponent::UDinosaurAudioComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.5f; // Update every 0.5 seconds

    // Initialize default values
    DinosaurSpecies = TEXT("Generic");
    DinosaurSize = EDinosaurSize::Medium;
    bIsCarnivore = false;
    bIsPackHunter = false;
    AggressionLevel = 0.5f;
    VolumeMultiplier = 1.0f;
    PitchMultiplier = 1.0f;
    bEnableRandomCalls = true;
    bEnableFootsteps = true;
    bEnableBreathing = true;
    HealthPercentage = 1.0f;
}

void UDinosaurAudioComponent::BeginPlay()
{
    Super::BeginPlay();

    // Create audio components
    AActor* Owner = GetOwner();
    if (Owner)
    {
        VocalizationComponent = Owner->CreateDefaultSubobject<UAudioComponent>(TEXT("DinosaurVocalization"));
        FootstepComponent = Owner->CreateDefaultSubobject<UAudioComponent>(TEXT("DinosaurFootsteps"));
        BreathingComponent = Owner->CreateDefaultSubobject<UAudioComponent>(TEXT("DinosaurBreathing"));
        AmbientComponent = Owner->CreateDefaultSubobject<UAudioComponent>(TEXT("DinosaurAmbient"));

        // Configure audio components
        if (VocalizationComponent)
        {
            VocalizationComponent->SetVolumeMultiplier(GetSizeVolumeMultiplier());
            VocalizationComponent->SetPitchMultiplier(GetSizePitchMultiplier());
            VocalizationComponent->bAutoActivate = false;
        }

        if (FootstepComponent)
        {
            FootstepComponent->SetVolumeMultiplier(GetSizeVolumeMultiplier() * 0.7f);
            FootstepComponent->bAutoActivate = false;
        }

        if (BreathingComponent)
        {
            BreathingComponent->SetVolumeMultiplier(GetSizeVolumeMultiplier() * 0.3f);
            BreathingComponent->bAutoActivate = false;
        }

        if (AmbientComponent)
        {
            AmbientComponent->SetVolumeMultiplier(GetSizeVolumeMultiplier() * 0.5f);
            AmbientComponent->bAutoActivate = false;
        }
    }

    // Initialize last play times
    LastPlayTimes.Add(EDinosaurAudioType::Idle, 0.0f);
    LastPlayTimes.Add(EDinosaurAudioType::Alert, 0.0f);
    LastPlayTimes.Add(EDinosaurAudioType::Aggressive, 0.0f);
    LastPlayTimes.Add(EDinosaurAudioType::Feeding, 0.0f);
    LastPlayTimes.Add(EDinosaurAudioType::Mating, 0.0f);
    LastPlayTimes.Add(EDinosaurAudioType::Territorial, 0.0f);
    LastPlayTimes.Add(EDinosaurAudioType::Pain, 0.0f);
    LastPlayTimes.Add(EDinosaurAudioType::Death, 0.0f);
    LastPlayTimes.Add(EDinosaurAudioType::Footsteps, 0.0f);
    LastPlayTimes.Add(EDinosaurAudioType::Breathing, 0.0f);

    // Start breathing loop if enabled
    if (bEnableBreathing)
    {
        StartBreathingLoop();
    }

    // Schedule first random call
    if (bEnableRandomCalls)
    {
        ScheduleNextRandomCall();
    }

    UE_LOG(LogTemp, Log, TEXT("DinosaurAudioComponent: Initialized for species '%s' (Size: %d)"), 
           *DinosaurSpecies, (int32)DinosaurSize);
}

void UDinosaurAudioComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    // Update audio based on current state
    UpdateAudioBasedOnHealth();
    UpdateAudioBasedOnBehavior();
}

void UDinosaurAudioComponent::PlayDinosaurSound(EDinosaurAudioType AudioType, float VolumeOverride, float PitchOverride)
{
    if (!ShouldPlaySound(AudioType))
    {
        return;
    }

    FDinosaurAudioData* AudioData = GetAudioData(AudioType);
    if (!AudioData || !AudioData->SoundCue)
    {
        UE_LOG(LogTemp, Warning, TEXT("DinosaurAudioComponent: No audio data found for type %d"), (int32)AudioType);
        return;
    }

    UAudioComponent* TargetComponent = VocalizationComponent;

    // Choose appropriate audio component
    switch (AudioType)
    {
        case EDinosaurAudioType::Footsteps:
            TargetComponent = FootstepComponent;
            break;
        case EDinosaurAudioType::Breathing:
            TargetComponent = BreathingComponent;
            break;
        default:
            TargetComponent = VocalizationComponent;
            break;
    }

    if (!TargetComponent)
    {
        return;
    }

    // Stop current sound if it should be interrupted
    if (AudioData->bInterruptsOtherSounds && TargetComponent->IsPlaying())
    {
        TargetComponent->Stop();
    }

    // Calculate final volume
    float FinalVolume = AudioData->BaseVolume * VolumeMultiplier * GetSizeVolumeMultiplier();
    if (VolumeOverride >= 0.0f)
    {
        FinalVolume = VolumeOverride;
    }

    // Add volume variation
    if (AudioData->VolumeVariation > 0.0f)
    {
        float VolumeVar = FMath::RandRange(-AudioData->VolumeVariation, AudioData->VolumeVariation);
        FinalVolume *= (1.0f + VolumeVar);
    }

    // Calculate final pitch
    float FinalPitch = PitchMultiplier * GetSizePitchMultiplier();
    if (PitchOverride >= 0.0f)
    {
        FinalPitch = PitchOverride;
    }

    // Add pitch variation
    if (AudioData->PitchVariation > 0.0f)
    {
        float PitchVar = FMath::RandRange(-AudioData->PitchVariation, AudioData->PitchVariation);
        FinalPitch *= (1.0f + PitchVar);
    }

    // Adjust for health if injured
    if (HealthPercentage < 1.0f && AudioType != EDinosaurAudioType::Death)
    {
        FinalPitch *= (0.8f + 0.2f * HealthPercentage); // Lower pitch when injured
        FinalVolume *= (0.7f + 0.3f * HealthPercentage); // Lower volume when injured
    }

    // Play the sound
    TargetComponent->SetSound(AudioData->SoundCue);
    TargetComponent->SetVolumeMultiplier(FinalVolume);
    TargetComponent->SetPitchMultiplier(FinalPitch);
    TargetComponent->Play();

    // Update state
    CurrentlyPlayingType = AudioType;
    LastPlayTimes[AudioType] = GetWorld()->GetTimeSeconds();

    UE_LOG(LogTemp, Log, TEXT("DinosaurAudioComponent: Playing %s sound for %s (Vol: %.2f, Pitch: %.2f)"), 
           *UEnum::GetValueAsString(AudioType), *DinosaurSpecies, FinalVolume, FinalPitch);
}

void UDinosaurAudioComponent::PlayIdleSound()
{
    PlayDinosaurSound(EDinosaurAudioType::Idle);
}

void UDinosaurAudioComponent::PlayAlertSound()
{
    bIsAlerted = true;
    PlayDinosaurSound(EDinosaurAudioType::Alert);
}

void UDinosaurAudioComponent::PlayAggressiveSound()
{
    PlayDinosaurSound(EDinosaurAudioType::Aggressive);
}

void UDinosaurAudioComponent::PlayPainSound()
{
    PlayDinosaurSound(EDinosaurAudioType::Pain);
}

void UDinosaurAudioComponent::PlayDeathSound()
{
    StopAllSounds();
    PlayDinosaurSound(EDinosaurAudioType::Death);
}

void UDinosaurAudioComponent::PlayFootstepSound()
{
    if (bEnableFootsteps)
    {
        PlayDinosaurSound(EDinosaurAudioType::Footsteps);
    }
}

void UDinosaurAudioComponent::StartBreathingLoop()
{
    if (bEnableBreathing && BreathingComponent)
    {
        FDinosaurAudioData* AudioData = GetAudioData(EDinosaurAudioType::Breathing);
        if (AudioData && AudioData->SoundCue)
        {
            BreathingComponent->SetSound(AudioData->SoundCue);
            BreathingComponent->SetVolumeMultiplier(AudioData->BaseVolume * VolumeMultiplier * 0.3f);
            BreathingComponent->Play();
            
            UE_LOG(LogTemp, Log, TEXT("DinosaurAudioComponent: Started breathing loop for %s"), *DinosaurSpecies);
        }
    }
}

void UDinosaurAudioComponent::StopBreathingLoop()
{
    if (BreathingComponent && BreathingComponent->IsPlaying())
    {
        BreathingComponent->Stop();
        UE_LOG(LogTemp, Log, TEXT("DinosaurAudioComponent: Stopped breathing loop for %s"), *DinosaurSpecies);
    }
}

void UDinosaurAudioComponent::StopAllSounds()
{
    if (VocalizationComponent) VocalizationComponent->Stop();
    if (FootstepComponent) FootstepComponent->Stop();
    if (BreathingComponent) BreathingComponent->Stop();
    if (AmbientComponent) AmbientComponent->Stop();

    // Clear random call timer
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().ClearTimer(RandomCallTimer);
    }

    UE_LOG(LogTemp, Log, TEXT("DinosaurAudioComponent: Stopped all sounds for %s"), *DinosaurSpecies);
}

void UDinosaurAudioComponent::OnDinosaurSpotted()
{
    if (bIsPackHunter)
    {
        CallForHelp();
    }
    else
    {
        PlayAlertSound();
    }
}

void UDinosaurAudioComponent::OnPlayerDetected()
{
    if (bIsCarnivore && AggressionLevel > 0.5f)
    {
        PlayAggressiveSound();
    }
    else
    {
        PlayAlertSound();
    }
}

void UDinosaurAudioComponent::OnTakeDamage(float DamageAmount)
{
    // Update health percentage (this would typically come from a health component)
    HealthPercentage = FMath::Clamp(HealthPercentage - (DamageAmount / 100.0f), 0.0f, 1.0f);

    if (HealthPercentage <= 0.0f)
    {
        PlayDeathSound();
    }
    else
    {
        PlayPainSound();
        
        // Become more aggressive when injured
        if (bIsCarnivore)
        {
            PlayAggressiveSound();
        }
    }
}

void UDinosaurAudioComponent::OnEnterCombat()
{
    bIsInCombat = true;
    StopBreathingLoop(); // Stop breathing during combat for clarity
    PlayAggressiveSound();
}

void UDinosaurAudioComponent::OnExitCombat()
{
    bIsInCombat = false;
    bIsAlerted = false;
    
    // Resume breathing
    if (bEnableBreathing)
    {
        StartBreathingLoop();
    }
    
    // Resume random calls
    if (bEnableRandomCalls)
    {
        ScheduleNextRandomCall();
    }
}

void UDinosaurAudioComponent::OnStartFeeding()
{
    PlayDinosaurSound(EDinosaurAudioType::Feeding);
}

void UDinosaurAudioComponent::OnTerritorialDisplay()
{
    PlayDinosaurSound(EDinosaurAudioType::Territorial);
}

void UDinosaurAudioComponent::CallForHelp()
{
    if (bIsPackHunter)
    {
        // Play a specific aggressive call that other pack members can respond to
        PlayDinosaurSound(EDinosaurAudioType::Aggressive, 1.5f); // Louder call for help
        
        UE_LOG(LogTemp, Log, TEXT("DinosaurAudioComponent: %s calling for pack help"), *DinosaurSpecies);
    }
}

void UDinosaurAudioComponent::RespondToPackCall()
{
    if (bIsPackHunter)
    {
        PlayDinosaurSound(EDinosaurAudioType::Alert);
        UE_LOG(LogTemp, Log, TEXT("DinosaurAudioComponent: %s responding to pack call"), *DinosaurSpecies);
    }
}

void UDinosaurAudioComponent::CoordinateAttack()
{
    if (bIsPackHunter)
    {
        PlayDinosaurSound(EDinosaurAudioType::Aggressive);
        UE_LOG(LogTemp, Log, TEXT("DinosaurAudioComponent: %s coordinating pack attack"), *DinosaurSpecies);
    }
}

bool UDinosaurAudioComponent::IsPlayingSound(EDinosaurAudioType AudioType) const
{
    switch (AudioType)
    {
        case EDinosaurAudioType::Footsteps:
            return FootstepComponent && FootstepComponent->IsPlaying();
        case EDinosaurAudioType::Breathing:
            return BreathingComponent && BreathingComponent->IsPlaying();
        default:
            return VocalizationComponent && VocalizationComponent->IsPlaying() && 
                   CurrentlyPlayingType == AudioType;
    }
}

float UDinosaurAudioComponent::GetCurrentVolumeLevel() const
{
    if (VocalizationComponent)
    {
        return VocalizationComponent->GetVolumeMultiplier();
    }
    return 0.0f;
}

bool UDinosaurAudioComponent::CanPlaySound(EDinosaurAudioType AudioType) const
{
    return ShouldPlaySound(AudioType);
}

void UDinosaurAudioComponent::PlayRandomCall()
{
    if (!bEnableRandomCalls || bIsInCombat)
    {
        return;
    }

    // Choose a random call type based on behavior
    TArray<EDinosaurAudioType> PossibleCalls;
    
    if (!bIsAlerted)
    {
        PossibleCalls.Add(EDinosaurAudioType::Idle);
    }
    
    if (bIsCarnivore && AggressionLevel > 0.3f)
    {
        PossibleCalls.Add(EDinosaurAudioType::Territorial);
    }
    
    // Add feeding calls occasionally
    if (FMath::RandRange(0.0f, 1.0f) < 0.2f)
    {
        PossibleCalls.Add(EDinosaurAudioType::Feeding);
    }

    if (PossibleCalls.Num() > 0)
    {
        int32 RandomIndex = FMath::RandRange(0, PossibleCalls.Num() - 1);
        PlayDinosaurSound(PossibleCalls[RandomIndex]);
    }

    // Schedule next call
    ScheduleNextRandomCall();
}

void UDinosaurAudioComponent::ScheduleNextRandomCall()
{
    if (!GetWorld() || !bEnableRandomCalls)
    {
        return;
    }

    // Get timing data from audio data table
    FDinosaurAudioData* IdleData = GetAudioData(EDinosaurAudioType::Idle);
    float MinTime = IdleData ? IdleData->MinTimeBetweenCalls : 10.0f;
    float MaxTime = IdleData ? IdleData->MaxTimeBetweenCalls : 30.0f;

    // Adjust timing based on aggression level
    if (bIsCarnivore && AggressionLevel > 0.5f)
    {
        MinTime *= 0.7f; // More frequent calls for aggressive carnivores
        MaxTime *= 0.7f;
    }

    float NextCallTime = FMath::RandRange(MinTime, MaxTime);
    
    GetWorld()->GetTimerManager().SetTimer(RandomCallTimer, this, 
        &UDinosaurAudioComponent::PlayRandomCall, NextCallTime, false);
}

FDinosaurAudioData* UDinosaurAudioComponent::GetAudioData(EDinosaurAudioType AudioType)
{
    if (!AudioDataTable)
    {
        return nullptr;
    }

    // Construct row name based on species and audio type
    FString RowName = FString::Printf(TEXT("%s_%s"), 
        *DinosaurSpecies, 
        *UEnum::GetValueAsString(AudioType));

    return AudioDataTable->FindRow<FDinosaurAudioData>(FName(*RowName), TEXT("DinosaurAudio"));
}

float UDinosaurAudioComponent::GetSizeVolumeMultiplier() const
{
    switch (DinosaurSize)
    {
        case EDinosaurSize::Small:
            return 0.5f;
        case EDinosaurSize::Medium:
            return 1.0f;
        case EDinosaurSize::Large:
            return 1.5f;
        case EDinosaurSize::Massive:
            return 2.0f;
        default:
            return 1.0f;
    }
}

float UDinosaurAudioComponent::GetSizePitchMultiplier() const
{
    switch (DinosaurSize)
    {
        case EDinosaurSize::Small:
            return 1.4f; // Higher pitch for smaller dinosaurs
        case EDinosaurSize::Medium:
            return 1.0f;
        case EDinosaurSize::Large:
            return 0.8f;
        case EDinosaurSize::Massive:
            return 0.6f; // Much lower pitch for massive dinosaurs
        default:
            return 1.0f;
    }
}

float UDinosaurAudioComponent::GetSizeAttenuationDistance() const
{
    switch (DinosaurSize)
    {
        case EDinosaurSize::Small:
            return 500.0f;
        case EDinosaurSize::Medium:
            return 1000.0f;
        case EDinosaurSize::Large:
            return 2000.0f;
        case EDinosaurSize::Massive:
            return 4000.0f; // Massive dinosaurs can be heard from very far away
        default:
            return 1000.0f;
    }
}

bool UDinosaurAudioComponent::ShouldPlaySound(EDinosaurAudioType AudioType) const
{
    if (!GetWorld())
    {
        return false;
    }

    // Check if enough time has passed since last play
    const float* LastPlayTime = LastPlayTimes.Find(AudioType);
    if (LastPlayTime)
    {
        FDinosaurAudioData* AudioData = const_cast<UDinosaurAudioComponent*>(this)->GetAudioData(AudioType);
        float MinTimeBetween = AudioData ? AudioData->MinTimeBetweenCalls : 1.0f;
        
        float TimeSinceLastPlay = GetWorld()->GetTimeSeconds() - *LastPlayTime;
        if (TimeSinceLastPlay < MinTimeBetween)
        {
            return false;
        }
    }

    return true;
}

void UDinosaurAudioComponent::UpdateAudioBasedOnHealth()
{
    if (HealthPercentage < 0.3f && HealthPercentage > 0.0f)
    {
        // Injured dinosaurs breathe more heavily
        if (BreathingComponent && bEnableBreathing)
        {
            BreathingComponent->SetVolumeMultiplier(GetSizeVolumeMultiplier() * 0.6f);
            BreathingComponent->SetPitchMultiplier(1.2f); // Faster, more labored breathing
        }
    }
}

void UDinosaurAudioComponent::UpdateAudioBasedOnBehavior()
{
    // Adjust audio parameters based on current behavior state
    if (bIsAlerted && !bIsInCombat)
    {
        // Reduce random call frequency when alert
        if (GetWorld() && GetWorld()->GetTimerManager().IsTimerActive(RandomCallTimer))
        {
            // Don't schedule new random calls while alert
        }
    }
}