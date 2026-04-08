#include "DinosaurAudioComponent.h"
#include "AudioManager.h"
#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "TimerManager.h"

UDinosaurAudioComponent::UDinosaurAudioComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    
    // Default values
    Species = EDinosaurSpecies::TRex;
    LastPlayedSoundType = EDinosaurSoundType::Idle;
    TimeSinceLastCall = 0.0f;
    bIsVocalizing = false;
}

void UDinosaurAudioComponent::BeginPlay()
{
    Super::BeginPlay();
    
    // Create audio components
    PrimaryAudioComponent = NewObject<UAudioComponent>(GetOwner());
    SecondaryAudioComponent = NewObject<UAudioComponent>(GetOwner());
    
    if (PrimaryAudioComponent)
    {
        PrimaryAudioComponent->AttachToComponent(GetOwner()->GetRootComponent(), 
            FAttachmentTransformRules::KeepWorldTransform);
        PrimaryAudioComponent->RegisterComponent();
        PrimaryAudioComponent->bAutoActivate = false;
    }
    
    if (SecondaryAudioComponent)
    {
        SecondaryAudioComponent->AttachToComponent(GetOwner()->GetRootComponent(), 
            FAttachmentTransformRules::KeepWorldTransform);
        SecondaryAudioComponent->RegisterComponent();
        SecondaryAudioComponent->bAutoActivate = false;
    }
    
    // Find AudioManager
    AudioManager = Cast<AAudioManager>(UGameplayStatics::GetActorOfClass(GetWorld(), AAudioManager::StaticClass()));
    
    // Set up idle call timer
    if (VocalBehavior.IdleCallFrequency > 0.0f)
    {
        float RandomDelay = FMath::RandRange(5.0f, VocalBehavior.IdleCallFrequency);
        GetWorld()->GetTimerManager().SetTimer(IdleCallTimer, this, &UDinosaurAudioComponent::PlayIdleCall, 
            VocalBehavior.IdleCallFrequency, true, RandomDelay);
    }
}

void UDinosaurAudioComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    TimeSinceLastCall += DeltaTime;
    
    // Process different behavioral patterns
    ProcessIdleBehavior(DeltaTime);
    ProcessTerritorialBehavior();
    ProcessSocialBehavior();
    
    // Update vocalization state
    if (PrimaryAudioComponent && PrimaryAudioComponent->IsPlaying())
    {
        bIsVocalizing = true;
    }
    else
    {
        bIsVocalizing = false;
    }
}

void UDinosaurAudioComponent::PlaySound(EDinosaurSoundType SoundType, float VolumeMultiplier, float PitchMultiplier)
{
    if (!SoundSet.SoundCues.Contains(SoundType))
        return;
        
    USoundCue* SoundCue = SoundSet.SoundCues[SoundType];
    if (!SoundCue)
        return;
        
    UAudioComponent* ComponentToUse = PrimaryAudioComponent;
    
    // Use secondary component if primary is busy with important sounds
    if (PrimaryAudioComponent && PrimaryAudioComponent->IsPlaying() && 
        (LastPlayedSoundType == EDinosaurSoundType::Aggressive || 
         LastPlayedSoundType == EDinosaurSoundType::Pain ||
         LastPlayedSoundType == EDinosaurSoundType::Death))
    {
        ComponentToUse = SecondaryAudioComponent;
    }
    
    if (!ComponentToUse)
        return;
        
    // Calculate final audio parameters
    float FinalVolume = SoundSet.BaseVolume * VolumeMultiplier * GetRandomizedVolume();
    float FinalPitch = SoundSet.BasePitch * PitchMultiplier * GetRandomizedPitch();
    
    // Set up the audio component
    ComponentToUse->SetSound(SoundCue);
    ComponentToUse->SetVolumeMultiplier(FinalVolume);
    ComponentToUse->SetPitchMultiplier(FinalPitch);
    ComponentToUse->Play();
    
    // Update state
    LastPlayedSoundType = SoundType;
    TimeSinceLastCall = 0.0f;
    
    // Notify AudioManager for tension system
    if (AudioManager)
    {
        // Convert enum to string for AudioManager
        FString SoundTypeString;
        switch (SoundType)
        {
            case EDinosaurSoundType::Aggressive:
                SoundTypeString = TEXT("Aggressive");
                break;
            case EDinosaurSoundType::Alert:
                SoundTypeString = TEXT("Alert");
                break;
            case EDinosaurSoundType::Pain:
                SoundTypeString = TEXT("Pain");
                break;
            case EDinosaurSoundType::Territorial:
                SoundTypeString = TEXT("Territorial");
                break;
            default:
                SoundTypeString = TEXT("Idle");
                break;
        }
        
        // This will be called when Dinosaur class is available
        // AudioManager->PlayDinosaurSound(Cast<ADinosaur>(GetOwner()), SoundTypeString);
    }
}

void UDinosaurAudioComponent::PlayIdleCall()
{
    if (!bIsVocalizing && TimeSinceLastCall > 10.0f)
    {
        PlaySound(EDinosaurSoundType::Idle);
    }
}

void UDinosaurAudioComponent::PlayTerritorialCall()
{
    if (VocalBehavior.TerritorialCallRange > 0.0f)
    {
        PlaySound(EDinosaurSoundType::Territorial, 1.2f, 0.9f);
    }
}

void UDinosaurAudioComponent::PlaySocialCall()
{
    if (VocalBehavior.SocialCallRange > 0.0f)
    {
        PlaySound(EDinosaurSoundType::Social, 0.8f, 1.1f);
    }
}

void UDinosaurAudioComponent::StopAllSounds()
{
    if (PrimaryAudioComponent)
    {
        PrimaryAudioComponent->Stop();
    }
    
    if (SecondaryAudioComponent)
    {
        SecondaryAudioComponent->Stop();
    }
    
    bIsVocalizing = false;
}

void UDinosaurAudioComponent::SetSpecies(EDinosaurSpecies NewSpecies)
{
    Species = NewSpecies;
    
    // Adjust vocal behavior based on species
    switch (Species)
    {
        case EDinosaurSpecies::TRex:
            VocalBehavior.IdleCallFrequency = 45.0f;
            VocalBehavior.TerritorialCallRange = 3000.0f;
            SoundSet.MaxAudibleDistance = 8000.0f;
            break;
            
        case EDinosaurSpecies::Velociraptor:
            VocalBehavior.IdleCallFrequency = 20.0f;
            VocalBehavior.SocialCallRange = 1500.0f;
            SoundSet.MaxAudibleDistance = 2000.0f;
            break;
            
        case EDinosaurSpecies::Brachiosaurus:
            VocalBehavior.IdleCallFrequency = 60.0f;
            VocalBehavior.TerritorialCallRange = 5000.0f;
            SoundSet.MaxAudibleDistance = 10000.0f;
            break;
            
        case EDinosaurSpecies::Compsognathus:
            VocalBehavior.IdleCallFrequency = 15.0f;
            VocalBehavior.SocialCallRange = 500.0f;
            SoundSet.MaxAudibleDistance = 800.0f;
            break;
            
        default:
            // Keep default values
            break;
    }
}

bool UDinosaurAudioComponent::IsAudibleToPlayer() const
{
    APawn* Player = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (!Player)
        return false;
        
    float Distance = FVector::Dist(GetOwner()->GetActorLocation(), Player->GetActorLocation());
    return Distance <= SoundSet.MaxAudibleDistance;
}

void UDinosaurAudioComponent::ProcessIdleBehavior(float DeltaTime)
{
    // Random chance for spontaneous vocalizations
    if (!bIsVocalizing && TimeSinceLastCall > VocalBehavior.IdleCallFrequency * 0.5f)
    {
        float RandomChance = FMath::RandRange(0.0f, 1.0f);
        if (RandomChance < 0.001f) // Very low chance per frame
        {
            PlayIdleCall();
        }
    }
}

void UDinosaurAudioComponent::ProcessTerritorialBehavior()
{
    // Check for other dinosaurs in territorial range
    if (VocalBehavior.TerritorialCallRange <= 0.0f)
        return;
        
    // This would be implemented when other dinosaur detection is available
    // For now, occasional territorial calls
    if (!bIsVocalizing && TimeSinceLastCall > 120.0f)
    {
        float RandomChance = FMath::RandRange(0.0f, 1.0f);
        if (RandomChance < 0.1f)
        {
            PlayTerritorialCall();
        }
    }
}

void UDinosaurAudioComponent::ProcessSocialBehavior()
{
    // Check for same-species dinosaurs in social range
    if (VocalBehavior.SocialCallRange <= 0.0f)
        return;
        
    // This would be implemented when pack behavior is available
    // For now, occasional social calls
    if (!bIsVocalizing && TimeSinceLastCall > 90.0f)
    {
        float RandomChance = FMath::RandRange(0.0f, 1.0f);
        if (RandomChance < 0.05f)
        {
            PlaySocialCall();
        }
    }
}

float UDinosaurAudioComponent::GetRandomizedVolume() const
{
    float Variation = VocalBehavior.VolumeVariation;
    return FMath::RandRange(1.0f - Variation, 1.0f + Variation);
}

float UDinosaurAudioComponent::GetRandomizedPitch() const
{
    float Variation = VocalBehavior.PitchVariation;
    return FMath::RandRange(1.0f - Variation, 1.0f + Variation);
}