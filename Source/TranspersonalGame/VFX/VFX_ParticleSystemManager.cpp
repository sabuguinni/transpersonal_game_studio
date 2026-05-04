#include "VFX_ParticleSystemManager.h"
#include "NiagaraFunctionLibrary.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"

AVFX_ParticleSystemManager::AVFX_ParticleSystemManager()
{
    PrimaryActorTick.bCanEverTick = false;

    // Criar componente raiz
    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;

    // Configurações padrão
    bVFXEnabled = true;
    GlobalVFXScale = 1.0f;
    GlobalVFXIntensity = 1.0f;

    // Inicializar array de sistemas activos
    ActiveParticleSystems.Empty();
}

void AVFX_ParticleSystemManager::BeginPlay()
{
    Super::BeginPlay();

    // Configurar timer de limpeza automática (a cada 5 segundos)
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().SetTimer(
            CleanupTimerHandle,
            this,
            &AVFX_ParticleSystemManager::AutoCleanupEffects,
            5.0f,
            true
        );
    }

    UE_LOG(LogTemp, Log, TEXT("VFX_ParticleSystemManager initialized"));
}

UNiagaraComponent* AVFX_ParticleSystemManager::SpawnParticleEffect(
    EVFX_ParticleType ParticleType,
    const FVector& Location,
    const FRotator& Rotation,
    const FVFX_ParticleConfig& Config)
{
    if (!bVFXEnabled)
    {
        return nullptr;
    }

    UNiagaraSystem* ParticleSystem = LoadParticleSystem(ParticleType);
    if (!ParticleSystem)
    {
        UE_LOG(LogTemp, Warning, TEXT("Failed to load particle system for type: %d"), (int32)ParticleType);
        return nullptr;
    }

    // Spawnar componente Niagara no mundo
    UNiagaraComponent* NiagaraComponent = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
        GetWorld(),
        ParticleSystem,
        Location,
        Rotation,
        Config.Scale * GlobalVFXScale,
        Config.bAutoDestroy
    );

    if (NiagaraComponent)
    {
        // Aplicar configurações
        if (Config.Intensity != 1.0f || GlobalVFXIntensity != 1.0f)
        {
            float FinalIntensity = Config.Intensity * GlobalVFXIntensity;
            NiagaraComponent->SetFloatParameter(TEXT("Intensity"), FinalIntensity);
        }

        // Adicionar à lista de sistemas activos
        ActiveParticleSystems.Add(NiagaraComponent);

        UE_LOG(LogTemp, Log, TEXT("Spawned VFX effect type %d at location %s"), 
               (int32)ParticleType, *Location.ToString());
    }

    return NiagaraComponent;
}

void AVFX_ParticleSystemManager::CreateDinosaurFootstepEffect(const FVector& Location, float DinosaurSize)
{
    FVFX_ParticleConfig Config;
    Config.ParticleType = EVFX_ParticleType::DinosaurFootstep;
    Config.Duration = 1.5f;
    Config.Intensity = FMath::Clamp(DinosaurSize, 0.5f, 3.0f);
    Config.Scale = FVector(DinosaurSize, DinosaurSize, 1.0f);
    Config.bAutoDestroy = true;

    SpawnParticleEffect(EVFX_ParticleType::DinosaurFootstep, Location, FRotator::ZeroRotator, Config);
}

void AVFX_ParticleSystemManager::CreateCampfireEffect(const FVector& Location)
{
    FVFX_ParticleConfig Config;
    Config.ParticleType = EVFX_ParticleType::CampfireFlames;
    Config.Duration = 0.0f; // Contínuo
    Config.Intensity = 1.0f;
    Config.Scale = FVector(1.0f, 1.0f, 1.0f);
    Config.bAutoDestroy = false; // Fogo contínuo

    SpawnParticleEffect(EVFX_ParticleType::CampfireFlames, Location, FRotator::ZeroRotator, Config);
}

void AVFX_ParticleSystemManager::CreateBloodSplatterEffect(const FVector& Location, const FVector& Direction)
{
    FVFX_ParticleConfig Config;
    Config.ParticleType = EVFX_ParticleType::BloodSplatter;
    Config.Duration = 2.0f;
    Config.Intensity = 1.2f;
    Config.Scale = FVector(1.0f, 1.0f, 1.0f);
    Config.bAutoDestroy = true;

    // Calcular rotação baseada na direcção
    FRotator EffectRotation = Direction.Rotation();

    SpawnParticleEffect(EVFX_ParticleType::BloodSplatter, Location, EffectRotation, Config);
}

void AVFX_ParticleSystemManager::CreateDustCloudEffect(const FVector& Location, float Intensity)
{
    FVFX_ParticleConfig Config;
    Config.ParticleType = EVFX_ParticleType::DustCloud;
    Config.Duration = 3.0f;
    Config.Intensity = Intensity;
    Config.Scale = FVector(1.0f, 1.0f, 1.0f);
    Config.bAutoDestroy = true;

    SpawnParticleEffect(EVFX_ParticleType::DustCloud, Location, FRotator::ZeroRotator, Config);
}

void AVFX_ParticleSystemManager::CreateRainEffect(const FVector& Location, float Radius)
{
    FVFX_ParticleConfig Config;
    Config.ParticleType = EVFX_ParticleType::RainDrops;
    Config.Duration = 0.0f; // Contínuo
    Config.Intensity = 1.0f;
    Config.Scale = FVector(Radius / 1000.0f, Radius / 1000.0f, 1.0f);
    Config.bAutoDestroy = false;

    SpawnParticleEffect(EVFX_ParticleType::RainDrops, Location, FRotator::ZeroRotator, Config);
}

void AVFX_ParticleSystemManager::CleanupFinishedEffects()
{
    ActiveParticleSystems.RemoveAll([](UNiagaraComponent* Component) {
        if (!IsValid(Component))
        {
            return true; // Remove invalid components
        }

        // Verificar se o sistema terminou
        if (Component->GetSystemInstanceController().IsValid())
        {
            return !Component->GetSystemInstanceController()->GetSystemInstance_Unsafe()->IsComplete();
        }

        return false;
    });

    UE_LOG(LogTemp, Log, TEXT("Cleanup completed. Active VFX systems: %d"), ActiveParticleSystems.Num());
}

void AVFX_ParticleSystemManager::StopAllEffects()
{
    for (UNiagaraComponent* Component : ActiveParticleSystems)
    {
        if (IsValid(Component))
        {
            Component->DestroyComponent();
        }
    }

    ActiveParticleSystems.Empty();
    UE_LOG(LogTemp, Log, TEXT("All VFX effects stopped"));
}

int32 AVFX_ParticleSystemManager::GetActiveEffectCount() const
{
    return ActiveParticleSystems.Num();
}

void AVFX_ParticleSystemManager::SetVFXEnabled(bool bEnabled)
{
    bVFXEnabled = bEnabled;

    if (!bEnabled)
    {
        StopAllEffects();
    }

    UE_LOG(LogTemp, Log, TEXT("VFX enabled set to: %s"), bEnabled ? TEXT("true") : TEXT("false"));
}

void AVFX_ParticleSystemManager::SetGlobalVFXScale(float Scale)
{
    GlobalVFXScale = FMath::Clamp(Scale, 0.1f, 2.0f);
    UE_LOG(LogTemp, Log, TEXT("Global VFX scale set to: %f"), GlobalVFXScale);
}

void AVFX_ParticleSystemManager::SetGlobalVFXIntensity(float Intensity)
{
    GlobalVFXIntensity = FMath::Clamp(Intensity, 0.1f, 5.0f);
    UE_LOG(LogTemp, Log, TEXT("Global VFX intensity set to: %f"), GlobalVFXIntensity);
}

UNiagaraSystem* AVFX_ParticleSystemManager::LoadParticleSystem(EVFX_ParticleType ParticleType)
{
    // Por agora, retornar nullptr - os assets Niagara serão criados posteriormente
    // Esta função será expandida quando os assets estiverem disponíveis
    
    switch (ParticleType)
    {
        case EVFX_ParticleType::DinosaurFootstep:
            return FootstepParticleSystem.LoadSynchronous();
        case EVFX_ParticleType::CampfireFlames:
            return CampfireParticleSystem.LoadSynchronous();
        case EVFX_ParticleType::BloodSplatter:
            return BloodParticleSystem.LoadSynchronous();
        case EVFX_ParticleType::DustCloud:
            return DustParticleSystem.LoadSynchronous();
        case EVFX_ParticleType::RainDrops:
            return RainParticleSystem.LoadSynchronous();
        default:
            UE_LOG(LogTemp, Warning, TEXT("Unknown particle type: %d"), (int32)ParticleType);
            return nullptr;
    }
}

void AVFX_ParticleSystemManager::AutoCleanupEffects()
{
    CleanupFinishedEffects();
}