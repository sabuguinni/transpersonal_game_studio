#include "VFX_FootstepManager.h"
#include "Components/SceneComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"

AVFX_FootstepManager::AVFX_FootstepManager()
{
    PrimaryActorTick.bCanEverTick = true;

    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
    RootComponent = RootSceneComponent;

    DustParticleComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("DustParticleComponent"));
    DustParticleComponent->SetupAttachment(RootComponent);

    MaxFootstepRange = 2000.0f;
    DustLifetime = 3.0f;

    SetupSpeciesPresets();
}

void AVFX_FootstepManager::BeginPlay()
{
    Super::BeginPlay();
    InitializeNiagaraSystem();
}

void AVFX_FootstepManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    UpdateActiveEffects(DeltaTime);
    CleanupExpiredEffects();
}

void AVFX_FootstepManager::TriggerFootstepVFX(const FVector& Location, EDinosaurSpecies Species, float DinosaurMass)
{
    FVFX_FootstepData FootstepData = GetFootstepDataForSpecies(Species);
    FootstepData.ImpactLocation = Location;
    FootstepData.DinosaurMass = DinosaurMass;

    float DistanceIntensity = CalculateDistanceIntensity(Location);
    if (DistanceIntensity <= 0.0f)
    {
        return;
    }

    float FinalIntensity = FootstepData.DustIntensity * DistanceIntensity * (DinosaurMass / 1000.0f);
    float FinalSize = FootstepData.ParticleSize * (DinosaurMass / 1000.0f);

    CreateDustCloud(Location, FinalIntensity, FinalSize);

    UE_LOG(LogTemp, Log, TEXT("VFX_FootstepManager: Triggered footstep VFX at %s for species %d with mass %.1f kg"), 
           *Location.ToString(), (int32)Species, DinosaurMass);
}

void AVFX_FootstepManager::CreateDustCloud(const FVector& Location, float Intensity, float Size)
{
    if (!FootstepDustSystem)
    {
        UE_LOG(LogTemp, Warning, TEXT("VFX_FootstepManager: FootstepDustSystem is null"));
        return;
    }

    UNiagaraComponent* DustEffect = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
        GetWorld(), 
        FootstepDustSystem, 
        Location, 
        FRotator::ZeroRotator
    );

    if (DustEffect)
    {
        DustEffect->SetFloatParameter(TEXT("DustIntensity"), Intensity);
        DustEffect->SetFloatParameter(TEXT("ParticleSize"), Size);
        DustEffect->SetFloatParameter(TEXT("Lifetime"), DustLifetime);

        ActiveDustEffects.Add(DustEffect);
        EffectTimestamps.Add(GetWorld()->GetTimeSeconds());

        UE_LOG(LogTemp, Log, TEXT("VFX_FootstepManager: Created dust cloud with intensity %.2f and size %.1f"), 
               Intensity, Size);
    }
}

void AVFX_FootstepManager::SetupSpeciesPresets()
{
    FVFX_FootstepData TRexData;
    TRexData.DustIntensity = 2.0f;
    TRexData.ParticleSize = 200.0f;
    TRexData.DinosaurMass = 12000.0f;
    TRexData.Species = EDinosaurSpecies::TRex;
    SpeciesPresets.Add(EDinosaurSpecies::TRex, TRexData);

    FVFX_FootstepData RaptorData;
    RaptorData.DustIntensity = 0.8f;
    RaptorData.ParticleSize = 80.0f;
    RaptorData.DinosaurMass = 150.0f;
    RaptorData.Species = EDinosaurSpecies::Raptor;
    SpeciesPresets.Add(EDinosaurSpecies::Raptor, RaptorData);

    FVFX_FootstepData BrachiosaurusData;
    BrachiosaurusData.DustIntensity = 3.0f;
    BrachiosaurusData.ParticleSize = 300.0f;
    BrachiosaurusData.DinosaurMass = 80000.0f;
    BrachiosaurusData.Species = EDinosaurSpecies::Brachiosaurus;
    SpeciesPresets.Add(EDinosaurSpecies::Brachiosaurus, BrachiosaurusData);

    FVFX_FootstepData TriceratopsData;
    TriceratopsData.DustIntensity = 1.5f;
    TriceratopsData.ParticleSize = 150.0f;
    TriceratopsData.DinosaurMass = 9000.0f;
    TriceratopsData.Species = EDinosaurSpecies::Triceratops;
    SpeciesPresets.Add(EDinosaurSpecies::Triceratops, TriceratopsData);

    FVFX_FootstepData StegosaurusData;
    StegosaurusData.DustIntensity = 1.3f;
    StegosaurusData.ParticleSize = 130.0f;
    StegosaurusData.DinosaurMass = 5000.0f;
    StegosaurusData.Species = EDinosaurSpecies::Stegosaurus;
    SpeciesPresets.Add(EDinosaurSpecies::Stegosaurus, StegosaurusData);

    UE_LOG(LogTemp, Log, TEXT("VFX_FootstepManager: Setup %d species presets"), SpeciesPresets.Num());
}

FVFX_FootstepData AVFX_FootstepManager::GetFootstepDataForSpecies(EDinosaurSpecies Species) const
{
    if (const FVFX_FootstepData* FoundData = SpeciesPresets.Find(Species))
    {
        return *FoundData;
    }

    FVFX_FootstepData DefaultData;
    DefaultData.Species = Species;
    return DefaultData;
}

float AVFX_FootstepManager::CalculateDistanceIntensity(const FVector& ImpactLocation) const
{
    APlayerController* PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    if (!PlayerController || !PlayerController->GetPawn())
    {
        return 1.0f;
    }

    FVector PlayerLocation = PlayerController->GetPawn()->GetActorLocation();
    float Distance = FVector::Dist(PlayerLocation, ImpactLocation);

    if (Distance >= MaxFootstepRange)
    {
        return 0.0f;
    }

    float IntensityFactor = 1.0f - (Distance / MaxFootstepRange);
    return FMath::Clamp(IntensityFactor, 0.0f, 1.0f);
}

void AVFX_FootstepManager::CleanupExpiredEffects()
{
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    for (int32 i = ActiveDustEffects.Num() - 1; i >= 0; i--)
    {
        if (!ActiveDustEffects[i] || !IsValid(ActiveDustEffects[i]))
        {
            ActiveDustEffects.RemoveAt(i);
            EffectTimestamps.RemoveAt(i);
            continue;
        }

        float EffectAge = CurrentTime - EffectTimestamps[i];
        if (EffectAge > DustLifetime + 1.0f)
        {
            ActiveDustEffects[i]->DestroyComponent();
            ActiveDustEffects.RemoveAt(i);
            EffectTimestamps.RemoveAt(i);
        }
    }
}

void AVFX_FootstepManager::InitializeNiagaraSystem()
{
    if (!FootstepDustSystem)
    {
        FootstepDustSystem = LoadObject<UNiagaraSystem>(nullptr, TEXT("/Engine/VFX/Niagara/Systems/NS_DefaultDust"));
        if (!FootstepDustSystem)
        {
            UE_LOG(LogTemp, Warning, TEXT("VFX_FootstepManager: Failed to load default dust system"));
        }
    }
}

void AVFX_FootstepManager::UpdateActiveEffects(float DeltaTime)
{
    for (UNiagaraComponent* Effect : ActiveDustEffects)
    {
        if (Effect && IsValid(Effect))
        {
            float PlayerDistance = CalculateDistanceIntensity(Effect->GetComponentLocation());
            Effect->SetFloatParameter(TEXT("DistanceFade"), PlayerDistance);
        }
    }
}