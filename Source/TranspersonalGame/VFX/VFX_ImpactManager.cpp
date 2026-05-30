#include "VFX_ImpactManager.h"
#include "Engine/Engine.h"
#include "Particles/ParticleSystem.h"
#include "UObject/ConstructorHelpers.h"
#include "Engine/World.h"
#include "TimerManager.h"

AVFX_ImpactManager::AVFX_ImpactManager()
{
    PrimaryActorTick.bCanEverTick = false;

    // Create root component
    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootScene"));
    RootComponent = RootSceneComponent;

    // Create particle components
    DustParticleComponent = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("DustParticles"));
    DustParticleComponent->SetupAttachment(RootComponent);
    DustParticleComponent->bAutoActivate = false;

    DebrisParticleComponent = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("DebrisParticles"));
    DebrisParticleComponent->SetupAttachment(RootComponent);
    DebrisParticleComponent->bAutoActivate = false;

    // Initialize default values
    BaseImpactRadius = 200.0f;
    MaxParticleCount = 100.0f;
    EffectDuration = 3.0f;
    bAutoTriggerOnSpawn = false;
    CurrentBiome = EBiomeType::Savanna;

    InitializeParticleComponents();
}

void AVFX_ImpactManager::BeginPlay()
{
    Super::BeginPlay();
    
    ConfigureParticlesForBiome();
    
    if (bAutoTriggerOnSpawn)
    {
        FVFX_ImpactData DefaultImpact;
        DefaultImpact.ImpactLocation = GetActorLocation();
        DefaultImpact.ImpactForce = 1.0f;
        DefaultImpact.BiomeType = CurrentBiome;
        DefaultImpact.DinosaurSpecies = EDinosaurSpecies::TRex;
        
        TriggerFootstepImpact(DefaultImpact);
    }
}

void AVFX_ImpactManager::InitializeParticleComponents()
{
    if (DustParticleComponent)
    {
        DustParticleComponent->SetRelativeScale3D(FVector(1.0f, 1.0f, 1.0f));
        DustParticleComponent->SetVisibility(true);
    }

    if (DebrisParticleComponent)
    {
        DebrisParticleComponent->SetRelativeScale3D(FVector(0.8f, 0.8f, 0.8f));
        DebrisParticleComponent->SetVisibility(true);
    }
}

void AVFX_ImpactManager::TriggerFootstepImpact(const FVFX_ImpactData& ImpactData)
{
    if (!DustParticleComponent || !DebrisParticleComponent)
    {
        return;
    }

    // Set biome type for proper particle configuration
    SetBiomeType(ImpactData.BiomeType);
    
    // Scale effect based on dinosaur species
    ScaleEffectForDinosaur(ImpactData.DinosaurSpecies);

    // Set impact location
    SetActorLocation(ImpactData.ImpactLocation);

    // Activate particle effects
    DustParticleComponent->Activate(true);
    DebrisParticleComponent->Activate(true);

    // Log impact for debugging
    UE_LOG(LogTemp, Warning, TEXT("VFX Impact triggered at %s for %s in biome %d"), 
           *ImpactData.ImpactLocation.ToString(),
           *UEnum::GetValueAsString(ImpactData.DinosaurSpecies),
           (int32)ImpactData.BiomeType);

    // Auto-stop after duration
    FTimerHandle TimerHandle;
    GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &AVFX_ImpactManager::StopAllEffects, EffectDuration, false);
}

void AVFX_ImpactManager::TriggerDinosaurImpact(EDinosaurSpecies Species, FVector Location, float Force)
{
    FVFX_ImpactData ImpactData;
    ImpactData.ImpactLocation = Location;
    ImpactData.ImpactForce = Force;
    ImpactData.BiomeType = CurrentBiome;
    ImpactData.DinosaurSpecies = Species;

    TriggerFootstepImpact(ImpactData);
}

void AVFX_ImpactManager::SetBiomeType(EBiomeType NewBiome)
{
    CurrentBiome = NewBiome;
    ConfigureParticlesForBiome();
}

void AVFX_ImpactManager::ConfigureParticlesForBiome()
{
    if (!DustParticleComponent || !DebrisParticleComponent)
    {
        return;
    }

    // Configure particle colors and properties based on biome
    switch (CurrentBiome)
    {
        case EBiomeType::Savanna:
            // Yellow-brown dust for savanna
            DustParticleComponent->SetColorParameter(FName("ParticleColor"), FLinearColor(0.8f, 0.6f, 0.3f, 1.0f));
            break;
            
        case EBiomeType::Forest:
            // Dark brown earth for forest
            DustParticleComponent->SetColorParameter(FName("ParticleColor"), FLinearColor(0.4f, 0.3f, 0.2f, 1.0f));
            break;
            
        case EBiomeType::Desert:
            // Light sandy dust for desert
            DustParticleComponent->SetColorParameter(FName("ParticleColor"), FLinearColor(0.9f, 0.8f, 0.6f, 1.0f));
            break;
            
        case EBiomeType::Swamp:
            // Dark muddy particles for swamp
            DustParticleComponent->SetColorParameter(FName("ParticleColor"), FLinearColor(0.3f, 0.25f, 0.2f, 1.0f));
            break;
            
        case EBiomeType::Mountain:
            // Gray rocky dust for mountains
            DustParticleComponent->SetColorParameter(FName("ParticleColor"), FLinearColor(0.6f, 0.6f, 0.6f, 1.0f));
            break;
    }
}

void AVFX_ImpactManager::ScaleEffectForDinosaur(EDinosaurSpecies Species)
{
    float ScaleFactor = 1.0f;
    float ParticleMultiplier = 1.0f;

    switch (Species)
    {
        case EDinosaurSpecies::TRex:
            ScaleFactor = 2.5f;
            ParticleMultiplier = 3.0f;
            break;
            
        case EDinosaurSpecies::Brachiosaurus:
            ScaleFactor = 3.0f;
            ParticleMultiplier = 4.0f;
            break;
            
        case EDinosaurSpecies::Triceratops:
            ScaleFactor = 2.0f;
            ParticleMultiplier = 2.5f;
            break;
            
        case EDinosaurSpecies::Velociraptor:
            ScaleFactor = 0.8f;
            ParticleMultiplier = 0.6f;
            break;
            
        case EDinosaurSpecies::Ankylosaurus:
            ScaleFactor = 1.8f;
            ParticleMultiplier = 2.0f;
            break;
            
        default:
            ScaleFactor = 1.0f;
            ParticleMultiplier = 1.0f;
            break;
    }

    // Apply scaling to particle components
    if (DustParticleComponent)
    {
        DustParticleComponent->SetRelativeScale3D(FVector(ScaleFactor));
        DustParticleComponent->SetFloatParameter(FName("ParticleCount"), MaxParticleCount * ParticleMultiplier);
    }

    if (DebrisParticleComponent)
    {
        DebrisParticleComponent->SetRelativeScale3D(FVector(ScaleFactor * 0.8f));
        DebrisParticleComponent->SetFloatParameter(FName("ParticleCount"), MaxParticleCount * ParticleMultiplier * 0.5f);
    }
}

void AVFX_ImpactManager::StopAllEffects()
{
    if (DustParticleComponent)
    {
        DustParticleComponent->Deactivate();
    }

    if (DebrisParticleComponent)
    {
        DebrisParticleComponent->Deactivate();
    }
}

void AVFX_ImpactManager::SetupBiomeSpecificMaterials()
{
    // This method can be extended to load different particle materials
    // based on biome type for more realistic effects
}