#include "VFX_NiagaraSystemManager.h"
#include "NiagaraFunctionLibrary.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "TimerManager.h"

AVFX_NiagaraSystemManager::AVFX_NiagaraSystemManager()
{
    PrimaryActorTick.bCanEverTick = true;
    
    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;
    
    InitializeEffectDatabase();
}

void AVFX_NiagaraSystemManager::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Warning, TEXT("VFX Niagara System Manager initialized"));
}

void AVFX_NiagaraSystemManager::InitializeEffectDatabase()
{
    EffectDatabase.Empty();
    
    // Campfire effect
    FVFX_EffectData CampfireData;
    CampfireData.EffectType = EVFX_EffectType::Fire_Campfire;
    CampfireData.DefaultScale = FVector(1.5f, 1.5f, 2.0f);
    CampfireData.Duration = 0.0f; // Infinite
    CampfireData.bLooping = true;
    EffectDatabase.Add(CampfireData);
    
    // Footstep dust
    FVFX_EffectData FootstepData;
    FootstepData.EffectType = EVFX_EffectType::Dust_Footstep;
    FootstepData.DefaultScale = FVector(2.0f, 2.0f, 1.0f);
    FootstepData.Duration = 2.0f;
    FootstepData.bLooping = false;
    EffectDatabase.Add(FootstepData);
    
    // Rain effect
    FVFX_EffectData RainData;
    RainData.EffectType = EVFX_EffectType::Weather_Rain;
    RainData.DefaultScale = FVector(10.0f, 10.0f, 5.0f);
    RainData.Duration = 0.0f; // Infinite
    RainData.bLooping = true;
    EffectDatabase.Add(RainData);
    
    // Blood impact
    FVFX_EffectData BloodData;
    BloodData.EffectType = EVFX_EffectType::Blood_Impact;
    BloodData.DefaultScale = FVector(1.0f, 1.0f, 1.0f);
    BloodData.Duration = 3.0f;
    BloodData.bLooping = false;
    EffectDatabase.Add(BloodData);
    
    // Water splash
    FVFX_EffectData SplashData;
    SplashData.EffectType = EVFX_EffectType::Water_Splash;
    SplashData.DefaultScale = FVector(1.5f, 1.5f, 1.5f);
    SplashData.Duration = 2.5f;
    SplashData.bLooping = false;
    EffectDatabase.Add(SplashData);
}

UNiagaraComponent* AVFX_NiagaraSystemManager::SpawnEffect(EVFX_EffectType EffectType, FVector Location, FRotator Rotation, FVector Scale)
{
    FVFX_EffectData* EffectData = GetEffectData(EffectType);
    if (!EffectData)
    {
        UE_LOG(LogTemp, Warning, TEXT("VFX: Effect type not found in database"));
        return nullptr;
    }
    
    // Create Niagara component
    UNiagaraComponent* NiagaraComp = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
        GetWorld(),
        EffectData->NiagaraSystem.LoadSynchronous(),
        Location,
        Rotation,
        Scale * EffectData->DefaultScale,
        true,
        true,
        ENCPoolMethod::None,
        true
    );
    
    if (NiagaraComp)
    {
        ActiveEffects.Add(NiagaraComp);
        
        // Set auto-destroy timer for non-looping effects
        if (!EffectData->bLooping && EffectData->Duration > 0.0f)
        {
            FTimerHandle TimerHandle;
            GetWorld()->GetTimerManager().SetTimer(TimerHandle, [this, NiagaraComp]()
            {
                StopEffect(NiagaraComp);
            }, EffectData->Duration, false);
        }
        
        UE_LOG(LogTemp, Log, TEXT("VFX: Spawned effect at location %s"), *Location.ToString());
    }
    
    return NiagaraComp;
}

void AVFX_NiagaraSystemManager::StopEffect(UNiagaraComponent* EffectComponent)
{
    if (EffectComponent && IsValid(EffectComponent))
    {
        EffectComponent->Deactivate();
        EffectComponent->DestroyComponent();
        ActiveEffects.Remove(EffectComponent);
    }
}

void AVFX_NiagaraSystemManager::StopAllEffects()
{
    for (UNiagaraComponent* Effect : ActiveEffects)
    {
        if (Effect && IsValid(Effect))
        {
            Effect->Deactivate();
            Effect->DestroyComponent();
        }
    }
    ActiveEffects.Empty();
}

UNiagaraComponent* AVFX_NiagaraSystemManager::SpawnCampfireEffect(FVector Location)
{
    return SpawnEffect(EVFX_EffectType::Fire_Campfire, Location);
}

UNiagaraComponent* AVFX_NiagaraSystemManager::SpawnFootstepDust(FVector Location, float IntensityScale)
{
    FVector Scale = FVector(IntensityScale, IntensityScale, IntensityScale);
    return SpawnEffect(EVFX_EffectType::Dust_Footstep, Location, FRotator::ZeroRotator, Scale);
}

UNiagaraComponent* AVFX_NiagaraSystemManager::SpawnRainEffect(FVector Location, float Intensity)
{
    FVector Scale = FVector(Intensity, Intensity, Intensity);
    return SpawnEffect(EVFX_EffectType::Weather_Rain, Location, FRotator::ZeroRotator, Scale);
}

UNiagaraComponent* AVFX_NiagaraSystemManager::SpawnBloodImpact(FVector Location, FVector ImpactDirection)
{
    FRotator Rotation = ImpactDirection.Rotation();
    return SpawnEffect(EVFX_EffectType::Blood_Impact, Location, Rotation);
}

UNiagaraComponent* AVFX_NiagaraSystemManager::SpawnWaterSplash(FVector Location, float SplashSize)
{
    FVector Scale = FVector(SplashSize, SplashSize, SplashSize);
    return SpawnEffect(EVFX_EffectType::Water_Splash, Location, FRotator::ZeroRotator, Scale);
}

FVFX_EffectData* AVFX_NiagaraSystemManager::GetEffectData(EVFX_EffectType EffectType)
{
    for (FVFX_EffectData& Data : EffectDatabase)
    {
        if (Data.EffectType == EffectType)
        {
            return &Data;
        }
    }
    return nullptr;
}

void AVFX_NiagaraSystemManager::CleanupFinishedEffects()
{
    ActiveEffects.RemoveAll([](UNiagaraComponent* Effect)
    {
        return !Effect || !IsValid(Effect) || !Effect->IsActive();
    });
}

void AVFX_NiagaraSystemManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // Cleanup finished effects every few seconds
    static float CleanupTimer = 0.0f;
    CleanupTimer += DeltaTime;
    if (CleanupTimer >= 5.0f)
    {
        CleanupFinishedEffects();
        CleanupTimer = 0.0f;
    }
}