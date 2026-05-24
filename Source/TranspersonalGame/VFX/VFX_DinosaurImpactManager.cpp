#include "VFX_DinosaurImpactManager.h"
#include "NiagaraFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"
#include "Engine/World.h"

UVFX_DinosaurImpactManager::UVFX_DinosaurImpactManager()
{
    MaxActiveEffects = 20.0f;
    EffectLifetime = 5.0f;
    
    SetupImpactData();
}

void UVFX_DinosaurImpactManager::SetupImpactData()
{
    // Small dinosaur footstep (Compsognathus, small raptors)
    FVFX_DinosaurImpactData SmallFootstep;
    SmallFootstep.ImpactRadius = 50.0f;
    SmallFootstep.DustIntensity = 0.3f;
    SmallFootstep.DebrisCount = 3.0f;
    ImpactDataMap.Add(EVFX_DinosaurImpactType::Footstep_Small, SmallFootstep);

    // Medium dinosaur footstep (Velociraptors, Parasaurolophus)
    FVFX_DinosaurImpactData MediumFootstep;
    MediumFootstep.ImpactRadius = 100.0f;
    MediumFootstep.DustIntensity = 0.6f;
    MediumFootstep.DebrisCount = 8.0f;
    ImpactDataMap.Add(EVFX_DinosaurImpactType::Footstep_Medium, MediumFootstep);

    // Large dinosaur footstep (Allosaurus, Carnotaurus)
    FVFX_DinosaurImpactData LargeFootstep;
    LargeFootstep.ImpactRadius = 200.0f;
    LargeFootstep.DustIntensity = 1.0f;
    LargeFootstep.DebrisCount = 15.0f;
    ImpactDataMap.Add(EVFX_DinosaurImpactType::Footstep_Large, LargeFootstep);

    // Massive dinosaur footstep (T-Rex, Brachiosaurus)
    FVFX_DinosaurImpactData MassiveFootstep;
    MassiveFootstep.ImpactRadius = 400.0f;
    MassiveFootstep.DustIntensity = 2.0f;
    MassiveFootstep.DebrisCount = 25.0f;
    ImpactDataMap.Add(EVFX_DinosaurImpactType::Footstep_Massive, MassiveFootstep);

    // Tail slam impact
    FVFX_DinosaurImpactData TailSlam;
    TailSlam.ImpactRadius = 300.0f;
    TailSlam.DustIntensity = 1.5f;
    TailSlam.DebrisCount = 20.0f;
    ImpactDataMap.Add(EVFX_DinosaurImpactType::TailSlam, TailSlam);

    // Body fall impact
    FVFX_DinosaurImpactData BodyFall;
    BodyFall.ImpactRadius = 500.0f;
    BodyFall.DustIntensity = 3.0f;
    BodyFall.DebrisCount = 35.0f;
    ImpactDataMap.Add(EVFX_DinosaurImpactType::BodyFall, BodyFall);
}

void UVFX_DinosaurImpactManager::TriggerDinosaurImpact(EVFX_DinosaurImpactType ImpactType, FVector Location, FRotator Rotation)
{
    if (!ImpactDataMap.Contains(ImpactType))
    {
        UE_LOG(LogTemp, Warning, TEXT("VFX_DinosaurImpactManager: Unknown impact type"));
        return;
    }

    const FVFX_DinosaurImpactData& ImpactData = ImpactDataMap[ImpactType];
    
    // Spawn dust cloud effect
    SpawnDustCloud(ImpactData, Location, Rotation);
    
    // Spawn debris effect
    SpawnDebris(ImpactData, Location, Rotation);
    
    // Play impact audio
    PlayImpactAudio(ImpactData, Location);
    
    // Clean up old effects if we have too many
    CleanupExpiredEffects();
}

void UVFX_DinosaurImpactManager::TriggerFootstepImpact(FVector Location, float DinosaurMass, ETerrainType TerrainType)
{
    EVFX_DinosaurImpactType ImpactType = EVFX_DinosaurImpactType::Footstep_Small;
    
    // Determine impact type based on dinosaur mass
    if (DinosaurMass < 100.0f)
    {
        ImpactType = EVFX_DinosaurImpactType::Footstep_Small;
    }
    else if (DinosaurMass < 1000.0f)
    {
        ImpactType = EVFX_DinosaurImpactType::Footstep_Medium;
    }
    else if (DinosaurMass < 5000.0f)
    {
        ImpactType = EVFX_DinosaurImpactType::Footstep_Large;
    }
    else
    {
        ImpactType = EVFX_DinosaurImpactType::Footstep_Massive;
    }
    
    TriggerDinosaurImpact(ImpactType, Location);
}

void UVFX_DinosaurImpactManager::TriggerTailSlamImpact(FVector Location, FVector Direction, float Force)
{
    FRotator ImpactRotation = Direction.Rotation();
    TriggerDinosaurImpact(EVFX_DinosaurImpactType::TailSlam, Location, ImpactRotation);
}

void UVFX_DinosaurImpactManager::SpawnDustCloud(const FVFX_DinosaurImpactData& ImpactData, FVector Location, FRotator Rotation)
{
    UWorld* World = GetWorld();
    if (!World || !ImpactData.DustCloudEffect.IsValid())
    {
        return;
    }

    UNiagaraComponent* DustEffect = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
        World,
        ImpactData.DustCloudEffect.Get(),
        Location,
        Rotation
    );

    if (DustEffect)
    {
        DustEffect->SetFloatParameter(TEXT("DustIntensity"), ImpactData.DustIntensity);
        DustEffect->SetFloatParameter(TEXT("ImpactRadius"), ImpactData.ImpactRadius);
        ActiveEffects.Add(DustEffect);
    }
}

void UVFX_DinosaurImpactManager::SpawnDebris(const FVFX_DinosaurImpactData& ImpactData, FVector Location, FRotator Rotation)
{
    UWorld* World = GetWorld();
    if (!World || !ImpactData.DebrisEffect.IsValid())
    {
        return;
    }

    UNiagaraComponent* DebrisEffect = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
        World,
        ImpactData.DebrisEffect.Get(),
        Location,
        Rotation
    );

    if (DebrisEffect)
    {
        DebrisEffect->SetFloatParameter(TEXT("DebrisCount"), ImpactData.DebrisCount);
        DebrisEffect->SetFloatParameter(TEXT("ImpactRadius"), ImpactData.ImpactRadius);
        ActiveEffects.Add(DebrisEffect);
    }
}

void UVFX_DinosaurImpactManager::PlayImpactAudio(const FVFX_DinosaurImpactData& ImpactData, FVector Location)
{
    UWorld* World = GetWorld();
    if (!World || !ImpactData.ImpactSound.IsValid())
    {
        return;
    }

    UGameplayStatics::PlaySoundAtLocation(
        World,
        ImpactData.ImpactSound.Get(),
        Location,
        1.0f,  // Volume multiplier
        1.0f,  // Pitch multiplier
        0.0f   // Start time
    );
}

float UVFX_DinosaurImpactManager::CalculateImpactIntensity(float DinosaurMass, ETerrainType TerrainType)
{
    float BaseIntensity = FMath::Sqrt(DinosaurMass / 1000.0f);
    
    // Modify intensity based on terrain type
    float TerrainModifier = 1.0f;
    switch (TerrainType)
    {
        case ETerrainType::Mud:
            TerrainModifier = 0.7f;  // Less dust, more splash
            break;
        case ETerrainType::Sand:
            TerrainModifier = 1.5f;  // More dust
            break;
        case ETerrainType::Rock:
            TerrainModifier = 0.5f;  // Less dust, more sparks
            break;
        case ETerrainType::Grass:
            TerrainModifier = 1.0f;  // Standard
            break;
        default:
            TerrainModifier = 1.0f;
            break;
    }
    
    return BaseIntensity * TerrainModifier;
}

void UVFX_DinosaurImpactManager::CleanupExpiredEffects()
{
    // Remove null or inactive effects
    ActiveEffects.RemoveAll([](UNiagaraComponent* Effect) {
        return !IsValid(Effect) || !Effect->IsActive();
    });

    // If we have too many effects, remove the oldest ones
    while (ActiveEffects.Num() > MaxActiveEffects)
    {
        if (UNiagaraComponent* OldestEffect = ActiveEffects[0])
        {
            OldestEffect->DestroyComponent();
        }
        ActiveEffects.RemoveAt(0);
    }
}

UWorld* UVFX_DinosaurImpactManager::GetWorld() const
{
    if (UObject* Outer = GetOuter())
    {
        return Outer->GetWorld();
    }
    return nullptr;
}