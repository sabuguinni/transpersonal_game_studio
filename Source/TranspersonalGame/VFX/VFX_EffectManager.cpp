#include "VFX_EffectManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "NiagaraFunctionLibrary.h"

AVFX_EffectManager::AVFX_EffectManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 1.0f; // Check for cleanup every second

    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;

    InitializeEffectDatabase();
}

void AVFX_EffectManager::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Warning, TEXT("VFX_EffectManager: System initialized with %d effect types"), EffectDatabase.Num());
}

void AVFX_EffectManager::InitializeEffectDatabase()
{
    // Initialize campfire effect
    FVFX_EffectData CampfireData;
    CampfireData.EffectScale = FVector(1.0f, 1.0f, 1.5f);
    CampfireData.EffectDuration = 0.0f; // Persistent fire
    CampfireData.bAutoDestroy = false;
    EffectDatabase.Add(EVFX_EffectType::Fire_Campfire, CampfireData);

    // Initialize footstep dust effect
    FVFX_EffectData FootstepData;
    FootstepData.EffectScale = FVector(1.0f);
    FootstepData.EffectDuration = 3.0f;
    FootstepData.bAutoDestroy = true;
    EffectDatabase.Add(EVFX_EffectType::Dust_Footstep, FootstepData);

    // Initialize blood impact effect
    FVFX_EffectData BloodData;
    BloodData.EffectScale = FVector(0.8f);
    BloodData.EffectDuration = 5.0f;
    BloodData.bAutoDestroy = true;
    EffectDatabase.Add(EVFX_EffectType::Blood_Impact, BloodData);

    // Initialize water splash effect
    FVFX_EffectData WaterData;
    WaterData.EffectScale = FVector(1.2f);
    WaterData.EffectDuration = 2.5f;
    WaterData.bAutoDestroy = true;
    EffectDatabase.Add(EVFX_EffectType::Water_Splash, WaterData);

    // Initialize cooking smoke effect
    FVFX_EffectData SmokeData;
    SmokeData.EffectScale = FVector(0.7f, 0.7f, 1.0f);
    SmokeData.EffectDuration = 0.0f; // Persistent while cooking
    SmokeData.bAutoDestroy = false;
    EffectDatabase.Add(EVFX_EffectType::Smoke_Cooking, SmokeData);

    // Initialize rain effect
    FVFX_EffectData RainData;
    RainData.EffectScale = FVector(10.0f, 10.0f, 5.0f);
    RainData.EffectDuration = 0.0f; // Weather-controlled
    RainData.bAutoDestroy = false;
    EffectDatabase.Add(EVFX_EffectType::Rain_Weather, RainData);

    // Initialize wind particles effect
    FVFX_EffectData WindData;
    WindData.EffectScale = FVector(5.0f, 5.0f, 2.0f);
    WindData.EffectDuration = 0.0f; // Environmental
    WindData.bAutoDestroy = false;
    EffectDatabase.Add(EVFX_EffectType::Wind_Particles, WindData);

    // Initialize crafting sparks effect
    FVFX_EffectData SparksData;
    SparksData.EffectScale = FVector(0.5f);
    SparksData.EffectDuration = 1.5f;
    SparksData.bAutoDestroy = true;
    EffectDatabase.Add(EVFX_EffectType::Sparks_Crafting, SparksData);
}

UNiagaraComponent* AVFX_EffectManager::SpawnEffect(EVFX_EffectType EffectType, FVector Location, FRotator Rotation)
{
    if (!EffectDatabase.Contains(EffectType))
    {
        UE_LOG(LogTemp, Warning, TEXT("VFX_EffectManager: Effect type not found in database"));
        return nullptr;
    }

    const FVFX_EffectData& EffectData = EffectDatabase[EffectType];
    
    if (!EffectData.NiagaraEffect.IsValid())
    {
        UE_LOG(LogTemp, Warning, TEXT("VFX_EffectManager: Niagara system not assigned for effect type"));
        return nullptr;
    }

    UNiagaraComponent* NewEffect = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
        GetWorld(),
        EffectData.NiagaraEffect.Get(),
        Location,
        Rotation,
        EffectData.EffectScale,
        true,
        true,
        ENCPoolMethod::None,
        true
    );

    if (NewEffect)
    {
        ActiveEffects.Add(NewEffect);
        
        // Set auto-destroy timer if needed
        if (EffectData.bAutoDestroy && EffectData.EffectDuration > 0.0f)
        {
            FTimerHandle CleanupTimer;
            GetWorld()->GetTimerManager().SetTimer(
                CleanupTimer,
                [this, NewEffect]()
                {
                    CleanupEffect(NewEffect);
                },
                EffectData.EffectDuration,
                false
            );
        }

        UE_LOG(LogTemp, Log, TEXT("VFX_EffectManager: Spawned effect at location %s"), *Location.ToString());
    }

    return NewEffect;
}

void AVFX_EffectManager::SpawnFootstepDust(FVector FootLocation, float DinosaurSize)
{
    FVector AdjustedLocation = FootLocation + FVector(0, 0, -10); // Slightly below ground
    UNiagaraComponent* DustEffect = SpawnEffect(EVFX_EffectType::Dust_Footstep, AdjustedLocation);
    
    if (DustEffect)
    {
        // Scale effect based on dinosaur size
        FVector ScaleMultiplier = FVector(DinosaurSize);
        DustEffect->SetWorldScale3D(ScaleMultiplier);
        
        // Set dust intensity parameter
        DustEffect->SetFloatParameter(TEXT("DustIntensity"), FMath::Clamp(DinosaurSize, 0.5f, 3.0f));
    }
}

void AVFX_EffectManager::SpawnCampfire(FVector FireLocation)
{
    UNiagaraComponent* FireEffect = SpawnEffect(EVFX_EffectType::Fire_Campfire, FireLocation);
    
    if (FireEffect)
    {
        // Set fire parameters
        FireEffect->SetFloatParameter(TEXT("FireIntensity"), 1.0f);
        FireEffect->SetVectorParameter(TEXT("WindDirection"), FVector(1, 0, 0));
    }
}

void AVFX_EffectManager::SpawnBloodImpact(FVector ImpactLocation, FVector ImpactNormal)
{
    FRotator ImpactRotation = FRotationMatrix::MakeFromZ(ImpactNormal).Rotator();
    UNiagaraComponent* BloodEffect = SpawnEffect(EVFX_EffectType::Blood_Impact, ImpactLocation, ImpactRotation);
    
    if (BloodEffect)
    {
        BloodEffect->SetVectorParameter(TEXT("ImpactNormal"), ImpactNormal);
        BloodEffect->SetFloatParameter(TEXT("BloodAmount"), 1.0f);
    }
}

void AVFX_EffectManager::SpawnWaterSplash(FVector WaterLocation, float SplashIntensity)
{
    UNiagaraComponent* SplashEffect = SpawnEffect(EVFX_EffectType::Water_Splash, WaterLocation);
    
    if (SplashEffect)
    {
        SplashEffect->SetFloatParameter(TEXT("SplashIntensity"), SplashIntensity);
        FVector SplashScale = FVector(SplashIntensity);
        SplashEffect->SetWorldScale3D(SplashScale);
    }
}

void AVFX_EffectManager::CleanupExpiredEffects()
{
    ActiveEffects.RemoveAll([this](UNiagaraComponent* Effect)
    {
        if (!IsValid(Effect) || !Effect->IsActive())
        {
            if (IsValid(Effect))
            {
                Effect->DestroyComponent();
            }
            return true;
        }
        return false;
    });
}

void AVFX_EffectManager::CleanupEffect(UNiagaraComponent* EffectComponent)
{
    if (IsValid(EffectComponent))
    {
        EffectComponent->Deactivate();
        ActiveEffects.Remove(EffectComponent);
        EffectComponent->DestroyComponent();
    }
}

void AVFX_EffectManager::TestAllEffects()
{
    FVector TestLocation = GetActorLocation();
    
    // Test each effect type
    SpawnFootstepDust(TestLocation + FVector(100, 0, 0), 2.0f);
    SpawnCampfire(TestLocation + FVector(0, 100, 0));
    SpawnBloodImpact(TestLocation + FVector(-100, 0, 0), FVector::UpVector);
    SpawnWaterSplash(TestLocation + FVector(0, -100, 0), 1.5f);
    
    UE_LOG(LogTemp, Warning, TEXT("VFX_EffectManager: Test effects spawned"));
}