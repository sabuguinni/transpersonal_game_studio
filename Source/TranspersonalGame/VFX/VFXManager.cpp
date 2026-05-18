#include "VFXManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/SceneComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"

AVFX_Manager::AVFX_Manager()
{
    PrimaryActorTick.bCanEverTick = false;

    // Create root scene component
    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;

    // Initialize default effect data
    FVFX_EffectData DustCloudData;
    DustCloudData.EffectType = EVFX_EffectType::DustCloud;
    DustCloudData.Duration = 3.0f;
    DustCloudData.Intensity = 1.0f;
    DustCloudData.bLooping = false;
    AvailableEffects.Add(DustCloudData);

    FVFX_EffectData BloodData;
    BloodData.EffectType = EVFX_EffectType::BloodSplatter;
    BloodData.Duration = 2.0f;
    BloodData.Intensity = 0.8f;
    BloodData.bLooping = false;
    AvailableEffects.Add(BloodData);

    FVFX_EffectData CampfireData;
    CampfireData.EffectType = EVFX_EffectType::CampfireFire;
    CampfireData.Duration = -1.0f; // Infinite
    CampfireData.Intensity = 1.0f;
    CampfireData.bLooping = true;
    AvailableEffects.Add(CampfireData);

    FVFX_EffectData FootstepData;
    FootstepData.EffectType = EVFX_EffectType::FootstepImpact;
    FootstepData.Duration = 1.5f;
    FootstepData.Intensity = 1.2f;
    FootstepData.bLooping = false;
    AvailableEffects.Add(FootstepData);

    FVFX_EffectData RainData;
    RainData.EffectType = EVFX_EffectType::WeatherRain;
    RainData.Duration = -1.0f; // Infinite
    RainData.Intensity = 0.7f;
    RainData.bLooping = true;
    AvailableEffects.Add(RainData);

    FVFX_EffectData VolcanicData;
    VolcanicData.EffectType = EVFX_EffectType::VolcanicAsh;
    VolcanicData.Duration = 10.0f;
    VolcanicData.Intensity = 0.9f;
    VolcanicData.bLooping = true;
    AvailableEffects.Add(VolcanicData);
}

void AVFX_Manager::BeginPlay()
{
    Super::BeginPlay();
    InitializeEffectComponents();
}

void AVFX_Manager::InitializeEffectComponents()
{
    // Create Niagara components for each effect type
    for (int32 i = 0; i < AvailableEffects.Num(); i++)
    {
        // Create Niagara component
        UNiagaraComponent* NiagaraComp = CreateDefaultSubobject<UNiagaraComponent>(*FString::Printf(TEXT("NiagaraComponent_%d"), i));
        if (NiagaraComp)
        {
            NiagaraComp->SetupAttachment(RootComponent);
            NiagaraComp->SetAutoActivate(false);
            NiagaraComponents.Add(NiagaraComp);
        }

        // Create legacy particle component as backup
        UParticleSystemComponent* ParticleComp = CreateDefaultSubobject<UParticleSystemComponent>(*FString::Printf(TEXT("ParticleComponent_%d"), i));
        if (ParticleComp)
        {
            ParticleComp->SetupAttachment(RootComponent);
            ParticleComp->SetAutoActivate(false);
            ParticleComponents.Add(ParticleComp);
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("VFX Manager initialized with %d effect types"), AvailableEffects.Num());
}

void AVFX_Manager::PlayEffect(EVFX_EffectType EffectType, FVector Location, FRotator Rotation)
{
    FVFX_EffectData* EffectData = GetEffectData(EffectType);
    if (!EffectData)
    {
        UE_LOG(LogTemp, Warning, TEXT("VFX Manager: Effect type not found"));
        return;
    }

    // Try Niagara first
    UNiagaraComponent* NiagaraComp = GetNiagaraComponentForEffect(EffectType);
    if (NiagaraComp && EffectData->NiagaraEffect)
    {
        NiagaraComp->SetAsset(EffectData->NiagaraEffect);
        NiagaraComp->SetWorldLocationAndRotation(Location, Rotation);
        NiagaraComp->SetFloatParameter(TEXT("Intensity"), EffectData->Intensity);
        NiagaraComp->Activate(true);
        
        if (!EffectData->bLooping && EffectData->Duration > 0.0f)
        {
            // Auto-deactivate after duration
            FTimerHandle TimerHandle;
            GetWorld()->GetTimerManager().SetTimer(TimerHandle, [NiagaraComp]()
            {
                if (IsValid(NiagaraComp))
                {
                    NiagaraComp->Deactivate();
                }
            }, EffectData->Duration, false);
        }

        UE_LOG(LogTemp, Log, TEXT("VFX Manager: Playing Niagara effect %s at location %s"), 
               *UEnum::GetValueAsString(EffectType), *Location.ToString());
        return;
    }

    // Fallback to legacy particle system
    UParticleSystemComponent* ParticleComp = GetParticleComponentForEffect(EffectType);
    if (ParticleComp && EffectData->LegacyEffect)
    {
        ParticleComp->SetTemplate(EffectData->LegacyEffect);
        ParticleComp->SetWorldLocationAndRotation(Location, Rotation);
        ParticleComp->Activate(true);
        
        UE_LOG(LogTemp, Log, TEXT("VFX Manager: Playing legacy particle effect %s"), 
               *UEnum::GetValueAsString(EffectType));
    }
}

void AVFX_Manager::StopEffect(EVFX_EffectType EffectType)
{
    UNiagaraComponent* NiagaraComp = GetNiagaraComponentForEffect(EffectType);
    if (NiagaraComp)
    {
        NiagaraComp->Deactivate();
    }

    UParticleSystemComponent* ParticleComp = GetParticleComponentForEffect(EffectType);
    if (ParticleComp)
    {
        ParticleComp->Deactivate();
    }

    UE_LOG(LogTemp, Log, TEXT("VFX Manager: Stopped effect %s"), *UEnum::GetValueAsString(EffectType));
}

void AVFX_Manager::StopAllEffects()
{
    for (UNiagaraComponent* NiagaraComp : NiagaraComponents)
    {
        if (IsValid(NiagaraComp))
        {
            NiagaraComp->Deactivate();
        }
    }

    for (UParticleSystemComponent* ParticleComp : ParticleComponents)
    {
        if (IsValid(ParticleComp))
        {
            ParticleComp->Deactivate();
        }
    }

    UE_LOG(LogTemp, Log, TEXT("VFX Manager: All effects stopped"));
}

void AVFX_Manager::PlayDinosaurFootstepEffect(FVector ImpactLocation, float DinosaurSize)
{
    // Scale effect intensity based on dinosaur size
    FVFX_EffectData* EffectData = GetEffectData(EVFX_EffectType::FootstepImpact);
    if (EffectData)
    {
        float OriginalIntensity = EffectData->Intensity;
        EffectData->Intensity = FMath::Clamp(DinosaurSize * 1.2f, 0.5f, 3.0f);
        
        PlayEffect(EVFX_EffectType::FootstepImpact, ImpactLocation);
        PlayEffect(EVFX_EffectType::DustCloud, ImpactLocation);
        
        // Restore original intensity
        EffectData->Intensity = OriginalIntensity;
    }

    UE_LOG(LogTemp, Log, TEXT("VFX Manager: Dinosaur footstep effect at %s (size: %.2f)"), 
           *ImpactLocation.ToString(), DinosaurSize);
}

void AVFX_Manager::PlayBloodSplatterEffect(FVector HitLocation, FVector HitNormal)
{
    // Calculate rotation based on hit normal
    FRotator EffectRotation = FRotationMatrix::MakeFromZ(HitNormal).Rotator();
    PlayEffect(EVFX_EffectType::BloodSplatter, HitLocation, EffectRotation);

    UE_LOG(LogTemp, Log, TEXT("VFX Manager: Blood splatter effect at %s"), *HitLocation.ToString());
}

void AVFX_Manager::PlayCampfireEffect(FVector FireLocation, bool bStartFire)
{
    if (bStartFire)
    {
        PlayEffect(EVFX_EffectType::CampfireFire, FireLocation);
    }
    else
    {
        StopEffect(EVFX_EffectType::CampfireFire);
    }

    UE_LOG(LogTemp, Log, TEXT("VFX Manager: Campfire effect %s at %s"), 
           bStartFire ? TEXT("started") : TEXT("stopped"), *FireLocation.ToString());
}

void AVFX_Manager::SetEffectIntensity(EVFX_EffectType EffectType, float NewIntensity)
{
    FVFX_EffectData* EffectData = GetEffectData(EffectType);
    if (EffectData)
    {
        EffectData->Intensity = FMath::Clamp(NewIntensity, 0.0f, 5.0f);
        
        // Update active Niagara component
        UNiagaraComponent* NiagaraComp = GetNiagaraComponentForEffect(EffectType);
        if (NiagaraComp && NiagaraComp->IsActive())
        {
            NiagaraComp->SetFloatParameter(TEXT("Intensity"), EffectData->Intensity);
        }

        UE_LOG(LogTemp, Log, TEXT("VFX Manager: Set intensity %.2f for effect %s"), 
               NewIntensity, *UEnum::GetValueAsString(EffectType));
    }
}

bool AVFX_Manager::IsEffectPlaying(EVFX_EffectType EffectType) const
{
    UNiagaraComponent* NiagaraComp = GetNiagaraComponentForEffect(EffectType);
    if (NiagaraComp)
    {
        return NiagaraComp->IsActive();
    }

    UParticleSystemComponent* ParticleComp = GetParticleComponentForEffect(EffectType);
    if (ParticleComp)
    {
        return ParticleComp->IsActive();
    }

    return false;
}

UNiagaraComponent* AVFX_Manager::GetNiagaraComponentForEffect(EVFX_EffectType EffectType)
{
    int32 EffectIndex = static_cast<int32>(EffectType);
    if (NiagaraComponents.IsValidIndex(EffectIndex))
    {
        return NiagaraComponents[EffectIndex];
    }
    return nullptr;
}

UParticleSystemComponent* AVFX_Manager::GetParticleComponentForEffect(EVFX_EffectType EffectType)
{
    int32 EffectIndex = static_cast<int32>(EffectType);
    if (ParticleComponents.IsValidIndex(EffectIndex))
    {
        return ParticleComponents[EffectIndex];
    }
    return nullptr;
}

FVFX_EffectData* AVFX_Manager::GetEffectData(EVFX_EffectType EffectType)
{
    for (FVFX_EffectData& EffectData : AvailableEffects)
    {
        if (EffectData.EffectType == EffectType)
        {
            return &EffectData;
        }
    }
    return nullptr;
}