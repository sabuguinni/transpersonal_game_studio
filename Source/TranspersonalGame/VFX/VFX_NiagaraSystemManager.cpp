#include "VFX_NiagaraSystemManager.h"
#include "NiagaraFunctionLibrary.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/SceneComponent.h"
#include "UObject/ConstructorHelpers.h"

AVFX_NiagaraSystemManager::AVFX_NiagaraSystemManager()
{
    PrimaryActorTick.bCanEverTick = false;

    // Create root scene component
    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;

    // Initialize default effect data
    LoadDefaultNiagaraSystems();
}

void AVFX_NiagaraSystemManager::BeginPlay()
{
    Super::BeginPlay();
    
    // Auto-initialize VFX systems on begin play
    InitializeVFXSystems();
}

void AVFX_NiagaraSystemManager::InitializeVFXSystems()
{
    UE_LOG(LogTemp, Warning, TEXT("VFX_NiagaraSystemManager: Initializing VFX systems..."));
    
    // Clear existing components
    for (UNiagaraComponent* Comp : ActiveNiagaraComponents)
    {
        if (IsValid(Comp))
        {
            Comp->DestroyComponent();
        }
    }
    ActiveNiagaraComponents.Empty();

    // Create Niagara components for each effect type
    for (const FVFX_EffectData& EffectData : EffectDataArray)
    {
        UNiagaraComponent* NiagaraComp = NewObject<UNiagaraComponent>(this);
        if (IsValid(NiagaraComp))
        {
            NiagaraComp->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
            
            // Try to load the Niagara system
            UNiagaraSystem* NiagaraSystem = EffectData.NiagaraSystem.LoadSynchronous();
            if (!IsValid(NiagaraSystem))
            {
                // Create placeholder system if loading failed
                NiagaraSystem = CreatePlaceholderNiagaraSystem(EffectData.EffectType);
            }
            
            if (IsValid(NiagaraSystem))
            {
                NiagaraComp->SetAsset(NiagaraSystem);
                NiagaraComp->SetRelativeScale3D(EffectData.Scale);
                NiagaraComp->SetAutoActivate(EffectData.bAutoActivate);
                
                if (EffectData.bAutoActivate)
                {
                    NiagaraComp->Activate();
                }
                
                ActiveNiagaraComponents.Add(NiagaraComp);
                
                UE_LOG(LogTemp, Log, TEXT("VFX_NiagaraSystemManager: Created VFX component for effect type %d"), (int32)EffectData.EffectType);
            }
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("VFX_NiagaraSystemManager: Initialized %d VFX systems"), ActiveNiagaraComponents.Num());
}

UNiagaraComponent* AVFX_NiagaraSystemManager::SpawnVFXEffect(EVFX_EffectType EffectType, FVector Location, FRotator Rotation)
{
    FVFX_EffectData* EffectData = GetEffectDataByType(EffectType);
    if (!EffectData)
    {
        UE_LOG(LogTemp, Warning, TEXT("VFX_NiagaraSystemManager: No effect data found for type %d"), (int32)EffectType);
        return nullptr;
    }

    // Load or create Niagara system
    UNiagaraSystem* NiagaraSystem = EffectData->NiagaraSystem.LoadSynchronous();
    if (!IsValid(NiagaraSystem))
    {
        NiagaraSystem = CreatePlaceholderNiagaraSystem(EffectType);
    }

    if (!IsValid(NiagaraSystem))
    {
        UE_LOG(LogTemp, Error, TEXT("VFX_NiagaraSystemManager: Failed to create Niagara system for effect type %d"), (int32)EffectType);
        return nullptr;
    }

    // Spawn the effect at world location
    UNiagaraComponent* SpawnedComponent = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
        GetWorld(),
        NiagaraSystem,
        Location,
        Rotation,
        EffectData->Scale,
        true, // Auto destroy
        true, // Auto activate
        ENCPoolMethod::None,
        true  // Pre cull check
    );

    if (IsValid(SpawnedComponent))
    {
        ActiveNiagaraComponents.Add(SpawnedComponent);
        UE_LOG(LogTemp, Log, TEXT("VFX_NiagaraSystemManager: Spawned VFX effect at location %s"), *Location.ToString());
        
        // Set duration if specified
        if (EffectData->Duration > 0.0f)
        {
            // Schedule destruction after duration
            FTimerHandle TimerHandle;
            GetWorld()->GetTimerManager().SetTimer(TimerHandle, [SpawnedComponent, this]()
            {
                if (IsValid(SpawnedComponent))
                {
                    StopVFXEffect(SpawnedComponent);
                }
            }, EffectData->Duration, false);
        }
    }

    return SpawnedComponent;
}

void AVFX_NiagaraSystemManager::StopVFXEffect(UNiagaraComponent* NiagaraComp)
{
    if (IsValid(NiagaraComp))
    {
        NiagaraComp->Deactivate();
        ActiveNiagaraComponents.Remove(NiagaraComp);
        UE_LOG(LogTemp, Log, TEXT("VFX_NiagaraSystemManager: Stopped VFX effect"));
    }
}

void AVFX_NiagaraSystemManager::StopAllVFXEffects()
{
    for (UNiagaraComponent* Comp : ActiveNiagaraComponents)
    {
        if (IsValid(Comp))
        {
            Comp->Deactivate();
        }
    }
    ActiveNiagaraComponents.Empty();
    UE_LOG(LogTemp, Warning, TEXT("VFX_NiagaraSystemManager: Stopped all VFX effects"));
}

TArray<UNiagaraComponent*> AVFX_NiagaraSystemManager::GetActiveVFXComponents() const
{
    TArray<UNiagaraComponent*> ValidComponents;
    for (UNiagaraComponent* Comp : ActiveNiagaraComponents)
    {
        if (IsValid(Comp))
        {
            ValidComponents.Add(Comp);
        }
    }
    return ValidComponents;
}

void AVFX_NiagaraSystemManager::SetVFXScale(UNiagaraComponent* NiagaraComp, FVector NewScale)
{
    if (IsValid(NiagaraComp))
    {
        NiagaraComp->SetRelativeScale3D(NewScale);
    }
}

void AVFX_NiagaraSystemManager::SetVFXLocation(UNiagaraComponent* NiagaraComp, FVector NewLocation)
{
    if (IsValid(NiagaraComp))
    {
        NiagaraComp->SetWorldLocation(NewLocation);
    }
}

FVFX_EffectData* AVFX_NiagaraSystemManager::GetEffectDataByType(EVFX_EffectType EffectType)
{
    for (FVFX_EffectData& EffectData : EffectDataArray)
    {
        if (EffectData.EffectType == EffectType)
        {
            return &EffectData;
        }
    }
    return nullptr;
}

void AVFX_NiagaraSystemManager::LoadDefaultNiagaraSystems()
{
    // Initialize default effect data array
    EffectDataArray.Empty();

    // Campfire effect
    FVFX_EffectData CampfireData;
    CampfireData.EffectType = EVFX_EffectType::Fire_Campfire;
    CampfireData.Scale = FVector(1.0f, 1.0f, 1.0f);
    CampfireData.Duration = -1.0f; // Infinite
    CampfireData.bAutoActivate = true;
    EffectDataArray.Add(CampfireData);

    // Footstep dust effect
    FVFX_EffectData FootstepData;
    FootstepData.EffectType = EVFX_EffectType::Dust_Footstep;
    FootstepData.Scale = FVector(0.5f, 0.5f, 0.5f);
    FootstepData.Duration = 2.0f;
    FootstepData.bAutoActivate = false;
    EffectDataArray.Add(FootstepData);

    // Blood impact effect
    FVFX_EffectData BloodData;
    BloodData.EffectType = EVFX_EffectType::Blood_Impact;
    BloodData.Scale = FVector(0.8f, 0.8f, 0.8f);
    BloodData.Duration = 3.0f;
    BloodData.bAutoActivate = false;
    EffectDataArray.Add(BloodData);

    // Water splash effect
    FVFX_EffectData WaterData;
    WaterData.EffectType = EVFX_EffectType::Water_Splash;
    WaterData.Scale = FVector(1.2f, 1.2f, 1.2f);
    WaterData.Duration = 1.5f;
    WaterData.bAutoActivate = false;
    EffectDataArray.Add(WaterData);

    // Rain effect
    FVFX_EffectData RainData;
    RainData.EffectType = EVFX_EffectType::Weather_Rain;
    RainData.Scale = FVector(5.0f, 5.0f, 2.0f);
    RainData.Duration = -1.0f; // Infinite
    RainData.bAutoActivate = false;
    EffectDataArray.Add(RainData);

    UE_LOG(LogTemp, Log, TEXT("VFX_NiagaraSystemManager: Loaded %d default effect configurations"), EffectDataArray.Num());
}

UNiagaraSystem* AVFX_NiagaraSystemManager::CreatePlaceholderNiagaraSystem(EVFX_EffectType EffectType)
{
    // Try to load engine default Niagara systems as placeholders
    UNiagaraSystem* PlaceholderSystem = nullptr;
    
    switch (EffectType)
    {
        case EVFX_EffectType::Fire_Campfire:
        case EVFX_EffectType::Smoke_Fire:
            // Try to load a basic fire/smoke system from engine content
            PlaceholderSystem = LoadObject<UNiagaraSystem>(nullptr, TEXT("/Engine/VFX/Niagara/Systems/NS_DefaultFire"));
            break;
            
        case EVFX_EffectType::Dust_Footstep:
        case EVFX_EffectType::Sparks_Impact:
            // Try to load a basic particle burst system
            PlaceholderSystem = LoadObject<UNiagaraSystem>(nullptr, TEXT("/Engine/VFX/Niagara/Systems/NS_DefaultBurst"));
            break;
            
        case EVFX_EffectType::Water_Splash:
            // Try to load a basic splash system
            PlaceholderSystem = LoadObject<UNiagaraSystem>(nullptr, TEXT("/Engine/VFX/Niagara/Systems/NS_DefaultSplash"));
            break;
            
        case EVFX_EffectType::Weather_Rain:
            // Try to load a basic rain system
            PlaceholderSystem = LoadObject<UNiagaraSystem>(nullptr, TEXT("/Engine/VFX/Niagara/Systems/NS_DefaultRain"));
            break;
            
        default:
            // Fallback to any available engine Niagara system
            PlaceholderSystem = LoadObject<UNiagaraSystem>(nullptr, TEXT("/Engine/VFX/Niagara/Systems/NS_DefaultParticles"));
            break;
    }
    
    if (!IsValid(PlaceholderSystem))
    {
        UE_LOG(LogTemp, Warning, TEXT("VFX_NiagaraSystemManager: Could not load placeholder Niagara system for effect type %d"), (int32)EffectType);
    }
    
    return PlaceholderSystem;
}