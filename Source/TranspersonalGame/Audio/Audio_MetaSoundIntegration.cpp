#include "Audio_MetaSoundIntegration.h"
#include "Components/AudioComponent.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"

UAudio_MetaSoundIntegration::UAudio_MetaSoundIntegration()
{
    PrimaryComponentTick.bCanEverTick = true;
    
    // Initialize default values
    NarrativeAudioCooldown = 5.0f;
    bEnableNarrativeIntegration = true;
    LastNarrativeAudioTime = 0.0f;
    bIsPlayingNarrativeAudio = false;
    CurrentAudioIntensity = 0.5f;
    CurrentTriggerType = EAudio_MetaSoundTrigger::None;
    
    // Create audio components
    PrimaryAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("PrimaryAudioComponent"));
    SecondaryAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("SecondaryAudioComponent"));
    EnvironmentalAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("EnvironmentalAudioComponent"));
    
    // Configure audio components
    if (PrimaryAudioComponent)
    {
        PrimaryAudioComponent->bAutoActivate = false;
        PrimaryAudioComponent->bStopWhenOwnerDestroyed = true;
        PrimaryAudioComponent->SetVolumeMultiplier(1.0f);
    }
    
    if (SecondaryAudioComponent)
    {
        SecondaryAudioComponent->bAutoActivate = false;
        SecondaryAudioComponent->bStopWhenOwnerDestroyed = true;
        SecondaryAudioComponent->SetVolumeMultiplier(0.8f);
    }
    
    if (EnvironmentalAudioComponent)
    {
        EnvironmentalAudioComponent->bAutoActivate = false;
        EnvironmentalAudioComponent->bStopWhenOwnerDestroyed = true;
        EnvironmentalAudioComponent->SetVolumeMultiplier(0.6f);
    }
}

void UAudio_MetaSoundIntegration::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeAudioComponents();
    LoadMetaSoundAssets();
    
    UE_LOG(LogTemp, Log, TEXT("Audio_MetaSoundIntegration: Component initialized successfully"));
}

void UAudio_MetaSoundIntegration::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Update narrative audio state
    if (bIsPlayingNarrativeAudio && PrimaryAudioComponent && !PrimaryAudioComponent->IsPlaying())
    {
        bIsPlayingNarrativeAudio = false;
        CurrentNarrativeCharacter.Empty();
        CurrentTriggerType = EAudio_MetaSoundTrigger::None;
        UE_LOG(LogTemp, Log, TEXT("Narrative audio playback completed"));
    }
}

void UAudio_MetaSoundIntegration::InitializeAudioComponents()
{
    // Attach audio components to owner's root component
    if (AActor* Owner = GetOwner())
    {
        if (USceneComponent* RootComp = Owner->GetRootComponent())
        {
            if (PrimaryAudioComponent)
            {
                PrimaryAudioComponent->AttachToComponent(RootComp, FAttachmentTransformRules::KeepRelativeTransform);
                PrimaryAudioComponent->SetRelativeLocation(FVector::ZeroVector);
            }
            
            if (SecondaryAudioComponent)
            {
                SecondaryAudioComponent->AttachToComponent(RootComp, FAttachmentTransformRules::KeepRelativeTransform);
                SecondaryAudioComponent->SetRelativeLocation(FVector(0.0f, 100.0f, 0.0f));
            }
            
            if (EnvironmentalAudioComponent)
            {
                EnvironmentalAudioComponent->AttachToComponent(RootComp, FAttachmentTransformRules::KeepRelativeTransform);
                EnvironmentalAudioComponent->SetRelativeLocation(FVector(0.0f, 0.0f, 200.0f));
            }
        }
    }
}

void UAudio_MetaSoundIntegration::LoadMetaSoundAssets()
{
    // Load MetaSound assets asynchronously
    if (!ProximityAlertMetaSound.IsNull())
    {
        ProximityAlertMetaSound.LoadSynchronous();
        UE_LOG(LogTemp, Log, TEXT("Proximity Alert MetaSound loaded"));
    }
    
    if (!MedicalEmergencyMetaSound.IsNull())
    {
        MedicalEmergencyMetaSound.LoadSynchronous();
        UE_LOG(LogTemp, Log, TEXT("Medical Emergency MetaSound loaded"));
    }
    
    if (!EnvironmentalHazardMetaSound.IsNull())
    {
        EnvironmentalHazardMetaSound.LoadSynchronous();
        UE_LOG(LogTemp, Log, TEXT("Environmental Hazard MetaSound loaded"));
    }
}

UAudioComponent* UAudio_MetaSoundIntegration::GetAvailableAudioComponent()
{
    if (PrimaryAudioComponent && !PrimaryAudioComponent->IsPlaying())
    {
        return PrimaryAudioComponent;
    }
    
    if (SecondaryAudioComponent && !SecondaryAudioComponent->IsPlaying())
    {
        return SecondaryAudioComponent;
    }
    
    if (EnvironmentalAudioComponent && !EnvironmentalAudioComponent->IsPlaying())
    {
        return EnvironmentalAudioComponent;
    }
    
    // If all are busy, interrupt the secondary component
    if (SecondaryAudioComponent)
    {
        SecondaryAudioComponent->Stop();
        return SecondaryAudioComponent;
    }
    
    return nullptr;
}

void UAudio_MetaSoundIntegration::UpdateAudioComponentSettings(UAudioComponent* AudioComp, const FAudio_NarrativeAudioData& AudioData)
{
    if (!AudioComp)
    {
        return;
    }
    
    // Configure 3D audio settings
    if (AudioData.bIs3DPositional)
    {
        AudioComp->SetVolumeMultiplier(1.0f);
        AudioComp->SetPitchMultiplier(1.0f);
        AudioComp->bAllowSpatialization = true;
        AudioComp->bOverrideAttenuation = true;
        
        // Create attenuation settings for 3D audio
        if (AudioComp->AttenuationSettings)
        {
            AudioComp->AttenuationSettings->Attenuation.bAttenuate = true;
            AudioComp->AttenuationSettings->Attenuation.AttenuationShape = EAttenuationShape::Sphere;
            AudioComp->AttenuationSettings->Attenuation.AttenuationShapeExtents = FVector(AudioData.MaxAudibleDistance);
        }
    }
    else
    {
        AudioComp->bAllowSpatialization = false;
        AudioComp->bOverrideAttenuation = false;
    }
}

void UAudio_MetaSoundIntegration::TriggerMetaSoundEvent(EAudio_MetaSoundTrigger TriggerType, float Intensity)
{
    CurrentTriggerType = TriggerType;
    CurrentAudioIntensity = FMath::Clamp(Intensity, 0.0f, 1.0f);
    
    UMetaSoundSource* TargetMetaSound = nullptr;
    
    switch (TriggerType)
    {
        case EAudio_MetaSoundTrigger::ProximityAlert:
            TargetMetaSound = ProximityAlertMetaSound.Get();
            break;
        case EAudio_MetaSoundTrigger::MedicalEmergency:
            TargetMetaSound = MedicalEmergencyMetaSound.Get();
            break;
        case EAudio_MetaSoundTrigger::EnvironmentalHazard:
            TargetMetaSound = EnvironmentalHazardMetaSound.Get();
            break;
        default:
            UE_LOG(LogTemp, Warning, TEXT("Unknown MetaSound trigger type: %d"), static_cast<int32>(TriggerType));
            return;
    }
    
    if (TargetMetaSound)
    {
        UAudioComponent* AudioComp = GetAvailableAudioComponent();
        if (AudioComp)
        {
            AudioComp->SetSound(TargetMetaSound);
            AudioComp->SetVolumeMultiplier(CurrentAudioIntensity);
            AudioComp->Play();
            
            UE_LOG(LogTemp, Log, TEXT("MetaSound triggered: %d with intensity: %f"), static_cast<int32>(TriggerType), Intensity);
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("MetaSound asset not found for trigger type: %d"), static_cast<int32>(TriggerType));
    }
}

void UAudio_MetaSoundIntegration::SetMetaSoundParameter(const FString& ParameterName, float Value)
{
    // Find the appropriate MetaSound config
    for (const FAudio_MetaSoundConfig& Config : MetaSoundConfigs)
    {
        if (Config.ParameterName == ParameterName)
        {
            float ClampedValue = FMath::Clamp(Value, Config.MinValue, Config.MaxValue);
            
            // Apply parameter to active audio components
            if (PrimaryAudioComponent && PrimaryAudioComponent->IsPlaying())
            {
                // Note: MetaSound parameter setting would require MetaSound-specific implementation
                UE_LOG(LogTemp, Log, TEXT("Setting MetaSound parameter %s to %f"), *ParameterName, ClampedValue);
            }
            
            return;
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("MetaSound parameter not found: %s"), *ParameterName);
}

void UAudio_MetaSoundIntegration::PlayNarrativeAudio(const FString& CharacterName, const FString& AudioURL, bool bInterrupt)
{
    if (!bEnableNarrativeIntegration)
    {
        return;
    }
    
    // Check cooldown
    float CurrentTime = GetWorld()->GetTimeSeconds();
    if (!bInterrupt && (CurrentTime - LastNarrativeAudioTime) < NarrativeAudioCooldown)
    {
        UE_LOG(LogTemp, Log, TEXT("Narrative audio on cooldown, ignoring request"));
        return;
    }
    
    // Find audio data for this character
    const FAudio_NarrativeAudioData* AudioData = nullptr;
    for (const FAudio_NarrativeAudioData& Data : NarrativeAudioLibrary)
    {
        if (Data.CharacterName == CharacterName && Data.AudioURL == AudioURL)
        {
            AudioData = &Data;
            break;
        }
    }
    
    if (!AudioData)
    {
        UE_LOG(LogTemp, Warning, TEXT("Narrative audio data not found for character: %s"), *CharacterName);
        return;
    }
    
    UAudioComponent* AudioComp = GetAvailableAudioComponent();
    if (AudioComp)
    {
        UpdateAudioComponentSettings(AudioComp, *AudioData);
        
        // Note: In a real implementation, you would load the audio from the URL
        // For now, we'll just log the action
        UE_LOG(LogTemp, Log, TEXT("Playing narrative audio for %s: %s"), *CharacterName, *AudioURL);
        
        bIsPlayingNarrativeAudio = true;
        CurrentNarrativeCharacter = CharacterName;
        LastNarrativeAudioTime = CurrentTime;
    }
}

void UAudio_MetaSoundIntegration::StopAllNarrativeAudio()
{
    if (PrimaryAudioComponent)
    {
        PrimaryAudioComponent->Stop();
    }
    
    if (SecondaryAudioComponent)
    {
        SecondaryAudioComponent->Stop();
    }
    
    if (EnvironmentalAudioComponent)
    {
        EnvironmentalAudioComponent->Stop();
    }
    
    bIsPlayingNarrativeAudio = false;
    CurrentNarrativeCharacter.Empty();
    CurrentTriggerType = EAudio_MetaSoundTrigger::None;
    
    UE_LOG(LogTemp, Log, TEXT("All narrative audio stopped"));
}

bool UAudio_MetaSoundIntegration::IsNarrativeAudioPlaying() const
{
    return bIsPlayingNarrativeAudio;
}

void UAudio_MetaSoundIntegration::OnEmergencyDialogueTriggered(EAudio_MetaSoundTrigger EmergencyType, float UrgencyLevel)
{
    UE_LOG(LogTemp, Log, TEXT("Emergency dialogue triggered: %d with urgency: %f"), static_cast<int32>(EmergencyType), UrgencyLevel);
    
    // Trigger corresponding MetaSound event
    TriggerMetaSoundEvent(EmergencyType, UrgencyLevel);
    
    // Find and play appropriate narrative audio
    for (const FAudio_NarrativeAudioData& AudioData : NarrativeAudioLibrary)
    {
        if (AudioData.TriggerType == EmergencyType)
        {
            PlayNarrativeAudio(AudioData.CharacterName, AudioData.AudioURL, true);
            break;
        }
    }
}

void UAudio_MetaSoundIntegration::RegisterNarrativeAudio(const FString& AudioURL, const FString& CharacterName, EAudio_MetaSoundTrigger TriggerType)
{
    FAudio_NarrativeAudioData NewAudioData;
    NewAudioData.AudioURL = AudioURL;
    NewAudioData.CharacterName = CharacterName;
    NewAudioData.TriggerType = TriggerType;
    NewAudioData.Duration = 20.0f; // Default duration
    NewAudioData.bIs3DPositional = true;
    NewAudioData.MaxAudibleDistance = 2000.0f;
    
    NarrativeAudioLibrary.Add(NewAudioData);
    
    UE_LOG(LogTemp, Log, TEXT("Registered narrative audio: %s for character: %s"), *AudioURL, *CharacterName);
}

void UAudio_MetaSoundIntegration::Set3DAudioPosition(FVector WorldPosition)
{
    if (AActor* Owner = GetOwner())
    {
        Owner->SetActorLocation(WorldPosition);
        UE_LOG(LogTemp, Log, TEXT("3D Audio position set to: %s"), *WorldPosition.ToString());
    }
}

void UAudio_MetaSoundIntegration::Enable3DAudio(bool bEnable)
{
    if (PrimaryAudioComponent)
    {
        PrimaryAudioComponent->bAllowSpatialization = bEnable;
    }
    
    if (SecondaryAudioComponent)
    {
        SecondaryAudioComponent->bAllowSpatialization = bEnable;
    }
    
    if (EnvironmentalAudioComponent)
    {
        EnvironmentalAudioComponent->bAllowSpatialization = bEnable;
    }
    
    UE_LOG(LogTemp, Log, TEXT("3D Audio %s"), bEnable ? TEXT("enabled") : TEXT("disabled"));
}

void UAudio_MetaSoundIntegration::SetAudioIntensity(float Intensity)
{
    CurrentAudioIntensity = FMath::Clamp(Intensity, 0.0f, 1.0f);
    
    if (PrimaryAudioComponent)
    {
        PrimaryAudioComponent->SetVolumeMultiplier(CurrentAudioIntensity);
    }
    
    if (SecondaryAudioComponent)
    {
        SecondaryAudioComponent->SetVolumeMultiplier(CurrentAudioIntensity * 0.8f);
    }
    
    if (EnvironmentalAudioComponent)
    {
        EnvironmentalAudioComponent->SetVolumeMultiplier(CurrentAudioIntensity * 0.6f);
    }
}

float UAudio_MetaSoundIntegration::GetCurrentAudioIntensity() const
{
    return CurrentAudioIntensity;
}