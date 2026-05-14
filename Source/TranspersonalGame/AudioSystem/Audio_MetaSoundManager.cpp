#include "Audio_MetaSoundManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "MetasoundSource.h"
#include "../TranspersonalGameState.h"
#include "../TranspersonalCharacter.h"

UAudio_MetaSoundManager::UAudio_MetaSoundManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // 10 FPS for audio updates

    // Initialize default values
    MasterVolume = 1.0f;
    EnvironmentalVolume = 0.8f;
    MusicVolume = 0.6f;
    CurrentGameplayState = EGameplayState::Exploration;
    CurrentTimeOfDay = ETimeOfDay::Dawn;

    // Performance optimization settings
    LastProximityCheck = 0.0f;
    ProximityCheckInterval = 0.5f; // Check proximity twice per second
    LastAudioUpdate = 0.0f;
    AudioUpdateInterval = 0.1f; // Update audio 10 times per second

    // Initialize audio components
    EnvironmentalAudioComponent = nullptr;
    MusicAudioComponent = nullptr;
    ProximityAudioComponent = nullptr;

    // Setup default environmental layers
    FAudio_SoundLayer BaseLayer;
    BaseLayer.LayerName = TEXT("BaseAmbient");
    BaseLayer.Volume = 0.7f;
    BaseLayer.bIsActive = true;
    EnvironmentalLayers.Add(BaseLayer);

    FAudio_SoundLayer WindLayer;
    WindLayer.LayerName = TEXT("Wind");
    WindLayer.Volume = 0.5f;
    WindLayer.bIsActive = false;
    EnvironmentalLayers.Add(WindLayer);

    FAudio_SoundLayer InsectLayer;
    InsectLayer.LayerName = TEXT("Insects");
    InsectLayer.Volume = 0.6f;
    InsectLayer.bIsActive = true;
    EnvironmentalLayers.Add(InsectLayer);

    FAudio_SoundLayer BirdLayer;
    BirdLayer.LayerName = TEXT("Birds");
    BirdLayer.Volume = 0.4f;
    BirdLayer.bIsActive = true;
    EnvironmentalLayers.Add(BirdLayer);

    // Setup default proximity triggers
    FAudio_ProximityTrigger TRexTrigger;
    TRexTrigger.TriggerSpecies = EDinosaurSpecies::TRex;
    TRexTrigger.TriggerDistance = 3000.0f;
    TRexTrigger.CooldownTime = 45.0f;
    ProximityTriggers.Add(TRexTrigger);

    FAudio_ProximityTrigger RaptorTrigger;
    RaptorTrigger.TriggerSpecies = EDinosaurSpecies::Velociraptor;
    RaptorTrigger.TriggerDistance = 1500.0f;
    RaptorTrigger.CooldownTime = 25.0f;
    ProximityTriggers.Add(RaptorTrigger);

    FAudio_ProximityTrigger BrachiosaurusTrigger;
    BrachiosaurusTrigger.TriggerSpecies = EDinosaurSpecies::Brachiosaurus;
    BrachiosaurusTrigger.TriggerDistance = 2500.0f;
    BrachiosaurusTrigger.CooldownTime = 60.0f;
    ProximityTriggers.Add(BrachiosaurusTrigger);
}

void UAudio_MetaSoundManager::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeAudioComponents();
    
    // Start with exploration music
    TransitionToGameplayState(EGameplayState::Exploration, 0.0f);
    
    UE_LOG(LogTemp, Warning, TEXT("Audio_MetaSoundManager: Initialized with %d environmental layers and %d proximity triggers"), 
           EnvironmentalLayers.Num(), ProximityTriggers.Num());
}

void UAudio_MetaSoundManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    float CurrentTime = GetWorld()->GetTimeSeconds();

    // Update audio systems at different intervals for performance
    if (CurrentTime - LastAudioUpdate >= AudioUpdateInterval)
    {
        UpdateEnvironmentalAudio(DeltaTime);
        UpdateDynamicMusic(DeltaTime);
        ProcessAudioFades(DeltaTime);
        LastAudioUpdate = CurrentTime;
    }

    // Check proximity triggers less frequently
    if (CurrentTime - LastProximityCheck >= ProximityCheckInterval)
    {
        CheckProximityTriggers();
        LastProximityCheck = CurrentTime;
    }
}

void UAudio_MetaSoundManager::InitializeAudioComponents()
{
    AActor* Owner = GetOwner();
    if (!Owner)
    {
        UE_LOG(LogTemp, Error, TEXT("Audio_MetaSoundManager: No owner actor found"));
        return;
    }

    // Create environmental audio component
    EnvironmentalAudioComponent = NewObject<UAudioComponent>(Owner);
    if (EnvironmentalAudioComponent)
    {
        EnvironmentalAudioComponent->AttachToComponent(Owner->GetRootComponent(), 
                                                     FAttachmentTransformRules::KeepWorldTransform);
        EnvironmentalAudioComponent->SetVolumeMultiplier(EnvironmentalVolume);
        EnvironmentalAudioComponent->bAutoActivate = true;
    }

    // Create music audio component
    MusicAudioComponent = NewObject<UAudioComponent>(Owner);
    if (MusicAudioComponent)
    {
        MusicAudioComponent->AttachToComponent(Owner->GetRootComponent(), 
                                             FAttachmentTransformRules::KeepWorldTransform);
        MusicAudioComponent->SetVolumeMultiplier(MusicVolume);
        MusicAudioComponent->bAutoActivate = true;
    }

    // Create proximity warning audio component
    ProximityAudioComponent = NewObject<UAudioComponent>(Owner);
    if (ProximityAudioComponent)
    {
        ProximityAudioComponent->AttachToComponent(Owner->GetRootComponent(), 
                                                 FAttachmentTransformRules::KeepWorldTransform);
        ProximityAudioComponent->SetVolumeMultiplier(1.0f);
        ProximityAudioComponent->bAutoActivate = false;
    }

    // Create layer-specific audio components
    for (int32 i = 0; i < EnvironmentalLayers.Num(); i++)
    {
        UAudioComponent* LayerComponent = NewObject<UAudioComponent>(Owner);
        if (LayerComponent)
        {
            LayerComponent->AttachToComponent(Owner->GetRootComponent(), 
                                            FAttachmentTransformRules::KeepWorldTransform);
            LayerComponent->SetVolumeMultiplier(EnvironmentalLayers[i].Volume);
            LayerComponent->bAutoActivate = EnvironmentalLayers[i].bIsActive;
            LayerAudioComponents.Add(LayerComponent);
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("Audio_MetaSoundManager: Initialized %d audio components"), 
           LayerAudioComponents.Num() + 3);
}

void UAudio_MetaSoundManager::SetEnvironmentalLayer(const FString& LayerName, bool bActive, float FadeTime)
{
    for (int32 i = 0; i < EnvironmentalLayers.Num(); i++)
    {
        if (EnvironmentalLayers[i].LayerName == LayerName)
        {
            EnvironmentalLayers[i].bIsActive = bActive;
            EnvironmentalLayers[i].FadeTime = FadeTime;
            
            // Set fade target and speed
            float TargetVolume = bActive ? EnvironmentalLayers[i].Volume : 0.0f;
            LayerFadeTargets.Add(LayerName, TargetVolume);
            LayerFadeSpeeds.Add(LayerName, FMath::Abs(TargetVolume) / FadeTime);
            
            UE_LOG(LogTemp, Log, TEXT("Audio_MetaSoundManager: Set layer '%s' to %s with fade time %.2f"), 
                   *LayerName, bActive ? TEXT("active") : TEXT("inactive"), FadeTime);
            return;
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Audio_MetaSoundManager: Layer '%s' not found"), *LayerName);
}

void UAudio_MetaSoundManager::UpdateTimeOfDayAudio(ETimeOfDay NewTimeOfDay)
{
    if (CurrentTimeOfDay == NewTimeOfDay)
        return;

    CurrentTimeOfDay = NewTimeOfDay;

    // Adjust environmental layers based on time of day
    switch (NewTimeOfDay)
    {
        case ETimeOfDay::Dawn:
            SetEnvironmentalLayer(TEXT("Birds"), true, 3.0f);
            SetEnvironmentalLayer(TEXT("Insects"), false, 2.0f);
            SetEnvironmentalLayer(TEXT("Wind"), false, 2.0f);
            break;
            
        case ETimeOfDay::Day:
            SetEnvironmentalLayer(TEXT("Birds"), true, 2.0f);
            SetEnvironmentalLayer(TEXT("Insects"), true, 3.0f);
            SetEnvironmentalLayer(TEXT("Wind"), true, 2.0f);
            break;
            
        case ETimeOfDay::Dusk:
            SetEnvironmentalLayer(TEXT("Birds"), false, 3.0f);
            SetEnvironmentalLayer(TEXT("Insects"), true, 2.0f);
            SetEnvironmentalLayer(TEXT("Wind"), true, 1.0f);
            break;
            
        case ETimeOfDay::Night:
            SetEnvironmentalLayer(TEXT("Birds"), false, 1.0f);
            SetEnvironmentalLayer(TEXT("Insects"), true, 2.0f);
            SetEnvironmentalLayer(TEXT("Wind"), true, 3.0f);
            break;
    }

    UE_LOG(LogTemp, Log, TEXT("Audio_MetaSoundManager: Updated audio for time of day: %d"), (int32)NewTimeOfDay);
}

void UAudio_MetaSoundManager::UpdateWeatherAudio(EWeatherType WeatherType, float Intensity)
{
    switch (WeatherType)
    {
        case EWeatherType::Clear:
            SetEnvironmentalLayer(TEXT("Wind"), false, 2.0f);
            break;
            
        case EWeatherType::Cloudy:
            SetEnvironmentalLayer(TEXT("Wind"), true, 3.0f);
            break;
            
        case EWeatherType::Rainy:
            SetEnvironmentalLayer(TEXT("Wind"), true, 1.0f);
            // Would add rain layer here if available
            break;
            
        case EWeatherType::Stormy:
            SetEnvironmentalLayer(TEXT("Wind"), true, 0.5f);
            SetEnvironmentalLayer(TEXT("Birds"), false, 1.0f);
            break;
    }

    UE_LOG(LogTemp, Log, TEXT("Audio_MetaSoundManager: Updated weather audio: %d, Intensity: %.2f"), 
           (int32)WeatherType, Intensity);
}

void UAudio_MetaSoundManager::CheckProximityTriggers()
{
    TArray<AActor*> NearbyDinosaurs = GetNearbyDinosaurs(5000.0f); // Check within 5km
    float CurrentTime = GetWorld()->GetTimeSeconds();

    for (const FAudio_ProximityTrigger& Trigger : ProximityTriggers)
    {
        // Check cooldown
        if (CurrentTime - Trigger.LastTriggered < Trigger.CooldownTime)
            continue;

        // Find closest dinosaur of this species
        float ClosestDistance = FLT_MAX;
        AActor* ClosestDinosaur = nullptr;

        for (AActor* Dinosaur : NearbyDinosaurs)
        {
            // Would check dinosaur species here - simplified for now
            float Distance = FVector::Dist(GetOwner()->GetActorLocation(), Dinosaur->GetActorLocation());
            if (Distance < ClosestDistance && Distance <= Trigger.TriggerDistance)
            {
                ClosestDistance = Distance;
                ClosestDinosaur = Dinosaur;
            }
        }

        if (ClosestDinosaur)
        {
            TriggerProximityWarning(Trigger.TriggerSpecies, ClosestDistance);
        }
    }
}

void UAudio_MetaSoundManager::TriggerProximityWarning(EDinosaurSpecies Species, float Distance)
{
    // Update last triggered time
    for (FAudio_ProximityTrigger& Trigger : ProximityTriggers)
    {
        if (Trigger.TriggerSpecies == Species)
        {
            Trigger.LastTriggered = GetWorld()->GetTimeSeconds();
            
            // Play proximity sound
            if (Trigger.ProximitySound && ProximityAudioComponent)
            {
                ProximityAudioComponent->SetSound(Trigger.ProximitySound);
                ProximityAudioComponent->Play();
            }
            
            // Play voice warning
            if (Trigger.VoiceWarning && ProximityAudioComponent)
            {
                // Could queue voice warning after proximity sound
                UGameplayStatics::PlaySoundAtLocation(GetWorld(), Trigger.VoiceWarning, 
                                                    GetOwner()->GetActorLocation());
            }
            
            UE_LOG(LogTemp, Warning, TEXT("Audio_MetaSoundManager: Triggered proximity warning for species %d at distance %.2f"), 
                   (int32)Species, Distance);
            break;
        }
    }
}

void UAudio_MetaSoundManager::TransitionToGameplayState(EGameplayState NewState, float TransitionTime)
{
    if (CurrentGameplayState == NewState)
        return;

    CurrentGameplayState = NewState;

    // Transition music based on gameplay state
    if (MusicAudioComponent)
    {
        UMetaSoundSource* TargetMusic = nullptr;
        
        switch (NewState)
        {
            case EGameplayState::Exploration:
                TargetMusic = ExplorationMusicMetaSound;
                break;
                
            case EGameplayState::Combat:
                TargetMusic = CombatMusicMetaSound;
                break;
                
            case EGameplayState::Tension:
                TargetMusic = TensionMusicMetaSound;
                break;
                
            case EGameplayState::Rest:
                // Fade out music for rest state
                break;
        }

        if (TargetMusic)
        {
            MusicAudioComponent->SetSound(TargetMusic);
            if (!MusicAudioComponent->IsPlaying())
            {
                MusicAudioComponent->Play();
            }
        }
    }

    UE_LOG(LogTemp, Log, TEXT("Audio_MetaSoundManager: Transitioned to gameplay state: %d"), (int32)NewState);
}

void UAudio_MetaSoundManager::SetMusicIntensity(float Intensity)
{
    Intensity = FMath::Clamp(Intensity, 0.0f, 1.0f);
    
    if (MusicAudioComponent)
    {
        float VolumeMultiplier = MusicVolume * Intensity;
        MusicAudioComponent->SetVolumeMultiplier(VolumeMultiplier);
    }
    
    UE_LOG(LogTemp, Log, TEXT("Audio_MetaSoundManager: Set music intensity to %.2f"), Intensity);
}

void UAudio_MetaSoundManager::PlayStinger(USoundCue* StingerSound)
{
    if (StingerSound)
    {
        UGameplayStatics::PlaySoundAtLocation(GetWorld(), StingerSound, 
                                            GetOwner()->GetActorLocation());
        UE_LOG(LogTemp, Log, TEXT("Audio_MetaSoundManager: Played audio stinger"));
    }
}

void UAudio_MetaSoundManager::SetMasterVolume(float Volume)
{
    MasterVolume = FMath::Clamp(Volume, 0.0f, 1.0f);
    
    // Update all audio components
    if (EnvironmentalAudioComponent)
        EnvironmentalAudioComponent->SetVolumeMultiplier(EnvironmentalVolume * MasterVolume);
    
    if (MusicAudioComponent)
        MusicAudioComponent->SetVolumeMultiplier(MusicVolume * MasterVolume);
    
    for (int32 i = 0; i < LayerAudioComponents.Num(); i++)
    {
        if (LayerAudioComponents[i] && i < EnvironmentalLayers.Num())
        {
            LayerAudioComponents[i]->SetVolumeMultiplier(EnvironmentalLayers[i].Volume * MasterVolume);
        }
    }
}

void UAudio_MetaSoundManager::SetEnvironmentalVolume(float Volume)
{
    EnvironmentalVolume = FMath::Clamp(Volume, 0.0f, 1.0f);
    
    if (EnvironmentalAudioComponent)
        EnvironmentalAudioComponent->SetVolumeMultiplier(EnvironmentalVolume * MasterVolume);
}

void UAudio_MetaSoundManager::SetMusicVolume(float Volume)
{
    MusicVolume = FMath::Clamp(Volume, 0.0f, 1.0f);
    
    if (MusicAudioComponent)
        MusicAudioComponent->SetVolumeMultiplier(MusicVolume * MasterVolume);
}

void UAudio_MetaSoundManager::PlayDinosaurCall(EDinosaurSpecies Species, FVector Location, float VolumeMultiplier)
{
    // Would implement species-specific calls here
    UE_LOG(LogTemp, Log, TEXT("Audio_MetaSoundManager: Playing dinosaur call for species %d at location %s"), 
           (int32)Species, *Location.ToString());
}

void UAudio_MetaSoundManager::PlayFootstepAudio(EDinosaurSpecies Species, FVector Location, float Intensity)
{
    // Would implement species-specific footstep audio here
    UE_LOG(LogTemp, Log, TEXT("Audio_MetaSoundManager: Playing footstep audio for species %d, intensity %.2f"), 
           (int32)Species, Intensity);
}

void UAudio_MetaSoundManager::PlayEnvironmentalEvent(const FString& EventName, FVector Location)
{
    UE_LOG(LogTemp, Log, TEXT("Audio_MetaSoundManager: Playing environmental event '%s' at %s"), 
           *EventName, *Location.ToString());
}

void UAudio_MetaSoundManager::UpdateEnvironmentalAudio(float DeltaTime)
{
    // Update environmental layers based on game state
    // This would be more sophisticated in a full implementation
}

void UAudio_MetaSoundManager::UpdateDynamicMusic(float DeltaTime)
{
    // Update music parameters based on game state
    // This would interface with MetaSound parameters
}

void UAudio_MetaSoundManager::ProcessAudioFades(float DeltaTime)
{
    // Process layer fades
    for (int32 i = 0; i < EnvironmentalLayers.Num(); i++)
    {
        const FString& LayerName = EnvironmentalLayers[i].LayerName;
        
        if (LayerFadeTargets.Contains(LayerName) && LayerFadeSpeeds.Contains(LayerName))
        {
            float TargetVolume = LayerFadeTargets[LayerName];
            float FadeSpeed = LayerFadeSpeeds[LayerName];
            
            if (i < LayerAudioComponents.Num() && LayerAudioComponents[i])
            {
                float CurrentVolume = LayerAudioComponents[i]->VolumeMultiplier;
                float NewVolume = FMath::FInterpTo(CurrentVolume, TargetVolume, DeltaTime, FadeSpeed);
                
                LayerAudioComponents[i]->SetVolumeMultiplier(NewVolume * MasterVolume);
                
                // Remove from fade maps when target reached
                if (FMath::IsNearlyEqual(NewVolume, TargetVolume, 0.01f))
                {
                    LayerFadeTargets.Remove(LayerName);
                    LayerFadeSpeeds.Remove(LayerName);
                }
            }
        }
    }
}

TArray<AActor*> UAudio_MetaSoundManager::GetNearbyDinosaurs(float SearchRadius)
{
    TArray<AActor*> FoundDinosaurs;
    
    if (UWorld* World = GetWorld())
    {
        FVector SearchLocation = GetOwner()->GetActorLocation();
        
        // Get all actors in range - would filter for dinosaur actors in full implementation
        for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
        {
            AActor* Actor = *ActorItr;
            if (Actor && Actor != GetOwner())
            {
                float Distance = FVector::Dist(SearchLocation, Actor->GetActorLocation());
                if (Distance <= SearchRadius)
                {
                    // Would check if actor is a dinosaur here
                    FoundDinosaurs.Add(Actor);
                }
            }
        }
    }
    
    return FoundDinosaurs;
}

float UAudio_MetaSoundManager::CalculateProximityIntensity(float Distance, float MaxDistance)
{
    if (Distance >= MaxDistance)
        return 0.0f;
    
    // Inverse square falloff for realistic audio attenuation
    float NormalizedDistance = Distance / MaxDistance;
    return 1.0f - (NormalizedDistance * NormalizedDistance);
}