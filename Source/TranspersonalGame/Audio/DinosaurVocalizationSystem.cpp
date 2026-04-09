#include "DinosaurVocalizationSystem.h"
#include "Components/AudioComponent.h"
#include "Engine/World.h"
#include "Engine/DataTable.h"
#include "Sound/SoundCue.h"
#include "Kismet/GameplayStatics.h"
#include "AudioSystemManager.h"
#include "GameFramework/Actor.h"
#include "Engine/Engine.h"
#include "TimerManager.h"

DEFINE_LOG_CATEGORY_STATIC(LogDinosaurVocalization, Log, All);

UDinosaurVocalizationSystem::UDinosaurVocalizationSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Check vocalizations every 100ms
    
    // Create audio component
    VocalizationAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("VocalizationAudioComponent"));
    
    // Initialize voice profile defaults
    VoiceProfile.DinosaurSpecies = TEXT("Unknown");
    VoiceProfile.Size = EDinosaurSize::Medium;
    VoiceProfile.VoicePitchVariation = 0.2f;
    VoiceProfile.VolumeVariation = 0.1f;
    VoiceProfile.AggressionModifier = 1.0f;
    VoiceProfile.bIsAlpha = false;
    
    // Set default vocalization frequencies
    VoiceProfile.VocalizationFrequencies.Add(EVocalizationType::Idle, 0.1f);
    VoiceProfile.VocalizationFrequencies.Add(EVocalizationType::Alert, 0.8f);
    VoiceProfile.VocalizationFrequencies.Add(EVocalizationType::Aggressive, 0.9f);
    VoiceProfile.VocalizationFrequencies.Add(EVocalizationType::Hunting, 0.6f);
    VoiceProfile.VocalizationFrequencies.Add(EVocalizationType::Feeding, 0.3f);
    VoiceProfile.VocalizationFrequencies.Add(EVocalizationType::Territorial, 0.7f);
    VoiceProfile.VocalizationFrequencies.Add(EVocalizationType::Distress, 1.0f);
    VoiceProfile.VocalizationFrequencies.Add(EVocalizationType::Warning, 0.9f);
}

void UDinosaurVocalizationSystem::BeginPlay()
{
    Super::BeginPlay();
    
    // Cache vocalization data for performance
    CacheVocalizationData();
    
    // Set up audio component
    if (VocalizationAudioComponent)
    {
        VocalizationAudioComponent->SetWorldLocation(GetOwner()->GetActorLocation());
        VocalizationAudioComponent->bAutoActivate = false;
        VocalizationAudioComponent->bStopWhenOwnerDestroyed = true;
    }
    
    // Initialize timing
    LastVocalizationTime = GetWorld()->GetTimeSeconds();
    StateChangeTime = LastVocalizationTime;
    NextIdleVocalizationTime = LastVocalizationTime + FMath::RandRange(5.0f, 15.0f);
    
    UE_LOG(LogDinosaurVocalization, Log, TEXT("DinosaurVocalizationSystem initialized for %s"), 
           *VoiceProfile.DinosaurSpecies);
}

void UDinosaurVocalizationSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!GetWorld())
        return;
    
    UpdateIdleVocalizations(DeltaTime);
    UpdateVocalizationState(DeltaTime);
    
    // Update audio component position
    if (VocalizationAudioComponent && GetOwner())
    {
        VocalizationAudioComponent->SetWorldLocation(GetOwner()->GetActorLocation());
    }
}

void UDinosaurVocalizationSystem::CacheVocalizationData()
{
    if (!VocalizationDataTable)
    {
        UE_LOG(LogDinosaurVocalization, Warning, TEXT("No VocalizationDataTable set for %s"), 
               *GetOwner()->GetName());
        return;
    }
    
    CachedVocalizationData.Empty();
    
    // Cache all vocalization data for quick lookup
    static const FString ContextString(TEXT("DinosaurVocalizationSystem"));
    TArray<FVocalizationData*> AllRows;
    VocalizationDataTable->GetAllRows<FVocalizationData>(ContextString, AllRows);
    
    for (FVocalizationData* Row : AllRows)
    {
        if (Row && Row->DinosaurSize == VoiceProfile.Size)
        {
            CachedVocalizationData.Add(Row->VocalizationType, Row);
        }
    }
    
    UE_LOG(LogDinosaurVocalization, Log, TEXT("Cached %d vocalization entries for size %d"), 
           CachedVocalizationData.Num(), (int32)VoiceProfile.Size);
}

void UDinosaurVocalizationSystem::TriggerVocalization(EVocalizationType VocalizationType, bool bForcePlay)
{
    const float CurrentTime = GetWorld()->GetTimeSeconds();
    
    // Check timing constraints unless forced
    if (!bForcePlay)
    {
        if (bIsVocalizing)
        {
            UE_LOG(LogDinosaurVocalization, VeryVerbose, TEXT("Already vocalizing, skipping %d"), 
                   (int32)VocalizationType);
            return;
        }
        
        if (CurrentTime - LastVocalizationTime < MinTimeBetweenVocalizations)
        {
            UE_LOG(LogDinosaurVocalization, VeryVerbose, TEXT("Too soon since last vocalization"));
            return;
        }
    }
    
    FVocalizationData* VocalizationData = GetVocalizationData(VocalizationType);
    if (!VocalizationData)
    {
        UE_LOG(LogDinosaurVocalization, Warning, TEXT("No vocalization data found for type %d"), 
               (int32)VocalizationType);
        return;
    }
    
    // Check probability
    const float VocalizationChance = CalculateVocalizationChance(VocalizationType);
    if (!bForcePlay && FMath::RandRange(0.0f, 1.0f) > VocalizationChance)
    {
        UE_LOG(LogDinosaurVocalization, VeryVerbose, TEXT("Vocalization chance failed: %f"), 
               VocalizationChance);
        return;
    }
    
    // Play the sound
    PlayVocalizationSound(*VocalizationData);
    
    // Update state
    CurrentVocalizationState = VocalizationType;
    LastVocalizationTime = CurrentTime;
    StateChangeTime = CurrentTime;
    
    // Notify other systems
    NotifyNearbyDinosaurs(VocalizationType);
    NotifyAudioSystem(VocalizationType);
    
    UE_LOG(LogDinosaurVocalization, Log, TEXT("%s triggered vocalization: %d"), 
           *GetOwner()->GetName(), (int32)VocalizationType);
}

void UDinosaurVocalizationSystem::SetVocalizationState(EVocalizationType NewState)
{
    if (CurrentVocalizationState != NewState)
    {
        CurrentVocalizationState = NewState;
        StateChangeTime = GetWorld()->GetTimeSeconds();
        
        UE_LOG(LogDinosaurVocalization, Log, TEXT("%s changed vocalization state to: %d"), 
               *GetOwner()->GetName(), (int32)NewState);
    }
}

void UDinosaurVocalizationSystem::OnSpotPlayer(float Distance)
{
    EVocalizationType VocalizationType = EVocalizationType::Alert;
    
    // Determine vocalization type based on distance and dinosaur characteristics
    if (Distance < 500.0f && VoiceProfile.AggressionModifier > 0.7f)
    {
        VocalizationType = EVocalizationType::Aggressive;
    }
    else if (Distance < 1000.0f)
    {
        VocalizationType = EVocalizationType::Alert;
    }
    else
    {
        VocalizationType = EVocalizationType::Warning;
    }
    
    TriggerVocalization(VocalizationType, false);
    SetVocalizationState(VocalizationType);
}

void UDinosaurVocalizationSystem::OnEnterCombat()
{
    TriggerVocalization(EVocalizationType::Aggressive, true);
    SetVocalizationState(EVocalizationType::Aggressive);
}

void UDinosaurVocalizationSystem::OnTakeDamage()
{
    // Chance for distress call when taking damage
    if (FMath::RandRange(0.0f, 1.0f) < 0.7f)
    {
        TriggerVocalization(EVocalizationType::Distress, true);
    }
}

void UDinosaurVocalizationSystem::OnStartFeeding()
{
    SetVocalizationState(EVocalizationType::Feeding);
    
    // Occasional feeding sounds
    if (FMath::RandRange(0.0f, 1.0f) < 0.3f)
    {
        TriggerVocalization(EVocalizationType::Feeding, false);
    }
}

void UDinosaurVocalizationSystem::OnHearOtherDinosaur(UDinosaurVocalizationSystem* OtherDinosaur, EVocalizationType VocalizationType)
{
    if (!OtherDinosaur || OtherDinosaur == this)
        return;
    
    // React to other dinosaur vocalizations
    switch (VocalizationType)
    {
        case EVocalizationType::Warning:
        case EVocalizationType::Alert:
            // Become alert if not already in a more intense state
            if (CurrentVocalizationState == EVocalizationType::Idle)
            {
                SetVocalizationState(EVocalizationType::Alert);
            }
            break;
            
        case EVocalizationType::Aggressive:
            // Respond to aggression if territorial
            if (VoiceProfile.AggressionModifier > 0.5f && FMath::RandRange(0.0f, 1.0f) < 0.4f)
            {
                TriggerVocalization(EVocalizationType::Territorial, false);
            }
            break;
            
        case EVocalizationType::Distress:
            // Alert response to distress calls
            if (CurrentVocalizationState == EVocalizationType::Idle)
            {
                SetVocalizationState(EVocalizationType::Alert);
            }
            break;
            
        default:
            break;
    }
}

bool UDinosaurVocalizationSystem::CanHearVocalization(const FVector& SourceLocation, float MaxDistance) const
{
    if (!GetOwner())
        return false;
    
    const float Distance = FVector::Dist(GetOwner()->GetActorLocation(), SourceLocation);
    return Distance <= MaxDistance;
}

float UDinosaurVocalizationSystem::GetEmotionalImpactAtDistance(float Distance) const
{
    FVocalizationData* VocalizationData = GetVocalizationData(CurrentVocalizationState);
    if (!VocalizationData)
        return 0.0f;
    
    // Calculate emotional impact based on distance and vocalization type
    const float DistanceRatio = FMath::Clamp(Distance / VocalizationData->ThreatRadius, 0.0f, 1.0f);
    const float ImpactFalloff = 1.0f - (DistanceRatio * DistanceRatio); // Quadratic falloff
    
    return VocalizationData->EmotionalImpact * ImpactFalloff * VoiceProfile.AggressionModifier;
}

void UDinosaurVocalizationSystem::UpdateIdleVocalizations(float DeltaTime)
{
    if (CurrentVocalizationState != EVocalizationType::Idle)
        return;
    
    const float CurrentTime = GetWorld()->GetTimeSeconds();
    
    // Check for idle vocalizations
    if (CurrentTime >= NextIdleVocalizationTime)
    {
        const float IdleChance = IdleVocalizationChance * GetTimeOfDayModifier() * WeatherVocalizationModifier;
        
        if (FMath::RandRange(0.0f, 1.0f) < IdleChance)
        {
            TriggerVocalization(EVocalizationType::Idle, false);
        }
        
        // Schedule next idle vocalization check
        FVocalizationData* IdleData = GetVocalizationData(EVocalizationType::Idle);
        const float MinInterval = IdleData ? IdleData->MinInterval : 5.0f;
        const float MaxInterval = IdleData ? IdleData->MaxInterval : 15.0f;
        NextIdleVocalizationTime = CurrentTime + FMath::RandRange(MinInterval, MaxInterval);
    }
}

void UDinosaurVocalizationSystem::UpdateVocalizationState(float DeltaTime)
{
    const float CurrentTime = GetWorld()->GetTimeSeconds();
    
    // Auto-return to idle state after emotional cooldown
    if (CurrentVocalizationState != EVocalizationType::Idle)
    {
        if (CurrentTime - StateChangeTime > EmotionalCooldownTime)
        {
            SetVocalizationState(EVocalizationType::Idle);
        }
    }
    
    // Update vocalization status
    if (VocalizationAudioComponent)
    {
        bIsVocalizing = VocalizationAudioComponent->IsPlaying();
    }
}

void UDinosaurVocalizationSystem::PlayVocalizationSound(const FVocalizationData& VocalizationData)
{
    if (!VocalizationAudioComponent || VocalizationData.SoundVariations.Num() == 0)
        return;
    
    // Select a sound variation
    USoundCue* SelectedSound = SelectSoundVariation(VocalizationData.SoundVariations);
    if (!SelectedSound)
        return;
    
    // Calculate audio parameters with individual variation
    const float PitchVariation = FMath::RandRange(-VoiceProfile.VoicePitchVariation, VoiceProfile.VoicePitchVariation);
    const float VolumeVariation = FMath::RandRange(-VoiceProfile.VolumeVariation, VoiceProfile.VolumeVariation);
    
    const float FinalPitch = VocalizationData.BasePitch * (1.0f + PitchVariation);
    const float FinalVolume = VocalizationData.BaseVolume * (1.0f + VolumeVariation) * VoiceProfile.AggressionModifier;
    
    // Set audio component parameters
    VocalizationAudioComponent->SetSound(SelectedSound);
    VocalizationAudioComponent->SetPitchMultiplier(FinalPitch);
    VocalizationAudioComponent->SetVolumeMultiplier(FinalVolume);
    
    // Play the sound
    VocalizationAudioComponent->Play();
    
    UE_LOG(LogDinosaurVocalization, VeryVerbose, TEXT("Playing vocalization: Type=%d, Pitch=%f, Volume=%f"), 
           (int32)VocalizationData.VocalizationType, FinalPitch, FinalVolume);
}

FVocalizationData* UDinosaurVocalizationSystem::GetVocalizationData(EVocalizationType VocalizationType)
{
    return CachedVocalizationData.FindRef(VocalizationType);
}

void UDinosaurVocalizationSystem::NotifyNearbyDinosaurs(EVocalizationType VocalizationType)
{
    if (!GetWorld() || !GetOwner())
        return;
    
    FVocalizationData* VocalizationData = GetVocalizationData(VocalizationType);
    if (!VocalizationData)
        return;
    
    // Find nearby dinosaurs and notify them
    TArray<AActor*> NearbyActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), GetOwner()->GetClass(), NearbyActors);
    
    for (AActor* Actor : NearbyActors)
    {
        if (Actor == GetOwner())
            continue;
        
        UDinosaurVocalizationSystem* OtherVocalizationSystem = Actor->FindComponentByClass<UDinosaurVocalizationSystem>();
        if (OtherVocalizationSystem && 
            OtherVocalizationSystem->CanHearVocalization(GetOwner()->GetActorLocation(), VocalizationData->MaxAudibleDistance))
        {
            OtherVocalizationSystem->OnHearOtherDinosaur(this, VocalizationType);
        }
    }
}

void UDinosaurVocalizationSystem::NotifyAudioSystem(EVocalizationType VocalizationType)
{
    // Notify the central audio system about this vocalization for adaptive music/ambience
    if (UGameInstance* GameInstance = GetWorld()->GetGameInstance())
    {
        if (UAudioSystemManager* AudioManager = GameInstance->GetSubsystem<UAudioSystemManager>())
        {
            // Calculate emotional impact for audio system
            const float EmotionalImpact = GetEmotionalImpactAtDistance(0.0f); // Max impact at source
            
            // This could trigger changes in adaptive music based on vocalization type and intensity
            // Implementation would depend on the AudioSystemManager's interface
        }
    }
}

float UDinosaurVocalizationSystem::CalculateVocalizationChance(EVocalizationType VocalizationType) const
{
    float BaseChance = VoiceProfile.VocalizationFrequencies.FindRef(VocalizationType);
    
    // Modify based on various factors
    BaseChance *= GetTimeOfDayModifier();
    BaseChance *= WeatherVocalizationModifier;
    
    // Alpha individuals vocalize more frequently
    if (VoiceProfile.bIsAlpha)
    {
        BaseChance *= 1.3f;
    }
    
    return FMath::Clamp(BaseChance, 0.0f, 1.0f);
}

float UDinosaurVocalizationSystem::GetTimeOfDayModifier() const
{
    if (!GetWorld())
        return 1.0f;
    
    // Get time of day from world (this would need to be implemented based on your day/night system)
    // For now, assume a simple calculation
    const float GameTimeHours = FMath::Fmod(GetWorld()->GetTimeSeconds() / 3600.0f, 24.0f);
    
    // More vocalizations during dawn (5-7), dusk (18-20), and night (20-6)
    if ((GameTimeHours >= 5.0f && GameTimeHours <= 7.0f) ||
        (GameTimeHours >= 18.0f && GameTimeHours <= 20.0f))
    {
        return 1.5f; // Dawn/dusk activity
    }
    else if (GameTimeHours >= 20.0f || GameTimeHours <= 6.0f)
    {
        return NightVocalizationMultiplier; // Night activity
    }
    
    return 1.0f; // Day activity
}

USoundCue* UDinosaurVocalizationSystem::SelectSoundVariation(const TArray<USoundCue*>& SoundVariations) const
{
    if (SoundVariations.Num() == 0)
        return nullptr;
    
    // Simple random selection - could be enhanced with weighted selection or recent history tracking
    const int32 RandomIndex = FMath::RandRange(0, SoundVariations.Num() - 1);
    return SoundVariations[RandomIndex];
}