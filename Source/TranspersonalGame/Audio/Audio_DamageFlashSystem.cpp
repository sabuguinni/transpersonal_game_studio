#include "Audio_DamageFlashSystem.h"
#include "Components/AudioComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInterface.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Engine/Engine.h"
#include "Sound/SoundBase.h"
#include "UObject/ConstructorHelpers.h"

AAudio_DamageFlashSystem::AAudio_DamageFlashSystem()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create audio component
    AudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("AudioComponent"));
    RootComponent = AudioComponent;

    // Create screen overlay mesh component
    ScreenOverlay = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ScreenOverlay"));
    ScreenOverlay->SetupAttachment(RootComponent);
    
    // Set default flash data
    DefaultFlashData.FlashDuration = 0.3f;
    DefaultFlashData.FlashIntensity = 0.5f;
    DefaultFlashData.FlashColor = FLinearColor(1.0f, 0.0f, 0.0f, 0.3f); // Semi-transparent red
    DefaultFlashData.bPlayDamageSound = true;

    // Set critical flash data
    CriticalFlashData.FlashDuration = 0.6f;
    CriticalFlashData.FlashIntensity = 1.0f;
    CriticalFlashData.FlashColor = FLinearColor(1.0f, 0.0f, 0.0f, 0.7f); // More opaque red
    CriticalFlashData.bPlayDamageSound = true;

    // Initialize flash state
    CurrentFlashTime = 0.0f;
    FlashDuration = 0.0f;
    bIsFlashing = false;
    FlashColor = FLinearColor::Red;
    FlashIntensity = 0.0f;
}

void AAudio_DamageFlashSystem::BeginPlay()
{
    Super::BeginPlay();
    
    // Create dynamic material instance for flash effect
    if (FlashMaterial)
    {
        DynamicFlashMaterial = UMaterialInstanceDynamic::Create(FlashMaterial, this);
        if (DynamicFlashMaterial && ScreenOverlay)
        {
            ScreenOverlay->SetMaterial(0, DynamicFlashMaterial);
        }
    }
    
    // Hide overlay initially
    if (ScreenOverlay)
    {
        ScreenOverlay->SetVisibility(false);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Audio_DamageFlashSystem initialized"));
}

void AAudio_DamageFlashSystem::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    if (bIsFlashing)
    {
        UpdateFlash(DeltaTime);
    }
}

void AAudio_DamageFlashSystem::TriggerDamageFlash(float DamageAmount)
{
    // Determine if this is critical damage (>50 damage)
    if (DamageAmount >= 50.0f)
    {
        TriggerCriticalDamageFlash();
    }
    else
    {
        StartFlash(DefaultFlashData);
        PlayDamageAudio(false);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Damage flash triggered: %f damage"), DamageAmount);
}

void AAudio_DamageFlashSystem::TriggerCriticalDamageFlash()
{
    StartFlash(CriticalFlashData);
    PlayDamageAudio(true);
    
    UE_LOG(LogTemp, Warning, TEXT("Critical damage flash triggered"));
}

void AAudio_DamageFlashSystem::TriggerCustomFlash(const FAudio_DamageFlashData& FlashData)
{
    StartFlash(FlashData);
    
    if (FlashData.bPlayDamageSound)
    {
        PlayDamageAudio(false);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Custom damage flash triggered"));
}

void AAudio_DamageFlashSystem::StopFlash()
{
    bIsFlashing = false;
    CurrentFlashTime = 0.0f;
    
    if (ScreenOverlay)
    {
        ScreenOverlay->SetVisibility(false);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Damage flash stopped"));
}

void AAudio_DamageFlashSystem::StartFlash(const FAudio_DamageFlashData& FlashData)
{
    bIsFlashing = true;
    CurrentFlashTime = 0.0f;
    FlashDuration = FlashData.FlashDuration;
    FlashColor = FlashData.FlashColor;
    FlashIntensity = FlashData.FlashIntensity;
    
    if (ScreenOverlay)
    {
        ScreenOverlay->SetVisibility(true);
    }
}

void AAudio_DamageFlashSystem::UpdateFlash(float DeltaTime)
{
    CurrentFlashTime += DeltaTime;
    
    if (CurrentFlashTime >= FlashDuration)
    {
        StopFlash();
        return;
    }
    
    // Calculate flash alpha based on time (fade out effect)
    float Alpha = 1.0f - (CurrentFlashTime / FlashDuration);
    Alpha = FMath::Clamp(Alpha, 0.0f, 1.0f);
    
    // Apply flash intensity curve (quick fade)
    Alpha = FMath::Pow(Alpha, 2.0f);
    
    if (DynamicFlashMaterial)
    {
        FLinearColor CurrentFlashColor = FlashColor;
        CurrentFlashColor.A = Alpha * FlashIntensity;
        
        // Update material parameters
        DynamicFlashMaterial->SetVectorParameterValue(TEXT("FlashColor"), CurrentFlashColor);
        DynamicFlashMaterial->SetScalarParameterValue(TEXT("FlashIntensity"), Alpha * FlashIntensity);
    }
}

void AAudio_DamageFlashSystem::PlayDamageAudio(bool bIsCritical)
{
    if (!AudioComponent)
    {
        return;
    }
    
    USoundBase* SoundToPlay = bIsCritical ? CriticalDamageSound : DamageSound;
    
    if (SoundToPlay)
    {
        AudioComponent->SetSound(SoundToPlay);
        AudioComponent->SetVolumeMultiplier(bIsCritical ? 1.2f : 0.8f);
        AudioComponent->Play();
        
        UE_LOG(LogTemp, Warning, TEXT("Damage audio played: %s"), bIsCritical ? TEXT("Critical") : TEXT("Normal"));
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("No damage sound assigned"));
    }
}