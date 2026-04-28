#include "VFX_ImpactManager.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

AVFX_ImpactManager::AVFX_ImpactManager()
{
    PrimaryActorTick.bCanEverTick = false;

    // Initialize default values
    GlobalEffectScale = 1.0f;
    bAudioEnabled = true;
    AudioVolumeMultiplier = 1.0f;

    // Create root component
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

    // Initialize Niagara components
    InitializeNiagaraComponents();

    // Initialize audio component
    InitializeAudioComponent();
}

void AVFX_ImpactManager::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Log, TEXT("VFX Impact Manager initialized"));
}

void AVFX_ImpactManager::InitializeNiagaraComponents()
{
    // Create Niagara components for different effect types
    DustImpactComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("DustImpactComponent"));
    if (DustImpactComponent)
    {
        DustImpactComponent->SetupAttachment(RootComponent);
        DustImpactComponent->SetAutoActivate(false);
    }

    BloodSplatterComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("BloodSplatterComponent"));
    if (BloodSplatterComponent)
    {
        BloodSplatterComponent->SetupAttachment(RootComponent);
        BloodSplatterComponent->SetAutoActivate(false);
    }

    WaterSplashComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("WaterSplashComponent"));
    if (WaterSplashComponent)
    {
        WaterSplashComponent->SetupAttachment(RootComponent);
        WaterSplashComponent->SetAutoActivate(false);
    }

    SparksComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("SparksComponent"));
    if (SparksComponent)
    {
        SparksComponent->SetupAttachment(RootComponent);
        SparksComponent->SetAutoActivate(false);
    }
}

void AVFX_ImpactManager::InitializeAudioComponent()
{
    ImpactAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("ImpactAudioComponent"));
    if (ImpactAudioComponent)
    {
        ImpactAudioComponent->SetupAttachment(RootComponent);
        ImpactAudioComponent->SetAutoActivate(false);
    }
}

void AVFX_ImpactManager::PlayImpactEffect(const FVFX_ImpactData& ImpactData)
{
    if (!IsValid(this))
    {
        return;
    }

    // Get the appropriate Niagara component for this impact type
    UNiagaraComponent* EffectComponent = GetComponentForImpactType(ImpactData.ImpactType);
    
    if (EffectComponent && EffectComponent->GetAsset())
    {
        // Set the component location
        EffectComponent->SetWorldLocation(ImpactData.ImpactLocation);
        
        // Set effect parameters
        EffectComponent->SetNiagaraVariableFloat(TEXT("EffectScale"), ImpactData.EffectScale * GlobalEffectScale);
        EffectComponent->SetNiagaraVariableFloat(TEXT("ImpactForce"), ImpactData.ImpactForce);
        EffectComponent->SetNiagaraVariableVec3(TEXT("ImpactNormal"), ImpactData.ImpactNormal);
        
        // Activate the effect
        EffectComponent->Activate();
        EffectComponent->ResetSystem();
    }
    else
    {
        // Fallback: Spawn one-shot Niagara effect at location
        UNiagaraSystem* SystemToUse = nullptr;
        
        switch (ImpactData.ImpactType)
        {
            case EVFX_ImpactType::DinosaurFootstep:
            case EVFX_ImpactType::PlayerFootstep:
            case EVFX_ImpactType::DustCloud:
                SystemToUse = DustImpactSystem;
                break;
            case EVFX_ImpactType::BloodSplatter:
                SystemToUse = BloodSplatterSystem;
                break;
            case EVFX_ImpactType::WaterSplash:
                SystemToUse = WaterSplashSystem;
                break;
            case EVFX_ImpactType::Sparks:
                SystemToUse = SparksSystem;
                break;
            default:
                SystemToUse = DustImpactSystem; // Default fallback
                break;
        }
        
        if (SystemToUse)
        {
            UNiagaraFunctionLibrary::SpawnSystemAtLocation(
                GetWorld(),
                SystemToUse,
                ImpactData.ImpactLocation,
                FRotator::ZeroRotator,
                FVector(ImpactData.EffectScale * GlobalEffectScale)
            );
        }
    }

    // Play impact sound if enabled
    if (ImpactData.bPlaySound && bAudioEnabled)
    {
        PlayImpactSound(ImpactData.ImpactType, ImpactData.SurfaceType, ImpactData.ImpactLocation);
    }
}

void AVFX_ImpactManager::PlayDinosaurFootstep(FVector Location, EVFX_SurfaceType SurfaceType, float Scale)
{
    FVFX_ImpactData ImpactData;
    ImpactData.ImpactType = EVFX_ImpactType::DinosaurFootstep;
    ImpactData.SurfaceType = SurfaceType;
    ImpactData.ImpactLocation = Location;
    ImpactData.ImpactNormal = FVector::UpVector;
    ImpactData.ImpactForce = 5.0f; // Heavy dinosaur impact
    ImpactData.EffectScale = Scale;
    ImpactData.bPlaySound = true;

    PlayImpactEffect(ImpactData);
}

void AVFX_ImpactManager::PlayPlayerFootstep(FVector Location, EVFX_SurfaceType SurfaceType, float Scale)
{
    FVFX_ImpactData ImpactData;
    ImpactData.ImpactType = EVFX_ImpactType::PlayerFootstep;
    ImpactData.SurfaceType = SurfaceType;
    ImpactData.ImpactLocation = Location;
    ImpactData.ImpactNormal = FVector::UpVector;
    ImpactData.ImpactForce = 1.0f; // Light human impact
    ImpactData.EffectScale = Scale;
    ImpactData.bPlaySound = true;

    PlayImpactEffect(ImpactData);
}

void AVFX_ImpactManager::PlayBloodSplatter(FVector Location, FVector Normal, float Intensity)
{
    FVFX_ImpactData ImpactData;
    ImpactData.ImpactType = EVFX_ImpactType::BloodSplatter;
    ImpactData.SurfaceType = EVFX_SurfaceType::Dirt; // Surface doesn't matter for blood
    ImpactData.ImpactLocation = Location;
    ImpactData.ImpactNormal = Normal;
    ImpactData.ImpactForce = Intensity;
    ImpactData.EffectScale = Intensity;
    ImpactData.bPlaySound = false; // Blood splatter is usually silent

    PlayImpactEffect(ImpactData);
}

void AVFX_ImpactManager::PlayWaterSplash(FVector Location, float Scale)
{
    FVFX_ImpactData ImpactData;
    ImpactData.ImpactType = EVFX_ImpactType::WaterSplash;
    ImpactData.SurfaceType = EVFX_SurfaceType::Water;
    ImpactData.ImpactLocation = Location;
    ImpactData.ImpactNormal = FVector::UpVector;
    ImpactData.ImpactForce = 2.0f;
    ImpactData.EffectScale = Scale;
    ImpactData.bPlaySound = true;

    PlayImpactEffect(ImpactData);
}

void AVFX_ImpactManager::PlaySparks(FVector Location, FVector Direction, float Intensity)
{
    FVFX_ImpactData ImpactData;
    ImpactData.ImpactType = EVFX_ImpactType::Sparks;
    ImpactData.SurfaceType = EVFX_SurfaceType::Rock; // Sparks usually from stone on stone
    ImpactData.ImpactLocation = Location;
    ImpactData.ImpactNormal = Direction;
    ImpactData.ImpactForce = Intensity;
    ImpactData.EffectScale = Intensity;
    ImpactData.bPlaySound = true;

    PlayImpactEffect(ImpactData);
}

void AVFX_ImpactManager::SetEffectScale(float NewScale)
{
    GlobalEffectScale = FMath::Clamp(NewScale, 0.1f, 10.0f);
}

void AVFX_ImpactManager::EnableAudio(bool bEnable)
{
    bAudioEnabled = bEnable;
}

void AVFX_ImpactManager::StopAllEffects()
{
    if (DustImpactComponent)
    {
        DustImpactComponent->Deactivate();
    }
    if (BloodSplatterComponent)
    {
        BloodSplatterComponent->Deactivate();
    }
    if (WaterSplashComponent)
    {
        WaterSplashComponent->Deactivate();
    }
    if (SparksComponent)
    {
        SparksComponent->Deactivate();
    }
    if (ImpactAudioComponent)
    {
        ImpactAudioComponent->Stop();
    }
}

UNiagaraComponent* AVFX_ImpactManager::GetComponentForImpactType(EVFX_ImpactType ImpactType)
{
    switch (ImpactType)
    {
        case EVFX_ImpactType::DinosaurFootstep:
        case EVFX_ImpactType::PlayerFootstep:
        case EVFX_ImpactType::DustCloud:
            return DustImpactComponent;
        case EVFX_ImpactType::BloodSplatter:
            return BloodSplatterComponent;
        case EVFX_ImpactType::WaterSplash:
            return WaterSplashComponent;
        case EVFX_ImpactType::Sparks:
            return SparksComponent;
        default:
            return DustImpactComponent; // Default fallback
    }
}

USoundCue* AVFX_ImpactManager::GetSoundForImpactType(EVFX_ImpactType ImpactType, EVFX_SurfaceType SurfaceType)
{
    switch (ImpactType)
    {
        case EVFX_ImpactType::DinosaurFootstep:
            return DinosaurFootstepSound;
        case EVFX_ImpactType::PlayerFootstep:
            return PlayerFootstepSound;
        case EVFX_ImpactType::RockImpact:
            return RockImpactSound;
        case EVFX_ImpactType::WoodImpact:
            return WoodImpactSound;
        case EVFX_ImpactType::WaterSplash:
            return WaterSplashSound;
        default:
            return nullptr;
    }
}

void AVFX_ImpactManager::PlayImpactSound(EVFX_ImpactType ImpactType, EVFX_SurfaceType SurfaceType, FVector Location)
{
    USoundCue* SoundToPlay = GetSoundForImpactType(ImpactType, SurfaceType);
    
    if (SoundToPlay && ImpactAudioComponent)
    {
        ImpactAudioComponent->SetSound(SoundToPlay);
        ImpactAudioComponent->SetWorldLocation(Location);
        ImpactAudioComponent->SetVolumeMultiplier(AudioVolumeMultiplier);
        ImpactAudioComponent->Play();
    }
    else if (SoundToPlay)
    {
        // Fallback: Play sound at location without component
        UGameplayStatics::PlaySoundAtLocation(
            GetWorld(),
            SoundToPlay,
            Location,
            AudioVolumeMultiplier
        );
    }
}