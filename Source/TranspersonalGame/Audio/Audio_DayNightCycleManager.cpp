#include "Audio_DayNightCycleManager.h"
#include "Engine/DirectionalLight.h"
#include "Components/DirectionalLightComponent.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Audio_AdaptiveMusicManager.h"
#include "Audio_BiomeAudioManager.h"

AAudio_DayNightCycleManager::AAudio_DayNightCycleManager()
{
    PrimaryActorTick.bCanEverTick = true;
    
    // Cretaceous period day/night cycle settings
    CycleDurationMinutes = 20.0f;  // 20 minute full cycle
    CurrentTimeOfDay = 0.5f;       // Start at midday
    bCycleActive = true;
    
    // Cretaceous lighting parameters
    DayLightIntensity = 5.0f;      // Bright tropical sun
    NightLightIntensity = 0.1f;    // Minimal moonlight
    DayLightColor = FLinearColor(1.0f, 0.98f, 0.9f, 1.0f);    // Warm white
    NightLightColor = FLinearColor(0.2f, 0.3f, 0.6f, 1.0f);   // Cool blue
    
    // Sun movement parameters
    SunPitch = -45.0f;             // High tropical sun
    SunYawRange = 180.0f;          // East to west movement
    
    // Audio transition thresholds
    DawnTime = 0.2f;               // 20% through cycle
    DuskTime = 0.8f;               // 80% through cycle
    
    DirectionalLight = nullptr;
    MusicManager = nullptr;
    BiomeAudioManager = nullptr;
}

void AAudio_DayNightCycleManager::BeginPlay()
{
    Super::BeginPlay();
    
    // Find the directional light in the world
    FindDirectionalLight();
    
    // Find audio managers
    FindAudioManagers();
    
    // Set initial lighting state
    UpdateLighting();
    
    UE_LOG(LogTemp, Warning, TEXT("Day/Night Cycle Manager initialized - Cretaceous period lighting active"));
}

void AAudio_DayNightCycleManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    if (bCycleActive && CycleDurationMinutes > 0.0f)
    {
        // Update time of day
        float CycleSpeed = 1.0f / (CycleDurationMinutes * 60.0f);
        CurrentTimeOfDay += DeltaTime * CycleSpeed;
        
        // Wrap around at end of day
        if (CurrentTimeOfDay >= 1.0f)
        {
            CurrentTimeOfDay -= 1.0f;
        }
        
        // Update lighting and audio
        UpdateLighting();
        UpdateAudioForTimeOfDay();
    }
}

void AAudio_DayNightCycleManager::FindDirectionalLight()
{
    if (UWorld* World = GetWorld())
    {
        TArray<AActor*> FoundActors;
        UGameplayStatics::GetAllActorsOfClass(World, ADirectionalLight::StaticClass(), FoundActors);
        
        if (FoundActors.Num() > 0)
        {
            DirectionalLight = Cast<ADirectionalLight>(FoundActors[0]);
            UE_LOG(LogTemp, Log, TEXT("Found directional light: %s"), *DirectionalLight->GetName());
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("No directional light found in world"));
        }
    }
}

void AAudio_DayNightCycleManager::FindAudioManagers()
{
    if (UWorld* World = GetWorld())
    {
        // Find adaptive music manager
        TArray<AActor*> MusicActors;
        UGameplayStatics::GetAllActorsOfClass(World, AAudio_AdaptiveMusicManager::StaticClass(), MusicActors);
        if (MusicActors.Num() > 0)
        {
            MusicManager = Cast<AAudio_AdaptiveMusicManager>(MusicActors[0]);
        }
        
        // Find biome audio manager
        TArray<AActor*> BiomeActors;
        UGameplayStatics::GetAllActorsOfClass(World, AAudio_BiomeAudioManager::StaticClass(), BiomeActors);
        if (BiomeActors.Num() > 0)
        {
            BiomeAudioManager = Cast<AAudio_BiomeAudioManager>(BiomeActors[0]);
        }
    }
}

void AAudio_DayNightCycleManager::UpdateLighting()
{
    if (!DirectionalLight)
    {
        return;
    }
    
    UDirectionalLightComponent* LightComponent = DirectionalLight->GetLightComponent();
    if (!LightComponent)
    {
        return;
    }
    
    // Calculate sun position based on time of day
    float SunYaw = (CurrentTimeOfDay - 0.5f) * SunYawRange;  // -90 to +90 degrees
    FRotator SunRotation = FRotator(SunPitch, SunYaw, 0.0f);
    DirectionalLight->SetActorRotation(SunRotation);
    
    // Calculate lighting intensity and color
    float LightIntensity;
    FLinearColor LightColor;
    
    if (CurrentTimeOfDay >= DawnTime && CurrentTimeOfDay <= DuskTime)
    {
        // Daytime - bright tropical lighting
        float DayProgress = (CurrentTimeOfDay - DawnTime) / (DuskTime - DawnTime);
        float SinProgress = FMath::Sin(DayProgress * PI);  // Smooth curve
        
        LightIntensity = DayLightIntensity * SinProgress;
        LightColor = FLinearColor::LerpUsingHSV(DayLightColor, DayLightColor, SinProgress);
    }
    else
    {
        // Nighttime - minimal lighting
        LightIntensity = NightLightIntensity;
        LightColor = NightLightColor;
    }
    
    // Apply lighting changes
    LightComponent->SetIntensity(LightIntensity);
    LightComponent->SetLightColor(LightColor);
}

void AAudio_DayNightCycleManager::UpdateAudioForTimeOfDay()
{
    // Determine current period
    EAudio_TimeOfDay NewPeriod;
    
    if (CurrentTimeOfDay < DawnTime || CurrentTimeOfDay > DuskTime)
    {
        NewPeriod = EAudio_TimeOfDay::Night;
    }
    else if (CurrentTimeOfDay < (DawnTime + 0.1f))
    {
        NewPeriod = EAudio_TimeOfDay::Dawn;
    }
    else if (CurrentTimeOfDay > (DuskTime - 0.1f))
    {
        NewPeriod = EAudio_TimeOfDay::Dusk;
    }
    else
    {
        NewPeriod = EAudio_TimeOfDay::Day;
    }
    
    // Update audio if period changed
    if (NewPeriod != CurrentPeriod)
    {
        CurrentPeriod = NewPeriod;
        
        // Notify music manager
        if (MusicManager)
        {
            MusicManager->OnTimeOfDayChanged(CurrentPeriod);
        }
        
        // Notify biome audio manager
        if (BiomeAudioManager)
        {
            BiomeAudioManager->OnTimeOfDayChanged(CurrentPeriod);
        }
        
        UE_LOG(LogTemp, Log, TEXT("Time of day changed to: %d"), (int32)CurrentPeriod);
    }
}

void AAudio_DayNightCycleManager::SetTimeOfDay(float NewTimeOfDay)
{
    CurrentTimeOfDay = FMath::Clamp(NewTimeOfDay, 0.0f, 1.0f);
    UpdateLighting();
    UpdateAudioForTimeOfDay();
}

void AAudio_DayNightCycleManager::SetCycleActive(bool bActive)
{
    bCycleActive = bActive;
    UE_LOG(LogTemp, Log, TEXT("Day/Night cycle %s"), bActive ? TEXT("activated") : TEXT("paused"));
}

float AAudio_DayNightCycleManager::GetCurrentTimeOfDay() const
{
    return CurrentTimeOfDay;
}

EAudio_TimeOfDay AAudio_DayNightCycleManager::GetCurrentPeriod() const
{
    return CurrentPeriod;
}