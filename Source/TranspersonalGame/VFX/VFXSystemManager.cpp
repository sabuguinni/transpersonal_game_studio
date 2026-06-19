#include "VFXSystemManager.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/PointLightComponent.h"
#include "Kismet/GameplayStatics.h"

UVFX_SystemManager::UVFX_SystemManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    GlobalVFXQuality = 1.0f;
    MaxActiveParticleSystems = 32;
    bEnableLOD = true;
    LOD_HighQualityDistance = 1500.0f;
    LOD_MediumQualityDistance = 4000.0f;
    ActiveParticleCount = 0;
}

void UVFX_SystemManager::BeginPlay()
{
    Super::BeginPlay();
}

void UVFX_SystemManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    // Cull distant particle systems for performance
    if (bEnableLOD && GetOwner())
    {
        FVector PlayerLoc = GetOwner()->GetActorLocation();
        for (AActor* Campfire : ActiveCampfires)
        {
            if (!Campfire) continue;
            float Dist = FVector::Dist(PlayerLoc, Campfire->GetActorLocation());
            // LOD: disable light beyond medium distance
            UPointLightComponent* Light = Campfire->FindComponentByClass<UPointLightComponent>();
            if (Light)
            {
                Light->SetVisibility(Dist < LOD_MediumQualityDistance);
            }
        }
    }
}

void UVFX_SystemManager::SpawnCampfireEffect(const FVFX_CampfireData& CampfireData)
{
    if (!GetWorld()) return;
    if (ActiveParticleCount >= MaxActiveParticleSystems) return;

    // Spawn a point light actor to represent campfire glow
    FActorSpawnParameters Params;
    Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    // Use a basic actor as campfire anchor
    AActor* CampfireActor = GetWorld()->SpawnActor<AActor>(AActor::StaticClass(), CampfireData.Location, FRotator::ZeroRotator, Params);
    if (CampfireActor)
    {
        CampfireActor->SetActorLabel(FString::Printf(TEXT("VFX_Campfire_%d"), ActiveCampfires.Num() + 1));

        UPointLightComponent* FireLight = NewObject<UPointLightComponent>(CampfireActor);
        if (FireLight)
        {
            FireLight->RegisterComponent();
            FireLight->AttachToComponent(CampfireActor->GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
            FireLight->SetIntensity(CampfireData.Intensity);
            FireLight->SetLightColor(CampfireData.FireColor);
            FireLight->SetAttenuationRadius(CampfireData.AttenuationRadius);
            FireLight->SetVisibility(CampfireData.bIsActive);
        }

        ActiveCampfires.Add(CampfireActor);
        ActiveParticleCount++;
    }
}

void UVFX_SystemManager::SpawnFootstepDust(const FVFX_FootstepData& FootstepData)
{
    if (!GetWorld()) return;

    // Footstep dust: spawn a brief point light flash at impact location
    // In full implementation, this would trigger a Niagara NS_Dino_Footstep system
    // For now, log the impact for debugging
    UE_LOG(LogTemp, Log, TEXT("VFX_FootstepDust: Species=%s Loc=(%f,%f,%f) Force=%f"),
        *FootstepData.DinosaurSpecies.ToString(),
        FootstepData.ImpactLocation.X,
        FootstepData.ImpactLocation.Y,
        FootstepData.ImpactLocation.Z,
        FootstepData.ImpactForce);
}

void UVFX_SystemManager::SetWeatherIntensity(float RainIntensity, float FogDensity, float WindStrength)
{
    WeatherRainIntensity = FMath::Clamp(RainIntensity, 0.0f, 1.0f);
    WeatherFogDensity = FMath::Clamp(FogDensity, 0.0f, 1.0f);
    WeatherWindStrength = FMath::Clamp(WindStrength, 0.0f, 1.0f);

    // Apply fog density to ExponentialHeightFog if present
    if (GetWorld())
    {
        TArray<AActor*> FogActors;
        UGameplayStatics::GetAllActorsOfClass(GetWorld(), AExponentialHeightFog::StaticClass(), FogActors);
        for (AActor* FogActor : FogActors)
        {
            AExponentialHeightFog* Fog = Cast<AExponentialHeightFog>(FogActor);
            if (Fog)
            {
                UExponentialHeightFogComponent* FogComp = Fog->GetComponent();
                if (FogComp)
                {
                    FogComp->SetFogDensity(WeatherFogDensity * 0.05f);
                    FogComp->SetFogInscatteringColor(FLinearColor(0.6f, 0.7f, 0.8f, 1.0f));
                }
            }
        }
    }
}

void UVFX_SystemManager::SpawnBloodImpact(FVector Location, FVector Direction, float Intensity)
{
    if (!GetWorld()) return;

    // Blood impact: in full implementation triggers NS_Combat_BloodSplatter Niagara system
    // Intensity drives particle count and spread radius
    float ClampedIntensity = FMath::Clamp(Intensity, 0.0f, 1.0f);
    UE_LOG(LogTemp, Log, TEXT("VFX_BloodImpact: Loc=(%f,%f,%f) Dir=(%f,%f,%f) Intensity=%f"),
        Location.X, Location.Y, Location.Z,
        Direction.X, Direction.Y, Direction.Z,
        ClampedIntensity);
}

void UVFX_SystemManager::SpawnVolcanicAsh(FVector SourceLocation, float Radius)
{
    if (!GetWorld()) return;

    // Volcanic ash: ambient particle system covering a radius
    // In full implementation: NS_World_VolcanicAsh Niagara system with GPU particles
    UE_LOG(LogTemp, Log, TEXT("VFX_VolcanicAsh: Source=(%f,%f,%f) Radius=%f"),
        SourceLocation.X, SourceLocation.Y, SourceLocation.Z, Radius);
}
