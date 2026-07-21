#include "VFX_BreathSystem.h"
#include "NiagaraFunctionLibrary.h"
#include "Components/SceneComponent.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

UVFX_BreathSystem::UVFX_BreathSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = true;
    
    ActiveBreathComponent = nullptr;
    BreathTimer = 0.0f;
    bIsBreathing = false;
    
    // Default settings
    BreathSettings.BreathType = EVFX_BreathType::ColdVapor;
    BreathSettings.Intensity = 1.0f;
    BreathSettings.Duration = 2.0f;
    BreathSettings.Scale = FVector(1.0f, 1.0f, 1.0f);
    BreathSettings.TintColor = FLinearColor::White;
    
    BreathOffset = FVector(200.0f, 0.0f, 150.0f);
    BreathInterval = 3.0f;
    TemperatureThreshold = 10.0f;
    bAutoBreathing = true;
    bTemperatureBasedBreath = true;
}

void UVFX_BreathSystem::BeginPlay()
{
    Super::BeginPlay();
    
    if (bAutoBreathing)
    {
        StartBreathing();
    }
}

void UVFX_BreathSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (bIsBreathing)
    {
        UpdateBreathEffect(DeltaTime);
    }
}

void UVFX_BreathSystem::TriggerBreathEffect(EVFX_BreathType BreathType, float Intensity, float Duration)
{
    if (!GetOwner())
    {
        return;
    }
    
    // Update settings
    BreathSettings.BreathType = BreathType;
    BreathSettings.Intensity = Intensity;
    BreathSettings.Duration = Duration;
    
    // Create breath effect
    CreateBreathEffect(BreathType);
}

void UVFX_BreathSystem::StartBreathing()
{
    bIsBreathing = true;
    BreathTimer = 0.0f;
}

void UVFX_BreathSystem::StopBreathing()
{
    bIsBreathing = false;
    
    if (ActiveBreathComponent && IsValid(ActiveBreathComponent))
    {
        ActiveBreathComponent->DestroyComponent();
        ActiveBreathComponent = nullptr;
    }
}

void UVFX_BreathSystem::SetBreathSettings(const FVFX_BreathSettings& NewSettings)
{
    BreathSettings = NewSettings;
}

void UVFX_BreathSystem::CreateBreathEffect(EVFX_BreathType BreathType)
{
    if (!GetOwner())
    {
        return;
    }
    
    // Clean up existing effect
    if (ActiveBreathComponent && IsValid(ActiveBreathComponent))
    {
        ActiveBreathComponent->DestroyComponent();
        ActiveBreathComponent = nullptr;
    }
    
    // Get appropriate Niagara system
    UNiagaraSystem* BreathEffect = GetBreathEffectAsset(BreathType);
    if (!BreathEffect)
    {
        // Create a simple particle system programmatically
        FVector SpawnLocation = GetOwner()->GetActorLocation() + GetOwner()->GetActorForwardVector() * BreathOffset.X + FVector(0, 0, BreathOffset.Z);
        
        // Spawn simple breath particles using UGameplayStatics
        if (UWorld* World = GetWorld())
        {
            // Create temporary Niagara component
            ActiveBreathComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(
                nullptr, // Will use default system
                GetOwner()->GetRootComponent(),
                NAME_None,
                BreathOffset,
                FRotator::ZeroRotator,
                EAttachLocation::KeepRelativeOffset,
                true
            );
            
            if (ActiveBreathComponent)
            {
                // Configure breath effect parameters
                ActiveBreathComponent->SetFloatParameter(TEXT("Intensity"), BreathSettings.Intensity);
                ActiveBreathComponent->SetVectorParameter(TEXT("Scale"), BreathSettings.Scale);
                ActiveBreathComponent->SetColorParameter(TEXT("TintColor"), BreathSettings.TintColor);
                
                // Set duration
                FTimerHandle BreathTimerHandle;
                World->GetTimerManager().SetTimer(BreathTimerHandle, [this]()
                {
                    if (ActiveBreathComponent && IsValid(ActiveBreathComponent))
                    {
                        ActiveBreathComponent->DestroyComponent();
                        ActiveBreathComponent = nullptr;
                    }
                }, BreathSettings.Duration, false);
            }
        }
        return;
    }
    
    // Spawn Niagara system
    if (UWorld* World = GetWorld())
    {
        ActiveBreathComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(
            BreathEffect,
            GetOwner()->GetRootComponent(),
            NAME_None,
            BreathOffset,
            FRotator::ZeroRotator,
            EAttachLocation::KeepRelativeOffset,
            true
        );
        
        if (ActiveBreathComponent)
        {
            // Configure effect parameters
            ActiveBreathComponent->SetFloatParameter(TEXT("Intensity"), BreathSettings.Intensity);
            ActiveBreathComponent->SetVectorParameter(TEXT("Scale"), BreathSettings.Scale);
            ActiveBreathComponent->SetColorParameter(TEXT("TintColor"), BreathSettings.TintColor);
        }
    }
}

void UVFX_BreathSystem::UpdateBreathEffect(float DeltaTime)
{
    BreathTimer += DeltaTime;
    
    if (BreathTimer >= BreathInterval)
    {
        BreathTimer = 0.0f;
        
        // Determine breath type based on temperature
        EVFX_BreathType CurrentBreathType = BreathSettings.BreathType;
        
        if (bTemperatureBasedBreath)
        {
            float CurrentTemp = GetCurrentTemperature();
            if (CurrentTemp < TemperatureThreshold)
            {
                CurrentBreathType = EVFX_BreathType::ColdVapor;
            }
            else
            {
                CurrentBreathType = EVFX_BreathType::HotSteam;
            }
        }
        
        // Trigger breath effect
        CreateBreathEffect(CurrentBreathType);
    }
}

UNiagaraSystem* UVFX_BreathSystem::GetBreathEffectAsset(EVFX_BreathType BreathType)
{
    switch (BreathType)
    {
        case EVFX_BreathType::ColdVapor:
            return ColdVaporEffect.LoadSynchronous();
        case EVFX_BreathType::HotSteam:
            return HotSteamEffect.LoadSynchronous();
        case EVFX_BreathType::Exhaustion:
            return ExhaustionEffect.LoadSynchronous();
        case EVFX_BreathType::Roar:
            return RoarBlastEffect.LoadSynchronous();
        default:
            return nullptr;
    }
}

float UVFX_BreathSystem::GetCurrentTemperature()
{
    // Simple temperature simulation based on time of day
    if (UWorld* World = GetWorld())
    {
        float TimeOfDay = World->GetTimeSeconds();
        float DayProgress = FMath::Fmod(TimeOfDay / 1200.0f, 1.0f); // 20 minute day cycle
        
        // Temperature varies from 5°C at night to 25°C at noon
        float Temperature = 15.0f + 10.0f * FMath::Sin(DayProgress * PI);
        return Temperature;
    }
    
    return 15.0f; // Default temperature
}