#include "Audio_EffectsManager.h"
#include "Engine/World.h"
#include "Engine/DirectionalLight.h"
#include "Components/DirectionalLightComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"
#include "Camera/PlayerCameraManager.h"
#include "GameFramework/PlayerController.h"
#include "Sound/SoundCue.h"
#include "Components/AudioComponent.h"

UAudio_EffectsManager::UAudio_EffectsManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PostUpdateWork;

    // Initialize default values
    DamageFlashData.Duration = 0.5f;
    DamageFlashData.Intensity = 0.8f;
    DamageFlashData.Color = FLinearColor::Red;

    DayNightCycleDuration = 1200.0f; // 20 minutes
    CurrentTimeOfDay = 0.5f; // Start at noon

    // Create ambient audio component
    AmbientAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("AmbientAudio"));
}

void UAudio_EffectsManager::BeginPlay()
{
    Super::BeginPlay();

    InitializeFootstepData();

    // Find the directional light in the scene
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ADirectionalLight::StaticClass(), FoundActors);
    
    if (FoundActors.Num() > 0)
    {
        SunLight = Cast<ADirectionalLight>(FoundActors[0]);
        if (SunLight)
        {
            UE_LOG(LogTemp, Warning, TEXT("Audio_EffectsManager: Found DirectionalLight for day/night cycle"));
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Audio_EffectsManager: No DirectionalLight found for day/night cycle"));
    }

    // Setup ambient audio
    if (AmbientAudioComponent)
    {
        AmbientAudioComponent->bAutoActivate = true;
        AmbientAudioComponent->SetVolumeMultiplier(0.3f);
    }
}

void UAudio_EffectsManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    // Update day/night cycle
    UpdateDayNightCycle(DeltaTime);

    // Handle screen flash effect
    if (bIsFlashing)
    {
        ScreenFlashTimer -= DeltaTime;
        if (ScreenFlashTimer <= 0.0f)
        {
            bIsFlashing = false;
            ScreenFlashTimer = 0.0f;
        }
    }
}

void UAudio_EffectsManager::TriggerDamageFlash(float Duration, float Intensity, FLinearColor Color)
{
    ActiveFlashData.Duration = Duration;
    ActiveFlashData.Intensity = Intensity;
    ActiveFlashData.Color = Color;
    
    ScreenFlashTimer = Duration;
    bIsFlashing = true;

    UE_LOG(LogTemp, Warning, TEXT("Audio_EffectsManager: Damage flash triggered - Duration: %f, Intensity: %f"), Duration, Intensity);

    // Apply screen flash effect via player camera manager
    APlayerController* PC = GetWorld()->GetFirstPlayerController();
    if (PC && PC->PlayerCameraManager)
    {
        PC->PlayerCameraManager->StartCameraFade(0.0f, Intensity, Duration * 0.5f, Color, false, true);
        PC->PlayerCameraManager->StartCameraFade(Intensity, 0.0f, Duration * 0.5f, Color, false, true);
    }
}

void UAudio_EffectsManager::PlayFootstepEffect(ESurvivalCreatureType CreatureType, FVector Location, float VolumeOverride)
{
    if (!FootstepDataMap.Contains(CreatureType))
    {
        UE_LOG(LogTemp, Warning, TEXT("Audio_EffectsManager: No footstep data for creature type %d"), (int32)CreatureType);
        return;
    }

    const FAudio_FootstepData& FootstepData = FootstepDataMap[CreatureType];
    
    // Play footstep sound
    if (FootstepData.FootstepSound)
    {
        float FinalVolume = FootstepData.VolumeMultiplier * VolumeOverride;
        UGameplayStatics::PlaySoundAtLocation(GetWorld(), FootstepData.FootstepSound, Location, FinalVolume);
        
        UE_LOG(LogTemp, Log, TEXT("Audio_EffectsManager: Played footstep for creature %d at location %s"), 
               (int32)CreatureType, *Location.ToString());
    }

    // Trigger camera shake for large creatures
    if (CreatureType == ESurvivalCreatureType::TRex || CreatureType == ESurvivalCreatureType::Brachiosaurus)
    {
        if (TRexFootstepShake)
        {
            TriggerCameraShake(TRexFootstepShake, FootstepData.ShakeIntensity, FootstepData.ShakeRadius);
        }
    }
}

void UAudio_EffectsManager::TriggerCameraShake(TSubclassOf<UCameraShakeBase> ShakeClass, float Intensity, float Radius)
{
    if (!ShakeClass)
    {
        UE_LOG(LogTemp, Warning, TEXT("Audio_EffectsManager: No camera shake class provided"));
        return;
    }

    APlayerController* PC = GetWorld()->GetFirstPlayerController();
    if (PC && PC->PlayerCameraManager)
    {
        PC->PlayerCameraManager->StartCameraShake(ShakeClass, Intensity);
        UE_LOG(LogTemp, Log, TEXT("Audio_EffectsManager: Camera shake triggered with intensity %f"), Intensity);
    }
}

void UAudio_EffectsManager::UpdateDayNightCycle(float DeltaTime)
{
    if (DayNightCycleDuration <= 0.0f) return;

    // Update time of day (0.0 to 1.0)
    float TimeIncrement = DeltaTime / DayNightCycleDuration;
    CurrentTimeOfDay += TimeIncrement;
    
    // Wrap around at 1.0
    if (CurrentTimeOfDay >= 1.0f)
    {
        CurrentTimeOfDay -= 1.0f;
    }

    UpdateDirectionalLightRotation();
}

void UAudio_EffectsManager::UpdateDirectionalLightRotation()
{
    if (!SunLight || !SunLight->GetLightComponent()) return;

    // Convert time of day to sun angle (0.0 = midnight, 0.5 = noon)
    float SunAngle = (CurrentTimeOfDay - 0.5f) * 180.0f; // -90 to +90 degrees
    
    // Create rotation for the sun
    FRotator SunRotation = FRotator(SunAngle, 45.0f, 0.0f); // 45 degree yaw offset
    SunLight->GetLightComponent()->SetWorldRotation(SunRotation);

    // Adjust light intensity based on time of day
    float LightIntensity = 1.0f;
    if (IsNightTime())
    {
        LightIntensity = 0.1f; // Dim moonlight
    }
    else
    {
        // Gradual transition during dawn/dusk
        float NoonDistance = FMath::Abs(CurrentTimeOfDay - 0.5f) * 2.0f; // 0.0 at noon, 1.0 at midnight
        LightIntensity = FMath::Lerp(3.0f, 0.1f, NoonDistance);
    }
    
    SunLight->GetLightComponent()->SetIntensity(LightIntensity);
}

void UAudio_EffectsManager::SetAmbientVolume(float Volume)
{
    if (AmbientAudioComponent)
    {
        AmbientAudioComponent->SetVolumeMultiplier(FMath::Clamp(Volume, 0.0f, 1.0f));
    }
}

void UAudio_EffectsManager::PlaySoundAtLocation(USoundCue* Sound, FVector Location, float Volume, float Pitch)
{
    if (Sound)
    {
        UGameplayStatics::PlaySoundAtLocation(GetWorld(), Sound, Location, Volume, Pitch);
    }
}

void UAudio_EffectsManager::InitializeFootstepData()
{
    // Initialize footstep data for different creature types
    FAudio_FootstepData TRexFootstep;
    TRexFootstep.VolumeMultiplier = 2.0f;
    TRexFootstep.ShakeRadius = 1500.0f;
    TRexFootstep.ShakeIntensity = 3.0f;
    FootstepDataMap.Add(ESurvivalCreatureType::TRex, TRexFootstep);

    FAudio_FootstepData RaptorFootstep;
    RaptorFootstep.VolumeMultiplier = 0.8f;
    RaptorFootstep.ShakeRadius = 200.0f;
    RaptorFootstep.ShakeIntensity = 0.5f;
    FootstepDataMap.Add(ESurvivalCreatureType::Velociraptor, RaptorFootstep);

    FAudio_FootstepData BrachioFootstep;
    BrachioFootstep.VolumeMultiplier = 2.5f;
    BrachioFootstep.ShakeRadius = 2000.0f;
    BrachioFootstep.ShakeIntensity = 2.0f;
    FootstepDataMap.Add(ESurvivalCreatureType::Brachiosaurus, BrachioFootstep);

    FAudio_FootstepData PlayerFootstep;
    PlayerFootstep.VolumeMultiplier = 0.3f;
    PlayerFootstep.ShakeRadius = 0.0f;
    PlayerFootstep.ShakeIntensity = 0.0f;
    FootstepDataMap.Add(ESurvivalCreatureType::Human, PlayerFootstep);

    UE_LOG(LogTemp, Warning, TEXT("Audio_EffectsManager: Initialized footstep data for %d creature types"), FootstepDataMap.Num());
}