#include "VFXSystemCore.h"
#include "NiagaraFunctionLibrary.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"

UVFXSystemCore::UVFXSystemCore()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Update every 100ms for performance

    // Initialize LOD Settings
    // High Quality (Close range)
    LODSettings[2].MaxDrawDistance = 2000.0f;
    LODSettings[2].ParticleCountMultiplier = 1.0f;
    LODSettings[2].UpdateFrequency = 60.0f;
    LODSettings[2].bEnableCollision = true;
    LODSettings[2].bCastShadows = true;

    // Medium Quality (Medium range)
    LODSettings[1].MaxDrawDistance = 5000.0f;
    LODSettings[1].ParticleCountMultiplier = 0.6f;
    LODSettings[1].UpdateFrequency = 30.0f;
    LODSettings[1].bEnableCollision = false;
    LODSettings[1].bCastShadows = false;

    // Low Quality (Far range)
    LODSettings[0].MaxDrawDistance = 10000.0f;
    LODSettings[0].ParticleCountMultiplier = 0.3f;
    LODSettings[0].UpdateFrequency = 15.0f;
    LODSettings[0].bEnableCollision = false;
    LODSettings[0].bCastShadows = false;
}

void UVFXSystemCore::BeginPlay()
{
    Super::BeginPlay();
    
    // Set initial quality based on platform
    #if PLATFORM_MOBILE
        SetVFXQuality(EVFXQualityLevel::Low);
    #elif PLATFORM_CONSOLE
        SetVFXQuality(EVFXQualityLevel::Medium);
    #else
        SetVFXQuality(EVFXQualityLevel::High);
    #endif
}

void UVFXSystemCore::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    LastPerformanceCheck += DeltaTime;
    if (LastPerformanceCheck >= PerformanceCheckInterval)
    {
        UpdateVFXPerformance();
        CleanupInactiveVFX();
        LastPerformanceCheck = 0.0f;
    }
}

UNiagaraComponent* UVFXSystemCore::SpawnVFX(UNiagaraSystem* VFXSystem, FVector Location, FRotator Rotation, EVFXCategory Category)
{
    if (!VFXSystem || !GetWorld())
    {
        return nullptr;
    }

    // Check if we're at the limit
    if (ActiveVFXComponents.Num() >= MaxActiveVFXCount)
    {
        // Remove oldest VFX if at limit
        if (ActiveVFXComponents.Num() > 0)
        {
            UNiagaraComponent* OldestVFX = ActiveVFXComponents[0];
            if (OldestVFX && IsValid(OldestVFX))
            {
                OldestVFX->DestroyComponent();
            }
            ActiveVFXComponents.RemoveAt(0);
        }
    }

    // Spawn the VFX
    UNiagaraComponent* VFXComponent = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
        GetWorld(),
        VFXSystem,
        Location,
        Rotation,
        FVector(1.0f),
        true,
        true,
        ENCPoolMethod::AutoRelease
    );

    if (VFXComponent)
    {
        // Apply LOD settings based on distance from player
        APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
        if (PlayerPawn)
        {
            float Distance = FVector::Dist(Location, PlayerPawn->GetActorLocation());
            UpdateLODForDistance(VFXComponent, Distance);
        }

        ActiveVFXComponents.Add(VFXComponent);
    }

    return VFXComponent;
}

void UVFXSystemCore::SetVFXQuality(EVFXQualityLevel QualityLevel)
{
    CurrentQualityLevel = QualityLevel;
    
    // Update all active VFX with new quality settings
    for (UNiagaraComponent* VFXComp : ActiveVFXComponents)
    {
        if (VFXComp && IsValid(VFXComp))
        {
            FVFXLODSettings CurrentLOD = GetCurrentLODSettings();
            
            // Apply quality settings
            VFXComp->SetFloatParameter(TEXT("ParticleCountMultiplier"), CurrentLOD.ParticleCountMultiplier);
            VFXComp->SetCastShadow(CurrentLOD.bCastShadows);
        }
    }
}

void UVFXSystemCore::UpdateLODForDistance(UNiagaraComponent* VFXComponent, float Distance)
{
    if (!VFXComponent || !IsValid(VFXComponent))
    {
        return;
    }

    EVFXQualityLevel LODLevel = EVFXQualityLevel::High;
    
    if (Distance > 5000.0f)
    {
        LODLevel = EVFXQualityLevel::Low;
    }
    else if (Distance > 2000.0f)
    {
        LODLevel = EVFXQualityLevel::Medium;
    }

    FVFXLODSettings LODSettings = this->LODSettings[(int32)LODLevel];
    
    // Apply LOD settings
    VFXComponent->SetFloatParameter(TEXT("ParticleCountMultiplier"), LODSettings.ParticleCountMultiplier);
    VFXComponent->SetFloatParameter(TEXT("UpdateFrequency"), LODSettings.UpdateFrequency);
    VFXComponent->SetCastShadow(LODSettings.bCastShadows);
    
    // Hide VFX if beyond max draw distance
    if (Distance > LODSettings.MaxDrawDistance)
    {
        VFXComponent->SetVisibility(false);
    }
    else
    {
        VFXComponent->SetVisibility(true);
    }
}

// Atmosphere VFX Implementation
void UVFXSystemCore::SpawnAtmosphericFog(FVector Location, float Intensity)
{
    if (AtmosphericFogVFX)
    {
        UNiagaraComponent* VFX = SpawnVFX(AtmosphericFogVFX, Location, FRotator::ZeroRotator, EVFXCategory::Atmosphere);
        if (VFX)
        {
            VFX->SetFloatParameter(TEXT("Intensity"), Intensity);
            VFX->SetFloatParameter(TEXT("FogDensity"), Intensity * 0.5f);
        }
    }
}

void UVFXSystemCore::SpawnDustParticles(FVector Location, FVector WindDirection, float Intensity)
{
    if (DustParticlesVFX)
    {
        FRotator WindRotation = WindDirection.Rotation();
        UNiagaraComponent* VFX = SpawnVFX(DustParticlesVFX, Location, WindRotation, EVFXCategory::Atmosphere);
        if (VFX)
        {
            VFX->SetFloatParameter(TEXT("Intensity"), Intensity);
            VFX->SetVectorParameter(TEXT("WindDirection"), WindDirection);
        }
    }
}

void UVFXSystemCore::SpawnInsectSwarm(FVector Location, float Radius)
{
    if (InsectSwarmVFX)
    {
        UNiagaraComponent* VFX = SpawnVFX(InsectSwarmVFX, Location, FRotator::ZeroRotator, EVFXCategory::Atmosphere);
        if (VFX)
        {
            VFX->SetFloatParameter(TEXT("SwarmRadius"), Radius);
            VFX->SetFloatParameter(TEXT("InsectCount"), FMath::Clamp(Radius / 10.0f, 10.0f, 100.0f));
        }
    }
}

// Dinosaur Presence VFX Implementation
void UVFXSystemCore::SpawnFootstepDust(FVector Location, float DinosaurSize)
{
    if (FootstepDustVFX)
    {
        UNiagaraComponent* VFX = SpawnVFX(FootstepDustVFX, Location, FRotator::ZeroRotator, EVFXCategory::DinosaurPresence);
        if (VFX)
        {
            float DustAmount = FMath::Clamp(DinosaurSize, 0.1f, 5.0f);
            VFX->SetFloatParameter(TEXT("DinosaurSize"), DinosaurSize);
            VFX->SetFloatParameter(TEXT("DustAmount"), DustAmount);
            VFX->SetFloatParameter(TEXT("ImpactForce"), DinosaurSize * 2.0f);
        }
    }
}

void UVFXSystemCore::SpawnBreathVapor(FVector Location, FVector Direction, float Temperature)
{
    if (BreathVaporVFX)
    {
        FRotator BreathRotation = Direction.Rotation();
        UNiagaraComponent* VFX = SpawnVFX(BreathVaporVFX, Location, BreathRotation, EVFXCategory::DinosaurPresence);
        if (VFX)
        {
            VFX->SetFloatParameter(TEXT("Temperature"), Temperature);
            VFX->SetVectorParameter(TEXT("BreathDirection"), Direction);
            VFX->SetFloatParameter(TEXT("VaporDensity"), 1.0f - Temperature); // More vapor in cold
        }
    }
}

void UVFXSystemCore::SpawnVegetationDisturbance(FVector Location, float Intensity)
{
    if (VegetationDisturbanceVFX)
    {
        UNiagaraComponent* VFX = SpawnVFX(VegetationDisturbanceVFX, Location, FRotator::ZeroRotator, EVFXCategory::DinosaurPresence);
        if (VFX)
        {
            VFX->SetFloatParameter(TEXT("DisturbanceIntensity"), Intensity);
            VFX->SetFloatParameter(TEXT("LeafCount"), Intensity * 20.0f);
        }
    }
}

// Survival VFX Implementation
void UVFXSystemCore::SpawnCampfire(FVector Location, float Size)
{
    if (CampfireVFX)
    {
        UNiagaraComponent* VFX = SpawnVFX(CampfireVFX, Location, FRotator::ZeroRotator, EVFXCategory::Survival);
        if (VFX)
        {
            VFX->SetFloatParameter(TEXT("FireSize"), Size);
            VFX->SetFloatParameter(TEXT("FlameHeight"), Size * 100.0f);
            VFX->SetFloatParameter(TEXT("SmokeAmount"), Size * 0.5f);
        }
    }
}

void UVFXSystemCore::SpawnToolSparks(FVector Location, FVector Direction)
{
    if (ToolSparksVFX)
    {
        FRotator SparkRotation = Direction.Rotation();
        UNiagaraComponent* VFX = SpawnVFX(ToolSparksVFX, Location, SparkRotation, EVFXCategory::Survival);
        if (VFX)
        {
            VFX->SetVectorParameter(TEXT("SparkDirection"), Direction);
            VFX->SetFloatParameter(TEXT("SparkCount"), 15.0f);
        }
    }
}

void UVFXSystemCore::SpawnSmoke(FVector Location, FVector Direction, float Intensity)
{
    if (SmokeVFX)
    {
        FRotator SmokeRotation = Direction.Rotation();
        UNiagaraComponent* VFX = SpawnVFX(SmokeVFX, Location, SmokeRotation, EVFXCategory::Survival);
        if (VFX)
        {
            VFX->SetFloatParameter(TEXT("SmokeIntensity"), Intensity);
            VFX->SetVectorParameter(TEXT("WindDirection"), Direction);
        }
    }
}

// Environment VFX Implementation
void UVFXSystemCore::SpawnWaterSplash(FVector Location, float Force)
{
    if (WaterSplashVFX)
    {
        UNiagaraComponent* VFX = SpawnVFX(WaterSplashVFX, Location, FRotator::ZeroRotator, EVFXCategory::Environment);
        if (VFX)
        {
            VFX->SetFloatParameter(TEXT("SplashForce"), Force);
            VFX->SetFloatParameter(TEXT("DropletCount"), Force * 25.0f);
        }
    }
}

void UVFXSystemCore::SpawnLeafFall(FVector Location, float Radius)
{
    if (LeafFallVFX)
    {
        UNiagaraComponent* VFX = SpawnVFX(LeafFallVFX, Location, FRotator::ZeroRotator, EVFXCategory::Environment);
        if (VFX)
        {
            VFX->SetFloatParameter(TEXT("FallRadius"), Radius);
            VFX->SetFloatParameter(TEXT("LeafDensity"), Radius / 100.0f);
        }
    }
}

// Combat VFX Implementation
void UVFXSystemCore::SpawnBloodSplatter(FVector Location, FVector Direction, float Amount)
{
    if (BloodSplatterVFX)
    {
        FRotator BloodRotation = Direction.Rotation();
        UNiagaraComponent* VFX = SpawnVFX(BloodSplatterVFX, Location, BloodRotation, EVFXCategory::Combat);
        if (VFX)
        {
            VFX->SetFloatParameter(TEXT("BloodAmount"), Amount);
            VFX->SetVectorParameter(TEXT("SplatterDirection"), Direction);
            VFX->SetFloatParameter(TEXT("DropletSize"), Amount * 0.5f);
        }
    }
}

void UVFXSystemCore::SpawnImpactEffect(FVector Location, FVector Normal, UPhysicalMaterial* Material)
{
    if (ImpactEffectVFX)
    {
        FRotator ImpactRotation = Normal.Rotation();
        UNiagaraComponent* VFX = SpawnVFX(ImpactEffectVFX, Location, ImpactRotation, EVFXCategory::Combat);
        if (VFX)
        {
            VFX->SetVectorParameter(TEXT("SurfaceNormal"), Normal);
            
            // Adjust effect based on material type
            if (Material)
            {
                // Different effects for different materials
                FName MaterialName = Material->GetFName();
                if (MaterialName.ToString().Contains(TEXT("Wood")))
                {
                    VFX->SetFloatParameter(TEXT("WoodChipCount"), 10.0f);
                }
                else if (MaterialName.ToString().Contains(TEXT("Stone")))
                {
                    VFX->SetFloatParameter(TEXT("StoneSparkCount"), 5.0f);
                }
            }
        }
    }
}

// Performance Management
void UVFXSystemCore::UpdateVFXPerformance()
{
    // Monitor frame rate and adjust quality if needed
    float CurrentFPS = 1.0f / GetWorld()->GetDeltaSeconds();
    
    if (CurrentFPS < 25.0f && CurrentQualityLevel != EVFXQualityLevel::Low)
    {
        // Reduce quality if FPS is too low
        EVFXQualityLevel NewQuality = static_cast<EVFXQualityLevel>(
            FMath::Max(0, static_cast<int32>(CurrentQualityLevel) - 1)
        );
        SetVFXQuality(NewQuality);
    }
    else if (CurrentFPS > 50.0f && CurrentQualityLevel != EVFXQualityLevel::High)
    {
        // Increase quality if FPS is stable
        EVFXQualityLevel NewQuality = static_cast<EVFXQualityLevel>(
            FMath::Min(2, static_cast<int32>(CurrentQualityLevel) + 1)
        );
        SetVFXQuality(NewQuality);
    }
}

void UVFXSystemCore::CleanupInactiveVFX()
{
    // Remove null or destroyed VFX components
    ActiveVFXComponents.RemoveAll([](UNiagaraComponent* VFXComp)
    {
        return !VFXComp || !IsValid(VFXComp) || !VFXComp->IsActive();
    });
}

FVFXLODSettings UVFXSystemCore::GetCurrentLODSettings() const
{
    return LODSettings[static_cast<int32>(CurrentQualityLevel)];
}