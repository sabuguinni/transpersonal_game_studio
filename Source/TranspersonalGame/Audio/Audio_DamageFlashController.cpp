#include "Audio_DamageFlashController.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Engine/LocalPlayer.h"
#include "Components/PostProcessComponent.h"

UAudio_DamageFlashController::UAudio_DamageFlashController()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = true;

    // Initialize default flash settings
    LightDamageSettings.FlashIntensity = 0.2f;
    LightDamageSettings.FlashDuration = 0.15f;
    LightDamageSettings.FlashColor = FLinearColor(1.0f, 0.8f, 0.8f, 1.0f);
    LightDamageSettings.FadeOutTime = 0.1f;

    MediumDamageSettings.FlashIntensity = 0.4f;
    MediumDamageSettings.FlashDuration = 0.25f;
    MediumDamageSettings.FlashColor = FLinearColor(1.0f, 0.5f, 0.5f, 1.0f);
    MediumDamageSettings.FadeOutTime = 0.15f;

    HeavyDamageSettings.FlashIntensity = 0.6f;
    HeavyDamageSettings.FlashDuration = 0.4f;
    HeavyDamageSettings.FlashColor = FLinearColor(1.0f, 0.2f, 0.2f, 1.0f);
    HeavyDamageSettings.FadeOutTime = 0.2f;

    CriticalDamageSettings.FlashIntensity = 0.8f;
    CriticalDamageSettings.FlashDuration = 0.6f;
    CriticalDamageSettings.FlashColor = FLinearColor(1.0f, 0.0f, 0.0f, 1.0f);
    CriticalDamageSettings.FadeOutTime = 0.3f;
}

void UAudio_DamageFlashController::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize component state
    bIsFlashing = false;
    CurrentFlashAlpha = 0.0f;
    FlashTimer = 0.0f;
}

void UAudio_DamageFlashController::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (bIsFlashing)
    {
        UpdateFlash(DeltaTime);
    }
}

void UAudio_DamageFlashController::TriggerDamageFlash(EAudio_DamageType DamageType, float CustomIntensity)
{
    FAudio_DamageFlashSettings FlashSettings = GetFlashSettingsForDamageType(DamageType);
    
    // Override intensity if custom value provided
    if (CustomIntensity >= 0.0f)
    {
        FlashSettings.FlashIntensity = FMath::Clamp(CustomIntensity, 0.0f, 1.0f);
    }

    TriggerCustomFlash(FlashSettings);
}

void UAudio_DamageFlashController::TriggerCustomFlash(const FAudio_DamageFlashSettings& FlashSettings)
{
    // Stop any existing flash
    StopFlash();

    // Set new flash parameters
    ActiveFlashSettings = FlashSettings;
    bIsFlashing = true;
    FlashTimer = 0.0f;
    CurrentFlashAlpha = ActiveFlashSettings.FlashIntensity;

    // Set timer for flash duration
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(
            FlashTimerHandle,
            this,
            &UAudio_DamageFlashController::EndFlash,
            ActiveFlashSettings.FlashDuration,
            false
        );
    }

    // Apply immediate flash effect via console command
    if (UWorld* World = GetWorld())
    {
        if (APlayerController* PC = World->GetFirstPlayerController())
        {
            FString FlashCommand = FString::Printf(
                TEXT("r.PostProcessAAQuality 0; r.Tonemapper.Sharpen %f"),
                ActiveFlashSettings.FlashIntensity * 2.0f
            );
            PC->ConsoleCommand(FlashCommand);
        }
    }
}

void UAudio_DamageFlashController::StopFlash()
{
    if (bIsFlashing)
    {
        bIsFlashing = false;
        CurrentFlashAlpha = 0.0f;
        FlashTimer = 0.0f;

        // Clear timer
        if (UWorld* World = GetWorld())
        {
            World->GetTimerManager().ClearTimer(FlashTimerHandle);
        }

        // Reset post-process effects
        if (UWorld* World = GetWorld())
        {
            if (APlayerController* PC = World->GetFirstPlayerController())
            {
                PC->ConsoleCommand(TEXT("r.PostProcessAAQuality 1; r.Tonemapper.Sharpen 0"));
            }
        }
    }
}

void UAudio_DamageFlashController::UpdateFlash(float DeltaTime)
{
    FlashTimer += DeltaTime;

    // Calculate fade-out alpha during the fade period
    float FadeStartTime = ActiveFlashSettings.FlashDuration - ActiveFlashSettings.FadeOutTime;
    
    if (FlashTimer >= FadeStartTime)
    {
        float FadeProgress = (FlashTimer - FadeStartTime) / ActiveFlashSettings.FadeOutTime;
        FadeProgress = FMath::Clamp(FadeProgress, 0.0f, 1.0f);
        
        CurrentFlashAlpha = ActiveFlashSettings.FlashIntensity * (1.0f - FadeProgress);
        
        // Update visual effect intensity
        if (UWorld* World = GetWorld())
        {
            if (APlayerController* PC = World->GetFirstPlayerController())
            {
                FString FadeCommand = FString::Printf(
                    TEXT("r.Tonemapper.Sharpen %f"),
                    CurrentFlashAlpha * 2.0f
                );
                PC->ConsoleCommand(FadeCommand);
            }
        }
    }
}

void UAudio_DamageFlashController::EndFlash()
{
    StopFlash();
}

FAudio_DamageFlashSettings UAudio_DamageFlashController::GetFlashSettingsForDamageType(EAudio_DamageType DamageType) const
{
    switch (DamageType)
    {
        case EAudio_DamageType::Light:
            return LightDamageSettings;
        case EAudio_DamageType::Medium:
            return MediumDamageSettings;
        case EAudio_DamageType::Heavy:
            return HeavyDamageSettings;
        case EAudio_DamageType::Critical:
            return CriticalDamageSettings;
        default:
            return LightDamageSettings;
    }
}