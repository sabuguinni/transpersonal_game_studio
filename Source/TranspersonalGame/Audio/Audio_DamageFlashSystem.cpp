#include "Audio_DamageFlashSystem.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"
#include "Components/PostProcessComponent.h"

UAudio_DamageFlashSystem::UAudio_DamageFlashSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = true;
    
    bIsFlashActive = false;
    CurrentFlashTime = 0.0f;
    CurrentFlashDuration = 0.0f;
    CachedPlayerController = nullptr;
}

void UAudio_DamageFlashSystem::BeginPlay()
{
    Super::BeginPlay();
    
    // Cache player controller
    CachedPlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    
    // Initialize flash presets
    InitializeFlashPresets();
}

void UAudio_DamageFlashSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Update cached player controller if needed
    if (!CachedPlayerController)
    {
        CachedPlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    }
    
    // Update flash effect
    if (bIsFlashActive)
    {
        UpdateFlashEffect(DeltaTime);
    }
}

void UAudio_DamageFlashSystem::TriggerDamageFlash(EAudio_DamageType DamageType)
{
    // Get flash settings for this damage type
    FAudio_DamageFlashSettings* FlashSettings = FlashPresets.Find(DamageType);
    if (!FlashSettings)
    {
        return;
    }
    
    TriggerCustomFlash(*FlashSettings);
}

void UAudio_DamageFlashSystem::TriggerCustomFlash(FAudio_DamageFlashSettings FlashSettings)
{
    if (!CachedPlayerController)
    {
        return;
    }
    
    // Set flash state
    bIsFlashActive = true;
    CurrentFlashTime = 0.0f;
    CurrentFlashDuration = FlashSettings.FlashDuration;
    CurrentFlashSettings = FlashSettings;
    
    // Log flash for debugging
    UE_LOG(LogTemp, Log, TEXT("Audio Damage Flash: Type triggered, Duration=%.2f, Intensity=%.2f"), 
           FlashSettings.FlashDuration, FlashSettings.FlashIntensity);
}

void UAudio_DamageFlashSystem::StopFlash()
{
    bIsFlashActive = false;
    CurrentFlashTime = 0.0f;
    CurrentFlashDuration = 0.0f;
    
    // Remove flash effect
    ApplyFlashEffect(0.0f);
}

void UAudio_DamageFlashSystem::InitializeFlashPresets()
{
    // Light damage flash (minor hits)
    FAudio_DamageFlashSettings LightFlash;
    LightFlash.FlashDuration = 0.2f;
    LightFlash.FlashIntensity = 0.3f;
    LightFlash.FlashColor = FLinearColor(1.0f, 0.8f, 0.8f, 1.0f); // Light red
    LightFlash.FadeOutTime = 0.15f;
    FlashPresets.Add(EAudio_DamageType::Light, LightFlash);
    
    // Medium damage flash (moderate hits)
    FAudio_DamageFlashSettings MediumFlash;
    MediumFlash.FlashDuration = 0.4f;
    MediumFlash.FlashIntensity = 0.5f;
    MediumFlash.FlashColor = FLinearColor(1.0f, 0.5f, 0.5f, 1.0f); // Medium red
    MediumFlash.FadeOutTime = 0.3f;
    FlashPresets.Add(EAudio_DamageType::Medium, MediumFlash);
    
    // Heavy damage flash (serious hits)
    FAudio_DamageFlashSettings HeavyFlash;
    HeavyFlash.FlashDuration = 0.6f;
    HeavyFlash.FlashIntensity = 0.7f;
    HeavyFlash.FlashColor = FLinearColor(1.0f, 0.2f, 0.2f, 1.0f); // Dark red
    HeavyFlash.FadeOutTime = 0.4f;
    FlashPresets.Add(EAudio_DamageType::Heavy, HeavyFlash);
    
    // Critical damage flash (near death)
    FAudio_DamageFlashSettings CriticalFlash;
    CriticalFlash.FlashDuration = 0.8f;
    CriticalFlash.FlashIntensity = 0.9f;
    CriticalFlash.FlashColor = FLinearColor(1.0f, 0.0f, 0.0f, 1.0f); // Pure red
    CriticalFlash.FadeOutTime = 0.6f;
    FlashPresets.Add(EAudio_DamageType::Critical, CriticalFlash);
    
    // Death flash (player death)
    FAudio_DamageFlashSettings DeathFlash;
    DeathFlash.FlashDuration = 1.5f;
    DeathFlash.FlashIntensity = 1.0f;
    DeathFlash.FlashColor = FLinearColor(0.8f, 0.0f, 0.0f, 1.0f); // Dark red
    DeathFlash.FadeOutTime = 1.0f;
    FlashPresets.Add(EAudio_DamageType::Death, DeathFlash);
}

void UAudio_DamageFlashSystem::UpdateFlashEffect(float DeltaTime)
{
    CurrentFlashTime += DeltaTime;
    
    if (CurrentFlashTime >= CurrentFlashDuration)
    {
        // Flash finished
        StopFlash();
        return;
    }
    
    // Calculate flash alpha based on time
    float FlashAlpha = 0.0f;
    
    if (CurrentFlashTime <= (CurrentFlashDuration - CurrentFlashSettings.FadeOutTime))
    {
        // Full intensity phase
        FlashAlpha = CurrentFlashSettings.FlashIntensity;
    }
    else
    {
        // Fade out phase
        float FadeProgress = (CurrentFlashTime - (CurrentFlashDuration - CurrentFlashSettings.FadeOutTime)) / CurrentFlashSettings.FadeOutTime;
        FlashAlpha = CurrentFlashSettings.FlashIntensity * (1.0f - FadeProgress);
    }
    
    // Apply flash effect
    ApplyFlashEffect(FlashAlpha);
}

void UAudio_DamageFlashSystem::ApplyFlashEffect(float FlashAlpha)
{
    if (!CachedPlayerController)
    {
        return;
    }
    
    // For now, we'll use a simple approach with the player controller
    // In a full implementation, this would modify post-process settings
    // or use a UI overlay widget
    
    // Log the flash effect for debugging
    if (FlashAlpha > 0.0f)
    {
        UE_LOG(LogTemp, VeryVerbose, TEXT("Damage Flash Alpha: %.2f"), FlashAlpha);
    }
    
    // TODO: Implement actual screen flash effect
    // This could be done via:
    // 1. Post-process volume settings
    // 2. UI widget overlay
    // 3. Material parameter collection
    // 4. Camera post-process component
}