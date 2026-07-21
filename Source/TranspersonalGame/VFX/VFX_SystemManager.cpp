#include "VFX_SystemManager.h"
#include "NiagaraFunctionLibrary.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/SceneComponent.h"

AVFX_SystemManager::AVFX_SystemManager()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create root scene component
    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;

    // Initialize default values
    GlobalVFXIntensity = 1.0f;

    // Initialize available effects
    AvailableEffects.Empty();
    
    // Fire effect data
    FVFX_EffectData FireEffect;
    FireEffect.EffectType = EVFX_EffectType::Fire;
    FireEffect.Duration = 0.0f; // Continuous
    FireEffect.Intensity = 1.0f;
    FireEffect.bLooping = true;
    AvailableEffects.Add(FireEffect);

    // Dust effect data
    FVFX_EffectData DustEffect;
    DustEffect.EffectType = EVFX_EffectType::Dust;
    DustEffect.Duration = 3.0f;
    DustEffect.Intensity = 0.8f;
    DustEffect.bLooping = false;
    AvailableEffects.Add(DustEffect);

    // Blood effect data
    FVFX_EffectData BloodEffect;
    BloodEffect.EffectType = EVFX_EffectType::Blood;
    BloodEffect.Duration = 2.0f;
    BloodEffect.Intensity = 1.0f;
    BloodEffect.bLooping = false;
    AvailableEffects.Add(BloodEffect);

    // Rain effect data
    FVFX_EffectData RainEffect;
    RainEffect.EffectType = EVFX_EffectType::Rain;
    RainEffect.Duration = 0.0f; // Continuous
    RainEffect.Intensity = 0.6f;
    RainEffect.bLooping = true;
    AvailableEffects.Add(RainEffect);
}

void AVFX_SystemManager::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeEffectSystems();
    
    // Start cleanup timer
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().SetTimer(
            CleanupTimerHandle,
            this,
            &AVFX_SystemManager::CleanupExpiredEffects,
            5.0f,
            true
        );
    }

    UE_LOG(LogTemp, Warning, TEXT("VFX System Manager initialized with %d effect types"), AvailableEffects.Num());
}

void AVFX_SystemManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // Update active effects intensity based on global setting
    for (UNiagaraComponent* Effect : ActiveEffects)
    {
        if (Effect && IsValid(Effect))
        {
            Effect->SetFloatParameter(TEXT("GlobalIntensity"), GlobalVFXIntensity);
        }
    }
}

void AVFX_SystemManager::SpawnEffect(EVFX_EffectType EffectType, FVector Location, FRotator Rotation, float Scale)
{
    // Find effect data
    FVFX_EffectData* EffectData = AvailableEffects.FindByPredicate([EffectType](const FVFX_EffectData& Data)
    {
        return Data.EffectType == EffectType;
    });

    if (!EffectData)
    {
        UE_LOG(LogTemp, Warning, TEXT("VFX Effect type not found: %d"), (int32)EffectType);
        return;
    }

    // Get Niagara system from map
    TSoftObjectPtr<UNiagaraSystem>* SystemPtr = EffectSystemMap.Find(EffectType);
    if (SystemPtr && SystemPtr->IsValid())
    {
        UNiagaraSystem* System = SystemPtr->Get();
        if (System)
        {
            UNiagaraComponent* NewEffect = CreateNiagaraComponent(System, Location, Rotation, Scale);
            if (NewEffect)
            {
                ActiveEffects.Add(NewEffect);
                UE_LOG(LogTemp, Log, TEXT("VFX Effect spawned: %s at %s"), 
                    *UEnum::GetValueAsString(EffectType), 
                    *Location.ToString());
            }
        }
    }
    else
    {
        // Fallback: Use UNiagaraFunctionLibrary for basic effects
        if (GetWorld())
        {
            UNiagaraComponent* NewEffect = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
                GetWorld(),
                nullptr, // Will use default system
                Location,
                Rotation,
                FVector(Scale),
                true,
                true,
                ENCPoolMethod::None
            );
            
            if (NewEffect)
            {
                ActiveEffects.Add(NewEffect);
                UE_LOG(LogTemp, Log, TEXT("VFX Effect spawned (fallback): %s"), *UEnum::GetValueAsString(EffectType));
            }
        }
    }
}

void AVFX_SystemManager::SpawnFireEffect(FVector Location, float Intensity)
{
    SpawnEffect(EVFX_EffectType::Fire, Location, FRotator::ZeroRotator, Intensity);
}

void AVFX_SystemManager::SpawnDustEffect(FVector Location, float Intensity)
{
    SpawnEffect(EVFX_EffectType::Dust, Location, FRotator::ZeroRotator, Intensity);
}

void AVFX_SystemManager::SpawnBloodEffect(FVector Location, float Intensity)
{
    SpawnEffect(EVFX_EffectType::Blood, Location, FRotator::ZeroRotator, Intensity);
}

void AVFX_SystemManager::SpawnWeatherEffect(EVFX_EffectType WeatherType, float Intensity)
{
    FVector WeatherLocation = GetActorLocation() + FVector(0, 0, 1000); // High altitude
    SpawnEffect(WeatherType, WeatherLocation, FRotator::ZeroRotator, Intensity);
}

void AVFX_SystemManager::ClearAllEffects()
{
    for (UNiagaraComponent* Effect : ActiveEffects)
    {
        if (Effect && IsValid(Effect))
        {
            Effect->DestroyComponent();
        }
    }
    ActiveEffects.Empty();
    UE_LOG(LogTemp, Log, TEXT("All VFX effects cleared"));
}

void AVFX_SystemManager::SetGlobalVFXIntensity(float NewIntensity)
{
    GlobalVFXIntensity = FMath::Clamp(NewIntensity, 0.0f, 2.0f);
    UE_LOG(LogTemp, Log, TEXT("Global VFX intensity set to: %f"), GlobalVFXIntensity);
}

void AVFX_SystemManager::InitializeEffectSystems()
{
    // Initialize effect system map with default paths
    // In a real project, these would be loaded from asset references
    EffectSystemMap.Empty();
    
    // Note: These paths would need to be updated with actual Niagara system assets
    // For now, we'll use nullptr and rely on fallback spawning
    EffectSystemMap.Add(EVFX_EffectType::Fire, nullptr);
    EffectSystemMap.Add(EVFX_EffectType::Dust, nullptr);
    EffectSystemMap.Add(EVFX_EffectType::Blood, nullptr);
    EffectSystemMap.Add(EVFX_EffectType::Rain, nullptr);
    EffectSystemMap.Add(EVFX_EffectType::Fog, nullptr);
    EffectSystemMap.Add(EVFX_EffectType::Ash, nullptr);
    EffectSystemMap.Add(EVFX_EffectType::Insects, nullptr);
    
    UE_LOG(LogTemp, Log, TEXT("VFX effect systems initialized"));
}

void AVFX_SystemManager::CleanupExpiredEffects()
{
    // Remove null or invalid effects
    ActiveEffects.RemoveAll([](UNiagaraComponent* Effect)
    {
        return !Effect || !IsValid(Effect) || Effect->IsBeingDestroyed();
    });
    
    // Remove effects that have finished playing (non-looping)
    ActiveEffects.RemoveAll([](UNiagaraComponent* Effect)
    {
        if (Effect && IsValid(Effect))
        {
            return !Effect->IsActive() && Effect->GetAsset() != nullptr;
        }
        return true;
    });
}

UNiagaraComponent* AVFX_SystemManager::CreateNiagaraComponent(UNiagaraSystem* System, FVector Location, FRotator Rotation, float Scale)
{
    if (!System || !GetWorld())
    {
        return nullptr;
    }

    UNiagaraComponent* NewComponent = NewObject<UNiagaraComponent>(this);
    if (NewComponent)
    {
        NewComponent->SetAsset(System);
        NewComponent->SetWorldLocation(Location);
        NewComponent->SetWorldRotation(Rotation);
        NewComponent->SetWorldScale3D(FVector(Scale));
        NewComponent->SetAutoDestroy(true);
        NewComponent->Activate();
        
        return NewComponent;
    }
    
    return nullptr;
}