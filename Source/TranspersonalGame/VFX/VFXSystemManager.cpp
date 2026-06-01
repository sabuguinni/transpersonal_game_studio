#include "VFXSystemManager.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Particles/ParticleSystemComponent.h"
#include "Particles/ParticleSystem.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Player.h"
#include "Engine/LocalPlayer.h"

UVFX_SystemManager::UVFX_SystemManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Update VFX every 100ms for performance
    
    MaxActiveVFX = 50;
    bEnableVFXLOD = true;
    VFXCullingDistance = 10000.0f;
}

void UVFX_SystemManager::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Warning, TEXT("VFX System Manager initialized - Max VFX: %d"), MaxActiveVFX);
}

void UVFX_SystemManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Clean up expired VFX components
    CleanupExpiredVFX();
    
    // Apply LOD to active VFX based on distance to player
    if (bEnableVFXLOD)
    {
        APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
        if (PlayerPawn)
        {
            for (UParticleSystemComponent* VFXComp : ActiveVFXComponents)
            {
                if (IsValid(VFXComp))
                {
                    float Distance = FVector::Dist(PlayerPawn->GetActorLocation(), VFXComp->GetComponentLocation());
                    ApplyVFXLOD(VFXComp, Distance);
                }
            }
        }
    }
}

void UVFX_SystemManager::SpawnVFXEffect(EVFX_EffectType EffectType, FVector Location, FRotator Rotation, FVector Scale)
{
    if (ActiveVFXComponents.Num() >= MaxActiveVFX)
    {
        UE_LOG(LogTemp, Warning, TEXT("VFX limit reached (%d), skipping new effect"), MaxActiveVFX);
        return;
    }

    FVFX_EffectData EffectData;
    EffectData.EffectType = EffectType;
    EffectData.Location = Location;
    EffectData.Rotation = Rotation;
    EffectData.Scale = Scale;
    
    // Set duration based on effect type
    switch (EffectType)
    {
        case EVFX_EffectType::FootstepDust:
            EffectData.Duration = 2.0f;
            break;
        case EVFX_EffectType::CampfireFire:
            EffectData.Duration = 0.0f; // Persistent
            EffectData.bAutoDestroy = false;
            break;
        case EVFX_EffectType::WeatherRain:
        case EVFX_EffectType::WeatherDustStorm:
        case EVFX_EffectType::WeatherMist:
            EffectData.Duration = 0.0f; // Persistent weather
            EffectData.bAutoDestroy = false;
            break;
        case EVFX_EffectType::BloodSplatter:
            EffectData.Duration = 3.0f;
            break;
        case EVFX_EffectType::ImpactSpark:
            EffectData.Duration = 1.5f;
            break;
        default:
            EffectData.Duration = 5.0f;
            break;
    }

    UParticleSystemComponent* NewVFX = CreateVFXComponent(EffectType, EffectData);
    if (NewVFX)
    {
        ActiveVFXComponents.Add(NewVFX);
        UE_LOG(LogTemp, Log, TEXT("Spawned VFX effect type %d at location %s"), (int32)EffectType, *Location.ToString());
    }
}

void UVFX_SystemManager::SpawnFootstepDust(FVector Location, float Intensity)
{
    FVector Scale = FVector(Intensity, Intensity, Intensity);
    SpawnVFXEffect(EVFX_EffectType::FootstepDust, Location, FRotator::ZeroRotator, Scale);
}

void UVFX_SystemManager::SpawnCampfire(FVector Location)
{
    SpawnVFXEffect(EVFX_EffectType::CampfireFire, Location, FRotator::ZeroRotator, FVector::OneVector);
}

void UVFX_SystemManager::SpawnWeatherEffect(EVFX_EffectType WeatherType, FVector Location, FVector Scale)
{
    if (WeatherType == EVFX_EffectType::WeatherRain || 
        WeatherType == EVFX_EffectType::WeatherDustStorm || 
        WeatherType == EVFX_EffectType::WeatherMist)
    {
        SpawnVFXEffect(WeatherType, Location, FRotator::ZeroRotator, Scale);
    }
}

void UVFX_SystemManager::SpawnImpactEffect(FVector Location, EVFX_EffectType ImpactType)
{
    SpawnVFXEffect(ImpactType, Location, FRotator::ZeroRotator, FVector::OneVector);
}

void UVFX_SystemManager::ClearAllVFX()
{
    for (UParticleSystemComponent* VFXComp : ActiveVFXComponents)
    {
        if (IsValid(VFXComp))
        {
            VFXComp->DestroyComponent();
        }
    }
    ActiveVFXComponents.Empty();
    UE_LOG(LogTemp, Warning, TEXT("Cleared all VFX effects"));
}

int32 UVFX_SystemManager::GetActiveVFXCount() const
{
    return ActiveVFXComponents.Num();
}

void UVFX_SystemManager::SetupBiomeVFX(EBiomeType BiomeType, FVector BiomeCenter)
{
    switch (BiomeType)
    {
        case EBiomeType::Savanna:
            // Dust and heat shimmer effects
            SpawnWeatherEffect(EVFX_EffectType::WeatherDustStorm, BiomeCenter + FVector(0, 0, 500), FVector(3.0f, 3.0f, 2.0f));
            break;
        case EBiomeType::Swamp:
            // Mist and fog effects
            SpawnWeatherEffect(EVFX_EffectType::WeatherMist, BiomeCenter + FVector(0, 0, 200), FVector(4.0f, 4.0f, 1.5f));
            break;
        case EBiomeType::Forest:
            // Light rain and particle effects
            SpawnWeatherEffect(EVFX_EffectType::WeatherRain, BiomeCenter + FVector(0, 0, 800), FVector(5.0f, 5.0f, 3.0f));
            break;
        case EBiomeType::Desert:
            // Heavy dust storms
            SpawnWeatherEffect(EVFX_EffectType::WeatherDustStorm, BiomeCenter + FVector(0, 0, 600), FVector(6.0f, 6.0f, 4.0f));
            break;
        case EBiomeType::Mountain:
            // Mountain mist and wind effects
            SpawnWeatherEffect(EVFX_EffectType::WeatherMist, BiomeCenter + FVector(0, 0, 1000), FVector(4.0f, 4.0f, 2.0f));
            break;
        default:
            break;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Setup VFX for biome %d at center %s"), (int32)BiomeType, *BiomeCenter.ToString());
}

void UVFX_SystemManager::CleanupExpiredVFX()
{
    for (int32 i = ActiveVFXComponents.Num() - 1; i >= 0; i--)
    {
        UParticleSystemComponent* VFXComp = ActiveVFXComponents[i];
        if (!IsValid(VFXComp) || !VFXComp->IsActive())
        {
            if (IsValid(VFXComp))
            {
                VFXComp->DestroyComponent();
            }
            ActiveVFXComponents.RemoveAt(i);
        }
    }
}

UParticleSystemComponent* UVFX_SystemManager::CreateVFXComponent(EVFX_EffectType EffectType, const FVFX_EffectData& EffectData)
{
    if (!GetWorld())
    {
        return nullptr;
    }

    // Create a new actor to hold the particle system
    AActor* VFXActor = GetWorld()->SpawnActor<AActor>();
    if (!VFXActor)
    {
        return nullptr;
    }

    // Set actor transform
    VFXActor->SetActorLocation(EffectData.Location);
    VFXActor->SetActorRotation(EffectData.Rotation);
    VFXActor->SetActorScale3D(EffectData.Scale);

    // Create particle system component
    UParticleSystemComponent* ParticleComp = NewObject<UParticleSystemComponent>(VFXActor);
    if (!ParticleComp)
    {
        VFXActor->Destroy();
        return nullptr;
    }

    VFXActor->SetRootComponent(ParticleComp);
    ParticleComp->RegisterComponent();

    // Auto-destroy if specified
    if (EffectData.bAutoDestroy && EffectData.Duration > 0.0f)
    {
        FTimerHandle TimerHandle;
        GetWorld()->GetTimerManager().SetTimer(TimerHandle, [VFXActor]()
        {
            if (IsValid(VFXActor))
            {
                VFXActor->Destroy();
            }
        }, EffectData.Duration, false);
    }

    return ParticleComp;
}

void UVFX_SystemManager::ApplyVFXLOD(UParticleSystemComponent* VFXComponent, float DistanceToPlayer)
{
    if (!IsValid(VFXComponent))
    {
        return;
    }

    // Cull VFX beyond maximum distance
    if (DistanceToPlayer > VFXCullingDistance)
    {
        VFXComponent->SetVisibility(false);
        return;
    }

    VFXComponent->SetVisibility(true);

    // Apply LOD based on distance
    if (DistanceToPlayer > VFXCullingDistance * 0.7f)
    {
        // Far LOD - reduce particle count
        VFXComponent->SetFloatParameter(FName("ParticleMultiplier"), 0.3f);
    }
    else if (DistanceToPlayer > VFXCullingDistance * 0.4f)
    {
        // Medium LOD
        VFXComponent->SetFloatParameter(FName("ParticleMultiplier"), 0.6f);
    }
    else
    {
        // Near LOD - full quality
        VFXComponent->SetFloatParameter(FName("ParticleMultiplier"), 1.0f);
    }
}