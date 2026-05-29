#include "VFX_ImpactEffectManager.h"
#include "NiagaraFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"

AVFX_ImpactEffectManager::AVFX_ImpactEffectManager()
{
    PrimaryActorTick.bCanEverTick = false;

    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootScene"));
    RootComponent = RootSceneComponent;

    MaxActiveEffects = 50;
    EffectPoolingRadius = 5000.0f;

    // Initialize impact settings
    ImpactSettings.EffectScale = 1.0f;
    ImpactSettings.Duration = 2.0f;
}

void AVFX_ImpactEffectManager::BeginPlay()
{
    Super::BeginPlay();
    
    // Reserve space for active effects array
    ActiveEffects.Reserve(MaxActiveEffects);
}

void AVFX_ImpactEffectManager::SpawnDustImpact(const FVector& Location, const FVector& Normal, float Intensity)
{
    if (!ImpactSettings.DustEffect.IsValid())
    {
        // Try to load default dust effect
        ImpactSettings.DustEffect = TSoftObjectPtr<UNiagaraSystem>(FSoftObjectPath(TEXT("/Engine/VFX/Niagara/Systems/NS_DefaultDust")));
    }

    UNiagaraSystem* DustSystem = ImpactSettings.DustEffect.LoadSynchronous();
    if (DustSystem)
    {
        FRotator EffectRotation = FRotationMatrix::MakeFromZ(Normal).Rotator();
        float EffectScale = ImpactSettings.EffectScale * Intensity;
        
        SpawnNiagaraEffect(DustSystem, Location, EffectRotation, EffectScale);
        
        UE_LOG(LogTemp, Log, TEXT("VFX: Spawned dust impact at %s with intensity %f"), 
               *Location.ToString(), Intensity);
    }
}

void AVFX_ImpactEffectManager::SpawnBloodSplatter(const FVector& Location, const FVector& Direction, float Amount)
{
    if (!ImpactSettings.BloodEffect.IsValid())
    {
        // Try to load default blood effect
        ImpactSettings.BloodEffect = TSoftObjectPtr<UNiagaraSystem>(FSoftObjectPath(TEXT("/Engine/VFX/Niagara/Systems/NS_DefaultBlood")));
    }

    UNiagaraSystem* BloodSystem = ImpactSettings.BloodEffect.LoadSynchronous();
    if (BloodSystem)
    {
        FRotator EffectRotation = Direction.Rotation();
        float EffectScale = ImpactSettings.EffectScale * Amount;
        
        SpawnNiagaraEffect(BloodSystem, Location, EffectRotation, EffectScale);
        
        UE_LOG(LogTemp, Log, TEXT("VFX: Spawned blood splatter at %s with amount %f"), 
               *Location.ToString(), Amount);
    }
}

void AVFX_ImpactEffectManager::SpawnWaterSplash(const FVector& Location, float SplashSize)
{
    if (!ImpactSettings.WaterSplashEffect.IsValid())
    {
        // Try to load default water splash effect
        ImpactSettings.WaterSplashEffect = TSoftObjectPtr<UNiagaraSystem>(FSoftObjectPath(TEXT("/Engine/VFX/Niagara/Systems/NS_DefaultWaterSplash")));
    }

    UNiagaraSystem* WaterSystem = ImpactSettings.WaterSplashEffect.LoadSynchronous();
    if (WaterSystem)
    {
        FRotator EffectRotation = FRotator::ZeroRotator;
        float EffectScale = ImpactSettings.EffectScale * SplashSize;
        
        SpawnNiagaraEffect(WaterSystem, Location, EffectRotation, EffectScale);
        
        UE_LOG(LogTemp, Log, TEXT("VFX: Spawned water splash at %s with size %f"), 
               *Location.ToString(), SplashSize);
    }
}

void AVFX_ImpactEffectManager::SpawnNiagaraEffect(UNiagaraSystem* System, const FVector& Location, const FRotator& Rotation, float Scale)
{
    if (!System || !GetWorld())
    {
        return;
    }

    // Check if we need to remove old effects
    if (ActiveEffects.Num() >= MaxActiveEffects)
    {
        RemoveOldestEffect();
    }

    // Spawn the Niagara effect
    UNiagaraComponent* NewEffect = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
        GetWorld(),
        System,
        Location,
        Rotation,
        FVector(Scale),
        true,
        true,
        ENCPoolMethod::None,
        true
    );

    if (NewEffect)
    {
        ActiveEffects.Add(NewEffect);
        
        // Set effect duration
        FTimerHandle TimerHandle;
        GetWorld()->GetTimerManager().SetTimer(TimerHandle, [this, NewEffect]()
        {
            if (NewEffect && IsValid(NewEffect))
            {
                ActiveEffects.Remove(NewEffect);
                NewEffect->DestroyComponent();
            }
        }, ImpactSettings.Duration, false);
    }
}

void AVFX_ImpactEffectManager::RemoveOldestEffect()
{
    if (ActiveEffects.Num() > 0)
    {
        UNiagaraComponent* OldestEffect = ActiveEffects[0];
        ActiveEffects.RemoveAt(0);
        
        if (OldestEffect && IsValid(OldestEffect))
        {
            OldestEffect->DestroyComponent();
        }
    }
}

void AVFX_ImpactEffectManager::CleanupExpiredEffects()
{
    for (int32 i = ActiveEffects.Num() - 1; i >= 0; i--)
    {
        UNiagaraComponent* Effect = ActiveEffects[i];
        if (!Effect || !IsValid(Effect) || !Effect->IsActive())
        {
            ActiveEffects.RemoveAt(i);
            if (Effect && IsValid(Effect))
            {
                Effect->DestroyComponent();
            }
        }
    }
}