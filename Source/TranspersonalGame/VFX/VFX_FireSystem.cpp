#include "VFX_FireSystem.h"
#include "Components/StaticMeshComponent.h"
#include "NiagaraComponent.h"
#include "Engine/Engine.h"
#include "Engine/StaticMesh.h"
#include "UObject/ConstructorHelpers.h"

AVFX_FireSystem::AVFX_FireSystem()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create root component
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

    // Initialize fire base mesh (logs/wood)
    FireBaseMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FireBaseMesh"));
    FireBaseMesh->SetupAttachment(RootComponent);

    // Initialize Niagara particle components
    FlameParticles = CreateDefaultSubobject<UNiagaraComponent>(TEXT("FlameParticles"));
    FlameParticles->SetupAttachment(RootComponent);
    FlameParticles->SetAutoActivate(false);

    SmokeParticles = CreateDefaultSubobject<UNiagaraComponent>(TEXT("SmokeParticles"));
    SmokeParticles->SetupAttachment(RootComponent);
    SmokeParticles->SetAutoActivate(false);

    EmberParticles = CreateDefaultSubobject<UNiagaraComponent>(TEXT("EmberParticles"));
    EmberParticles->SetupAttachment(RootComponent);
    EmberParticles->SetAutoActivate(false);

    // Set default fire parameters
    FireParams.Intensity = EVFX_FireIntensity::Medium;
    FireParams.FlameHeight = 1.0f;
    FireParams.FlameWidth = 0.8f;
    FireParams.SmokeAmount = 0.5f;
    FireParams.EmberCount = 0.3f;
    FireParams.FlameColor = FLinearColor(1.0f, 0.4f, 0.1f, 1.0f);

    BaseFlameHeight = FireParams.FlameHeight;
}

void AVFX_FireSystem::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeFireComponents();
    
    if (bAutoStart)
    {
        StartFire();
    }
}

void AVFX_FireSystem::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (bFireActive && bFlickerEffect)
    {
        UpdateFlickerEffect(DeltaTime);
    }
}

void AVFX_FireSystem::InitializeFireComponents()
{
    if (!FireBaseMesh || !FlameParticles || !SmokeParticles || !EmberParticles)
    {
        UE_LOG(LogTemp, Warning, TEXT("VFX_FireSystem: Missing components during initialization"));
        return;
    }

    // Set relative positions for particle systems
    FlameParticles->SetRelativeLocation(FVector(0.0f, 0.0f, 10.0f));
    SmokeParticles->SetRelativeLocation(FVector(0.0f, 0.0f, 50.0f));
    EmberParticles->SetRelativeLocation(FVector(0.0f, 0.0f, 15.0f));

    ApplyFireParameters();
}

void AVFX_FireSystem::StartFire()
{
    if (!bFireActive)
    {
        bFireActive = true;
        
        if (FlameParticles)
        {
            FlameParticles->Activate();
        }
        
        if (SmokeParticles && FireParams.SmokeAmount > 0.1f)
        {
            SmokeParticles->Activate();
        }
        
        if (EmberParticles && FireParams.EmberCount > 0.1f)
        {
            EmberParticles->Activate();
        }

        UE_LOG(LogTemp, Log, TEXT("VFX_FireSystem: Fire started"));
    }
}

void AVFX_FireSystem::StopFire()
{
    if (bFireActive)
    {
        bFireActive = false;
        
        if (FlameParticles)
        {
            FlameParticles->Deactivate();
        }
        
        if (SmokeParticles)
        {
            SmokeParticles->Deactivate();
        }
        
        if (EmberParticles)
        {
            EmberParticles->Deactivate();
        }

        UE_LOG(LogTemp, Log, TEXT("VFX_FireSystem: Fire stopped"));
    }
}

void AVFX_FireSystem::SetFireIntensity(EVFX_FireIntensity NewIntensity)
{
    FireParams.Intensity = NewIntensity;
    
    // Adjust parameters based on intensity
    switch (NewIntensity)
    {
        case EVFX_FireIntensity::Ember:
            FireParams.FlameHeight = 0.3f;
            FireParams.FlameWidth = 0.2f;
            FireParams.SmokeAmount = 0.1f;
            FireParams.EmberCount = 0.8f;
            break;
            
        case EVFX_FireIntensity::Small:
            FireParams.FlameHeight = 0.6f;
            FireParams.FlameWidth = 0.5f;
            FireParams.SmokeAmount = 0.3f;
            FireParams.EmberCount = 0.4f;
            break;
            
        case EVFX_FireIntensity::Medium:
            FireParams.FlameHeight = 1.0f;
            FireParams.FlameWidth = 0.8f;
            FireParams.SmokeAmount = 0.5f;
            FireParams.EmberCount = 0.3f;
            break;
            
        case EVFX_FireIntensity::Large:
            FireParams.FlameHeight = 1.5f;
            FireParams.FlameWidth = 1.2f;
            FireParams.SmokeAmount = 0.7f;
            FireParams.EmberCount = 0.2f;
            break;
            
        case EVFX_FireIntensity::Inferno:
            FireParams.FlameHeight = 2.5f;
            FireParams.FlameWidth = 2.0f;
            FireParams.SmokeAmount = 0.9f;
            FireParams.EmberCount = 0.1f;
            break;
    }
    
    BaseFlameHeight = FireParams.FlameHeight;
    ApplyFireParameters();
}

void AVFX_FireSystem::UpdateFireParameters(const FVFX_FireParams& NewParams)
{
    FireParams = NewParams;
    BaseFlameHeight = FireParams.FlameHeight;
    ApplyFireParameters();
}

bool AVFX_FireSystem::IsFireActive() const
{
    return bFireActive;
}

void AVFX_FireSystem::UpdateFlickerEffect(float DeltaTime)
{
    FlickerTimer += DeltaTime * FlickerSpeed;
    
    // Create flickering effect using sine wave
    float FlickerMultiplier = 1.0f + (FMath::Sin(FlickerTimer) * 0.15f);
    float CurrentFlameHeight = BaseFlameHeight * FlickerMultiplier;
    
    // Apply flicker to flame particles
    if (FlameParticles)
    {
        FlameParticles->SetFloatParameter(FName("FlameHeight"), CurrentFlameHeight);
        FlameParticles->SetFloatParameter(FName("FlameIntensity"), FlickerMultiplier);
    }
    
    // Subtle flicker for ember particles
    if (EmberParticles)
    {
        float EmberFlicker = 1.0f + (FMath::Sin(FlickerTimer * 0.7f) * 0.1f);
        EmberParticles->SetFloatParameter(FName("EmberIntensity"), EmberFlicker);
    }
}

void AVFX_FireSystem::ApplyFireParameters()
{
    if (!FlameParticles || !SmokeParticles || !EmberParticles)
    {
        return;
    }

    // Apply flame parameters
    FlameParticles->SetFloatParameter(FName("FlameHeight"), FireParams.FlameHeight);
    FlameParticles->SetFloatParameter(FName("FlameWidth"), FireParams.FlameWidth);
    FlameParticles->SetVectorParameter(FName("FlameColor"), FVector(FireParams.FlameColor.R, FireParams.FlameColor.G, FireParams.FlameColor.B));

    // Apply smoke parameters
    SmokeParticles->SetFloatParameter(FName("SmokeAmount"), FireParams.SmokeAmount);
    SmokeParticles->SetFloatParameter(FName("SmokeHeight"), FireParams.FlameHeight * 2.0f);

    // Apply ember parameters
    EmberParticles->SetFloatParameter(FName("EmberCount"), FireParams.EmberCount);
    EmberParticles->SetFloatParameter(FName("EmberSpread"), FireParams.FlameWidth);
}