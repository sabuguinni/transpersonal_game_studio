#include "VFXImpactManager.h"
#include "NiagaraFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"
#include "Components/AudioComponent.h"

AVFX_ImpactManager::AVFX_ImpactManager()
{
    PrimaryActorTick.bCanEverTick = false;

    // Create root component
    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;

    // Initialize performance settings
    MaxActiveEffects = 50;
    PoolSize = 20;
    EffectLifetime = 5.0f;

    // Initialize impact systems map (will be populated in Blueprint or BeginPlay)
    ImpactSystems.Empty();
    SurfaceEffects.Empty();
    ImpactSounds.Empty();
}

void AVFX_ImpactManager::BeginPlay()
{
    Super::BeginPlay();

    // Initialize the effect pool
    InitializeEffectPool();

    // Start cleanup timer
    GetWorldTimerManager().SetTimer(
        CleanupTimer,
        this,
        &AVFX_ImpactManager::CleanupExpiredEffects,
        1.0f,
        true
    );

    UE_LOG(LogTemp, Log, TEXT("VFX Impact Manager initialized with %d pooled effects"), PoolSize);
}

void AVFX_ImpactManager::CreateImpactEffect(const FVFX_ImpactData& ImpactData)
{
    // Check if we've reached the maximum active effects
    if (ActiveEffects.Num() >= MaxActiveEffects)
    {
        UE_LOG(LogTemp, Warning, TEXT("Maximum active effects reached (%d), skipping impact"), MaxActiveEffects);
        return;
    }

    // Get the appropriate Niagara system
    UNiagaraSystem* SystemToUse = GetSystemForImpact(ImpactData);
    if (!SystemToUse)
    {
        UE_LOG(LogTemp, Warning, TEXT("No Niagara system found for impact type"));
        return;
    }

    // Get a pooled effect component
    UNiagaraComponent* EffectComponent = GetPooledEffect();
    if (!EffectComponent)
    {
        UE_LOG(LogTemp, Warning, TEXT("No available effect components in pool"));
        return;
    }

    // Configure the effect
    EffectComponent->SetAsset(SystemToUse);
    EffectComponent->SetWorldLocation(ImpactData.ImpactLocation);
    EffectComponent->SetWorldRotation(FRotationMatrix::MakeFromZ(ImpactData.ImpactNormal).Rotator());

    // Configure effect parameters
    ConfigureEffectParameters(EffectComponent, ImpactData);

    // Activate the effect
    EffectComponent->Activate();
    ActiveEffects.Add(EffectComponent);

    // Play sound effect
    PlayImpactSound(ImpactData);

    UE_LOG(LogTemp, Log, TEXT("Created impact effect at location: %s"), *ImpactData.ImpactLocation.ToString());
}

void AVFX_ImpactManager::CreateFootstepEffect(FVector Location, EVFX_SurfaceType Surface, float CreatureSize)
{
    FVFX_ImpactData ImpactData;
    ImpactData.ImpactType = (CreatureSize > 5.0f) ? EVFX_ImpactType::DinosaurFootstep : EVFX_ImpactType::PlayerFootstep;
    ImpactData.SurfaceType = Surface;
    ImpactData.ImpactLocation = Location;
    ImpactData.ImpactNormal = FVector::UpVector;
    ImpactData.ImpactForce = CreatureSize;
    ImpactData.ParticleScale = FMath::Clamp(CreatureSize, 0.5f, 10.0f);

    CreateImpactEffect(ImpactData);
}

void AVFX_ImpactManager::CreateWeaponImpact(FVector Location, FVector Normal, EVFX_SurfaceType Surface, float Force)
{
    FVFX_ImpactData ImpactData;
    ImpactData.ImpactType = EVFX_ImpactType::WeaponImpact;
    ImpactData.SurfaceType = Surface;
    ImpactData.ImpactLocation = Location;
    ImpactData.ImpactNormal = Normal;
    ImpactData.ImpactForce = Force;
    ImpactData.ParticleScale = FMath::Clamp(Force, 0.5f, 3.0f);

    CreateImpactEffect(ImpactData);
}

void AVFX_ImpactManager::CreateBloodEffect(FVector Location, FVector Direction, float Intensity)
{
    FVFX_ImpactData ImpactData;
    ImpactData.ImpactType = EVFX_ImpactType::BloodSpatter;
    ImpactData.SurfaceType = EVFX_SurfaceType::Bone; // Default surface for blood
    ImpactData.ImpactLocation = Location;
    ImpactData.ImpactNormal = Direction;
    ImpactData.ImpactForce = Intensity;
    ImpactData.ParticleScale = FMath::Clamp(Intensity, 0.3f, 2.0f);

    CreateImpactEffect(ImpactData);
}

void AVFX_ImpactManager::CreateDustCloud(FVector Location, float Radius, float Duration)
{
    FVFX_ImpactData ImpactData;
    ImpactData.ImpactType = EVFX_ImpactType::DustCloud;
    ImpactData.SurfaceType = EVFX_SurfaceType::Dirt;
    ImpactData.ImpactLocation = Location;
    ImpactData.ImpactNormal = FVector::UpVector;
    ImpactData.ImpactForce = Radius / 100.0f;
    ImpactData.ParticleScale = FMath::Clamp(Radius / 100.0f, 0.5f, 5.0f);

    CreateImpactEffect(ImpactData);
}

void AVFX_ImpactManager::InitializeEffectPool()
{
    EffectPool.Empty();
    
    for (int32 i = 0; i < PoolSize; i++)
    {
        UNiagaraComponent* PooledEffect = CreateDefaultSubobject<UNiagaraComponent>(*FString::Printf(TEXT("PooledEffect_%d"), i));
        PooledEffect->SetupAttachment(RootComponent);
        PooledEffect->SetAutoDestroy(false);
        PooledEffect->SetVisibility(false);
        PooledEffect->Deactivate();
        
        EffectPool.Add(PooledEffect);
    }

    UE_LOG(LogTemp, Log, TEXT("Initialized VFX pool with %d components"), PoolSize);
}

UNiagaraComponent* AVFX_ImpactManager::GetPooledEffect()
{
    for (UNiagaraComponent* Effect : EffectPool)
    {
        if (Effect && !Effect->IsActive())
        {
            Effect->SetVisibility(true);
            return Effect;
        }
    }

    // If no pooled effects available, create a new one temporarily
    UNiagaraComponent* NewEffect = NewObject<UNiagaraComponent>(this);
    NewEffect->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepWorldTransform);
    NewEffect->SetAutoDestroy(false);
    
    UE_LOG(LogTemp, Warning, TEXT("Effect pool exhausted, created temporary effect"));
    return NewEffect;
}

void AVFX_ImpactManager::ReturnEffectToPool(UNiagaraComponent* Effect)
{
    if (!Effect)
    {
        return;
    }

    Effect->Deactivate();
    Effect->SetVisibility(false);
    Effect->SetAsset(nullptr);

    // Remove from active effects
    ActiveEffects.Remove(Effect);

    UE_LOG(LogTemp, VeryVerbose, TEXT("Returned effect to pool"));
}

void AVFX_ImpactManager::CleanupExpiredEffects()
{
    TArray<UNiagaraComponent*> ExpiredEffects;

    for (UNiagaraComponent* Effect : ActiveEffects)
    {
        if (Effect && !Effect->IsActive())
        {
            ExpiredEffects.Add(Effect);
        }
    }

    for (UNiagaraComponent* ExpiredEffect : ExpiredEffects)
    {
        ReturnEffectToPool(ExpiredEffect);
    }

    if (ExpiredEffects.Num() > 0)
    {
        UE_LOG(LogTemp, VeryVerbose, TEXT("Cleaned up %d expired effects"), ExpiredEffects.Num());
    }
}

void AVFX_ImpactManager::SetMaxActiveEffects(int32 NewMax)
{
    MaxActiveEffects = FMath::Max(1, NewMax);
    UE_LOG(LogTemp, Log, TEXT("Max active effects set to %d"), MaxActiveEffects);
}

void AVFX_ImpactManager::PlayImpactSound(const FVFX_ImpactData& ImpactData)
{
    USoundCue* SoundToPlay = ImpactSounds.FindRef(ImpactData.ImpactType);
    if (SoundToPlay)
    {
        UGameplayStatics::PlaySoundAtLocation(
            GetWorld(),
            SoundToPlay,
            ImpactData.ImpactLocation,
            FMath::Clamp(ImpactData.ImpactForce, 0.3f, 2.0f)
        );
    }
}

UNiagaraSystem* AVFX_ImpactManager::GetSystemForImpact(const FVFX_ImpactData& ImpactData)
{
    // First try to get impact-specific system
    UNiagaraSystem* ImpactSystem = ImpactSystems.FindRef(ImpactData.ImpactType);
    if (ImpactSystem)
    {
        return ImpactSystem;
    }

    // Fallback to surface-specific system
    UNiagaraSystem* SurfaceSystem = SurfaceEffects.FindRef(ImpactData.SurfaceType);
    if (SurfaceSystem)
    {
        return SurfaceSystem;
    }

    UE_LOG(LogTemp, Warning, TEXT("No Niagara system found for impact type %d and surface type %d"), 
           (int32)ImpactData.ImpactType, (int32)ImpactData.SurfaceType);
    
    return nullptr;
}

void AVFX_ImpactManager::ConfigureEffectParameters(UNiagaraComponent* Effect, const FVFX_ImpactData& ImpactData)
{
    if (!Effect)
    {
        return;
    }

    // Set common parameters that most Niagara systems should support
    Effect->SetFloatParameter(TEXT("ParticleScale"), ImpactData.ParticleScale);
    Effect->SetFloatParameter(TEXT("ImpactForce"), ImpactData.ImpactForce);
    Effect->SetVectorParameter(TEXT("ImpactNormal"), ImpactData.ImpactNormal);
    Effect->SetFloatParameter(TEXT("Lifetime"), EffectLifetime);

    // Set surface-specific parameters
    switch (ImpactData.SurfaceType)
    {
        case EVFX_SurfaceType::Dirt:
            Effect->SetColorParameter(TEXT("ParticleColor"), FLinearColor(0.4f, 0.3f, 0.2f, 1.0f));
            break;
        case EVFX_SurfaceType::Rock:
            Effect->SetColorParameter(TEXT("ParticleColor"), FLinearColor(0.6f, 0.6f, 0.6f, 1.0f));
            break;
        case EVFX_SurfaceType::Mud:
            Effect->SetColorParameter(TEXT("ParticleColor"), FLinearColor(0.3f, 0.2f, 0.1f, 1.0f));
            break;
        case EVFX_SurfaceType::Sand:
            Effect->SetColorParameter(TEXT("ParticleColor"), FLinearColor(0.8f, 0.7f, 0.5f, 1.0f));
            break;
        case EVFX_SurfaceType::Grass:
            Effect->SetColorParameter(TEXT("ParticleColor"), FLinearColor(0.2f, 0.4f, 0.1f, 1.0f));
            break;
        case EVFX_SurfaceType::Water:
            Effect->SetColorParameter(TEXT("ParticleColor"), FLinearColor(0.3f, 0.5f, 0.8f, 0.7f));
            break;
        default:
            Effect->SetColorParameter(TEXT("ParticleColor"), FLinearColor::White);
            break;
    }
}

void AVFX_ImpactManager::OnEffectFinished(UNiagaraComponent* FinishedEffect)
{
    ReturnEffectToPool(FinishedEffect);
}