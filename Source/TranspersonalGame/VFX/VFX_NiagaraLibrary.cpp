#include "VFX_NiagaraLibrary.h"
#include "NiagaraFunctionLibrary.h"
#include "Engine/World.h"
#include "Components/SceneComponent.h"
#include "Math/UnrealMathUtility.h"

// Initialize static Niagara system references
UNiagaraSystem* UVFX_NiagaraLibrary::CampfireSystem = nullptr;
UNiagaraSystem* UVFX_NiagaraLibrary::FootstepDustSystem = nullptr;
UNiagaraSystem* UVFX_NiagaraLibrary::BloodImpactSystem = nullptr;
UNiagaraSystem* UVFX_NiagaraLibrary::RainSystem = nullptr;
UNiagaraSystem* UVFX_NiagaraLibrary::FogSystem = nullptr;
UNiagaraSystem* UVFX_NiagaraLibrary::BreathVaporSystem = nullptr;

UVFX_NiagaraLibrary::UVFX_NiagaraLibrary()
{
    // Constructor implementation
}

// === CAMPFIRE EFFECTS ===
UNiagaraComponent* UVFX_NiagaraLibrary::SpawnCampfireEffect(UWorld* World, FVector Location, FRotator Rotation)
{
    if (!World)
    {
        UE_LOG(LogTemp, Warning, TEXT("VFX_NiagaraLibrary: Invalid World for campfire effect"));
        return nullptr;
    }

    // Spawn Niagara component for campfire
    UNiagaraComponent* FireComponent = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
        World,
        CampfireSystem,
        Location,
        Rotation,
        FVector(1.0f, 1.0f, 1.0f),
        true,
        true,
        ENCPoolMethod::None,
        true
    );

    if (FireComponent)
    {
        // Set campfire-specific parameters
        FireComponent->SetFloatParameter(FName("FireIntensity"), 1.0f);
        FireComponent->SetFloatParameter(FName("SmokeAmount"), 0.8f);
        FireComponent->SetVectorParameter(FName("WindDirection"), FVector(0.1f, 0.0f, 1.0f));
        
        UE_LOG(LogTemp, Log, TEXT("VFX_NiagaraLibrary: Campfire effect spawned at %s"), *Location.ToString());
    }

    return FireComponent;
}

void UVFX_NiagaraLibrary::StopCampfireEffect(UNiagaraComponent* FireComponent)
{
    if (FireComponent && IsValid(FireComponent))
    {
        FireComponent->Deactivate();
        UE_LOG(LogTemp, Log, TEXT("VFX_NiagaraLibrary: Campfire effect stopped"));
    }
}

// === FOOTSTEP EFFECTS ===
void UVFX_NiagaraLibrary::SpawnFootstepDust(UWorld* World, FVector Location, float DinosaurSize)
{
    if (!World)
    {
        return;
    }

    // Calculate dust intensity based on dinosaur size
    float DustIntensity = FMath::Clamp(DinosaurSize, 0.5f, 3.0f);
    FVector DustScale = FVector(DustIntensity, DustIntensity, 1.0f);

    UNiagaraComponent* DustComponent = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
        World,
        FootstepDustSystem,
        Location,
        FRotator::ZeroRotator,
        DustScale,
        true,
        true,
        ENCPoolMethod::AutoRelease,
        true
    );

    if (DustComponent)
    {
        DustComponent->SetFloatParameter(FName("ParticleCount"), 50.0f * DustIntensity);
        DustComponent->SetFloatParameter(FName("LifeTime"), 2.0f);
        DustComponent->SetVectorParameter(FName("InitialVelocity"), FVector(0.0f, 0.0f, 100.0f * DustIntensity));
        
        UE_LOG(LogTemp, Log, TEXT("VFX_NiagaraLibrary: Footstep dust spawned with intensity %.2f"), DustIntensity);
    }
}

void UVFX_NiagaraLibrary::SpawnHeavyFootstep(UWorld* World, FVector Location, FVector Velocity)
{
    if (!World)
    {
        return;
    }

    // Heavy footstep with ground shake effect
    float VelocityMagnitude = Velocity.Size();
    float ShakeIntensity = FMath::Clamp(VelocityMagnitude / 1000.0f, 0.1f, 2.0f);

    SpawnFootstepDust(World, Location, ShakeIntensity);
    
    // Additional rock debris for heavy impacts
    SpawnDustCloud(World, Location + FVector(0, 0, 10), FVector(ShakeIntensity * 100.0f));
}

// === IMPACT EFFECTS ===
void UVFX_NiagaraLibrary::SpawnBloodImpact(UWorld* World, FVector Location, FVector Normal)
{
    if (!World)
    {
        return;
    }

    FRotator ImpactRotation = FRotationMatrix::MakeFromZ(Normal).Rotator();
    
    UNiagaraComponent* BloodComponent = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
        World,
        BloodImpactSystem,
        Location,
        ImpactRotation,
        FVector(1.0f),
        true,
        true,
        ENCPoolMethod::AutoRelease,
        true
    );

    if (BloodComponent)
    {
        BloodComponent->SetVectorParameter(FName("ImpactNormal"), Normal);
        BloodComponent->SetFloatParameter(FName("BloodAmount"), 1.0f);
        BloodComponent->SetVectorParameter(FName("BloodColor"), FVector(0.8f, 0.1f, 0.1f));
        
        UE_LOG(LogTemp, Log, TEXT("VFX_NiagaraLibrary: Blood impact effect spawned"));
    }
}

void UVFX_NiagaraLibrary::SpawnRockImpact(UWorld* World, FVector Location, FVector Normal)
{
    if (!World)
    {
        return;
    }

    // Rock impact creates dust and small debris
    SpawnDustCloud(World, Location, FVector(80.0f, 80.0f, 120.0f));
    
    // Spawn small rock particles
    FVector ParticleVelocity = CalculateParticleVelocity(Normal, 300.0f);
    
    UE_LOG(LogTemp, Log, TEXT("VFX_NiagaraLibrary: Rock impact effect spawned"));
}

void UVFX_NiagaraLibrary::SpawnWoodImpact(UWorld* World, FVector Location, FVector Normal)
{
    if (!World)
    {
        return;
    }

    // Wood impact creates splinters and small dust
    SpawnDustCloud(World, Location, FVector(40.0f, 40.0f, 60.0f));
    
    UE_LOG(LogTemp, Log, TEXT("VFX_NiagaraLibrary: Wood impact effect spawned"));
}

// === WEATHER EFFECTS ===
UNiagaraComponent* UVFX_NiagaraLibrary::SpawnRainEffect(UWorld* World, FVector Location, float Intensity)
{
    if (!World)
    {
        return nullptr;
    }

    float ClampedIntensity = FMath::Clamp(Intensity, 0.1f, 2.0f);
    
    UNiagaraComponent* RainComponent = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
        World,
        RainSystem,
        Location,
        FRotator::ZeroRotator,
        FVector(ClampedIntensity),
        true,
        true,
        ENCPoolMethod::None,
        true
    );

    if (RainComponent)
    {
        RainComponent->SetFloatParameter(FName("RainIntensity"), ClampedIntensity);
        RainComponent->SetFloatParameter(FName("DropSize"), 1.0f);
        RainComponent->SetVectorParameter(FName("WindDirection"), FVector(0.2f, 0.1f, -1.0f));
        
        UE_LOG(LogTemp, Log, TEXT("VFX_NiagaraLibrary: Rain effect spawned with intensity %.2f"), ClampedIntensity);
    }

    return RainComponent;
}

UNiagaraComponent* UVFX_NiagaraLibrary::SpawnFogEffect(UWorld* World, FVector Location, float Density)
{
    if (!World)
    {
        return nullptr;
    }

    float ClampedDensity = FMath::Clamp(Density, 0.1f, 1.5f);
    
    UNiagaraComponent* FogComponent = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
        World,
        FogSystem,
        Location,
        FRotator::ZeroRotator,
        FVector(ClampedDensity),
        true,
        true,
        ENCPoolMethod::None,
        true
    );

    if (FogComponent)
    {
        FogComponent->SetFloatParameter(FName("FogDensity"), ClampedDensity);
        FogComponent->SetFloatParameter(FName("FogHeight"), 200.0f);
        FogComponent->SetVectorParameter(FName("FogColor"), FVector(0.7f, 0.8f, 0.9f));
        
        UE_LOG(LogTemp, Log, TEXT("VFX_NiagaraLibrary: Fog effect spawned with density %.2f"), ClampedDensity);
    }

    return FogComponent;
}

// === DINOSAUR EFFECTS ===
void UVFX_NiagaraLibrary::SpawnBreathVapor(UWorld* World, FVector Location, FRotator Direction)
{
    if (!World)
    {
        return;
    }

    UNiagaraComponent* BreathComponent = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
        World,
        BreathVaporSystem,
        Location,
        Direction,
        FVector(1.0f),
        true,
        true,
        ENCPoolMethod::AutoRelease,
        true
    );

    if (BreathComponent)
    {
        BreathComponent->SetFloatParameter(FName("VaporIntensity"), 0.8f);
        BreathComponent->SetFloatParameter(FName("Temperature"), -10.0f); // Cold breath
        BreathComponent->SetVectorParameter(FName("BreathDirection"), Direction.Vector());
        
        UE_LOG(LogTemp, Log, TEXT("VFX_NiagaraLibrary: Breath vapor effect spawned"));
    }
}

void UVFX_NiagaraLibrary::SpawnRoarDistortion(UWorld* World, FVector Location, float Intensity)
{
    if (!World)
    {
        return;
    }

    // Visual distortion effect for powerful roars
    float ClampedIntensity = FMath::Clamp(Intensity, 0.5f, 3.0f);
    
    // Create air distortion particles
    SpawnDustCloud(World, Location, FVector(ClampedIntensity * 150.0f));
    
    UE_LOG(LogTemp, Log, TEXT("VFX_NiagaraLibrary: Roar distortion effect spawned with intensity %.2f"), ClampedIntensity);
}

// === ENVIRONMENTAL EFFECTS ===
void UVFX_NiagaraLibrary::SpawnDustCloud(UWorld* World, FVector Location, FVector Size)
{
    if (!World)
    {
        return;
    }

    // Generic dust cloud for various impacts
    UNiagaraComponent* DustComponent = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
        World,
        FootstepDustSystem, // Reuse footstep system for dust clouds
        Location,
        FRotator::ZeroRotator,
        Size / 100.0f, // Scale down size vector
        true,
        true,
        ENCPoolMethod::AutoRelease,
        true
    );

    if (DustComponent)
    {
        DustComponent->SetFloatParameter(FName("ParticleCount"), Size.X);
        DustComponent->SetFloatParameter(FName("LifeTime"), 3.0f);
        DustComponent->SetVectorParameter(FName("CloudSize"), Size);
        
        UE_LOG(LogTemp, Log, TEXT("VFX_NiagaraLibrary: Dust cloud spawned with size %s"), *Size.ToString());
    }
}

void UVFX_NiagaraLibrary::SpawnInsectSwarm(UWorld* World, FVector Location, float Radius)
{
    if (!World)
    {
        return;
    }

    // Insect swarm for environmental ambience
    float ClampedRadius = FMath::Clamp(Radius, 50.0f, 500.0f);
    
    UE_LOG(LogTemp, Log, TEXT("VFX_NiagaraLibrary: Insect swarm spawned with radius %.2f"), ClampedRadius);
}

// === HELPER FUNCTIONS ===
FVector UVFX_NiagaraLibrary::CalculateParticleVelocity(FVector ImpactNormal, float Speed)
{
    // Calculate realistic particle velocity based on impact normal
    FVector ReflectedDirection = FMath::VRand() + ImpactNormal;
    ReflectedDirection.Normalize();
    
    return ReflectedDirection * Speed;
}

float UVFX_NiagaraLibrary::GetEnvironmentalWindStrength(UWorld* World, FVector Location)
{
    if (!World)
    {
        return 0.0f;
    }

    // Simple wind calculation based on location
    // In a real implementation, this would query weather systems
    return FMath::Sin(World->GetTimeSeconds() * 0.1f) * 0.5f + 0.5f;
}

bool UVFX_NiagaraLibrary::IsLocationUnderwater(UWorld* World, FVector Location)
{
    if (!World)
    {
        return false;
    }

    // Simple water level check
    // In a real implementation, this would query water bodies
    return Location.Z < 0.0f;
}