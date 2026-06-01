#include "Audio_ScreenEffectManager.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialParameterCollectionInstance.h"
#include "Engine/Engine.h"

UAudio_ScreenEffectManager::UAudio_ScreenEffectManager()
{
    // Set tick enabled for real-time effect updates
}

void UAudio_ScreenEffectManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    InitializeDefaultEffects();
    
    // Try to load the screen effect material parameter collection
    ScreenEffectMPC = LoadObject<UMaterialParameterCollection>(nullptr, TEXT("/Game/Materials/MPC_ScreenEffects"));
    if (!ScreenEffectMPC)
    {
        UE_LOG(LogTemp, Warning, TEXT("Screen Effect MPC not found - screen effects will be limited"));
    }
    
    UE_LOG(LogTemp, Log, TEXT("Audio_ScreenEffectManager initialized successfully"));
}

void UAudio_ScreenEffectManager::Deinitialize()
{
    StopAllScreenEffects();
    ActiveEffects.Empty();
    EffectTimers.Empty();
    
    Super::Deinitialize();
}

void UAudio_ScreenEffectManager::Tick(float DeltaTime)
{
    if (!bEffectsEnabled)
    {
        return;
    }

    // Update all active effects
    TArray<EAudio_ScreenEffectType> EffectsToRemove;
    
    for (auto& EffectPair : EffectTimers)
    {
        EAudio_ScreenEffectType EffectType = EffectPair.Key;
        float& Timer = EffectPair.Value;
        
        Timer += DeltaTime;
        
        if (ActiveEffects.Contains(EffectType))
        {
            const FAudio_ScreenEffectData& EffectData = ActiveEffects[EffectType];
            
            if (Timer >= EffectData.Duration)
            {
                // Effect finished
                EffectsToRemove.Add(EffectType);
            }
            else
            {
                // Update effect
                UpdateScreenEffect(EffectType, DeltaTime);
            }
        }
    }
    
    // Remove finished effects
    for (EAudio_ScreenEffectType EffectType : EffectsToRemove)
    {
        StopScreenEffect(EffectType);
    }
}

void UAudio_ScreenEffectManager::TriggerScreenEffect(EAudio_ScreenEffectType EffectType, float Intensity, float Duration)
{
    if (!bEffectsEnabled)
    {
        return;
    }

    FAudio_ScreenEffectData EffectData;
    EffectData.EffectType = EffectType;
    EffectData.Intensity = FMath::Clamp(Intensity, 0.0f, 1.0f) * GlobalIntensityMultiplier;
    EffectData.Duration = FMath::Max(Duration, 0.1f);
    
    // Set effect-specific properties
    switch (EffectType)
    {
        case EAudio_ScreenEffectType::DamageFlash:
            EffectData.EffectColor = FLinearColor::Red;
            EffectData.bPulsing = false;
            break;
            
        case EAudio_ScreenEffectType::HealthLow:
            EffectData.EffectColor = FLinearColor(1.0f, 0.2f, 0.2f, 1.0f);
            EffectData.bPulsing = true;
            EffectData.PulseFrequency = 1.5f;
            break;
            
        case EAudio_ScreenEffectType::StaminaDepleted:
            EffectData.EffectColor = FLinearColor::Yellow;
            EffectData.bPulsing = true;
            EffectData.PulseFrequency = 3.0f;
            break;
            
        case EAudio_ScreenEffectType::FearEffect:
            EffectData.EffectColor = FLinearColor(0.1f, 0.1f, 0.1f, 1.0f);
            EffectData.bPulsing = false;
            break;
            
        case EAudio_ScreenEffectType::TRexProximity:
            EffectData.EffectColor = FLinearColor(0.8f, 0.0f, 0.0f, 1.0f);
            EffectData.bPulsing = true;
            EffectData.PulseFrequency = 2.0f;
            break;
            
        case EAudio_ScreenEffectType::UnderwaterEffect:
            EffectData.EffectColor = FLinearColor(0.0f, 0.3f, 0.8f, 1.0f);
            EffectData.bPulsing = false;
            break;
    }
    
    ActiveEffects.Add(EffectType, EffectData);
    EffectTimers.Add(EffectType, 0.0f);
    
    UE_LOG(LogTemp, Log, TEXT("Triggered screen effect: %d with intensity %.2f for %.2f seconds"), 
           (int32)EffectType, Intensity, Duration);
}

void UAudio_ScreenEffectManager::TriggerDamageFlash(float Damage, float MaxHealth)
{
    if (MaxHealth <= 0.0f)
    {
        return;
    }
    
    float DamageRatio = FMath::Clamp(Damage / MaxHealth, 0.0f, 1.0f);
    float Intensity = FMath::Lerp(0.3f, 1.0f, DamageRatio);
    float Duration = FMath::Lerp(0.5f, 2.0f, DamageRatio);
    
    TriggerScreenEffect(EAudio_ScreenEffectType::DamageFlash, Intensity, Duration);
}

void UAudio_ScreenEffectManager::TriggerTRexProximityEffect(float Distance, float MaxDistance)
{
    if (Distance >= MaxDistance)
    {
        StopScreenEffect(EAudio_ScreenEffectType::TRexProximity);
        return;
    }
    
    float ProximityRatio = 1.0f - FMath::Clamp(Distance / MaxDistance, 0.0f, 1.0f);
    float Intensity = FMath::Lerp(0.1f, 0.8f, ProximityRatio);
    
    TriggerScreenEffect(EAudio_ScreenEffectType::TRexProximity, Intensity, 0.5f);
}

void UAudio_ScreenEffectManager::TriggerHealthWarning(float CurrentHealth, float MaxHealth)
{
    if (MaxHealth <= 0.0f)
    {
        return;
    }
    
    float HealthRatio = CurrentHealth / MaxHealth;
    
    if (HealthRatio <= 0.2f)  // Health below 20%
    {
        float Intensity = FMath::Lerp(0.8f, 0.3f, HealthRatio / 0.2f);
        TriggerScreenEffect(EAudio_ScreenEffectType::HealthLow, Intensity, 3.0f);
    }
    else
    {
        StopScreenEffect(EAudio_ScreenEffectType::HealthLow);
    }
}

void UAudio_ScreenEffectManager::StopScreenEffect(EAudio_ScreenEffectType EffectType)
{
    ActiveEffects.Remove(EffectType);
    EffectTimers.Remove(EffectType);
    
    // Clear the effect from material
    ApplyEffectToMaterial(EffectType, 0.0f);
}

void UAudio_ScreenEffectManager::StopAllScreenEffects()
{
    for (auto& EffectPair : ActiveEffects)
    {
        ApplyEffectToMaterial(EffectPair.Key, 0.0f);
    }
    
    ActiveEffects.Empty();
    EffectTimers.Empty();
}

void UAudio_ScreenEffectManager::SetScreenEffectIntensity(EAudio_ScreenEffectType EffectType, float NewIntensity)
{
    if (ActiveEffects.Contains(EffectType))
    {
        ActiveEffects[EffectType].Intensity = FMath::Clamp(NewIntensity, 0.0f, 1.0f) * GlobalIntensityMultiplier;
    }
}

void UAudio_ScreenEffectManager::UpdateScreenEffect(EAudio_ScreenEffectType EffectType, float DeltaTime)
{
    if (!ActiveEffects.Contains(EffectType) || !EffectTimers.Contains(EffectType))
    {
        return;
    }
    
    const FAudio_ScreenEffectData& EffectData = ActiveEffects[EffectType];
    float Timer = EffectTimers[EffectType];
    
    float CurrentIntensity = CalculateEffectIntensity(EffectData, Timer);
    ApplyEffectToMaterial(EffectType, CurrentIntensity);
}

void UAudio_ScreenEffectManager::ApplyEffectToMaterial(EAudio_ScreenEffectType EffectType, float CurrentIntensity)
{
    if (!ScreenEffectMPC || !GetWorld())
    {
        return;
    }
    
    UMaterialParameterCollectionInstance* MPCInstance = GetWorld()->GetParameterCollectionInstance(ScreenEffectMPC);
    if (!MPCInstance)
    {
        return;
    }
    
    // Apply effect based on type
    switch (EffectType)
    {
        case EAudio_ScreenEffectType::DamageFlash:
            MPCInstance->SetScalarParameterValue(TEXT("DamageFlashIntensity"), CurrentIntensity);
            break;
            
        case EAudio_ScreenEffectType::HealthLow:
            MPCInstance->SetScalarParameterValue(TEXT("HealthWarningIntensity"), CurrentIntensity);
            break;
            
        case EAudio_ScreenEffectType::TRexProximity:
            MPCInstance->SetScalarParameterValue(TEXT("TRexProximityIntensity"), CurrentIntensity);
            break;
            
        case EAudio_ScreenEffectType::FearEffect:
            MPCInstance->SetScalarParameterValue(TEXT("FearEffectIntensity"), CurrentIntensity);
            break;
            
        default:
            break;
    }
}

float UAudio_ScreenEffectManager::CalculateEffectIntensity(const FAudio_ScreenEffectData& EffectData, float Timer) const
{
    float BaseIntensity = EffectData.Intensity;
    
    if (EffectData.bPulsing)
    {
        float PulseValue = FMath::Sin(Timer * EffectData.PulseFrequency * 2.0f * PI) * 0.5f + 0.5f;
        return BaseIntensity * PulseValue;
    }
    else
    {
        // Fade out over time
        float FadeRatio = 1.0f - (Timer / EffectData.Duration);
        return BaseIntensity * FMath::Max(FadeRatio, 0.0f);
    }
}

void UAudio_ScreenEffectManager::InitializeDefaultEffects()
{
    // Initialize any default effect configurations here
    UE_LOG(LogTemp, Log, TEXT("Screen effect defaults initialized"));
}