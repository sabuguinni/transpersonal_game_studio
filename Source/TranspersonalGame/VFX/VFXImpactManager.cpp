#include "VFXImpactManager.h"
#include "NiagaraFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"
#include "Engine/World.h"

AVFX_ImpactManager::AVFX_ImpactManager()
{
    PrimaryActorTick.bCanEverTick = false;

    // Create Niagara component
    ActiveVFXComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("ActiveVFXComponent"));
    RootComponent = ActiveVFXComponent;

    // Create audio component
    ImpactAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("ImpactAudioComponent"));
    ImpactAudioComponent->SetupAttachment(RootComponent);
    ImpactAudioComponent->bAutoActivate = false;
}

void AVFX_ImpactManager::BeginPlay()
{
    Super::BeginPlay();
    LoadDefaultVFXSystems();
}

void AVFX_ImpactManager::LoadDefaultVFXSystems()
{
    // Load T-Rex footstep VFX
    FVFX_ImpactData FootstepData;
    FootstepData.VFXSystem = TSoftObjectPtr<UNiagaraSystem>(FSoftObjectPath(TEXT("/Game/VFX/NS_Dino_TRexFootstep")));
    FootstepData.VFXScale = 2.0f;
    FootstepData.SoundVolume = 0.8f;
    ImpactVFXMap.Add(EVFX_ImpactType::FootstepDust, FootstepData);

    // Load blood splatter VFX
    FVFX_ImpactData BloodData;
    BloodData.VFXSystem = TSoftObjectPtr<UNiagaraSystem>(FSoftObjectPath(TEXT("/Game/VFX/NS_Combat_BloodSplatter")));
    BloodData.VFXScale = 1.5f;
    BloodData.SoundVolume = 0.6f;
    ImpactVFXMap.Add(EVFX_ImpactType::BloodSplatter, BloodData);

    // Load campfire smoke VFX for environmental effects
    FVFX_ImpactData EnvironmentData;
    EnvironmentData.VFXSystem = TSoftObjectPtr<UNiagaraSystem>(FSoftObjectPath(TEXT("/Game/VFX/NS_Environment_CampfireSmoke")));
    EnvironmentData.VFXScale = 1.0f;
    EnvironmentData.SoundVolume = 0.4f;
    ImpactVFXMap.Add(EVFX_ImpactType::RockImpact, EnvironmentData);
}

void AVFX_ImpactManager::PlayImpactVFX(EVFX_ImpactType ImpactType, const FVector& Location, const FRotator& Rotation, float ScaleMultiplier)
{
    if (!ImpactVFXMap.Contains(ImpactType))
    {
        UE_LOG(LogTemp, Warning, TEXT("VFX Impact Manager: No VFX data for impact type %d"), (int32)ImpactType);
        return;
    }

    const FVFX_ImpactData& ImpactData = ImpactVFXMap[ImpactType];

    // Load and spawn VFX system
    if (!ImpactData.VFXSystem.IsNull())
    {
        UNiagaraSystem* VFXSystem = ImpactData.VFXSystem.LoadSynchronous();
        if (VFXSystem)
        {
            float FinalScale = ImpactData.VFXScale * ScaleMultiplier;
            SpawnVFXAtLocation(VFXSystem, Location, Rotation, FinalScale);
        }
    }

    // Play impact sound
    if (!ImpactData.ImpactSound.IsNull())
    {
        USoundBase* Sound = ImpactData.ImpactSound.LoadSynchronous();
        if (Sound)
        {
            PlaySoundAtLocation(Sound, Location, ImpactData.SoundVolume);
        }
    }
}

void AVFX_ImpactManager::StopAllVFX()
{
    if (ActiveVFXComponent && ActiveVFXComponent->IsActive())
    {
        ActiveVFXComponent->Deactivate();
    }

    if (ImpactAudioComponent && ImpactAudioComponent->IsPlaying())
    {
        ImpactAudioComponent->Stop();
    }
}

void AVFX_ImpactManager::SetupImpactData(EVFX_ImpactType ImpactType, UNiagaraSystem* VFXSystem, USoundBase* Sound, float Scale, float Volume)
{
    FVFX_ImpactData NewData;
    NewData.VFXSystem = VFXSystem;
    NewData.ImpactSound = Sound;
    NewData.VFXScale = Scale;
    NewData.SoundVolume = Volume;

    ImpactVFXMap.Add(ImpactType, NewData);
}

bool AVFX_ImpactManager::HasImpactData(EVFX_ImpactType ImpactType) const
{
    return ImpactVFXMap.Contains(ImpactType);
}

void AVFX_ImpactManager::InitializeDefaultVFX()
{
    LoadDefaultVFXSystems();
    UE_LOG(LogTemp, Log, TEXT("VFX Impact Manager: Default VFX systems initialized"));
}

void AVFX_ImpactManager::SpawnVFXAtLocation(UNiagaraSystem* VFXSystem, const FVector& Location, const FRotator& Rotation, float Scale)
{
    if (!VFXSystem || !GetWorld())
    {
        return;
    }

    UNiagaraComponent* SpawnedVFX = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
        GetWorld(),
        VFXSystem,
        Location,
        Rotation,
        FVector(Scale),
        true,
        true,
        ENCPoolMethod::None,
        true
    );

    if (SpawnedVFX)
    {
        UE_LOG(LogTemp, Log, TEXT("VFX spawned at location: %s"), *Location.ToString());
    }
}

void AVFX_ImpactManager::PlaySoundAtLocation(USoundBase* Sound, const FVector& Location, float Volume)
{
    if (!Sound || !GetWorld())
    {
        return;
    }

    UGameplayStatics::PlaySoundAtLocation(
        GetWorld(),
        Sound,
        Location,
        Volume,
        1.0f,
        0.0f,
        nullptr,
        nullptr,
        true
    );
}