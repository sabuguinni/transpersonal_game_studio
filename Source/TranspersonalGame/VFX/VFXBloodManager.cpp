#include "VFXBloodManager.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Particles/ParticleSystemComponent.h"
#include "Components/DecalComponent.h"
#include "Materials/MaterialInterface.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/DecalActor.h"

UVFX_BloodManager::UVFX_BloodManager()
{
    PrimaryComponentTick.bCanEverTick = false;
    
    // Initialize default values
    DecalLifetime = 30.0f;
    DecalSize = FVector(50.0f, 50.0f, 50.0f);
    
    // Try to load default blood particle systems
    static ConstructorHelpers::FObjectFinder<UParticleSystem> LightBloodFinder(TEXT("/Engine/VFX/P_Blood_Splat_01"));
    if (LightBloodFinder.Succeeded())
    {
        LightBloodParticles = LightBloodFinder.Object;
    }
    
    static ConstructorHelpers::FObjectFinder<UParticleSystem> MediumBloodFinder(TEXT("/Engine/VFX/P_Blood_Splat_02"));
    if (MediumBloodFinder.Succeeded())
    {
        MediumBloodParticles = MediumBloodFinder.Object;
    }
    
    // Try to load blood decal material
    static ConstructorHelpers::FObjectFinder<UMaterialInterface> BloodDecalFinder(TEXT("/Engine/EngineMaterials/M_Decal_Blood"));
    if (BloodDecalFinder.Succeeded())
    {
        BloodDecalMaterial = BloodDecalFinder.Object;
    }
}

void UVFX_BloodManager::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize blood effect arrays
    ActiveBloodParticles.Empty();
    ActiveBloodDecals.Empty();
}

void UVFX_BloodManager::SpawnBloodEffect(const FVFX_BloodEffectData& BloodData)
{
    if (!GetWorld())
    {
        return;
    }
    
    // Spawn blood particles
    SpawnBloodSplatter(BloodData.ImpactLocation, BloodData.ImpactNormal, BloodData.Intensity);
    
    // Create blood decal if requested
    if (BloodData.bCreateDecal)
    {
        CreateBloodDecal(BloodData.ImpactLocation, BloodData.ImpactNormal, DecalSize.X * BloodData.BloodAmount);
    }
}

void UVFX_BloodManager::SpawnBloodSplatter(FVector Location, FVector Normal, EVFX_BloodIntensity Intensity)
{
    UParticleSystem* ParticleSystem = GetParticleSystemForIntensity(Intensity);
    if (ParticleSystem)
    {
        SpawnBloodParticles(Location, Normal, ParticleSystem);
    }
}

void UVFX_BloodManager::CreateBloodDecal(FVector Location, FVector Normal, float Size)
{
    if (!GetWorld() || !BloodDecalMaterial)
    {
        return;
    }
    
    // Calculate rotation from normal
    FRotator DecalRotation = FRotationMatrix::MakeFromZ(Normal).Rotator();
    
    // Spawn decal actor
    ADecalActor* DecalActor = GetWorld()->SpawnActor<ADecalActor>(ADecalActor::StaticClass(), Location, DecalRotation);
    if (DecalActor && DecalActor->GetDecal())
    {
        DecalActor->GetDecal()->SetDecalMaterial(BloodDecalMaterial);
        DecalActor->GetDecal()->DecalSize = FVector(Size, Size, Size);
        DecalActor->SetLifeSpan(DecalLifetime);
        
        ActiveBloodDecals.Add(DecalActor->GetDecal());
    }
}

void UVFX_BloodManager::CleanupBloodEffects()
{
    // Clean up active particle components
    for (UParticleSystemComponent* ParticleComp : ActiveBloodParticles)
    {
        if (IsValid(ParticleComp))
        {
            ParticleComp->DestroyComponent();
        }
    }
    ActiveBloodParticles.Empty();
    
    // Clean up active decals
    for (UDecalComponent* DecalComp : ActiveBloodDecals)
    {
        if (IsValid(DecalComp))
        {
            DecalComp->DestroyComponent();
        }
    }
    ActiveBloodDecals.Empty();
}

UParticleSystem* UVFX_BloodManager::GetParticleSystemForIntensity(EVFX_BloodIntensity Intensity)
{
    switch (Intensity)
    {
        case EVFX_BloodIntensity::Light:
            return LightBloodParticles;
        case EVFX_BloodIntensity::Medium:
            return MediumBloodParticles;
        case EVFX_BloodIntensity::Heavy:
            return HeavyBloodParticles;
        case EVFX_BloodIntensity::Arterial:
            return ArterialBloodParticles;
        default:
            return LightBloodParticles;
    }
}

void UVFX_BloodManager::SpawnBloodParticles(FVector Location, FVector Normal, UParticleSystem* ParticleSystem)
{
    if (!GetWorld() || !ParticleSystem)
    {
        return;
    }
    
    // Calculate rotation from normal
    FRotator ParticleRotation = FRotationMatrix::MakeFromZ(Normal).Rotator();
    
    // Spawn particle system component
    UParticleSystemComponent* ParticleComp = UGameplayStatics::SpawnEmitterAtLocation(
        GetWorld(),
        ParticleSystem,
        Location,
        ParticleRotation,
        FVector(1.0f),
        true,
        EPSCPoolMethod::None,
        true
    );
    
    if (ParticleComp)
    {
        ActiveBloodParticles.Add(ParticleComp);
        
        // Set particle parameters based on impact
        ParticleComp->SetFloatParameter(TEXT("BloodIntensity"), 1.0f);
        ParticleComp->SetVectorParameter(TEXT("ImpactNormal"), Normal);
    }
}