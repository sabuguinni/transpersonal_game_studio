#include "Audio_DinosaurSoundSystem.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"

UAudio_DinosaurSoundSystem::UAudio_DinosaurSoundSystem()
{
    PrimaryComponentTick.bCanEverTick = true;

    // Initialize default values
    CurrentType = EAudio_DinosaurType::TRex;
    CurrentState = EAudio_DinosaurState::Idle;
    VolumeMultiplier = 1.0f;
    MovementSpeed = 0.0f;
    FootstepThreshold = 50.0f;
    LastFootstepTime = 0.0f;
    StateChangeTime = 0.0f;
    bIsMoving = false;

    // Initialize dinosaur sound data
    FAudio_DinosaurSoundData TRexData;
    TRexData.BaseVolume = 1.0f;
    TRexData.HearingRange = 5000.0f;
    TRexData.FootstepInterval = 2.0f;
    DinosaurSoundMap.Add(EAudio_DinosaurType::TRex, TRexData);

    FAudio_DinosaurSoundData RaptorData;
    RaptorData.BaseVolume = 0.7f;
    RaptorData.HearingRange = 1500.0f;
    RaptorData.FootstepInterval = 0.8f;
    DinosaurSoundMap.Add(EAudio_DinosaurType::Raptor, RaptorData);

    FAudio_DinosaurSoundData TriceratopsData;
    TriceratopsData.BaseVolume = 0.9f;
    TriceratopsData.HearingRange = 3000.0f;
    TriceratopsData.FootstepInterval = 1.8f;
    DinosaurSoundMap.Add(EAudio_DinosaurType::Triceratops, TriceratopsData);

    FAudio_DinosaurSoundData BrachiosaurusData;
    BrachiosaurusData.BaseVolume = 1.2f;
    BrachiosaurusData.HearingRange = 8000.0f;
    BrachiosaurusData.FootstepInterval = 3.0f;
    DinosaurSoundMap.Add(EAudio_DinosaurType::Brachiosaurus, BrachiosaurusData);

    FAudio_DinosaurSoundData PteranodonData;
    PteranodonData.BaseVolume = 0.5f;
    PteranodonData.HearingRange = 2000.0f;
    PteranodonData.FootstepInterval = 0.0f; // Flying creature
    DinosaurSoundMap.Add(EAudio_DinosaurType::Pteranodon, PteranodonData);
}

void UAudio_DinosaurSoundSystem::BeginPlay()
{
    Super::BeginPlay();

    // Create audio components dynamically
    AActor* Owner = GetOwner();
    if (Owner)
    {
        VocalizationComponent = NewObject<UAudioComponent>(Owner, TEXT("DinosaurVocalization"));
        if (VocalizationComponent)
        {
            VocalizationComponent->AttachToComponent(Owner->GetRootComponent(), 
                FAttachmentTransformRules::KeepRelativeTransform);
            VocalizationComponent->RegisterComponent();
            VocalizationComponent->bAutoActivate = false;
        }

        FootstepComponent = NewObject<UAudioComponent>(Owner, TEXT("DinosaurFootstep"));
        if (FootstepComponent)
        {
            FootstepComponent->AttachToComponent(Owner->GetRootComponent(), 
                FAttachmentTransformRules::KeepRelativeTransform);
            FootstepComponent->RegisterComponent();
            FootstepComponent->bAutoActivate = false;
        }

        BreathingComponent = NewObject<UAudioComponent>(Owner, TEXT("DinosaurBreathing"));
        if (BreathingComponent)
        {
            BreathingComponent->AttachToComponent(Owner->GetRootComponent(), 
                FAttachmentTransformRules::KeepRelativeTransform);
            BreathingComponent->RegisterComponent();
            BreathingComponent->bAutoActivate = false;
        }
    }

    StateChangeTime = GetWorld()->GetTimeSeconds();
}

void UAudio_DinosaurSoundSystem::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    // Clean up audio components
    if (VocalizationComponent)
    {
        VocalizationComponent->Stop();
    }
    if (FootstepComponent)
    {
        FootstepComponent->Stop();
    }
    if (BreathingComponent)
    {
        BreathingComponent->Stop();
    }

    Super::EndPlay(EndPlayReason);
}

void UAudio_DinosaurSoundSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    // Update movement state
    bIsMoving = MovementSpeed > FootstepThreshold;

    // Update footstep timing
    UpdateFootstepTiming();

    // Update breathing intensity based on state
    UpdateBreathingIntensity();
}

void UAudio_DinosaurSoundSystem::SetDinosaurType(EAudio_DinosaurType NewType)
{
    if (NewType != CurrentType)
    {
        CurrentType = NewType;
        
        // Update audio component settings based on new type
        if (DinosaurSoundMap.Contains(CurrentType))
        {
            const FAudio_DinosaurSoundData& SoundData = DinosaurSoundMap[CurrentType];
            
            if (VocalizationComponent)
            {
                VocalizationComponent->SetVolumeMultiplier(SoundData.BaseVolume * VolumeMultiplier);
            }
            if (FootstepComponent)
            {
                FootstepComponent->SetVolumeMultiplier(SoundData.BaseVolume * VolumeMultiplier);
            }
            if (BreathingComponent)
            {
                BreathingComponent->SetVolumeMultiplier(SoundData.BaseVolume * 0.3f * VolumeMultiplier);
            }
        }
    }
}

void UAudio_DinosaurSoundSystem::SetDinosaurState(EAudio_DinosaurState NewState)
{
    if (NewState != CurrentState)
    {
        CurrentState = NewState;
        StateChangeTime = GetWorld()->GetTimeSeconds();
        
        // Play appropriate sound for state change
        PlayStateSound();
    }
}

void UAudio_DinosaurSoundSystem::PlayStateSound()
{
    if (!VocalizationComponent || !DinosaurSoundMap.Contains(CurrentType))
    {
        return;
    }

    const FAudio_DinosaurSoundData& SoundData = DinosaurSoundMap[CurrentType];
    
    // Stop current vocalization
    VocalizationComponent->Stop();
    
    // Play appropriate sound based on current state
    // In a full implementation, this would load and play actual sound assets
    switch (CurrentState)
    {
        case EAudio_DinosaurState::Alert:
            // Play alert sound
            if (SoundData.AlertSound.IsValid())
            {
                // VocalizationComponent->SetSound(SoundData.AlertSound.LoadSynchronous());
                VocalizationComponent->Play();
            }
            break;
            
        case EAudio_DinosaurState::Hunting:
        case EAudio_DinosaurState::Territorial:
            // Play attack/territorial sound
            if (SoundData.AttackSound.IsValid())
            {
                // VocalizationComponent->SetSound(SoundData.AttackSound.LoadSynchronous());
                VocalizationComponent->Play();
            }
            break;
            
        case EAudio_DinosaurState::Idle:
        case EAudio_DinosaurState::Feeding:
        default:
            // Play idle sound occasionally
            if (SoundData.IdleSound.IsValid() && FMath::RandRange(0.0f, 1.0f) < 0.3f)
            {
                // VocalizationComponent->SetSound(SoundData.IdleSound.LoadSynchronous());
                VocalizationComponent->Play();
            }
            break;
    }
}

void UAudio_DinosaurSoundSystem::PlayFootstepSound()
{
    if (!FootstepComponent || !DinosaurSoundMap.Contains(CurrentType))
    {
        return;
    }

    const FAudio_DinosaurSoundData& SoundData = DinosaurSoundMap[CurrentType];
    
    // Play footstep sound if available
    if (SoundData.FootstepSound.IsValid())
    {
        FootstepComponent->Stop();
        // FootstepComponent->SetSound(SoundData.FootstepSound.LoadSynchronous());
        FootstepComponent->Play();
    }
    
    LastFootstepTime = GetWorld()->GetTimeSeconds();
}

void UAudio_DinosaurSoundSystem::SetMovementSpeed(float Speed)
{
    MovementSpeed = FMath::Max(0.0f, Speed);
}

void UAudio_DinosaurSoundSystem::TriggerTerritorialRoar()
{
    SetDinosaurState(EAudio_DinosaurState::Territorial);
}

void UAudio_DinosaurSoundSystem::SetVolumeMultiplier(float Multiplier)
{
    VolumeMultiplier = FMath::Clamp(Multiplier, 0.0f, 2.0f);
    
    // Update all component volumes
    if (DinosaurSoundMap.Contains(CurrentType))
    {
        const FAudio_DinosaurSoundData& SoundData = DinosaurSoundMap[CurrentType];
        
        if (VocalizationComponent)
        {
            VocalizationComponent->SetVolumeMultiplier(SoundData.BaseVolume * VolumeMultiplier);
        }
        if (FootstepComponent)
        {
            FootstepComponent->SetVolumeMultiplier(SoundData.BaseVolume * VolumeMultiplier);
        }
        if (BreathingComponent)
        {
            BreathingComponent->SetVolumeMultiplier(SoundData.BaseVolume * 0.3f * VolumeMultiplier);
        }
    }
}

void UAudio_DinosaurSoundSystem::UpdateFootstepTiming()
{
    if (!bIsMoving || !DinosaurSoundMap.Contains(CurrentType))
    {
        return;
    }

    const FAudio_DinosaurSoundData& SoundData = DinosaurSoundMap[CurrentType];
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    // Calculate footstep interval based on movement speed
    float SpeedMultiplier = MovementSpeed / 600.0f; // Normalize to typical walking speed
    float AdjustedInterval = SoundData.FootstepInterval / FMath::Max(0.5f, SpeedMultiplier);
    
    if (CurrentTime - LastFootstepTime >= AdjustedInterval)
    {
        PlayFootstepSound();
    }
}

void UAudio_DinosaurSoundSystem::UpdateBreathingIntensity()
{
    if (!BreathingComponent || !DinosaurSoundMap.Contains(CurrentType))
    {
        return;
    }

    // Adjust breathing based on current state and movement
    float BreathingIntensity = 0.3f; // Base breathing
    
    switch (CurrentState)
    {
        case EAudio_DinosaurState::Alert:
        case EAudio_DinosaurState::Hunting:
            BreathingIntensity = 0.7f;
            break;
        case EAudio_DinosaurState::Territorial:
            BreathingIntensity = 1.0f;
            break;
        case EAudio_DinosaurState::Wounded:
            BreathingIntensity = 0.9f;
            break;
        default:
            BreathingIntensity = 0.3f;
            break;
    }
    
    // Increase breathing with movement
    if (bIsMoving)
    {
        BreathingIntensity += (MovementSpeed / 1000.0f) * 0.4f;
    }
    
    BreathingIntensity = FMath::Clamp(BreathingIntensity, 0.1f, 1.0f);
    
    if (BreathingComponent)
    {
        BreathingComponent->SetVolumeMultiplier(BreathingIntensity * VolumeMultiplier);
        
        // Start breathing if not already playing
        if (!BreathingComponent->IsPlaying())
        {
            BreathingComponent->Play();
        }
    }
}