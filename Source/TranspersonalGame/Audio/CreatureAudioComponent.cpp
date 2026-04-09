// Copyright Transpersonal Game Studio. All Rights Reserved.

#include "CreatureAudioComponent.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"
#include "Components/AudioComponent.h"
#include "Kismet/KismetMathLibrary.h"

// Define constants
const float UCreatureAudioComponent::MinVocalizationInterval = 3.0f;
const float UCreatureAudioComponent::MaxVocalizationInterval = 45.0f;

UCreatureAudioComponent::UCreatureAudioComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 1.0f; // 1 FPS for creature audio updates

    // Initialize creature identity
    CreatureID = FGuid::NewGuid().ToString();
    VoicePitchVariation = FMath::RandRange(-0.3f, 0.3f); // Random individual voice
    VoiceTimbreVariation = FMath::RandRange(-0.2f, 0.2f);

    // Initialize behavioral state
    CurrentBehaviorState = ECreatureBehaviorState::Passive;
    EmotionalIntensity = 0.5f;
    bIsAlpha = false;
    bIsInjured = false;

    // Initialize vocalization settings
    VocalizationFrequency = 0.1f;
    VocalizationInterval = FVector2D(5.0f, 30.0f);
    bCanVocalize = true;
    SilenceChance = 0.3f;

    // Initialize proximity settings
    PlayerProximityThreshold = 1000.0f;
    ThreatProximityThreshold = 500.0f;
    bIsPlayerVisible = false;
    DistanceToPlayer = 10000.0f;

    // Initialize audio settings
    CurrentVoiceVolume = 1.0f;
    LastVocalizationTime = 0.0f;
    bRecentlyVocalized = false;
}

void UCreatureAudioComponent::BeginPlay()
{
    Super::BeginPlay();

    UE_LOG(LogTemp, Warning, TEXT("CreatureAudioComponent: Initializing creature audio for %s (ID: %s)"), 
           *GetOwner()->GetName(), *CreatureID);

    // Initialize audio components
    InitializeAudioComponents();

    // Generate unique voice characteristics
    UE_LOG(LogTemp, Log, TEXT("CreatureAudioComponent: Creature %s has voice pitch %.2f, timbre %.2f"), 
           *CreatureID, VoicePitchVariation, VoiceTimbreVariation);

    // Start vocalization timer
    if (bCanVocalize)
    {
        ScheduleNextVocalization();
    }

    // Start breathing update timer
    GetWorld()->GetTimerManager().SetTimer(
        BreathingUpdateTimer,
        this,
        &UCreatureAudioComponent::UpdateBreathingAudio,
        2.0f, // Every 2 seconds
        true
    );

    // Start proximity check timer
    GetWorld()->GetTimerManager().SetTimer(
        ProximityCheckTimer,
        [this]()
        {
            // Update distance to player
            APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
            if (PlayerPawn)
            {
                float Distance = FVector::Dist(GetOwner()->GetActorLocation(), PlayerPawn->GetActorLocation());
                UpdatePlayerProximity(Distance, bIsPlayerVisible);
            }
        },
        1.0f, // Every second
        true
    );
}

void UCreatureAudioComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    // Update footstep audio based on movement
    UpdateFootstepAudio();
}

void UCreatureAudioComponent::SetBehaviorState(ECreatureBehaviorState NewState, float Intensity)
{
    if (CurrentBehaviorState == NewState && FMath::Abs(EmotionalIntensity - Intensity) < 0.1f)
    {
        return; // No significant change
    }

    ECreatureBehaviorState PreviousState = CurrentBehaviorState;
    CurrentBehaviorState = NewState;
    EmotionalIntensity = FMath::Clamp(Intensity, 0.0f, 1.0f);

    UE_LOG(LogTemp, Log, TEXT("CreatureAudioComponent: %s behavior changed from %d to %d (intensity: %.2f)"), 
           *CreatureID, (int32)PreviousState, (int32)NewState, EmotionalIntensity);

    // Trigger immediate vocalization for significant state changes
    if (NewState == ECreatureBehaviorState::Alert || 
        NewState == ECreatureBehaviorState::Hunting || 
        NewState == ECreatureBehaviorState::Territorial)
    {
        TriggerVocalization(NewState, true);
    }

    // Update breathing patterns
    UpdateBreathingAudio();
}

void UCreatureAudioComponent::TriggerVocalization(ECreatureBehaviorState StateOverride, bool bForceVocalization)
{
    if (!bCanVocalize && !bForceVocalization)
    {
        return;
    }

    // Check silence chance
    if (!bForceVocalization && FMath::RandRange(0.0f, 1.0f) < SilenceChance)
    {
        UE_LOG(LogTemp, VeryVerbose, TEXT("CreatureAudioComponent: %s chose to remain silent"), *CreatureID);
        return;
    }

    ECreatureBehaviorState VocalizationState = (StateOverride != ECreatureBehaviorState::Passive) ? StateOverride : CurrentBehaviorState;
    USoundBase* VocalizationSound = GetSoundForCurrentState();

    if (VocalizationSound)
    {
        float VolumeMultiplier = GetBehaviorStateVolumeMultiplier() * GetDistanceVolumeMultiplier();
        
        // Alpha creatures are louder
        if (bIsAlpha)
        {
            VolumeMultiplier *= 1.3f;
        }

        // Injured creatures are quieter
        if (bIsInjured)
        {
            VolumeMultiplier *= 0.7f;
        }

        PlayVocalizationSound(VocalizationSound, VolumeMultiplier);
        
        LastVocalizationTime = GetWorld()->GetTimeSeconds();
        bRecentlyVocalized = true;

        UE_LOG(LogTemp, VeryVerbose, TEXT("CreatureAudioComponent: %s vocalized in state %d"), 
               *CreatureID, (int32)VocalizationState);

        // Schedule next vocalization
        if (bCanVocalize)
        {
            ScheduleNextVocalization();
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("CreatureAudioComponent: No sound found for creature %s in state %d"), 
               *CreatureID, (int32)VocalizationState);
    }
}

void UCreatureAudioComponent::TriggerPainSound(float PainIntensity)
{
    if (AudioProfile.PainSounds.Num() == 0)
    {
        return;
    }

    USoundBase* PainSound = AudioProfile.PainSounds[FMath::RandRange(0, AudioProfile.PainSounds.Num() - 1)];
    if (PainSound)
    {
        float VolumeMultiplier = FMath::Clamp(PainIntensity, 0.3f, 1.5f) * GetDistanceVolumeMultiplier();
        PlayVocalizationSound(PainSound, VolumeMultiplier, true); // Use secondary component for pain

        UE_LOG(LogTemp, Warning, TEXT("CreatureAudioComponent: %s triggered pain sound with intensity %.2f"), 
               *CreatureID, PainIntensity);
    }
}

void UCreatureAudioComponent::TriggerDeathSound()
{
    if (AudioProfile.DeathSounds.Num() == 0)
    {
        return;
    }

    // Stop all other vocalizations
    if (PrimaryVoiceComponent && PrimaryVoiceComponent->IsPlaying())
    {
        PrimaryVoiceComponent->Stop();
    }

    USoundBase* DeathSound = AudioProfile.DeathSounds[FMath::RandRange(0, AudioProfile.DeathSounds.Num() - 1)];
    if (DeathSound)
    {
        float VolumeMultiplier = 1.2f * GetDistanceVolumeMultiplier(); // Death sounds are prominent
        PlayVocalizationSound(DeathSound, VolumeMultiplier);

        UE_LOG(LogTemp, Warning, TEXT("CreatureAudioComponent: %s triggered death sound"), *CreatureID);

        // Disable further vocalizations
        bCanVocalize = false;
        GetWorld()->GetTimerManager().ClearTimer(VocalizationTimer);
    }
}

void UCreatureAudioComponent::UpdatePlayerProximity(float Distance, bool bVisible)
{
    DistanceToPlayer = Distance;
    bool bWasPlayerVisible = bIsPlayerVisible;
    bIsPlayerVisible = bVisible;

    // React to player proximity changes
    if (Distance < ThreatProximityThreshold && CurrentBehaviorState == ECreatureBehaviorState::Passive)
    {
        SetBehaviorState(ECreatureBehaviorState::Alert, 0.7f);
    }
    else if (Distance > PlayerProximityThreshold && CurrentBehaviorState == ECreatureBehaviorState::Alert)
    {
        SetBehaviorState(ECreatureBehaviorState::Passive, 0.3f);
    }

    // React to player visibility changes
    if (bVisible && !bWasPlayerVisible && Distance < PlayerProximityThreshold)
    {
        OnPlayerSpotted();
    }
    else if (!bVisible && bWasPlayerVisible)
    {
        OnPlayerLost();
    }
}

void UCreatureAudioComponent::OnPlayerSpotted()
{
    UE_LOG(LogTemp, Log, TEXT("CreatureAudioComponent: %s spotted the player at distance %.1f"), 
           *CreatureID, DistanceToPlayer);

    if (DistanceToPlayer < ThreatProximityThreshold)
    {
        SetBehaviorState(ECreatureBehaviorState::Alert, 0.8f);
        TriggerVocalization(ECreatureBehaviorState::Alert, true);
    }
}

void UCreatureAudioComponent::OnPlayerLost()
{
    UE_LOG(LogTemp, VeryVerbose, TEXT("CreatureAudioComponent: %s lost sight of the player"), *CreatureID);

    if (CurrentBehaviorState == ECreatureBehaviorState::Alert)
    {
        SetBehaviorState(ECreatureBehaviorState::Passive, 0.4f);
    }
}

void UCreatureAudioComponent::RespondToOtherCreature(UCreatureAudioComponent* OtherCreature, float Distance)
{
    if (!OtherCreature || !bCanVocalize)
    {
        return;
    }

    // Respond to other creatures based on distance and behavior
    if (Distance < 500.0f && OtherCreature->CurrentBehaviorState == ECreatureBehaviorState::Territorial)
    {
        // Respond to territorial calls
        if (FMath::RandRange(0.0f, 1.0f) < 0.3f) // 30% chance to respond
        {
            TriggerTerritorialCall();
        }
    }
    else if (Distance < 200.0f && CurrentBehaviorState == ECreatureBehaviorState::Passive)
    {
        // Social vocalizations for nearby creatures
        if (FMath::RandRange(0.0f, 1.0f) < 0.1f) // 10% chance
        {
            TriggerVocalization(ECreatureBehaviorState::Passive, false);
        }
    }
}

void UCreatureAudioComponent::TriggerTerritorialCall()
{
    SetBehaviorState(ECreatureBehaviorState::Territorial, 0.9f);
    TriggerVocalization(ECreatureBehaviorState::Territorial, true);
    
    UE_LOG(LogTemp, Log, TEXT("CreatureAudioComponent: %s triggered territorial call"), *CreatureID);
}

void UCreatureAudioComponent::TriggerMatingCall()
{
    SetBehaviorState(ECreatureBehaviorState::Mating, 0.8f);
    TriggerVocalization(ECreatureBehaviorState::Mating, true);
    
    UE_LOG(LogTemp, Log, TEXT("CreatureAudioComponent: %s triggered mating call"), *CreatureID);
}

void UCreatureAudioComponent::SetVocalizationEnabled(bool bEnabled)
{
    bCanVocalize = bEnabled;
    
    if (!bEnabled)
    {
        GetWorld()->GetTimerManager().ClearTimer(VocalizationTimer);
        
        if (PrimaryVoiceComponent && PrimaryVoiceComponent->IsPlaying())
        {
            PrimaryVoiceComponent->FadeOut(1.0f, 0.0f);
        }
    }
    else
    {
        ScheduleNextVocalization();
    }
}

void UCreatureAudioComponent::SetVoiceVolume(float Volume)
{
    CurrentVoiceVolume = FMath::Clamp(Volume, 0.0f, 2.0f);
    
    if (PrimaryVoiceComponent)
    {
        PrimaryVoiceComponent->SetVolumeMultiplier(CurrentVoiceVolume);
    }
    
    if (SecondaryVoiceComponent)
    {
        SecondaryVoiceComponent->SetVolumeMultiplier(CurrentVoiceVolume);
    }
}

void UCreatureAudioComponent::SetCreatureInjured(bool bInjured)
{
    bIsInjured = bInjured;
    
    if (bInjured)
    {
        // Injured creatures vocalize more frequently but quieter
        VocalizationFrequency *= 1.5f;
        EmotionalIntensity = FMath::Max(EmotionalIntensity, 0.6f);
        
        UE_LOG(LogTemp, Warning, TEXT("CreatureAudioComponent: %s is now injured"), *CreatureID);
    }
}

void UCreatureAudioComponent::SetCreatureAsAlpha(bool bAlpha)
{
    bIsAlpha = bAlpha;
    
    if (bAlpha)
    {
        // Alpha creatures vocalize more frequently and louder
        VocalizationFrequency *= 1.3f;
        VoicePitchVariation = FMath::Clamp(VoicePitchVariation - 0.1f, -0.5f, 0.5f); // Slightly deeper voice
        
        UE_LOG(LogTemp, Log, TEXT("CreatureAudioComponent: %s is now alpha"), *CreatureID);
    }
}

void UCreatureAudioComponent::ProcessVocalization()
{
    float VocalizationChance = CalculateVocalizationChance();
    
    if (FMath::RandRange(0.0f, 1.0f) < VocalizationChance)
    {
        TriggerVocalization();
    }
    
    // Schedule next check
    ScheduleNextVocalization();
}

void UCreatureAudioComponent::PlayVocalizationSound(USoundBase* Sound, float VolumeMultiplier, bool bUseSecondaryComponent)
{
    if (!Sound)
    {
        return;
    }

    UAudioComponent* TargetComponent = bUseSecondaryComponent ? SecondaryVoiceComponent : PrimaryVoiceComponent;
    
    if (!TargetComponent)
    {
        UE_LOG(LogTemp, Warning, TEXT("CreatureAudioComponent: No audio component available for %s"), *CreatureID);
        return;
    }

    // Stop current sound if playing
    if (TargetComponent->IsPlaying())
    {
        TargetComponent->Stop();
    }

    // Set up the sound
    TargetComponent->SetSound(Sound);
    TargetComponent->SetVolumeMultiplier(VolumeMultiplier * CurrentVoiceVolume);
    
    // Apply voice variation
    ApplyVoiceVariation(TargetComponent);
    
    // Play the sound
    TargetComponent->Play();
}

void UCreatureAudioComponent::UpdateBreathingAudio()
{
    if (!BreathingComponent)
    {
        return;
    }

    // Adjust breathing based on behavior state
    float BreathingIntensity = 0.3f;
    
    switch (CurrentBehaviorState)
    {
        case ECreatureBehaviorState::Hunting:
        case ECreatureBehaviorState::Fleeing:
            BreathingIntensity = 0.8f;
            break;
        case ECreatureBehaviorState::Alert:
        case ECreatureBehaviorState::Territorial:
            BreathingIntensity = 0.6f;
            break;
        case ECreatureBehaviorState::Passive:
        case ECreatureBehaviorState::Feeding:
            BreathingIntensity = 0.3f;
            break;
        default:
            BreathingIntensity = 0.4f;
            break;
    }

    BreathingComponent->SetVolumeMultiplier(BreathingIntensity * CurrentVoiceVolume * GetDistanceVolumeMultiplier());
}

void UCreatureAudioComponent::UpdateFootstepAudio()
{
    if (!FootstepsComponent || !GetOwner())
    {
        return;
    }

    // Get creature movement speed
    FVector Velocity = GetOwner()->GetVelocity();
    float Speed = Velocity.Size();
    
    if (Speed > 50.0f) // Moving
    {
        if (!FootstepsComponent->IsPlaying())
        {
            FootstepsComponent->Play();
        }
        
        // Adjust footstep volume and pitch based on speed
        float SpeedRatio = FMath::Clamp(Speed / 1000.0f, 0.1f, 2.0f);
        FootstepsComponent->SetVolumeMultiplier(SpeedRatio * CurrentVoiceVolume * GetDistanceVolumeMultiplier());
        FootstepsComponent->SetPitchMultiplier(0.8f + (SpeedRatio * 0.4f));
    }
    else
    {
        if (FootstepsComponent->IsPlaying())
        {
            FootstepsComponent->FadeOut(0.5f, 0.0f);
        }
    }
}

USoundBase* UCreatureAudioComponent::GetSoundForCurrentState() const
{
    const TArray<USoundBase*>* SoundArray = nullptr;
    
    switch (CurrentBehaviorState)
    {
        case ECreatureBehaviorState::Passive:
        case ECreatureBehaviorState::Feeding:
            SoundArray = &AudioProfile.IdleSounds;
            break;
        case ECreatureBehaviorState::Alert:
            SoundArray = &AudioProfile.AlertSounds;
            break;
        case ECreatureBehaviorState::Hunting:
        case ECreatureBehaviorState::Territorial:
        case ECreatureBehaviorState::Mating:
            SoundArray = &AudioProfile.AggressiveSounds;
            break;
        case ECreatureBehaviorState::Fleeing:
            // Use alert sounds for fleeing
            SoundArray = &AudioProfile.AlertSounds;
            break;
        default:
            SoundArray = &AudioProfile.IdleSounds;
            break;
    }
    
    if (SoundArray && SoundArray->Num() > 0)
    {
        int32 RandomIndex = FMath::RandRange(0, SoundArray->Num() - 1);
        return (*SoundArray)[RandomIndex];
    }
    
    return nullptr;
}

float UCreatureAudioComponent::CalculateVocalizationChance() const
{
    float BaseChance = VocalizationFrequency;
    
    // Modify based on behavior state
    switch (CurrentBehaviorState)
    {
        case ECreatureBehaviorState::Alert:
            BaseChance *= 2.0f;
            break;
        case ECreatureBehaviorState::Hunting:
        case ECreatureBehaviorState::Territorial:
            BaseChance *= 3.0f;
            break;
        case ECreatureBehaviorState::Mating:
            BaseChance *= 4.0f;
            break;
        case ECreatureBehaviorState::Fleeing:
            BaseChance *= 1.5f;
            break;
        case ECreatureBehaviorState::Feeding:
            BaseChance *= 0.3f;
            break;
        default:
            break;
    }
    
    // Modify based on emotional intensity
    BaseChance *= (0.5f + EmotionalIntensity);
    
    // Alpha creatures vocalize more
    if (bIsAlpha)
    {
        BaseChance *= 1.3f;
    }
    
    // Injured creatures vocalize more
    if (bIsInjured)
    {
        BaseChance *= 1.2f;
    }
    
    // Recent vocalization reduces chance
    if (bRecentlyVocalized)
    {
        BaseChance *= 0.3f;
        
        // Clear recent vocalization flag after some time
        if (GetWorld()->GetTimeSeconds() - LastVocalizationTime > 10.0f)
        {
            const_cast<UCreatureAudioComponent*>(this)->bRecentlyVocalized = false;
        }
    }
    
    return FMath::Clamp(BaseChance, 0.0f, 1.0f);
}

void UCreatureAudioComponent::ApplyVoiceVariation(UAudioComponent* AudioComponent) const
{
    if (!AudioComponent)
    {
        return;
    }

    // Apply pitch variation
    float PitchMultiplier = 1.0f + (VoicePitchVariation * 0.3f); // ±30% pitch variation
    AudioComponent->SetPitchMultiplier(PitchMultiplier);
    
    // Apply subtle volume variation for timbre
    float VolumeVariation = 1.0f + (VoiceTimbreVariation * 0.1f); // ±10% volume variation
    AudioComponent->SetVolumeMultiplier(AudioComponent->VolumeMultiplier * VolumeVariation);
}

void UCreatureAudioComponent::InitializeAudioComponents()
{
    AActor* Owner = GetOwner();
    if (!Owner)
    {
        return;
    }

    // Create primary voice component
    PrimaryVoiceComponent = NewObject<UAudioComponent>(Owner);
    if (PrimaryVoiceComponent)
    {
        PrimaryVoiceComponent->SetupAttachment(Owner->GetRootComponent());
        PrimaryVoiceComponent->bAutoActivate = false;
        PrimaryVoiceComponent->bAllowSpatialization = true;
        PrimaryVoiceComponent->RegisterComponent();
    }

    // Create secondary voice component (for overlapping sounds)
    SecondaryVoiceComponent = NewObject<UAudioComponent>(Owner);
    if (SecondaryVoiceComponent)
    {
        SecondaryVoiceComponent->SetupAttachment(Owner->GetRootComponent());
        SecondaryVoiceComponent->bAutoActivate = false;
        SecondaryVoiceComponent->bAllowSpatialization = true;
        SecondaryVoiceComponent->RegisterComponent();
    }

    // Create breathing component
    BreathingComponent = NewObject<UAudioComponent>(Owner);
    if (BreathingComponent)
    {
        BreathingComponent->SetupAttachment(Owner->GetRootComponent());
        BreathingComponent->bAutoActivate = false;
        BreathingComponent->bAllowSpatialization = true;
        BreathingComponent->RegisterComponent();
    }

    // Create footsteps component
    FootstepsComponent = NewObject<UAudioComponent>(Owner);
    if (FootstepsComponent)
    {
        FootstepsComponent->SetupAttachment(Owner->GetRootComponent());
        FootstepsComponent->bAutoActivate = false;
        FootstepsComponent->bAllowSpatialization = true;
        FootstepsComponent->RegisterComponent();
    }
}

void UCreatureAudioComponent::ScheduleNextVocalization()
{
    if (!bCanVocalize)
    {
        return;
    }

    float NextVocalizationTime = FMath::RandRange(VocalizationInterval.X, VocalizationInterval.Y);
    
    // Adjust based on behavior state
    switch (CurrentBehaviorState)
    {
        case ECreatureBehaviorState::Alert:
        case ECreatureBehaviorState::Hunting:
        case ECreatureBehaviorState::Territorial:
            NextVocalizationTime *= 0.5f; // More frequent
            break;
        case ECreatureBehaviorState::Feeding:
            NextVocalizationTime *= 2.0f; // Less frequent
            break;
        default:
            break;
    }
    
    NextVocalizationTime = FMath::Clamp(NextVocalizationTime, MinVocalizationInterval, MaxVocalizationInterval);
    
    GetWorld()->GetTimerManager().SetTimer(
        VocalizationTimer,
        this,
        &UCreatureAudioComponent::ProcessVocalization,
        NextVocalizationTime,
        false
    );
}

float UCreatureAudioComponent::GetBehaviorStateVolumeMultiplier() const
{
    switch (CurrentBehaviorState)
    {
        case ECreatureBehaviorState::Hunting:
        case ECreatureBehaviorState::Territorial:
            return 1.5f;
        case ECreatureBehaviorState::Alert:
            return 1.2f;
        case ECreatureBehaviorState::Mating:
            return 1.3f;
        case ECreatureBehaviorState::Fleeing:
            return 1.1f;
        case ECreatureBehaviorState::Feeding:
            return 0.7f;
        case ECreatureBehaviorState::Passive:
        default:
            return 1.0f;
    }
}

float UCreatureAudioComponent::GetDistanceVolumeMultiplier() const
{
    if (DistanceToPlayer <= 0.0f)
    {
        return 1.0f;
    }
    
    float MaxDistance = AudioProfile.MaxAudibleDistance;
    float NormalizedDistance = FMath::Clamp(DistanceToPlayer / MaxDistance, 0.0f, 1.0f);
    
    // Use inverse square law for realistic audio falloff
    return FMath::Pow(1.0f - NormalizedDistance, 2.0f);
}