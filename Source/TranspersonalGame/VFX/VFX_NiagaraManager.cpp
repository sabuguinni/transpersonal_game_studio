#include "VFX_NiagaraManager.h"
#include "NiagaraFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"

AVFX_NiagaraManager::AVFX_NiagaraManager()
{
    PrimaryActorTick.bCanEverTick = false;

    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;

    MaxEffectDistance = 5000.0f;
    MaxActiveEffects = 50;

    LoadDefaultEffects();
}

void AVFX_NiagaraManager::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Warning, TEXT("VFX Niagara Manager initialized with %d effect types"), ImpactEffects.Num());
}

void AVFX_NiagaraManager::LoadDefaultEffects()
{
    // Initialize default Niagara systems for each impact type
    // These would be loaded from content browser paths in a real implementation
    
    // For now, we set up the mapping structure
    ImpactEffects.Empty();
    
    // Note: In production, these would load actual Niagara system assets
    // ImpactEffects.Add(EVFX_ImpactType::FootstepLight, LoadObject<UNiagaraSystem>(nullptr, TEXT("/Game/VFX/NS_FootstepLight")));
    // ImpactEffects.Add(EVFX_ImpactType::FootstepHeavy, LoadObject<UNiagaraSystem>(nullptr, TEXT("/Game/VFX/NS_FootstepHeavy")));
    
    UE_LOG(LogTemp, Log, TEXT("VFX default effects structure initialized"));
}

void AVFX_NiagaraManager::PlayImpactEffect(const FVFX_ImpactData& ImpactData)
{
    if (!IsLocationValid(ImpactData.ImpactLocation))
    {
        return;
    }

    if (ActiveEffects.Num() >= MaxActiveEffects)
    {
        CleanupFinishedEffects();
        
        if (ActiveEffects.Num() >= MaxActiveEffects)
        {
            UE_LOG(LogTemp, Warning, TEXT("VFX Manager: Max active effects reached, skipping new effect"));
            return;
        }
    }

    UNiagaraSystem** FoundEffect = ImpactEffects.Find(ImpactData.ImpactType);
    if (FoundEffect && *FoundEffect)
    {
        FRotator EffectRotation = FRotationMatrix::MakeFromZ(ImpactData.ImpactNormal).Rotator();
        UNiagaraComponent* NewEffect = CreateEffectComponent(*FoundEffect, ImpactData.ImpactLocation, EffectRotation);
        
        if (NewEffect)
        {
            // Set effect parameters based on impact data
            NewEffect->SetFloatParameter(TEXT("Intensity"), ImpactData.ImpactIntensity);
            NewEffect->SetFloatParameter(TEXT("Duration"), ImpactData.EffectDuration);
            
            ActiveEffects.Add(NewEffect);
            
            UE_LOG(LogTemp, Log, TEXT("VFX Impact effect played at location: %s"), *ImpactData.ImpactLocation.ToString());
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("VFX Manager: No effect found for impact type"));
    }
}

void AVFX_NiagaraManager::PlayFootstepEffect(FVector Location, bool bIsHeavyFootstep)
{
    FVFX_ImpactData FootstepData;
    FootstepData.ImpactType = bIsHeavyFootstep ? EVFX_ImpactType::FootstepHeavy : EVFX_ImpactType::FootstepLight;
    FootstepData.ImpactLocation = Location;
    FootstepData.ImpactNormal = FVector::UpVector;
    FootstepData.ImpactIntensity = bIsHeavyFootstep ? 2.0f : 1.0f;
    FootstepData.EffectDuration = bIsHeavyFootstep ? 3.0f : 1.5f;
    
    PlayImpactEffect(FootstepData);
}

void AVFX_NiagaraManager::PlayBloodEffect(FVector Location, FVector Direction, float Intensity)
{
    FVFX_ImpactData BloodData;
    BloodData.ImpactType = EVFX_ImpactType::BloodSplatter;
    BloodData.ImpactLocation = Location;
    BloodData.ImpactNormal = Direction.GetSafeNormal();
    BloodData.ImpactIntensity = Intensity;
    BloodData.EffectDuration = 4.0f;
    
    PlayImpactEffect(BloodData);
}

void AVFX_NiagaraManager::PlayFireEffect(FVector Location, float Duration)
{
    FVFX_ImpactData FireData;
    FireData.ImpactType = EVFX_ImpactType::FireSparks;
    FireData.ImpactLocation = Location;
    FireData.ImpactNormal = FVector::UpVector;
    FireData.ImpactIntensity = 1.5f;
    FireData.EffectDuration = Duration;
    
    PlayImpactEffect(FireData);
}

void AVFX_NiagaraManager::PlayWaterSplashEffect(FVector Location, float Intensity)
{
    FVFX_ImpactData WaterData;
    WaterData.ImpactType = EVFX_ImpactType::WaterSplash;
    WaterData.ImpactLocation = Location;
    WaterData.ImpactNormal = FVector::UpVector;
    WaterData.ImpactIntensity = Intensity;
    WaterData.EffectDuration = 2.5f;
    
    PlayImpactEffect(WaterData);
}

void AVFX_NiagaraManager::CleanupFinishedEffects()
{
    for (int32 i = ActiveEffects.Num() - 1; i >= 0; i--)
    {
        UNiagaraComponent* Effect = ActiveEffects[i];
        if (!Effect || !Effect->IsActive() || Effect->IsBeingDestroyed())
        {
            if (Effect && !Effect->IsBeingDestroyed())
            {
                Effect->DestroyComponent();
            }
            ActiveEffects.RemoveAt(i);
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("VFX cleanup complete. Active effects: %d"), ActiveEffects.Num());
}

void AVFX_NiagaraManager::StopAllEffects()
{
    for (UNiagaraComponent* Effect : ActiveEffects)
    {
        if (Effect && !Effect->IsBeingDestroyed())
        {
            Effect->Deactivate();
            Effect->DestroyComponent();
        }
    }
    
    ActiveEffects.Empty();
    UE_LOG(LogTemp, Log, TEXT("All VFX effects stopped and cleared"));
}

UNiagaraComponent* AVFX_NiagaraManager::CreateEffectComponent(UNiagaraSystem* NiagaraSystem, const FVector& Location, const FRotator& Rotation)
{
    if (!NiagaraSystem)
    {
        return nullptr;
    }

    UNiagaraComponent* NewComponent = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
        GetWorld(),
        NiagaraSystem,
        Location,
        Rotation,
        FVector::OneVector,
        true,
        true,
        ENCPoolMethod::None,
        true
    );

    return NewComponent;
}

bool AVFX_NiagaraManager::IsLocationValid(const FVector& Location) const
{
    // Check if location is within reasonable bounds
    if (Location.Z < -10000.0f || Location.Z > 10000.0f)
    {
        return false;
    }
    
    // Check distance from manager
    float Distance = FVector::Dist(GetActorLocation(), Location);
    if (Distance > MaxEffectDistance)
    {
        return false;
    }
    
    return true;
}