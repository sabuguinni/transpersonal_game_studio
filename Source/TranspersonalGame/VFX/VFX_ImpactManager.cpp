#include "VFX_ImpactManager.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "Components/SceneComponent.h"

UVFX_ImpactManager::UVFX_ImpactManager()
{
    PrimaryComponentTick.bCanEverTick = false;
    bWantsInitializeComponent = true;
    
    MaxActiveEffects = 20;
    EffectCullDistance = 5000.0f;
    MinTimeBetweenEffects = 0.1f;
}

void UVFX_ImpactManager::BeginPlay()
{
    Super::BeginPlay();
    InitializeDefaultEffects();
}

void UVFX_ImpactManager::InitializeDefaultEffects()
{
    // Initialize default VFX data for prehistoric impact effects
    FVFX_ImpactData FootstepData;
    FootstepData.Scale = FVector(2.0f, 2.0f, 1.0f);
    FootstepData.Duration = 2.0f;
    FootstepData.bAttachToGround = true;
    ImpactEffects.Add(EVFX_ImpactType::FootstepDust, FootstepData);

    FVFX_ImpactData BloodData;
    BloodData.Scale = FVector(1.0f);
    BloodData.Duration = 5.0f;
    BloodData.bAttachToGround = false;
    ImpactEffects.Add(EVFX_ImpactType::BloodSplatter, BloodData);

    FVFX_ImpactData RockData;
    RockData.Scale = FVector(1.5f);
    RockData.Duration = 3.0f;
    RockData.bAttachToGround = true;
    ImpactEffects.Add(EVFX_ImpactType::RockDebris, RockData);

    FVFX_ImpactData WaterData;
    WaterData.Scale = FVector(1.2f, 1.2f, 2.0f);
    WaterData.Duration = 1.5f;
    WaterData.bAttachToGround = false;
    ImpactEffects.Add(EVFX_ImpactType::WaterSplash, WaterData);

    FVFX_ImpactData CrackData;
    CrackData.Scale = FVector(3.0f, 3.0f, 0.1f);
    CrackData.Duration = 8.0f;
    CrackData.bAttachToGround = true;
    ImpactEffects.Add(EVFX_ImpactType::GroundCrack, CrackData);

    FVFX_ImpactData VegData;
    VegData.Scale = FVector(1.0f);
    VegData.Duration = 1.0f;
    VegData.bAttachToGround = false;
    ImpactEffects.Add(EVFX_ImpactType::VegetationRustle, VegData);
}

void UVFX_ImpactManager::SpawnImpactEffect(EVFX_ImpactType ImpactType, const FVector& Location, const FRotator& Rotation, float ScaleMultiplier)
{
    if (!CanSpawnEffect())
    {
        return;
    }

    FVFX_ImpactData* EffectData = ImpactEffects.Find(ImpactType);
    if (!EffectData)
    {
        UE_LOG(LogTemp, Warning, TEXT("VFX_ImpactManager: No effect data found for impact type"));
        return;
    }

    // Check distance culling
    if (AActor* Owner = GetOwner())
    {
        if (FVector::Dist(Owner->GetActorLocation(), Location) > EffectCullDistance)
        {
            return;
        }
    }

    // Load and spawn Niagara system if available
    if (EffectData->NiagaraSystem.IsValid())
    {
        UNiagaraSystem* NiagaraSystem = EffectData->NiagaraSystem.LoadSynchronous();
        if (NiagaraSystem)
        {
            FVector FinalScale = EffectData->Scale * ScaleMultiplier;
            
            UNiagaraComponent* SpawnedEffect = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
                GetWorld(),
                NiagaraSystem,
                Location,
                Rotation,
                FinalScale,
                true,
                true,
                ENCPoolMethod::None,
                true
            );

            if (SpawnedEffect)
            {
                ActiveEffects.Add(SpawnedEffect);
                LastEffectTime = GetWorld()->GetTimeSeconds();

                // Schedule cleanup
                FTimerHandle CleanupTimer;
                GetWorld()->GetTimerManager().SetTimer(
                    CleanupTimer,
                    [this, SpawnedEffect]()
                    {
                        RemoveExpiredEffect(SpawnedEffect);
                    },
                    EffectData->Duration,
                    false
                );
            }
        }
    }

    // Cleanup if we exceed max effects
    if (ActiveEffects.Num() > MaxActiveEffects)
    {
        CleanupExpiredEffects();
    }
}

void UVFX_ImpactManager::SpawnFootstepEffect(const FVector& FootLocation, float DinosaurSize)
{
    float SizeScale = FMath::Clamp(DinosaurSize, 0.5f, 5.0f);
    SpawnImpactEffect(EVFX_ImpactType::FootstepDust, FootLocation, FRotator::ZeroRotator, SizeScale);
    
    // For very large dinosaurs, also spawn ground crack effect
    if (DinosaurSize > 3.0f)
    {
        SpawnImpactEffect(EVFX_ImpactType::GroundCrack, FootLocation, FRotator::ZeroRotator, SizeScale * 0.8f);
    }
}

void UVFX_ImpactManager::SpawnBloodEffect(const FVector& ImpactLocation, const FVector& ImpactDirection)
{
    FRotator BloodRotation = ImpactDirection.Rotation();
    SpawnImpactEffect(EVFX_ImpactType::BloodSplatter, ImpactLocation, BloodRotation, 1.0f);
}

void UVFX_ImpactManager::CleanupExpiredEffects()
{
    ActiveEffects.RemoveAll([](UNiagaraComponent* Effect)
    {
        return !IsValid(Effect) || !Effect->IsActive();
    });
}

void UVFX_ImpactManager::SetMaxActiveEffects(int32 NewMax)
{
    MaxActiveEffects = FMath::Max(1, NewMax);
    if (ActiveEffects.Num() > MaxActiveEffects)
    {
        CleanupExpiredEffects();
    }
}

bool UVFX_ImpactManager::CanSpawnEffect() const
{
    if (!GetWorld())
    {
        return false;
    }

    float CurrentTime = GetWorld()->GetTimeSeconds();
    if (CurrentTime - LastEffectTime < MinTimeBetweenEffects)
    {
        return false;
    }

    return ActiveEffects.Num() < MaxActiveEffects;
}

void UVFX_ImpactManager::RemoveExpiredEffect(UNiagaraComponent* Effect)
{
    if (IsValid(Effect))
    {
        ActiveEffects.Remove(Effect);
        Effect->DestroyComponent();
    }
}